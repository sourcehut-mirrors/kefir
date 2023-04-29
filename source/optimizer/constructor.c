/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

    This file is part of Kefir project.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kefir/optimizer/constructor.h"
#include "kefir/optimizer/builder.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#define KEFIR_OPTIMIZER_CONSTRUCTOR_INTERNAL_INCLUDE
#include "kefir/optimizer/constructor_internal.h"
#include <string.h>

static kefir_result_t identify_code_blocks(struct kefir_mem *mem, struct kefir_opt_constructor_state *state) {
    kefir_bool_t start_new_block = true;
    for (kefir_size_t i = 0; i < kefir_irblock_length(&state->function->ir_func->body); i++) {
        const struct kefir_irinstr *instr = kefir_irblock_at(&state->function->ir_func->body, i);
        REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid IR instruction to be returned"));

        if (start_new_block) {
            REQUIRE_OK(
                kefir_opt_constructor_start_code_block_at(mem, state, i, instr->opcode == KEFIR_IROPCODE_PUSHLABEL));
        }
        start_new_block = false;

        switch (instr->opcode) {
            case KEFIR_IROPCODE_PUSHLABEL:
                REQUIRE_OK(kefir_opt_constructor_start_code_block_at(mem, state, instr->arg.u64, true));
                break;

            case KEFIR_IROPCODE_JMP:
            case KEFIR_IROPCODE_BRANCH:
                REQUIRE_OK(kefir_opt_constructor_start_code_block_at(mem, state, instr->arg.u64, false));
                // Fallthrough

            case KEFIR_IROPCODE_IJMP:
            case KEFIR_IROPCODE_RET:
                start_new_block = true;
                break;

            default:
                // Intentionally left blank
                break;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_instruction(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                            struct kefir_opt_code_container *code,
                                            struct kefir_opt_constructor_state *state,
                                            const struct kefir_irinstr *instr) {
    UNUSED(module);
    kefir_opt_instruction_ref_t instr_ref, instr_ref2, instr_ref3, instr_ref4;
    const kefir_opt_block_id_t current_block_id = state->current_block->block_id;
    switch (instr->opcode) {
        case KEFIR_IROPCODE_JMP: {
            struct kefir_opt_constructor_code_block_state *jump_target_block = NULL;
            REQUIRE_OK(kefir_opt_constructor_find_code_block_for(state, instr->arg.u64, &jump_target_block));
            REQUIRE_OK(
                kefir_opt_code_builder_finalize_jump(mem, code, current_block_id, jump_target_block->block_id, NULL));
        } break;

        case KEFIR_IROPCODE_BRANCH: {
            struct kefir_opt_constructor_code_block_state *jump_target_block = NULL, *alternative_block = NULL;
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_find_code_block_for(state, instr->arg.u64, &jump_target_block));
            REQUIRE_OK(kefir_opt_constructor_find_code_block_for(state, state->ir_location + 1, &alternative_block));
            REQUIRE_OK(kefir_opt_code_builder_finalize_branch(mem, code, current_block_id, instr_ref,
                                                              jump_target_block->block_id, alternative_block->block_id,
                                                              NULL));
        } break;

        case KEFIR_IROPCODE_IJMP:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_finalize_indirect_jump(mem, code, current_block_id, instr_ref, NULL));
            break;

        case KEFIR_IROPCODE_RET:
            instr_ref = KEFIR_ID_NONE;
            if (kefir_ir_type_length(state->function->ir_func->declaration->result) > 0) {
                REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref));
            }
            REQUIRE_OK(kefir_opt_code_builder_finalize_return(mem, code, current_block_id, instr_ref, NULL));
            break;

        case KEFIR_IROPCODE_PUSHI64:
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, code, current_block_id, instr->arg.i64, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IROPCODE_PUSHU64:
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, code, current_block_id, instr->arg.u64, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IROPCODE_PUSHF32:
            REQUIRE_OK(
                kefir_opt_code_builder_float32_constant(mem, code, current_block_id, instr->arg.f32[0], &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IROPCODE_PUSHF64:
            REQUIRE_OK(
                kefir_opt_code_builder_float64_constant(mem, code, current_block_id, instr->arg.f64, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IROPCODE_PUSHSTRING:
            REQUIRE_OK(
                kefir_opt_code_builder_string_reference(mem, code, current_block_id, instr->arg.u64, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IROPCODE_PUSHLABEL: {
            struct kefir_opt_constructor_code_block_state *block = NULL;
            REQUIRE_OK(kefir_opt_constructor_find_code_block_for(state, instr->arg.u64, &block));
            REQUIRE_OK(kefir_opt_code_builder_block_label(mem, code, current_block_id, block->block_id, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
        } break;

        case KEFIR_IROPCODE_PICK:
            REQUIRE_OK(kefir_opt_constructor_stack_at(mem, state, instr->arg.u64, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IROPCODE_POP:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref));
            break;

        case KEFIR_IROPCODE_XCHG:
            REQUIRE_OK(kefir_opt_constructor_stack_exchange(mem, state, instr->arg.u64));
            break;

        case KEFIR_IROPCODE_GETGLOBAL:
            REQUIRE_OK(kefir_opt_code_builder_get_global(mem, code, current_block_id, instr->arg.u64, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IROPCODE_GETTHRLOCAL:
            REQUIRE_OK(
                kefir_opt_code_builder_get_thread_local(mem, code, current_block_id, instr->arg.u64, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IROPCODE_GETLOCAL:
            REQUIRE(instr->arg.u32[0] == state->function->ir_func->locals_type_id,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                    "Expected IR operation type reference to correspond to IR function local type"));
            REQUIRE_OK(kefir_opt_code_builder_get_local(mem, code, current_block_id, instr->arg.u32[1], &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IROPCODE_EXTSBITS:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, code, current_block_id, instr_ref2,
                                                                  instr->arg.u32[0], instr->arg.u32[1], &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IROPCODE_EXTUBITS:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, code, current_block_id, instr_ref2,
                                                                    instr->arg.u32[0], instr->arg.u32[1], &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IROPCODE_INSERTBITS:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));
            REQUIRE_OK(kefir_opt_code_builder_bits_insert(mem, code, current_block_id, instr_ref2, instr_ref3,
                                                          instr->arg.u32[0], instr->arg.u32[1], &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IROPCODE_BZERO:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_code_builder_zero_memory(mem, code, current_block_id, instr_ref2, instr->arg.u32[0],
                                                          instr->arg.u32[1], &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IROPCODE_BCOPY:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));
            REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, code, current_block_id, instr_ref3, instr_ref2,
                                                          instr->arg.u32[0], instr->arg.u32[1], &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

#define UNARY_OP(_id, _opcode)                                                                         \
    case _opcode:                                                                                      \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));                          \
        REQUIRE_OK(kefir_opt_code_builder_##_id(mem, code, current_block_id, instr_ref2, &instr_ref)); \
        REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));                           \
        break;

            UNARY_OP(int_not, KEFIR_IROPCODE_INOT)
            UNARY_OP(int_neg, KEFIR_IROPCODE_INEG)
            UNARY_OP(bool_not, KEFIR_IROPCODE_BNOT)

            UNARY_OP(int64_zero_extend_1bit, KEFIR_IROPCODE_TRUNCATE1)
            UNARY_OP(int64_sign_extend_8bits, KEFIR_IROPCODE_EXTEND8)
            UNARY_OP(int64_sign_extend_16bits, KEFIR_IROPCODE_EXTEND16)
            UNARY_OP(int64_sign_extend_32bits, KEFIR_IROPCODE_EXTEND32)

#undef UNARY_OP

#define BINARY_OP(_id, _opcode)                                                                                    \
    case _opcode:                                                                                                  \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));                                      \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));                                      \
        REQUIRE_OK(kefir_opt_code_builder_##_id(mem, code, current_block_id, instr_ref2, instr_ref3, &instr_ref)); \
        REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));                                       \
        break;

            BINARY_OP(int_add, KEFIR_IROPCODE_IADD)
            BINARY_OP(int_sub, KEFIR_IROPCODE_ISUB)
            BINARY_OP(int_mul, KEFIR_IROPCODE_IMUL)
            BINARY_OP(int_div, KEFIR_IROPCODE_IDIV)
            BINARY_OP(int_mod, KEFIR_IROPCODE_IMOD)
            BINARY_OP(uint_div, KEFIR_IROPCODE_UDIV)
            BINARY_OP(uint_mod, KEFIR_IROPCODE_UMOD)
            BINARY_OP(int_and, KEFIR_IROPCODE_IAND)
            BINARY_OP(int_or, KEFIR_IROPCODE_IOR)
            BINARY_OP(int_xor, KEFIR_IROPCODE_IXOR)
            BINARY_OP(int_lshift, KEFIR_IROPCODE_ILSHIFT)
            BINARY_OP(int_rshift, KEFIR_IROPCODE_IRSHIFT)
            BINARY_OP(int_arshift, KEFIR_IROPCODE_IARSHIFT)
            BINARY_OP(int_equals, KEFIR_IROPCODE_IEQUALS)
            BINARY_OP(int_greater, KEFIR_IROPCODE_IGREATER)
            BINARY_OP(int_lesser, KEFIR_IROPCODE_ILESSER)
            BINARY_OP(int_above, KEFIR_IROPCODE_IABOVE)
            BINARY_OP(int_below, KEFIR_IROPCODE_IBELOW)
            BINARY_OP(bool_and, KEFIR_IROPCODE_BAND)
            BINARY_OP(bool_or, KEFIR_IROPCODE_BOR)

#undef BINARY_OP

#define LOAD_OP(_id, _opcode)                                                                                \
    case _opcode: {                                                                                          \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));                                \
        kefir_bool_t volatile_access = (instr->arg.u64 & KEFIR_IR_MEMORY_FLAG_VOLATILE) != 0;                \
        REQUIRE_OK(kefir_opt_code_builder_##_id(                                                             \
            mem, code, current_block_id, instr_ref2,                                                         \
            &(const struct kefir_opt_memory_access_flags){.volatile_access = volatile_access}, &instr_ref)); \
        if (volatile_access) {                                                                               \
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));               \
        }                                                                                                    \
        REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));                                 \
    } break;

            LOAD_OP(int8_load_signed, KEFIR_IROPCODE_LOAD8I)
            LOAD_OP(int8_load_unsigned, KEFIR_IROPCODE_LOAD8U)
            LOAD_OP(int16_load_signed, KEFIR_IROPCODE_LOAD16I)
            LOAD_OP(int16_load_unsigned, KEFIR_IROPCODE_LOAD16U)
            LOAD_OP(int32_load_signed, KEFIR_IROPCODE_LOAD32I)
            LOAD_OP(int32_load_unsigned, KEFIR_IROPCODE_LOAD32U)
            LOAD_OP(int64_load, KEFIR_IROPCODE_LOAD64)

