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

static kefir_result_t identify_code_blocks(struct kefir_mem *mem, struct kefir_opt_constructor_state *state) {
    kefir_bool_t start_new_block = true;
    for (kefir_size_t i = 0; i < kefir_irblock_length(&state->function->ir_func->body); i++) {
        if (start_new_block) {
            REQUIRE_OK(kefir_opt_constructor_start_code_block_at(mem, state, i));
        }
        start_new_block = false;

        const struct kefir_irinstr *instr = kefir_irblock_at(&state->function->ir_func->body, i);
        REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid IR instruction to be returned"));
        switch (instr->opcode) {
            case KEFIR_IROPCODE_JMP:
            case KEFIR_IROPCODE_BRANCH:
            case KEFIR_IROPCODE_PUSHLABEL:
                REQUIRE_OK(kefir_opt_constructor_start_code_block_at(mem, state, instr->arg.u64));
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

        case KEFIR_IROPCODE_XCHG:
            REQUIRE_OK(kefir_opt_constructor_stack_exchange(mem, state, instr->arg.u64));
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

static kefir_result_t translate_code(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                     struct kefir_opt_constructor_state *state) {
    UNUSED(module);
    state->current_block = NULL;
    state->ir_location = 0;
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

static kefir_result_t construct_code_from_ir(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                             struct kefir_opt_constructor_state *state) {
    REQUIRE_OK(identify_code_blocks(mem, state));
    REQUIRE_OK(translate_code(mem, module, state));
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
