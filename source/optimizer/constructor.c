/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

static kefir_result_t identify_code_blocks(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                           struct kefir_opt_constructor_state *state) {
    kefir_bool_t start_new_block = true;
    kefir_size_t i = 0;
    REQUIRE_OK(kefir_opt_constructor_start_code_block_at(mem, state, (kefir_size_t) -1ll));
    for (; i < kefir_irblock_length(&state->function->ir_func->body); i++) {
        const struct kefir_irinstr *instr = kefir_irblock_at(&state->function->ir_func->body, i);
        REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid IR instruction to be returned"));

        if (start_new_block) {
            REQUIRE_OK(kefir_opt_constructor_start_code_block_at(mem, state, i));
        }
        start_new_block = false;

        switch (instr->opcode) {
            case KEFIR_IROPCODE_PUSHLABEL:
                REQUIRE_OK(kefir_opt_constructor_start_code_block_at(mem, state, instr->arg.u64));
                REQUIRE_OK(kefir_opt_constructor_mark_code_block_for_indirect_jump(mem, state, instr->arg.u64));
                break;

            case KEFIR_IROPCODE_JMP:
            case KEFIR_IROPCODE_BRANCH:
                REQUIRE_OK(kefir_opt_constructor_start_code_block_at(mem, state, instr->arg.u64));
                // Fallthrough

            case KEFIR_IROPCODE_IJMP:
            case KEFIR_IROPCODE_RET:
                start_new_block = true;
                break;

            case KEFIR_IROPCODE_INLINEASM: {
                const struct kefir_ir_inline_assembly *inline_asm =
                    kefir_ir_module_get_inline_assembly(module->ir_module, instr->arg.i64);
                REQUIRE(inline_asm != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR inline assembly"));

                if (kefir_list_length(&inline_asm->jump_target_list) > 0) {
                    for (const struct kefir_list_entry *iter = kefir_list_head(&inline_asm->jump_target_list);
                         iter != NULL; kefir_list_next(&iter)) {
                        ASSIGN_DECL_CAST(struct kefir_ir_inline_assembly_jump_target *, jump_target, iter->value);

                        REQUIRE_OK(kefir_opt_constructor_start_code_block_at(mem, state, jump_target->target));
                    }
                    start_new_block = true;
                }
            } break;

            default:
                // Intentionally left blank
                break;
        }
    }

    if (start_new_block) {
        REQUIRE_OK(kefir_opt_constructor_start_code_block_at(mem, state, i));
    }

    struct kefir_hashtree_node_iterator iter;
    kefir_result_t res;
    const char *public_label;
    kefir_size_t public_label_location;
    for (res = kefir_irblock_public_labels_iter(&state->function->ir_func->body, &iter, &public_label,
                                                &public_label_location);
         res == KEFIR_OK; res = kefir_irblock_public_labels_next(&iter, &public_label, &public_label_location)) {
        REQUIRE_OK(kefir_opt_constructor_start_code_block_at(mem, state, public_label_location));
        REQUIRE_OK(kefir_opt_constructor_mark_code_block_for_indirect_jump(mem, state, public_label_location));

        struct kefir_opt_constructor_code_block_state *block_state;
        REQUIRE_OK(kefir_opt_constructor_find_code_block_for(state, public_label_location, &block_state));
        REQUIRE_OK(kefir_opt_code_container_add_block_public_label(mem, &state->function->code, block_state->block_id,
                                                                   public_label));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t construct_inline_asm(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                           struct kefir_opt_code_container *code,
                                           struct kefir_opt_constructor_state *state,
                                           const struct kefir_irinstr *instr) {
    const struct kefir_ir_inline_assembly *ir_inline_asm =
        kefir_ir_module_get_inline_assembly(module->ir_module, instr->arg.u64);

    kefir_size_t num_of_parameter_indices = 0;
    for (const struct kefir_list_entry *iter = kefir_list_head(&ir_inline_asm->parameter_list); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, asm_param, iter->value);

        switch (asm_param->klass) {
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD_STORE:
                num_of_parameter_indices = MAX(num_of_parameter_indices, asm_param->load_store_index + 1);
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE:
                num_of_parameter_indices = MAX(num_of_parameter_indices, asm_param->load_store_index + 1);
                num_of_parameter_indices = MAX(num_of_parameter_indices, asm_param->read_index + 1);
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ:
                num_of_parameter_indices = MAX(num_of_parameter_indices, asm_param->read_index + 1);
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE:
                // Intentionally left blank
                break;
        }
    }

    kefir_opt_inline_assembly_id_t inline_asm_ref;
    REQUIRE_OK(kefir_opt_code_container_new_inline_assembly(
        mem, code, state->current_block->block_id, ir_inline_asm->id, kefir_list_length(&ir_inline_asm->parameter_list),
        &inline_asm_ref));

    kefir_size_t param_idx = 0;
    for (const struct kefir_list_entry *iter = kefir_list_head(&ir_inline_asm->parameter_list); iter != NULL;
         kefir_list_next(&iter), param_idx++) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, ir_asm_param, iter->value);

        struct kefir_opt_inline_assembly_parameter *inline_asm_param;
        kefir_opt_instruction_ref_t param_ref;
        switch (ir_asm_param->klass) {
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD_STORE:
                REQUIRE_OK(kefir_opt_code_container_inline_assembly_parameter(
                    code, inline_asm_ref, ir_asm_param->parameter_id, &inline_asm_param));
                REQUIRE_OK(kefir_opt_constructor_stack_at(mem, state, ir_asm_param->load_store_index, &param_ref));
                inline_asm_param->load_store_ref = param_ref;
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE:
                REQUIRE_OK(kefir_opt_code_container_inline_assembly_parameter(
                    code, inline_asm_ref, ir_asm_param->parameter_id, &inline_asm_param));
                REQUIRE_OK(kefir_opt_constructor_stack_at(mem, state, ir_asm_param->load_store_index, &param_ref));
                inline_asm_param->load_store_ref = param_ref;
                REQUIRE_OK(kefir_opt_constructor_stack_at(mem, state, ir_asm_param->read_index, &param_ref));
                inline_asm_param->read_ref = param_ref;
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ:
                REQUIRE_OK(kefir_opt_code_container_inline_assembly_parameter(
                    code, inline_asm_ref, ir_asm_param->parameter_id, &inline_asm_param));
                REQUIRE_OK(kefir_opt_constructor_stack_at(mem, state, ir_asm_param->read_index, &param_ref));
                inline_asm_param->read_ref = param_ref;
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE:
                // Intentionally left blank
                break;
        }
    }

    kefir_opt_instruction_ref_t instr_ref;
    REQUIRE_OK(
        kefir_opt_code_builder_inline_assembly(mem, code, state->current_block->block_id, inline_asm_ref, &instr_ref));
    REQUIRE_OK(kefir_opt_code_builder_add_control(code, state->current_block->block_id, instr_ref));

    kefir_opt_instruction_ref_t param_ref;
    while (num_of_parameter_indices--) {
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &param_ref));
    }

    if (kefir_list_length(&ir_inline_asm->jump_target_list) > 0) {
        struct kefir_opt_constructor_code_block_state *default_control_flow = NULL;
        REQUIRE_OK(kefir_opt_constructor_find_code_block_for(state, state->ir_location + 1, &default_control_flow));
        REQUIRE_OK(kefir_opt_code_container_inline_assembly_set_default_jump_target(code, inline_asm_ref,
                                                                                    default_control_flow->block_id));

        for (const struct kefir_list_entry *iter = kefir_list_head(&ir_inline_asm->jump_target_list); iter != NULL;
             kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(struct kefir_ir_inline_assembly_jump_target *, ir_jump_target, iter->value);

            struct kefir_opt_constructor_code_block_state *target_block_state = NULL;
            REQUIRE_OK(kefir_opt_constructor_find_code_block_for(state, ir_jump_target->target, &target_block_state));

            REQUIRE_OK(kefir_opt_code_container_inline_assembly_add_jump_target(
                mem, &state->function->code, inline_asm_ref, ir_jump_target->uid, target_block_state->block_id));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_instruction(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                            struct kefir_opt_code_container *code,
                                            struct kefir_opt_constructor_state *state,
                                            const struct kefir_irinstr *instr) {
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

        case KEFIR_IROPCODE_PUSHLD:
            REQUIRE_OK(kefir_opt_code_builder_long_double_constant(mem, code, current_block_id, instr->arg.long_double,
                                                                   &instr_ref));
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

        case KEFIR_IROPCODE_PLACEHI64:
            REQUIRE_OK(kefir_opt_code_builder_int_placeholder(mem, code, current_block_id, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IROPCODE_PLACEHF32:
            REQUIRE_OK(kefir_opt_code_builder_float32_placeholder(mem, code, current_block_id, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IROPCODE_PLACEHF64:
            REQUIRE_OK(kefir_opt_code_builder_float64_placeholder(mem, code, current_block_id, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

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
            REQUIRE(instr->arg.u32[0] == state->function->locals.type_id,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                    "Expected IR operation type reference to correspond to IR function local type"));
            REQUIRE_OK(kefir_opt_code_builder_get_local(mem, code, current_block_id, instr->arg.u32[1], 0, &instr_ref));
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
            REQUIRE_OK(kefir_opt_code_builder_bits_insert(mem, code, current_block_id, instr_ref3, instr_ref2,
                                                          instr->arg.u32[0], instr->arg.u32[1], &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IROPCODE_BZERO:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_code_builder_zero_memory(mem, code, current_block_id, instr_ref2, instr->arg.u32[0],
                                                          instr->arg.u32[1], &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IROPCODE_BCOPY:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));
            REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, code, current_block_id, instr_ref3, instr_ref2,
                                                          instr->arg.u32[0], instr->arg.u32[1], &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IROPCODE_VARARG_START:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_code_builder_vararg_start(mem, code, current_block_id, instr_ref2, &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IROPCODE_VARARG_END:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_code_builder_vararg_end(mem, code, current_block_id, instr_ref2, &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IROPCODE_VARARG_GET:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_code_builder_vararg_get(mem, code, current_block_id, instr_ref2, instr->arg.u32[0],
                                                         instr->arg.u32[1], &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IROPCODE_VARARG_COPY:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));
            REQUIRE_OK(
                kefir_opt_code_builder_vararg_copy(mem, code, current_block_id, instr_ref2, instr_ref3, &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IROPCODE_ALLOCA:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));
            REQUIRE_OK(kefir_opt_code_builder_stack_alloc(mem, code, current_block_id, instr_ref3, instr_ref2,
                                                          instr->arg.u64 == 0, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IROPCODE_PUSHSCOPE:
            REQUIRE_OK(kefir_opt_code_builder_scope_push(mem, code, current_block_id, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IROPCODE_POPSCOPE:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_code_builder_scope_pop(mem, code, current_block_id, instr_ref2, &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IROPCODE_INVOKE: {
            const struct kefir_ir_function_decl *ir_decl =
                kefir_ir_module_get_declaration(module->ir_module, (kefir_id_t) instr->arg.u64);
            REQUIRE(ir_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Failed to obtain IR function declaration"));
            kefir_size_t num_of_params = kefir_ir_type_children(ir_decl->params);
            kefir_bool_t has_return = kefir_ir_type_children(ir_decl->result) > 0;

            kefir_opt_call_id_t call_ref;
            REQUIRE_OK(
                kefir_opt_code_container_new_call(mem, code, current_block_id, ir_decl->id, num_of_params, &call_ref));
            for (kefir_size_t i = 0; i < num_of_params; i++) {
                REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, code, call_ref, num_of_params - i - 1, instr_ref));
            }

            REQUIRE_OK(kefir_opt_code_builder_invoke(mem, code, current_block_id, call_ref, &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            if (has_return) {
                REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            }
        } break;

        case KEFIR_IROPCODE_INVOKEV: {
            const struct kefir_ir_function_decl *ir_decl =
                kefir_ir_module_get_declaration(module->ir_module, (kefir_id_t) instr->arg.u64);
            REQUIRE(ir_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Failed to obtain IR function declaration"));
            kefir_size_t num_of_params = kefir_ir_type_children(ir_decl->params);
            kefir_bool_t has_return = kefir_ir_type_children(ir_decl->result) > 0;

            kefir_opt_call_id_t call_ref;
            REQUIRE_OK(
                kefir_opt_code_container_new_call(mem, code, current_block_id, ir_decl->id, num_of_params, &call_ref));
            for (kefir_size_t i = 0; i < num_of_params; i++) {
                REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, code, call_ref, num_of_params - i - 1, instr_ref));
            }
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));

            REQUIRE_OK(
                kefir_opt_code_builder_invoke_virtual(mem, code, current_block_id, call_ref, instr_ref2, &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            if (has_return) {
                REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            }
        } break;

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

        case KEFIR_IROPCODE_RESERVED:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR opcode");

        case KEFIR_IROPCODE_NOP:
            break;

        case KEFIR_IROPCODE_INLINEASM:
            REQUIRE_OK(construct_inline_asm(mem, module, code, state, instr));
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

            UNARY_OP(int64_truncate_1bit, KEFIR_IROPCODE_TRUNCATE1)
            UNARY_OP(int64_sign_extend_8bits, KEFIR_IROPCODE_EXTEND8)
            UNARY_OP(int64_sign_extend_16bits, KEFIR_IROPCODE_EXTEND16)
            UNARY_OP(int64_sign_extend_32bits, KEFIR_IROPCODE_EXTEND32)

            UNARY_OP(float32_neg, KEFIR_IROPCODE_F32NEG)
            UNARY_OP(float64_neg, KEFIR_IROPCODE_F64NEG)

            UNARY_OP(float32_to_int, KEFIR_IROPCODE_F32CINT)
            UNARY_OP(float64_to_int, KEFIR_IROPCODE_F64CINT)
            UNARY_OP(float32_to_uint, KEFIR_IROPCODE_F32CUINT)
            UNARY_OP(float64_to_uint, KEFIR_IROPCODE_F64CUINT)
            UNARY_OP(int_to_float32, KEFIR_IROPCODE_INTCF32)
            UNARY_OP(int_to_float64, KEFIR_IROPCODE_INTCF64)
            UNARY_OP(uint_to_float32, KEFIR_IROPCODE_UINTCF32)
            UNARY_OP(uint_to_float64, KEFIR_IROPCODE_UINTCF64)
            UNARY_OP(float32_to_float64, KEFIR_IROPCODE_F32CF64)
            UNARY_OP(float64_to_float32, KEFIR_IROPCODE_F64CF32)
            UNARY_OP(long_double_to_int, KEFIR_IROPCODE_LDCINT)
            UNARY_OP(long_double_to_uint, KEFIR_IROPCODE_LDCUINT)
            UNARY_OP(long_double_to_float32, KEFIR_IROPCODE_LDCF32)
            UNARY_OP(long_double_to_float64, KEFIR_IROPCODE_LDCF64)

            UNARY_OP(long_double_neg, KEFIR_IROPCODE_LDNEG)
            UNARY_OP(int_to_long_double, KEFIR_IROPCODE_INTCLD)
            UNARY_OP(uint_to_long_double, KEFIR_IROPCODE_UINTCLD)
            UNARY_OP(float32_to_long_double, KEFIR_IROPCODE_F32CLD)
            UNARY_OP(float64_to_long_double, KEFIR_IROPCODE_F64CLD)

            UNARY_OP(complex_float32_real, KEFIR_IROPCODE_CMPF32R)
            UNARY_OP(complex_float32_imaginary, KEFIR_IROPCODE_CMPF32I)
            UNARY_OP(complex_float64_real, KEFIR_IROPCODE_CMPF64R)
            UNARY_OP(complex_float64_imaginary, KEFIR_IROPCODE_CMPF64I)
            UNARY_OP(complex_long_double_real, KEFIR_IROPCODE_CMPLDR)
            UNARY_OP(complex_long_double_imaginary, KEFIR_IROPCODE_CMPLDI)

            UNARY_OP(complex_float32_truncate_1bit, KEFIR_IROPCODE_CMPF32TRUNC1)
            UNARY_OP(complex_float64_truncate_1bit, KEFIR_IROPCODE_CMPF64TRUNC1)
            UNARY_OP(complex_long_double_truncate_1bit, KEFIR_IROPCODE_CMPLDTRUNC1)

            UNARY_OP(complex_float32_neg, KEFIR_IROPCODE_CMPF32NEG)
            UNARY_OP(complex_float64_neg, KEFIR_IROPCODE_CMPF64NEG)
            UNARY_OP(complex_long_double_neg, KEFIR_IROPCODE_CMPLDNEG)

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

            BINARY_OP(float32_add, KEFIR_IROPCODE_F32ADD)
            BINARY_OP(float32_sub, KEFIR_IROPCODE_F32SUB)
            BINARY_OP(float32_mul, KEFIR_IROPCODE_F32MUL)
            BINARY_OP(float32_div, KEFIR_IROPCODE_F32DIV)
            BINARY_OP(float64_add, KEFIR_IROPCODE_F64ADD)
            BINARY_OP(float64_sub, KEFIR_IROPCODE_F64SUB)
            BINARY_OP(float64_mul, KEFIR_IROPCODE_F64MUL)
            BINARY_OP(float64_div, KEFIR_IROPCODE_F64DIV)

            BINARY_OP(float32_equals, KEFIR_IROPCODE_F32EQUALS)
            BINARY_OP(float32_greater, KEFIR_IROPCODE_F32GREATER)
            BINARY_OP(float32_lesser, KEFIR_IROPCODE_F32LESSER)
            BINARY_OP(float64_equals, KEFIR_IROPCODE_F64EQUALS)
            BINARY_OP(float64_greater, KEFIR_IROPCODE_F64GREATER)
            BINARY_OP(float64_lesser, KEFIR_IROPCODE_F64LESSER)
            BINARY_OP(long_double_equals, KEFIR_IROPCODE_LDEQUALS)
            BINARY_OP(long_double_greater, KEFIR_IROPCODE_LDGREATER)
            BINARY_OP(long_double_lesser, KEFIR_IROPCODE_LDLESSER)

            BINARY_OP(long_double_add, KEFIR_IROPCODE_LDADD)
            BINARY_OP(long_double_sub, KEFIR_IROPCODE_LDSUB)
            BINARY_OP(long_double_mul, KEFIR_IROPCODE_LDMUL)
            BINARY_OP(long_double_div, KEFIR_IROPCODE_LDDIV)

            BINARY_OP(complex_float32_from, KEFIR_IROPCODE_CMPF32)
            BINARY_OP(complex_float64_from, KEFIR_IROPCODE_CMPF64)
            BINARY_OP(complex_long_double_from, KEFIR_IROPCODE_CMPLD)

            BINARY_OP(complex_float32_equals, KEFIR_IROPCODE_CMPF32EQUALS)
            BINARY_OP(complex_float64_equals, KEFIR_IROPCODE_CMPF64EQUALS)
            BINARY_OP(complex_long_double_equals, KEFIR_IROPCODE_CMPLDEQUALS)

            BINARY_OP(complex_float32_add, KEFIR_IROPCODE_CMPF32ADD)
            BINARY_OP(complex_float64_add, KEFIR_IROPCODE_CMPF64ADD)
            BINARY_OP(complex_long_double_add, KEFIR_IROPCODE_CMPLDADD)
            BINARY_OP(complex_float32_sub, KEFIR_IROPCODE_CMPF32SUB)
            BINARY_OP(complex_float64_sub, KEFIR_IROPCODE_CMPF64SUB)
            BINARY_OP(complex_long_double_sub, KEFIR_IROPCODE_CMPLDSUB)
            BINARY_OP(complex_float32_mul, KEFIR_IROPCODE_CMPF32MUL)
            BINARY_OP(complex_float64_mul, KEFIR_IROPCODE_CMPF64MUL)
            BINARY_OP(complex_long_double_mul, KEFIR_IROPCODE_CMPLDMUL)
            BINARY_OP(complex_float32_div, KEFIR_IROPCODE_CMPF32DIV)
            BINARY_OP(complex_float64_div, KEFIR_IROPCODE_CMPF64DIV)
            BINARY_OP(complex_long_double_div, KEFIR_IROPCODE_CMPLDDIV)

#undef BINARY_OP

#define ATOMIC_LOAD_OP(_id, _opcode)                                                                          \
    case _opcode: {                                                                                           \
        kefir_opt_memory_order_t model;                                                                       \
        switch (instr->arg.i64) {                                                                             \
            case KEFIR_IR_MEMORY_ORDER_SEQ_CST:                                                               \
                model = KEFIR_OPT_MEMORY_ORDER_SEQ_CST;                                                       \
                break;                                                                                        \
        }                                                                                                     \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));                                 \
        REQUIRE_OK(kefir_opt_code_builder_##_id(mem, code, current_block_id, instr_ref2, model, &instr_ref)); \
        REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));                    \
        REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));                                  \
    } break;

            ATOMIC_LOAD_OP(atomic_load8, KEFIR_IROPCODE_ATOMIC_LOAD8)
            ATOMIC_LOAD_OP(atomic_load16, KEFIR_IROPCODE_ATOMIC_LOAD16)
            ATOMIC_LOAD_OP(atomic_load32, KEFIR_IROPCODE_ATOMIC_LOAD32)
            ATOMIC_LOAD_OP(atomic_load64, KEFIR_IROPCODE_ATOMIC_LOAD64)
            ATOMIC_LOAD_OP(atomic_load_long_double, KEFIR_IROPCODE_ATOMIC_LOAD_LONG_DOUBLE)

#undef ATOMIC_LOAD_OP

#define ATOMIC_STORE_OP(_id, _opcode)                                                                              \
    case _opcode: {                                                                                                \
        kefir_opt_memory_order_t model;                                                                            \
        switch (instr->arg.i64) {                                                                                  \
            case KEFIR_IR_MEMORY_ORDER_SEQ_CST:                                                                    \
                model = KEFIR_OPT_MEMORY_ORDER_SEQ_CST;                                                            \
                break;                                                                                             \
                                                                                                                   \
            default:                                                                                               \
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown IR atomic model flag");                       \
        }                                                                                                          \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));                                      \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));                                      \
        REQUIRE_OK(                                                                                                \
            kefir_opt_code_builder_##_id(mem, code, current_block_id, instr_ref2, instr_ref3, model, &instr_ref)); \
        REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));                         \
        REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));                                       \
    } break;

            ATOMIC_STORE_OP(atomic_store8, KEFIR_IROPCODE_ATOMIC_STORE8)
            ATOMIC_STORE_OP(atomic_store16, KEFIR_IROPCODE_ATOMIC_STORE16)
            ATOMIC_STORE_OP(atomic_store32, KEFIR_IROPCODE_ATOMIC_STORE32)
            ATOMIC_STORE_OP(atomic_store64, KEFIR_IROPCODE_ATOMIC_STORE64)
            ATOMIC_STORE_OP(atomic_store_long_double, KEFIR_IROPCODE_ATOMIC_STORE_LONG_DOUBLE)

#undef ATOMIC_STORE_OP

        case KEFIR_IROPCODE_ATOMIC_BCOPY_FROM: {
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));
            kefir_opt_memory_order_t model;
            switch (instr->arg.u32[0]) {
                case KEFIR_IR_MEMORY_ORDER_SEQ_CST:
                    model = KEFIR_OPT_MEMORY_ORDER_SEQ_CST;
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown IR atomic model flag");
            }
            REQUIRE_OK(kefir_opt_code_builder_atomic_copy_memory_from(mem, code, current_block_id, instr_ref3,
                                                                      instr_ref2, model, instr->arg.u32[1],
                                                                      instr->arg.u32[2], &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
        } break;

        case KEFIR_IROPCODE_ATOMIC_BCOPY_TO: {
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));
            kefir_opt_memory_order_t model;
            switch (instr->arg.u32[0]) {
                case KEFIR_IR_MEMORY_ORDER_SEQ_CST:
                    model = KEFIR_OPT_MEMORY_ORDER_SEQ_CST;
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown IR atomic model flag");
            }
            REQUIRE_OK(kefir_opt_code_builder_atomic_copy_memory_to(mem, code, current_block_id, instr_ref3, instr_ref2,
                                                                    model, instr->arg.u32[1], instr->arg.u32[2],
                                                                    &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
        } break;

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
            LOAD_OP(long_double_load, KEFIR_IROPCODE_LOADLD)

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
            STORE_OP(long_double_store, KEFIR_IROPCODE_STORELD)

#undef STORE_OP
    }
    return KEFIR_OK;
}

static kefir_result_t push_function_arguments(struct kefir_mem *mem, struct kefir_opt_constructor_state *state) {
    REQUIRE_OK(kefir_opt_constructor_update_current_code_block(mem, state, (kefir_size_t) -1ll));
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
        REQUIRE_OK(kefir_opt_constructor_update_current_code_block(mem, state, state->ir_location));

        const struct kefir_irinstr *instr = kefir_irblock_at(ir_block, state->ir_location);
        REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid IR instruction to be returned"));
        REQUIRE_OK(translate_instruction(mem, module, &state->function->code, state, instr));
    }

    REQUIRE_OK(kefir_opt_constructor_update_current_code_block(mem, state, state->ir_location));
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
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, (kefir_uptr_t) source_iter->value);
        ASSIGN_DECL_CAST(kefir_opt_phi_id_t, phi_ref, (kefir_uptr_t) target_iter->value);
        REQUIRE_OK(
            kefir_opt_code_container_phi_attach(mem, &state->function->code, phi_ref, source_block_id, instr_ref));
    }
    REQUIRE(target_iter == NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to link optimizer block outputs with target block phi nodes"));
    return KEFIR_OK;
}

static kefir_result_t link_blocks_match(struct kefir_mem *mem, struct kefir_opt_constructor_state *state,
                                        const struct kefir_opt_code_block *block) {
    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&state->function->code, block, &instr));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_JUMP:
            REQUIRE_OK(link_blocks_impl(mem, state, block->id, instr->operation.parameters.branch.target_block));
            break;

        case KEFIR_OPT_OPCODE_BRANCH:
        case KEFIR_OPT_OPCODE_COMPARE_BRANCH:
            REQUIRE_OK(link_blocks_impl(mem, state, block->id, instr->operation.parameters.branch.target_block));
            REQUIRE_OK(link_blocks_impl(mem, state, block->id, instr->operation.parameters.branch.alternative_block));
            break;

        case KEFIR_OPT_OPCODE_IJUMP: {
            kefir_result_t res;
            struct kefir_hashtreeset_iterator iter;
            for (res = kefir_hashtreeset_iter(&state->indirect_jump_targets, &iter); res == KEFIR_OK;
                 res = kefir_hashtreeset_next(&iter)) {
                ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block_id, iter.entry);

                struct kefir_hashtree_node *hashtree_node = NULL;
                REQUIRE_OK(kefir_hashtree_at(&state->code_block_index, (kefir_hashtree_key_t) target_block_id,
                                             &hashtree_node));
                ASSIGN_DECL_CAST(struct kefir_opt_constructor_code_block_state *, target_state, hashtree_node->value);
                REQUIRE(
                    kefir_list_length(&target_state->phi_stack) == 0,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Potential target of indirect jump shall have no phi nodes"));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        } break;

        case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY: {
            struct kefir_opt_inline_assembly_node *inline_asm = NULL;
            REQUIRE_OK(kefir_opt_code_container_inline_assembly(
                &state->function->code, instr->operation.parameters.inline_asm_ref, &inline_asm));
            if (!kefir_hashtree_empty(&inline_asm->jump_targets)) {
                REQUIRE_OK(link_blocks_impl(mem, state, block->id, inline_asm->default_jump_target));

                struct kefir_hashtree_node_iterator iter;
                for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->jump_targets, &iter);
                     node != NULL; node = kefir_hashtree_next(&iter)) {
                    ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block, node->value);
                    REQUIRE_OK(link_blocks_impl(mem, state, block->id, target_block));
                }
            }
        } break;

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
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&state->function->code, KEFIR_ID_NONE, instr_ref));
        REQUIRE_OK(kefir_list_insert_after(mem, &target_state->stack, NULL, (void *) (kefir_uptr_t) instr_ref));
        REQUIRE_OK(kefir_list_insert_after(mem, &target_state->phi_stack, NULL, (void *) (kefir_uptr_t) phi_ref));
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
    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_block(&state->function->code, block_id, &block));
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&state->function->code, block, &instr));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_JUMP: {
            kefir_opt_block_id_t target_block = instr->operation.parameters.branch.target_block;
            REQUIRE_OK(link_blocks_equalize_stack(mem, state, block_id, target_block));
            REQUIRE_OK(link_blocks_traverse(mem, state, target_block));
        } break;

        case KEFIR_OPT_OPCODE_BRANCH: {
            kefir_opt_block_id_t target_block = instr->operation.parameters.branch.target_block;
            kefir_opt_block_id_t alternative_block = instr->operation.parameters.branch.alternative_block;

            REQUIRE_OK(link_blocks_equalize_stack(mem, state, block_id, target_block));
            REQUIRE_OK(link_blocks_equalize_stack(mem, state, block_id, alternative_block));
            REQUIRE_OK(link_blocks_traverse(mem, state, target_block));
            REQUIRE_OK(link_blocks_traverse(mem, state, alternative_block));
        } break;

        case KEFIR_OPT_OPCODE_IJUMP:
        case KEFIR_OPT_OPCODE_RETURN:
            // Intentionally left blank
            break;

        case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY: {
            struct kefir_opt_inline_assembly_node *inline_asm = NULL;
            REQUIRE_OK(kefir_opt_code_container_inline_assembly(
                &state->function->code, instr->operation.parameters.inline_asm_ref, &inline_asm));
            if (!kefir_hashtree_empty(&inline_asm->jump_targets)) {
                REQUIRE_OK(link_blocks_equalize_stack(mem, state, block_id, inline_asm->default_jump_target));

                struct kefir_hashtree_node_iterator iter;
                for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->jump_targets, &iter);
                     node != NULL; node = kefir_hashtree_next(&iter)) {
                    ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block, node->value);
                    REQUIRE_OK(link_blocks_equalize_stack(mem, state, block_id, target_block));
                }

                REQUIRE_OK(link_blocks_traverse(mem, state, inline_asm->default_jump_target));
                for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->jump_targets, &iter);
                     node != NULL; node = kefir_hashtree_next(&iter)) {
                    ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block, node->value);
                    REQUIRE_OK(link_blocks_traverse(mem, state, target_block));
                }
            }
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Encountered unterminated optimizer code block");
    }
    return KEFIR_OK;
}

static kefir_result_t link_blocks(struct kefir_mem *mem, struct kefir_opt_constructor_state *state) {
    REQUIRE_OK(link_blocks_traverse(mem, state, state->function->code.entry_point));
    kefir_result_t res;
    struct kefir_hashtreeset_iterator htreeset_iter;
    for (res = kefir_hashtreeset_iter(&state->indirect_jump_targets, &htreeset_iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&htreeset_iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block_id, htreeset_iter.entry);
        REQUIRE_OK(link_blocks_traverse(mem, state, target_block_id));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    struct kefir_opt_code_container_iterator iter;
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(&state->function->code, &iter);
         block != NULL; block = kefir_opt_code_container_next(&iter)) {

        REQUIRE_OK(link_blocks_match(mem, state, block));
    }
    return KEFIR_OK;
}

static kefir_result_t construct_code_from_ir(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                             struct kefir_opt_constructor_state *state) {
    REQUIRE_OK(identify_code_blocks(mem, module, state));
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