#undef LOAD_OP

#define STORE_OP(_id, _opcode)                                                                               \
    case _opcode: {                                                                                          \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));                                \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));                                \
        kefir_bool_t volatile_access = (instr->arg.u64 & KEFIR_IR_MEMORY_FLAG_VOLATILE) != 0;                \
        REQUIRE_OK(kefir_opt_code_builder_##_id(                                                             \
            mem, code, current_block_id, instr_ref2, instr_ref3,                                             \
            &(const struct kefir_opt_memory_access_flags){.volatile_access = volatile_access}, &instr_ref)); \
        REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));                   \
    } break;

            STORE_OP(int8_store, KEFIR_IROPCODE_STORE8)
            STORE_OP(int16_store, KEFIR_IROPCODE_STORE16)
            STORE_OP(int32_store, KEFIR_IROPCODE_STORE32)
            STORE_OP(int64_store, KEFIR_IROPCODE_STORE64)

#undef STORE_OP

        case KEFIR_IROPCODE_IADD1:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, code, current_block_id, instr->arg.i64, &instr_ref3));
            REQUIRE_OK(kefir_opt_code_builder_int_add(mem, code, current_block_id, instr_ref2, instr_ref3, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IROPCODE_IADDX:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, code, current_block_id, instr->arg.i64, &instr_ref4));
            REQUIRE_OK(kefir_opt_code_builder_int_mul(mem, code, current_block_id, instr_ref2, instr_ref4, &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_int_add(mem, code, current_block_id, instr_ref, instr_ref3, &instr_ref2));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref2));
            break;

        default:
            // TODO Implement complete translation
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t push_function_arguments(struct kefir_mem *mem, struct kefir_opt_constructor_state *state) {
    REQUIRE_OK(kefir_opt_constructor_update_current_code_block(mem, state));
    kefir_opt_instruction_ref_t instr_ref;
    for (kefir_size_t i = 0; i < kefir_ir_type_children(state->function->ir_func->declaration->params); i++) {
        REQUIRE_OK(kefir_opt_code_builder_get_argument(mem, &state->function->code, state->current_block->block_id, i,
                                                       &instr_ref));
        REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_code(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                     struct kefir_opt_constructor_state *state) {
    UNUSED(module);
    state->current_block = NULL;
    state->ir_location = 0;
    REQUIRE_OK(push_function_arguments(mem, state));
    const struct kefir_irblock *ir_block = &state->function->ir_func->body;
    for (; state->ir_location < kefir_irblock_length(ir_block); state->ir_location++) {
        REQUIRE_OK(kefir_opt_constructor_update_current_code_block(mem, state));

        const struct kefir_irinstr *instr = kefir_irblock_at(ir_block, state->ir_location);
        REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid IR instruction to be returned"));
        REQUIRE_OK(translate_instruction(mem, module, &state->function->code, state, instr));
    }

    REQUIRE_OK(kefir_opt_constructor_update_current_code_block(mem, state));
    kefir_bool_t last_block_finalized = false;
    REQUIRE_OK(kefir_opt_code_builder_is_finalized(&state->function->code, state->current_block->block_id,
                                                   &last_block_finalized));
    if (!last_block_finalized) {
        REQUIRE_OK(kefir_opt_code_builder_finalize_return(mem, &state->function->code, state->current_block->block_id,
                                                          KEFIR_ID_NONE, NULL));
    }
    return KEFIR_OK;
}

static kefir_result_t link_blocks_impl(struct kefir_mem *mem, struct kefir_opt_constructor_state *state,
                                       kefir_opt_block_id_t source_block_id, kefir_opt_block_id_t target_block_id) {
    struct kefir_hashtree_node *hashtree_node = NULL;
    REQUIRE_OK(kefir_hashtree_at(&state->code_block_index, (kefir_hashtree_key_t) source_block_id, &hashtree_node));
    ASSIGN_DECL_CAST(struct kefir_opt_constructor_code_block_state *, source_state, hashtree_node->value);
    REQUIRE_OK(kefir_hashtree_at(&state->code_block_index, (kefir_hashtree_key_t) target_block_id, &hashtree_node));
    ASSIGN_DECL_CAST(struct kefir_opt_constructor_code_block_state *, target_state, hashtree_node->value);

    REQUIRE(source_state->reachable, KEFIR_OK);
    const struct kefir_list_entry *source_iter = kefir_list_tail(&source_state->stack);
    const struct kefir_list_entry *target_iter = kefir_list_tail(&target_state->phi_stack);
    for (; source_iter != NULL && target_iter != NULL;
         source_iter = source_iter->prev, target_iter = target_iter->prev) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, source_iter->value);
        ASSIGN_DECL_CAST(kefir_opt_phi_id_t, phi_ref, target_iter->value);
        REQUIRE_OK(
            kefir_opt_code_container_phi_attach(mem, &state->function->code, phi_ref, source_block_id, instr_ref));
    }
    REQUIRE(target_iter == NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to link optimizer block outputs with target block phi nodes"));
    return KEFIR_OK;
}

static kefir_result_t link_blocks_match(struct kefir_mem *mem, struct kefir_opt_constructor_state *state,
                                        const struct kefir_opt_code_block *block) {
    const struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&state->function->code, block, &instr));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_JUMP:
            REQUIRE_OK(link_blocks_impl(mem, state, block->id, instr->operation.parameters.branch.target_block));
            break;

        case KEFIR_OPT_OPCODE_BRANCH:
            REQUIRE_OK(link_blocks_impl(mem, state, block->id, instr->operation.parameters.branch.target_block));
            REQUIRE_OK(link_blocks_impl(mem, state, block->id, instr->operation.parameters.branch.alternative_block));
            break;

        case KEFIR_OPT_OPCODE_IJUMP:
            for (const struct kefir_list_entry *iter = kefir_list_head(&state->indirect_jump_targets); iter != NULL;
                 kefir_list_next(&iter)) {
                ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block_id, iter->value);
                REQUIRE_OK(link_blocks_impl(mem, state, block->id, target_block_id));
            }
            break;

        default:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t link_blocks_equalize_stack(struct kefir_mem *mem, struct kefir_opt_constructor_state *state,
                                                 kefir_opt_block_id_t source_block_id,
                                                 kefir_opt_block_id_t target_block_id) {
    struct kefir_hashtree_node *hashtree_node = NULL;
    REQUIRE_OK(kefir_hashtree_at(&state->code_block_index, (kefir_hashtree_key_t) source_block_id, &hashtree_node));
    ASSIGN_DECL_CAST(struct kefir_opt_constructor_code_block_state *, source_state, hashtree_node->value);
    REQUIRE_OK(kefir_hashtree_at(&state->code_block_index, (kefir_hashtree_key_t) target_block_id, &hashtree_node));
    ASSIGN_DECL_CAST(struct kefir_opt_constructor_code_block_state *, target_state, hashtree_node->value);

    while (kefir_list_length(&source_state->stack) > kefir_list_length(&target_state->phi_stack)) {
        kefir_opt_phi_id_t phi_ref;
        kefir_opt_instruction_ref_t instr_ref;
        REQUIRE_OK(kefir_opt_code_container_new_phi(mem, &state->function->code, target_block_id, &phi_ref));
        REQUIRE_OK(kefir_opt_code_builder_phi(mem, &state->function->code, target_block_id, phi_ref, &instr_ref));
        REQUIRE_OK(kefir_list_insert_after(mem, &target_state->stack, NULL, (void *) instr_ref));
        REQUIRE_OK(kefir_list_insert_after(mem, &target_state->phi_stack, NULL, (void *) phi_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t link_blocks_traverse(struct kefir_mem *mem, struct kefir_opt_constructor_state *state,
                                           kefir_opt_block_id_t block_id) {
    struct kefir_hashtree_node *hashtree_node = NULL;
    REQUIRE_OK(kefir_hashtree_at(&state->code_block_index, (kefir_hashtree_key_t) block_id, &hashtree_node));
    ASSIGN_DECL_CAST(struct kefir_opt_constructor_code_block_state *, block_state, hashtree_node->value);
    REQUIRE(!block_state->reachable, KEFIR_OK);

    block_state->reachable = true;

    struct kefir_opt_code_block *block = NULL;
    const struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_block(&state->function->code, block_id, &block));
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&state->function->code, block, &instr));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_JUMP:
            REQUIRE_OK(
                link_blocks_equalize_stack(mem, state, block_id, instr->operation.parameters.branch.target_block));
            REQUIRE_OK(link_blocks_traverse(mem, state, instr->operation.parameters.branch.target_block));
            break;

        case KEFIR_OPT_OPCODE_BRANCH:
            REQUIRE_OK(
                link_blocks_equalize_stack(mem, state, block_id, instr->operation.parameters.branch.target_block));
            REQUIRE_OK(
                link_blocks_equalize_stack(mem, state, block_id, instr->operation.parameters.branch.alternative_block));
            REQUIRE_OK(link_blocks_traverse(mem, state, instr->operation.parameters.branch.target_block));
            REQUIRE_OK(link_blocks_traverse(mem, state, instr->operation.parameters.branch.alternative_block));
            break;

        case KEFIR_OPT_OPCODE_IJUMP:
        case KEFIR_OPT_OPCODE_RETURN:
            // Intentionally left blank
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Encountered unterminated optimizer code block");
    }
    return KEFIR_OK;
}

static kefir_result_t link_blocks(struct kefir_mem *mem, struct kefir_opt_constructor_state *state) {
    REQUIRE_OK(link_blocks_traverse(mem, state, state->function->code.entry_point));
    for (const struct kefir_list_entry *iter = kefir_list_head(&state->indirect_jump_targets); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block_id, iter->value);
        REQUIRE_OK(link_blocks_traverse(mem, state, target_block_id));
    }

    struct kefir_opt_code_container_iterator iter;
    for (const struct kefir_opt_code_block *block = kefir_opt_code_container_iter(&state->function->code, &iter);
         block != NULL; block = kefir_opt_code_container_next(&iter)) {

        REQUIRE_OK(link_blocks_match(mem, state, block));
    }
    return KEFIR_OK;
}

static kefir_result_t construct_code_from_ir(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                             struct kefir_opt_constructor_state *state) {
    REQUIRE_OK(identify_code_blocks(mem, state));
    REQUIRE_OK(translate_code(mem, module, state));
    REQUIRE_OK(link_blocks(mem, state));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_construct_function_code(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                                 struct kefir_opt_function *function) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));
    REQUIRE(kefir_opt_code_container_is_empty(&function->code),
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid empty optimizer code container"));

    struct kefir_opt_constructor_state state;
    REQUIRE_OK(kefir_opt_constructor_init(function, &state));

    kefir_result_t res = construct_code_from_ir(mem, module, &state);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_constructor_free(mem, &state);
        return res;
    });

    REQUIRE_OK(kefir_opt_constructor_free(mem, &state));
    return KEFIR_OK;
}
