/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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
    for (; i < kefir_irblock_length(&state->function->ir_func->body); i++) {
        const struct kefir_irinstr *instr = kefir_irblock_at(&state->function->ir_func->body, i);
        REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid IR instruction to be returned"));

        if (start_new_block) {
            REQUIRE_OK(kefir_opt_constructor_start_code_block_at(mem, state, i));
        }
        start_new_block = false;

        switch (instr->opcode) {
            case KEFIR_IR_OPCODE_BLOCK_LABEL:
                REQUIRE_OK(kefir_opt_constructor_start_code_block_at(mem, state, instr->arg.u64));
                REQUIRE_OK(kefir_opt_constructor_mark_code_block_for_indirect_jump(mem, state, instr->arg.u64));
                break;

            case KEFIR_IR_OPCODE_JUMP:
            case KEFIR_IR_OPCODE_BRANCH:
                REQUIRE_OK(kefir_opt_constructor_start_code_block_at(mem, state, instr->arg.u64));
                // Fallthrough

            case KEFIR_IR_OPCODE_IJUMP:
            case KEFIR_IR_OPCODE_RETURN:
                start_new_block = true;
                break;

            case KEFIR_IR_OPCODE_INLINE_ASSEMBLY: {
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

    kefir_opt_instruction_ref_t instr_ref;
    kefir_opt_inline_assembly_id_t inline_asm_ref;
    REQUIRE_OK(kefir_opt_code_container_new_inline_assembly(
        mem, code, state->current_block->block_id, ir_inline_asm->id, kefir_list_length(&ir_inline_asm->parameter_list),
        &inline_asm_ref, &instr_ref));

    kefir_size_t param_idx = 0;
    for (const struct kefir_list_entry *iter = kefir_list_head(&ir_inline_asm->parameter_list); iter != NULL;
         kefir_list_next(&iter), param_idx++) {
        ASSIGN_DECL_CAST(const struct kefir_ir_inline_assembly_parameter *, ir_asm_param, iter->value);

        struct kefir_opt_inline_assembly_parameter inline_asm_param = {.load_store_ref = KEFIR_ID_NONE,
                                                                       .read_ref = KEFIR_ID_NONE};
        kefir_opt_instruction_ref_t param_ref;
        switch (ir_asm_param->klass) {
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_STORE:
            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD_STORE:
                REQUIRE_OK(kefir_opt_constructor_stack_at(mem, state, ir_asm_param->load_store_index, &param_ref));
                inline_asm_param.load_store_ref = param_ref;
                REQUIRE_OK(kefir_opt_code_container_inline_assembly_set_parameter(
                    mem, code, inline_asm_ref, ir_asm_param->parameter_id, &inline_asm_param));
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ_STORE:
                REQUIRE_OK(kefir_opt_constructor_stack_at(mem, state, ir_asm_param->load_store_index, &param_ref));
                inline_asm_param.load_store_ref = param_ref;
                REQUIRE_OK(kefir_opt_constructor_stack_at(mem, state, ir_asm_param->read_index, &param_ref));
                inline_asm_param.read_ref = param_ref;
                REQUIRE_OK(kefir_opt_code_container_inline_assembly_set_parameter(
                    mem, code, inline_asm_ref, ir_asm_param->parameter_id, &inline_asm_param));
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ:
                REQUIRE_OK(kefir_opt_constructor_stack_at(mem, state, ir_asm_param->read_index, &param_ref));
                inline_asm_param.read_ref = param_ref;
                REQUIRE_OK(kefir_opt_code_container_inline_assembly_set_parameter(
                    mem, code, inline_asm_ref, ir_asm_param->parameter_id, &inline_asm_param));
                break;

            case KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_IMMEDIATE:
                // Intentionally left blank
                break;
        }
    }

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

static kefir_result_t get_condition_variant(kefir_uint64_t variant,
                                            kefir_opt_branch_condition_variant_t *condition_variant) {
    switch (variant) {
        case KEFIR_IR_BRANCH_CONDITION_8BIT:
            *condition_variant = KEFIR_OPT_BRANCH_CONDITION_8BIT;
            break;

        case KEFIR_IR_BRANCH_CONDITION_16BIT:
            *condition_variant = KEFIR_OPT_BRANCH_CONDITION_16BIT;
            break;

        case KEFIR_IR_BRANCH_CONDITION_32BIT:
            *condition_variant = KEFIR_OPT_BRANCH_CONDITION_32BIT;
            break;

        case KEFIR_IR_BRANCH_CONDITION_64BIT:
            *condition_variant = KEFIR_OPT_BRANCH_CONDITION_64BIT;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR condition variant");
    }
    return KEFIR_OK;
}

static kefir_result_t get_comparison_operation(kefir_uint64_t operation, kefir_opt_comparison_operation_t *compare) {
    switch (operation) {
        case KEFIR_IR_COMPARE_INT8_EQUALS:
            *compare = KEFIR_OPT_COMPARISON_INT8_EQUALS;
            break;

        case KEFIR_IR_COMPARE_INT16_EQUALS:
            *compare = KEFIR_OPT_COMPARISON_INT16_EQUALS;
            break;

        case KEFIR_IR_COMPARE_INT32_EQUALS:
            *compare = KEFIR_OPT_COMPARISON_INT32_EQUALS;
            break;

        case KEFIR_IR_COMPARE_INT64_EQUALS:
            *compare = KEFIR_OPT_COMPARISON_INT64_EQUALS;
            break;

        case KEFIR_IR_COMPARE_INT8_GREATER:
            *compare = KEFIR_OPT_COMPARISON_INT8_GREATER;
            break;

        case KEFIR_IR_COMPARE_INT16_GREATER:
            *compare = KEFIR_OPT_COMPARISON_INT16_GREATER;
            break;

        case KEFIR_IR_COMPARE_INT32_GREATER:
            *compare = KEFIR_OPT_COMPARISON_INT32_GREATER;
            break;

        case KEFIR_IR_COMPARE_INT64_GREATER:
            *compare = KEFIR_OPT_COMPARISON_INT64_GREATER;
            break;

        case KEFIR_IR_COMPARE_INT8_LESSER:
            *compare = KEFIR_OPT_COMPARISON_INT8_LESSER;
            break;

        case KEFIR_IR_COMPARE_INT16_LESSER:
            *compare = KEFIR_OPT_COMPARISON_INT16_LESSER;
            break;

        case KEFIR_IR_COMPARE_INT32_LESSER:
            *compare = KEFIR_OPT_COMPARISON_INT32_LESSER;
            break;

        case KEFIR_IR_COMPARE_INT64_LESSER:
            *compare = KEFIR_OPT_COMPARISON_INT64_LESSER;
            break;

        case KEFIR_IR_COMPARE_INT8_ABOVE:
            *compare = KEFIR_OPT_COMPARISON_INT8_ABOVE;
            break;

        case KEFIR_IR_COMPARE_INT16_ABOVE:
            *compare = KEFIR_OPT_COMPARISON_INT16_ABOVE;
            break;

        case KEFIR_IR_COMPARE_INT32_ABOVE:
            *compare = KEFIR_OPT_COMPARISON_INT32_ABOVE;
            break;

        case KEFIR_IR_COMPARE_INT64_ABOVE:
            *compare = KEFIR_OPT_COMPARISON_INT64_ABOVE;
            break;

        case KEFIR_IR_COMPARE_INT8_BELOW:
            *compare = KEFIR_OPT_COMPARISON_INT8_BELOW;
            break;

        case KEFIR_IR_COMPARE_INT16_BELOW:
            *compare = KEFIR_OPT_COMPARISON_INT16_BELOW;
            break;

        case KEFIR_IR_COMPARE_INT32_BELOW:
            *compare = KEFIR_OPT_COMPARISON_INT32_BELOW;
            break;

        case KEFIR_IR_COMPARE_INT64_BELOW:
            *compare = KEFIR_OPT_COMPARISON_INT64_BELOW;
            break;

        case KEFIR_IR_COMPARE_FLOAT32_EQUALS:
            *compare = KEFIR_OPT_COMPARISON_FLOAT32_EQUAL;
            break;

        case KEFIR_IR_COMPARE_FLOAT32_GREATER:
            *compare = KEFIR_OPT_COMPARISON_FLOAT32_GREATER;
            break;

        case KEFIR_IR_COMPARE_FLOAT32_LESSER:
            *compare = KEFIR_OPT_COMPARISON_FLOAT32_LESSER;
            break;

        case KEFIR_IR_COMPARE_FLOAT64_EQUALS:
            *compare = KEFIR_OPT_COMPARISON_FLOAT64_EQUAL;
            break;

        case KEFIR_IR_COMPARE_FLOAT64_GREATER:
            *compare = KEFIR_OPT_COMPARISON_FLOAT64_GREATER;
            break;

        case KEFIR_IR_COMPARE_FLOAT64_LESSER:
            *compare = KEFIR_OPT_COMPARISON_FLOAT64_LESSER;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR scalar comparison operation");
    }
    return KEFIR_OK;
}

static kefir_result_t translate_instruction(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                            struct kefir_opt_code_container *code,
                                            struct kefir_opt_constructor_state *state,
                                            const struct kefir_irinstr *instr) {
    kefir_opt_instruction_ref_t instr_ref, instr_ref2, instr_ref3, instr_ref4, instr_ref5;
    const kefir_opt_block_id_t current_block_id = state->current_block->block_id;
    switch (instr->opcode) {
        case KEFIR_IR_OPCODE_NULL_REF:
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, KEFIR_ID_NONE));
            break;

        case KEFIR_IR_OPCODE_JUMP: {
            struct kefir_opt_constructor_code_block_state *jump_target_block = NULL;
            REQUIRE_OK(kefir_opt_constructor_find_code_block_for(state, instr->arg.u64, &jump_target_block));
            REQUIRE_OK(
                kefir_opt_code_builder_finalize_jump(mem, code, current_block_id, jump_target_block->block_id, NULL));
        } break;

        case KEFIR_IR_OPCODE_BRANCH: {
            struct kefir_opt_constructor_code_block_state *jump_target_block = NULL, *alternative_block = NULL;
            kefir_opt_branch_condition_variant_t condition_variant = 0;
            REQUIRE_OK(get_condition_variant(instr->arg.u64_2[1], &condition_variant));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_find_code_block_for(state, instr->arg.u64_2[0], &jump_target_block));
            REQUIRE_OK(kefir_opt_constructor_find_code_block_for(state, state->ir_location + 1, &alternative_block));
            REQUIRE_OK(kefir_opt_code_builder_finalize_branch(mem, code, current_block_id, condition_variant, instr_ref,
                                                              jump_target_block->block_id, alternative_block->block_id,
                                                              NULL));
        } break;

        case KEFIR_IR_OPCODE_BRANCH_COMPARE: {
            struct kefir_opt_constructor_code_block_state *jump_target_block = NULL, *alternative_block = NULL;
            kefir_opt_comparison_operation_t compare_op = 0;
            REQUIRE_OK(get_comparison_operation(instr->arg.u64_2[1], &compare_op));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_find_code_block_for(state, instr->arg.u64_2[0], &jump_target_block));
            REQUIRE_OK(kefir_opt_constructor_find_code_block_for(state, state->ir_location + 1, &alternative_block));
            REQUIRE_OK(kefir_opt_code_builder_finalize_branch_compare(
                mem, code, current_block_id, compare_op, instr_ref, instr_ref2, jump_target_block->block_id,
                alternative_block->block_id, NULL));
        } break;

        case KEFIR_IR_OPCODE_SELECT: {
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref4));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            kefir_opt_branch_condition_variant_t condition_variant;
            REQUIRE_OK(get_condition_variant(instr->arg.u64, &condition_variant));
            REQUIRE_OK(kefir_opt_code_builder_select(mem, code, current_block_id, condition_variant, instr_ref2,
                                                     instr_ref3, instr_ref4, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
        } break;

        case KEFIR_IR_OPCODE_SELECT_COMPARE: {
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref5));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref4));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            kefir_opt_comparison_operation_t compare_op;
            REQUIRE_OK(get_comparison_operation(instr->arg.u64, &compare_op));
            REQUIRE_OK(kefir_opt_code_builder_select_compare(mem, code, current_block_id, compare_op, instr_ref2,
                                                             instr_ref3, instr_ref4, instr_ref5, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
        } break;

        case KEFIR_IR_OPCODE_LOCAL_LIFETIME_MARK:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_code_builder_local_lifetime_mark(mem, code, current_block_id, instr_ref2, &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IR_OPCODE_IJUMP:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_finalize_indirect_jump(mem, code, current_block_id, instr_ref, NULL));
            break;

        case KEFIR_IR_OPCODE_RETURN:
            instr_ref = KEFIR_ID_NONE;
            if (kefir_ir_type_length(state->function->ir_func->declaration->result) > 0) {
                REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref));
            }
            REQUIRE_OK(kefir_opt_code_builder_finalize_return(mem, code, current_block_id, instr_ref, NULL));
            break;

        case KEFIR_IR_OPCODE_INT_CONST:
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, code, current_block_id, instr->arg.i64, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_UINT_CONST:
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, code, current_block_id, instr->arg.u64, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_BITINT_SIGNED_CONST:
            REQUIRE_OK(kefir_opt_code_builder_bitint_signed_constant(mem, code, current_block_id, instr->arg.u32[0],
                                                                     &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_BITINT_UNSIGNED_CONST:
            REQUIRE_OK(kefir_opt_code_builder_bitint_unsigned_constant(mem, code, current_block_id, instr->arg.u32[0],
                                                                       &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_FLOAT32_CONST:
            REQUIRE_OK(
                kefir_opt_code_builder_float32_constant(mem, code, current_block_id, instr->arg.f32[0], &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_FLOAT64_CONST:
            REQUIRE_OK(
                kefir_opt_code_builder_float64_constant(mem, code, current_block_id, instr->arg.f64, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_LONG_DOUBLE_CONST:
            REQUIRE_OK(kefir_opt_code_builder_long_double_constant(mem, code, current_block_id, instr->arg.long_double,
                                                                   &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_STRING_REF:
            REQUIRE_OK(
                kefir_opt_code_builder_string_reference(mem, code, current_block_id, instr->arg.u64, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_BLOCK_LABEL: {
            struct kefir_opt_constructor_code_block_state *block = NULL;
            REQUIRE_OK(kefir_opt_constructor_find_code_block_for(state, instr->arg.u64, &block));
            REQUIRE_OK(kefir_opt_code_builder_block_label(mem, code, current_block_id, block->block_id, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
        } break;

        case KEFIR_IR_OPCODE_INT_PLACEHOLDER:
            REQUIRE_OK(kefir_opt_code_builder_int_placeholder(mem, code, current_block_id, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_FLOAT32_PLACEHOLDER:
            REQUIRE_OK(kefir_opt_code_builder_float32_placeholder(mem, code, current_block_id, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_FLOAT64_PLACEHOLDER:
            REQUIRE_OK(kefir_opt_code_builder_float64_placeholder(mem, code, current_block_id, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_VSTACK_PICK:
            REQUIRE_OK(kefir_opt_constructor_stack_at(mem, state, instr->arg.u64, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_VSTACK_POP:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref));
            break;

        case KEFIR_IR_OPCODE_VSTACK_EXCHANGE:
            REQUIRE_OK(kefir_opt_constructor_stack_exchange(mem, state, instr->arg.u64));
            break;

        case KEFIR_IR_OPCODE_GET_GLOBAL:
            REQUIRE_OK(kefir_opt_code_builder_get_global(mem, code, current_block_id, instr->arg.u64, 0, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_GET_THREAD_LOCAL:
            REQUIRE_OK(
                kefir_opt_code_builder_get_thread_local(mem, code, current_block_id, instr->arg.u64, 0, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_GET_LOCAL:
            REQUIRE_OK(kefir_opt_constructor_get_local_allocation(
                mem, state, (((kefir_uint64_t) instr->arg.u32[0]) << 32) | instr->arg.u32[1], instr->arg.u32[2],
                instr->arg.u32[3], &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, code, current_block_id, instr_ref2,
                                                                  instr->arg.u32[0], instr->arg.u32[1], &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, code, current_block_id, instr_ref2,
                                                                    instr->arg.u32[0], instr->arg.u32[1], &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_BITS_INSERT:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));
            REQUIRE_OK(kefir_opt_code_builder_bits_insert(mem, code, current_block_id, instr_ref3, instr_ref2,
                                                          instr->arg.u32[0], instr->arg.u32[1], &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_ZERO_MEMORY:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_code_builder_zero_memory(mem, code, current_block_id, instr_ref2, instr->arg.u32[0],
                                                          instr->arg.u32[1], &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IR_OPCODE_COPY_MEMORY:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));
            REQUIRE_OK(kefir_opt_code_builder_copy_memory(mem, code, current_block_id, instr_ref3, instr_ref2,
                                                          instr->arg.u32[0], instr->arg.u32[1], &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IR_OPCODE_VARARG_START:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_code_builder_vararg_start(mem, code, current_block_id, instr_ref2, &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IR_OPCODE_VARARG_END:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_code_builder_vararg_end(mem, code, current_block_id, instr_ref2, &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IR_OPCODE_VARARG_GET:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));
            REQUIRE_OK(kefir_opt_code_builder_vararg_get(mem, code, current_block_id, instr_ref3, instr_ref2,
                                                         instr->arg.u32[0], instr->arg.u32[1], &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IR_OPCODE_VARARG_COPY:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));
            REQUIRE_OK(
                kefir_opt_code_builder_vararg_copy(mem, code, current_block_id, instr_ref2, instr_ref3, &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IR_OPCODE_STACK_ALLOC:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));
            REQUIRE_OK(kefir_opt_code_builder_stack_alloc(mem, code, current_block_id, instr_ref3, instr_ref2,
                                                          instr->arg.u64 == 0, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_SCOPE_PUSH:
            REQUIRE_OK(kefir_opt_code_builder_scope_push(mem, code, current_block_id, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IR_OPCODE_SCOPE_POP:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_code_builder_scope_pop(mem, code, current_block_id, instr_ref2, &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IR_OPCODE_INVOKE:
        case KEFIR_IR_OPCODE_TAIL_INVOKE: {
            const struct kefir_ir_function_decl *ir_decl =
                kefir_ir_module_get_declaration(module->ir_module, (kefir_id_t) instr->arg.u64);
            REQUIRE(ir_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Failed to obtain IR function declaration"));
            kefir_size_t num_of_params = kefir_ir_type_children(ir_decl->params);
            kefir_bool_t has_return = kefir_ir_type_children(ir_decl->result) > 0;

            kefir_opt_call_id_t call_ref;
            if (instr->opcode == KEFIR_IR_OPCODE_INVOKE) {
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, code, current_block_id, ir_decl->id, num_of_params,
                                                             KEFIR_ID_NONE, &call_ref, &instr_ref));
            } else {
                REQUIRE_OK(kefir_opt_code_container_new_tail_call(mem, code, current_block_id, ir_decl->id,
                                                                  num_of_params, KEFIR_ID_NONE, &call_ref, &instr_ref));
            }
            for (kefir_size_t i = 0; i < num_of_params; i++) {
                REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, code, call_ref, num_of_params - i - 1, instr_ref2));
            }

            const struct kefir_ir_typeentry *return_typeentry = kefir_ir_type_at(ir_decl->result, 0);
            if (return_typeentry != NULL) {
                switch (return_typeentry->typecode) {
                    case KEFIR_IR_TYPE_STRUCT:
                    case KEFIR_IR_TYPE_ARRAY:
                    case KEFIR_IR_TYPE_UNION:
                        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
                        REQUIRE_OK(kefir_opt_code_container_call_set_return_space(mem, code, call_ref, instr_ref2));
                        break;

                    default:
                        // Intentionally left blank
                        break;
                }
            }

            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            if (has_return) {
                REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            }
        } break;

        case KEFIR_IR_OPCODE_INVOKE_VIRTUAL:
        case KEFIR_IR_OPCODE_TAIL_INVOKE_VIRTUAL: {
            const struct kefir_ir_function_decl *ir_decl =
                kefir_ir_module_get_declaration(module->ir_module, (kefir_id_t) instr->arg.u64);
            REQUIRE(ir_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Failed to obtain IR function declaration"));
            kefir_size_t num_of_params = kefir_ir_type_children(ir_decl->params);
            kefir_bool_t has_return = kefir_ir_type_children(ir_decl->result) > 0;

            kefir_opt_call_id_t call_ref;
            REQUIRE_OK(kefir_opt_constructor_stack_at(mem, state, num_of_params, &instr_ref2));
            if (instr->opcode == KEFIR_IR_OPCODE_INVOKE_VIRTUAL) {
                REQUIRE_OK(kefir_opt_code_container_new_call(mem, code, current_block_id, ir_decl->id, num_of_params,
                                                             instr_ref2, &call_ref, &instr_ref));
            } else {
                REQUIRE_OK(kefir_opt_code_container_new_tail_call(mem, code, current_block_id, ir_decl->id,
                                                                  num_of_params, instr_ref2, &call_ref, &instr_ref));
            }
            for (kefir_size_t i = 0; i < num_of_params; i++) {
                REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, code, call_ref, num_of_params - i - 1, instr_ref2));
            }
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            const struct kefir_ir_typeentry *return_typeentry = kefir_ir_type_at(ir_decl->result, 0);
            if (return_typeentry != NULL) {
                switch (return_typeentry->typecode) {
                    case KEFIR_IR_TYPE_STRUCT:
                    case KEFIR_IR_TYPE_ARRAY:
                    case KEFIR_IR_TYPE_UNION:
                        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
                        REQUIRE_OK(kefir_opt_code_container_call_set_return_space(mem, code, call_ref, instr_ref2));
                        break;

                    default:
                        // Intentionally left blank
                        break;
                }
            }

            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            if (has_return) {
                REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            }
        } break;

        case KEFIR_IR_OPCODE_NOP:
            break;

        case KEFIR_IR_OPCODE_INLINE_ASSEMBLY:
            REQUIRE_OK(construct_inline_asm(mem, module, code, state, instr));
            break;

        case KEFIR_IR_OPCODE_FENV_SAVE:
            REQUIRE_OK(kefir_opt_code_builder_fenv_save(mem, code, current_block_id, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IR_OPCODE_FENV_CLEAR:
            REQUIRE_OK(kefir_opt_code_builder_fenv_clear(mem, code, current_block_id, &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

        case KEFIR_IR_OPCODE_FENV_UPDATE:
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_code_builder_fenv_update(mem, code, current_block_id, instr_ref2, &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            break;

#define UNARY_OP(_id, _opcode)                                                                         \
    case _opcode:                                                                                      \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));                          \
        REQUIRE_OK(kefir_opt_code_builder_##_id(mem, code, current_block_id, instr_ref2, &instr_ref)); \
        REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));                           \
        break;

            UNARY_OP(int8_not, KEFIR_IR_OPCODE_INT8_NOT)
            UNARY_OP(int16_not, KEFIR_IR_OPCODE_INT16_NOT)
            UNARY_OP(int32_not, KEFIR_IR_OPCODE_INT32_NOT)
            UNARY_OP(int64_not, KEFIR_IR_OPCODE_INT64_NOT)
            UNARY_OP(int8_neg, KEFIR_IR_OPCODE_INT8_NEG)
            UNARY_OP(int16_neg, KEFIR_IR_OPCODE_INT16_NEG)
            UNARY_OP(int32_neg, KEFIR_IR_OPCODE_INT32_NEG)
            UNARY_OP(int64_neg, KEFIR_IR_OPCODE_INT64_NEG)
            UNARY_OP(int8_bool_not, KEFIR_IR_OPCODE_INT8_BOOL_NOT)
            UNARY_OP(int16_bool_not, KEFIR_IR_OPCODE_INT16_BOOL_NOT)
            UNARY_OP(int32_bool_not, KEFIR_IR_OPCODE_INT32_BOOL_NOT)
            UNARY_OP(int64_bool_not, KEFIR_IR_OPCODE_INT64_BOOL_NOT)

            UNARY_OP(int8_to_bool, KEFIR_IR_OPCODE_INT8_TO_BOOL)
            UNARY_OP(int16_to_bool, KEFIR_IR_OPCODE_INT16_TO_BOOL)
            UNARY_OP(int32_to_bool, KEFIR_IR_OPCODE_INT32_TO_BOOL)
            UNARY_OP(int64_to_bool, KEFIR_IR_OPCODE_INT64_TO_BOOL)
            UNARY_OP(int64_sign_extend_8bits, KEFIR_IR_OPCODE_INT64_SIGN_EXTEND_8BITS)
            UNARY_OP(int64_sign_extend_16bits, KEFIR_IR_OPCODE_INT64_SIGN_EXTEND_16BITS)
            UNARY_OP(int64_sign_extend_32bits, KEFIR_IR_OPCODE_INT64_SIGN_EXTEND_32BITS)
            UNARY_OP(int64_zero_extend_8bits, KEFIR_IR_OPCODE_INT64_ZERO_EXTEND_8BITS)
            UNARY_OP(int64_zero_extend_16bits, KEFIR_IR_OPCODE_INT64_ZERO_EXTEND_16BITS)
            UNARY_OP(int64_zero_extend_32bits, KEFIR_IR_OPCODE_INT64_ZERO_EXTEND_32BITS)

            UNARY_OP(float32_neg, KEFIR_IR_OPCODE_FLOAT32_NEG)
            UNARY_OP(float64_neg, KEFIR_IR_OPCODE_FLOAT64_NEG)

            UNARY_OP(float32_to_int, KEFIR_IR_OPCODE_FLOAT32_TO_INT)
            UNARY_OP(float64_to_int, KEFIR_IR_OPCODE_FLOAT64_TO_INT)
            UNARY_OP(float32_to_uint, KEFIR_IR_OPCODE_FLOAT32_TO_UINT)
            UNARY_OP(float64_to_uint, KEFIR_IR_OPCODE_FLOAT64_TO_UINT)
            UNARY_OP(int_to_float32, KEFIR_IR_OPCODE_INT_TO_FLOAT32)
            UNARY_OP(int_to_float64, KEFIR_IR_OPCODE_INT_TO_FLOAT64)
            UNARY_OP(uint_to_float32, KEFIR_IR_OPCODE_UINT_TO_FLOAT32)
            UNARY_OP(uint_to_float64, KEFIR_IR_OPCODE_UINT_TO_FLOAT64)
            UNARY_OP(float32_to_float64, KEFIR_IR_OPCODE_FLOAT32_TO_FLOAT64)
            UNARY_OP(float64_to_float32, KEFIR_IR_OPCODE_FLOAT64_TO_FLOAT32)
            UNARY_OP(long_double_to_int, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_INT)
            UNARY_OP(long_double_to_uint, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_UINT)
            UNARY_OP(long_double_to_float32, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_FLOAT32)
            UNARY_OP(long_double_to_float64, KEFIR_IR_OPCODE_LONG_DOUBLE_TO_FLOAT64)

            UNARY_OP(long_double_neg, KEFIR_IR_OPCODE_LONG_DOUBLE_NEG)
            UNARY_OP(int_to_long_double, KEFIR_IR_OPCODE_INT_TO_LONG_DOUBLE)
            UNARY_OP(uint_to_long_double, KEFIR_IR_OPCODE_UINT_TO_LONG_DOUBLE)
            UNARY_OP(float32_to_long_double, KEFIR_IR_OPCODE_FLOAT32_TO_LONG_DOUBLE)
            UNARY_OP(float64_to_long_double, KEFIR_IR_OPCODE_FLOAT64_TO_LONG_DOUBLE)

            UNARY_OP(complex_float32_real, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_REAL)
            UNARY_OP(complex_float32_imaginary, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_IMAGINARY)
            UNARY_OP(complex_float64_real, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_REAL)
            UNARY_OP(complex_float64_imaginary, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_IMAGINARY)
            UNARY_OP(complex_long_double_real, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_REAL)
            UNARY_OP(complex_long_double_imaginary, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_IMAGINARY)

            UNARY_OP(complex_float32_truncate_1bit, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_TRUNCATE_1BIT)
            UNARY_OP(complex_float64_truncate_1bit, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_TRUNCATE_1BIT)
            UNARY_OP(complex_long_double_truncate_1bit, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_TRUNCATE_1BIT)

            UNARY_OP(complex_float32_neg, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_NEG)
            UNARY_OP(complex_float64_neg, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_NEG)
            UNARY_OP(complex_long_double_neg, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_NEG)

#undef UNARY_OP

#define BITINT_UNARY_OP(_id, _opcode)                                                                                  \
    case _opcode:                                                                                                      \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));                                          \
        REQUIRE_OK(kefir_opt_code_builder_##_id(mem, code, current_block_id, instr->arg.u64, instr_ref2, &instr_ref)); \
        REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));                                           \
        break;

            BITINT_UNARY_OP(bitint_get_signed, KEFIR_IR_OPCODE_BITINT_GET_SIGNED)
            BITINT_UNARY_OP(bitint_get_unsigned, KEFIR_IR_OPCODE_BITINT_GET_UNSIGNED)
            BITINT_UNARY_OP(bitint_from_signed, KEFIR_IR_OPCODE_BITINT_FROM_SIGNED)
            BITINT_UNARY_OP(bitint_from_unsigned, KEFIR_IR_OPCODE_BITINT_FROM_UNSIGNED)

#undef BITINT_UNARY_OP

        case KEFIR_IR_OPCODE_SCALAR_COMPARE: {
            kefir_opt_comparison_operation_t compare_op;
            REQUIRE_OK(get_comparison_operation(instr->arg.u64, &compare_op));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            REQUIRE_OK(kefir_opt_code_builder_scalar_compare(mem, code, current_block_id, compare_op, instr_ref2,
                                                             instr_ref3, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
        } break;

#undef COMPARE_OP

#define BINARY_OP(_id, _opcode)                                                                                    \
    case _opcode:                                                                                                  \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));                                      \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));                                      \
        REQUIRE_OK(kefir_opt_code_builder_##_id(mem, code, current_block_id, instr_ref2, instr_ref3, &instr_ref)); \
        REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));                                       \
        break;

            BINARY_OP(int8_add, KEFIR_IR_OPCODE_INT8_ADD)
            BINARY_OP(int16_add, KEFIR_IR_OPCODE_INT16_ADD)
            BINARY_OP(int32_add, KEFIR_IR_OPCODE_INT32_ADD)
            BINARY_OP(int64_add, KEFIR_IR_OPCODE_INT64_ADD)
            BINARY_OP(int8_sub, KEFIR_IR_OPCODE_INT8_SUB)
            BINARY_OP(int16_sub, KEFIR_IR_OPCODE_INT16_SUB)
            BINARY_OP(int32_sub, KEFIR_IR_OPCODE_INT32_SUB)
            BINARY_OP(int64_sub, KEFIR_IR_OPCODE_INT64_SUB)
            BINARY_OP(int8_mul, KEFIR_IR_OPCODE_INT8_MUL)
            BINARY_OP(int16_mul, KEFIR_IR_OPCODE_INT16_MUL)
            BINARY_OP(int32_mul, KEFIR_IR_OPCODE_INT32_MUL)
            BINARY_OP(int64_mul, KEFIR_IR_OPCODE_INT64_MUL)
            BINARY_OP(uint8_mul, KEFIR_IR_OPCODE_UINT8_MUL)
            BINARY_OP(uint16_mul, KEFIR_IR_OPCODE_UINT16_MUL)
            BINARY_OP(uint32_mul, KEFIR_IR_OPCODE_UINT32_MUL)
            BINARY_OP(uint64_mul, KEFIR_IR_OPCODE_UINT64_MUL)
            BINARY_OP(int8_div, KEFIR_IR_OPCODE_INT8_DIV)
            BINARY_OP(int16_div, KEFIR_IR_OPCODE_INT16_DIV)
            BINARY_OP(int32_div, KEFIR_IR_OPCODE_INT32_DIV)
            BINARY_OP(int64_div, KEFIR_IR_OPCODE_INT64_DIV)
            BINARY_OP(int8_mod, KEFIR_IR_OPCODE_INT8_MOD)
            BINARY_OP(int16_mod, KEFIR_IR_OPCODE_INT16_MOD)
            BINARY_OP(int32_mod, KEFIR_IR_OPCODE_INT32_MOD)
            BINARY_OP(int64_mod, KEFIR_IR_OPCODE_INT64_MOD)
            BINARY_OP(uint8_div, KEFIR_IR_OPCODE_UINT8_DIV)
            BINARY_OP(uint16_div, KEFIR_IR_OPCODE_UINT16_DIV)
            BINARY_OP(uint32_div, KEFIR_IR_OPCODE_UINT32_DIV)
            BINARY_OP(uint64_div, KEFIR_IR_OPCODE_UINT64_DIV)
            BINARY_OP(uint8_mod, KEFIR_IR_OPCODE_UINT8_MOD)
            BINARY_OP(uint16_mod, KEFIR_IR_OPCODE_UINT16_MOD)
            BINARY_OP(uint32_mod, KEFIR_IR_OPCODE_UINT32_MOD)
            BINARY_OP(uint64_mod, KEFIR_IR_OPCODE_UINT64_MOD)
            BINARY_OP(int8_and, KEFIR_IR_OPCODE_INT8_AND)
            BINARY_OP(int16_and, KEFIR_IR_OPCODE_INT16_AND)
            BINARY_OP(int32_and, KEFIR_IR_OPCODE_INT32_AND)
            BINARY_OP(int64_and, KEFIR_IR_OPCODE_INT64_AND)
            BINARY_OP(int8_or, KEFIR_IR_OPCODE_INT8_OR)
            BINARY_OP(int16_or, KEFIR_IR_OPCODE_INT16_OR)
            BINARY_OP(int32_or, KEFIR_IR_OPCODE_INT32_OR)
            BINARY_OP(int64_or, KEFIR_IR_OPCODE_INT64_OR)
            BINARY_OP(int8_xor, KEFIR_IR_OPCODE_INT8_XOR)
            BINARY_OP(int16_xor, KEFIR_IR_OPCODE_INT16_XOR)
            BINARY_OP(int32_xor, KEFIR_IR_OPCODE_INT32_XOR)
            BINARY_OP(int64_xor, KEFIR_IR_OPCODE_INT64_XOR)
            BINARY_OP(int8_lshift, KEFIR_IR_OPCODE_INT8_LSHIFT)
            BINARY_OP(int16_lshift, KEFIR_IR_OPCODE_INT16_LSHIFT)
            BINARY_OP(int32_lshift, KEFIR_IR_OPCODE_INT32_LSHIFT)
            BINARY_OP(int64_lshift, KEFIR_IR_OPCODE_INT64_LSHIFT)
            BINARY_OP(int8_rshift, KEFIR_IR_OPCODE_INT8_RSHIFT)
            BINARY_OP(int16_rshift, KEFIR_IR_OPCODE_INT16_RSHIFT)
            BINARY_OP(int32_rshift, KEFIR_IR_OPCODE_INT32_RSHIFT)
            BINARY_OP(int64_rshift, KEFIR_IR_OPCODE_INT64_RSHIFT)
            BINARY_OP(int8_arshift, KEFIR_IR_OPCODE_INT8_ARSHIFT)
            BINARY_OP(int16_arshift, KEFIR_IR_OPCODE_INT16_ARSHIFT)
            BINARY_OP(int32_arshift, KEFIR_IR_OPCODE_INT32_ARSHIFT)
            BINARY_OP(int64_arshift, KEFIR_IR_OPCODE_INT64_ARSHIFT)
            BINARY_OP(int8_bool_and, KEFIR_IR_OPCODE_INT8_BOOL_AND)
            BINARY_OP(int16_bool_and, KEFIR_IR_OPCODE_INT16_BOOL_AND)
            BINARY_OP(int32_bool_and, KEFIR_IR_OPCODE_INT32_BOOL_AND)
            BINARY_OP(int64_bool_and, KEFIR_IR_OPCODE_INT64_BOOL_AND)
            BINARY_OP(int8_bool_or, KEFIR_IR_OPCODE_INT8_BOOL_OR)
            BINARY_OP(int16_bool_or, KEFIR_IR_OPCODE_INT16_BOOL_OR)
            BINARY_OP(int32_bool_or, KEFIR_IR_OPCODE_INT32_BOOL_OR)
            BINARY_OP(int64_bool_or, KEFIR_IR_OPCODE_INT64_BOOL_OR)

            BINARY_OP(float32_add, KEFIR_IR_OPCODE_FLOAT32_ADD)
            BINARY_OP(float32_sub, KEFIR_IR_OPCODE_FLOAT32_SUB)
            BINARY_OP(float32_mul, KEFIR_IR_OPCODE_FLOAT32_MUL)
            BINARY_OP(float32_div, KEFIR_IR_OPCODE_FLOAT32_DIV)
            BINARY_OP(float64_add, KEFIR_IR_OPCODE_FLOAT64_ADD)
            BINARY_OP(float64_sub, KEFIR_IR_OPCODE_FLOAT64_SUB)
            BINARY_OP(float64_mul, KEFIR_IR_OPCODE_FLOAT64_MUL)
            BINARY_OP(float64_div, KEFIR_IR_OPCODE_FLOAT64_DIV)

            BINARY_OP(long_double_equals, KEFIR_IR_OPCODE_LONG_DOUBLE_EQUALS)
            BINARY_OP(long_double_greater, KEFIR_IR_OPCODE_LONG_DOUBLE_GREATER)
            BINARY_OP(long_double_lesser, KEFIR_IR_OPCODE_LONG_DOUBLE_LESSER)

            BINARY_OP(long_double_add, KEFIR_IR_OPCODE_LONG_DOUBLE_ADD)
            BINARY_OP(long_double_sub, KEFIR_IR_OPCODE_LONG_DOUBLE_SUB)
            BINARY_OP(long_double_mul, KEFIR_IR_OPCODE_LONG_DOUBLE_MUL)
            BINARY_OP(long_double_div, KEFIR_IR_OPCODE_LONG_DOUBLE_DIV)

            BINARY_OP(complex_float32_from, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_FROM)
            BINARY_OP(complex_float64_from, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_FROM)
            BINARY_OP(complex_long_double_from, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_FROM)

            BINARY_OP(complex_float32_equals, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_EQUALS)
            BINARY_OP(complex_float64_equals, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_EQUALS)
            BINARY_OP(complex_long_double_equals, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_EQUALS)

            BINARY_OP(complex_float32_add, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_ADD)
            BINARY_OP(complex_float64_add, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_ADD)
            BINARY_OP(complex_long_double_add, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_ADD)
            BINARY_OP(complex_float32_sub, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_SUB)
            BINARY_OP(complex_float64_sub, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_SUB)
            BINARY_OP(complex_long_double_sub, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_SUB)
            BINARY_OP(complex_float32_mul, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_MUL)
            BINARY_OP(complex_float64_mul, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_MUL)
            BINARY_OP(complex_long_double_mul, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_MUL)
            BINARY_OP(complex_float32_div, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_DIV)
            BINARY_OP(complex_float64_div, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_DIV)
            BINARY_OP(complex_long_double_div, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_DIV)

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

            ATOMIC_LOAD_OP(atomic_load8, KEFIR_IR_OPCODE_ATOMIC_LOAD8)
            ATOMIC_LOAD_OP(atomic_load16, KEFIR_IR_OPCODE_ATOMIC_LOAD16)
            ATOMIC_LOAD_OP(atomic_load32, KEFIR_IR_OPCODE_ATOMIC_LOAD32)
            ATOMIC_LOAD_OP(atomic_load64, KEFIR_IR_OPCODE_ATOMIC_LOAD64)
            ATOMIC_LOAD_OP(atomic_load_long_double, KEFIR_IR_OPCODE_ATOMIC_LOAD_LONG_DOUBLE)
            ATOMIC_LOAD_OP(atomic_load_complex_float32, KEFIR_IR_OPCODE_ATOMIC_LOAD_COMPLEX_FLOAT32)
            ATOMIC_LOAD_OP(atomic_load_complex_float64, KEFIR_IR_OPCODE_ATOMIC_LOAD_COMPLEX_FLOAT64)
            ATOMIC_LOAD_OP(atomic_load_complex_long_double, KEFIR_IR_OPCODE_ATOMIC_LOAD_COMPLEX_LONG_DOUBLE)

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
    } break;

            ATOMIC_STORE_OP(atomic_store8, KEFIR_IR_OPCODE_ATOMIC_STORE8)
            ATOMIC_STORE_OP(atomic_store16, KEFIR_IR_OPCODE_ATOMIC_STORE16)
            ATOMIC_STORE_OP(atomic_store32, KEFIR_IR_OPCODE_ATOMIC_STORE32)
            ATOMIC_STORE_OP(atomic_store64, KEFIR_IR_OPCODE_ATOMIC_STORE64)
            ATOMIC_STORE_OP(atomic_store_long_double, KEFIR_IR_OPCODE_ATOMIC_STORE_LONG_DOUBLE)
            ATOMIC_STORE_OP(atomic_store_complex_float32, KEFIR_IR_OPCODE_ATOMIC_STORE_COMPLEX_FLOAT32)
            ATOMIC_STORE_OP(atomic_store_complex_float64, KEFIR_IR_OPCODE_ATOMIC_STORE_COMPLEX_FLOAT64)
            ATOMIC_STORE_OP(atomic_store_complex_long_double, KEFIR_IR_OPCODE_ATOMIC_STORE_COMPLEX_LONG_DOUBLE)

#undef ATOMIC_STORE_OP

#define ATOMIC_CMPXCHG_OP(_id, _opcode)                                                                          \
    case _opcode: {                                                                                              \
        kefir_opt_memory_order_t model;                                                                          \
        switch (instr->arg.i64) {                                                                                \
            case KEFIR_IR_MEMORY_ORDER_SEQ_CST:                                                                  \
                model = KEFIR_OPT_MEMORY_ORDER_SEQ_CST;                                                          \
                break;                                                                                           \
                                                                                                                 \
            default:                                                                                             \
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown IR atomic model flag");                     \
        }                                                                                                        \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref4));                                    \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));                                    \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));                                    \
        REQUIRE_OK(kefir_opt_code_builder_##_id(mem, code, current_block_id, instr_ref2, instr_ref3, instr_ref4, \
                                                model, &instr_ref));                                             \
        REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));                       \
        REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));                                     \
    } break;

            ATOMIC_CMPXCHG_OP(atomic_compare_exchange8, KEFIR_IR_OPCODE_ATOMIC_CMPXCHG8)
            ATOMIC_CMPXCHG_OP(atomic_compare_exchange16, KEFIR_IR_OPCODE_ATOMIC_CMPXCHG16)
            ATOMIC_CMPXCHG_OP(atomic_compare_exchange32, KEFIR_IR_OPCODE_ATOMIC_CMPXCHG32)
            ATOMIC_CMPXCHG_OP(atomic_compare_exchange64, KEFIR_IR_OPCODE_ATOMIC_CMPXCHG64)
            ATOMIC_CMPXCHG_OP(atomic_compare_exchange_long_double, KEFIR_IR_OPCODE_ATOMIC_CMPXCHG_LONG_DOUBLE)

#undef ATOMIC_CMPXCHG_OP

#define ATOMIC_CMPXCHG_COMPLEX_OP(_id, _opcode, _enfoce_load)                                                          \
    case (_opcode): {                                                                                                  \
        kefir_opt_memory_order_t model;                                                                                \
        switch (instr->arg.i64) {                                                                                      \
            case KEFIR_IR_MEMORY_ORDER_SEQ_CST:                                                                        \
                model = KEFIR_OPT_MEMORY_ORDER_SEQ_CST;                                                                \
                break;                                                                                                 \
                                                                                                                       \
            default:                                                                                                   \
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown IR atomic model flag");                           \
        }                                                                                                              \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref4));                                          \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));                                          \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));                                          \
        if ((_enfoce_load)) {                                                                                          \
            const struct kefir_opt_instruction *expected_value_instr;                                                  \
            REQUIRE_OK(kefir_opt_code_container_instr(&state->function->code, instr_ref3, &expected_value_instr));     \
            REQUIRE(expected_value_instr->operation.opcode == KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_LONG_DOUBLE &&      \
                        expected_value_instr->operation.parameters.refs[0] == instr_ref2,                              \
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Atomic compare-exchange operation for complex long double "  \
                                                         "shall be preceeded by atomic load from the same location")); \
        }                                                                                                              \
        REQUIRE_OK(kefir_opt_code_builder_##_id(mem, code, current_block_id, instr_ref2, instr_ref3, instr_ref4,       \
                                                model, &instr_ref));                                                   \
        REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));                             \
        REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));                                           \
    } break

            ATOMIC_CMPXCHG_COMPLEX_OP(atomic_compare_exchange_complex_float32,
                                      KEFIR_IR_OPCODE_ATOMIC_CMPXCHG_COMPLEX_FLOAT32, false);
            ATOMIC_CMPXCHG_COMPLEX_OP(atomic_compare_exchange_complex_float64,
                                      KEFIR_IR_OPCODE_ATOMIC_CMPXCHG_COMPLEX_FLOAT64, false);
            ATOMIC_CMPXCHG_COMPLEX_OP(atomic_compare_exchange_complex_long_double,
                                      KEFIR_IR_OPCODE_ATOMIC_CMPXCHG_COMPLEX_LONG_DOUBLE, true);

#undef ATOMIC_CMPXCHG_COMPLEX_OP

        case KEFIR_IR_OPCODE_ATOMIC_COPY_MEMORY_FROM: {
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

        case KEFIR_IR_OPCODE_ATOMIC_COPY_MEMORY_TO: {
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

        case KEFIR_IR_OPCODE_ATOMIC_CMPXCHG_MEMORY: {
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref4));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));
            REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));
            kefir_opt_memory_order_t model;
            switch (instr->arg.u32[0]) {
                case KEFIR_IR_MEMORY_ORDER_SEQ_CST:
                    model = KEFIR_OPT_MEMORY_ORDER_SEQ_CST;
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown IR atomic model flag");
            }
            REQUIRE_OK(kefir_opt_code_builder_atomic_compare_exchange_memory(
                mem, code, current_block_id, instr_ref2, instr_ref3, instr_ref4, model, instr->arg.u32[1],
                instr->arg.u32[2], &instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
        } break;

#define LOAD_OP(_id, _extension, _opcode)                                                                         \
    case _opcode: {                                                                                               \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));                                     \
        kefir_bool_t volatile_access = (instr->arg.u64 & KEFIR_IR_MEMORY_FLAG_VOLATILE) != 0;                     \
        REQUIRE_OK(                                                                                               \
            kefir_opt_code_builder_##_id(mem, code, current_block_id, instr_ref2,                                 \
                                         &(const struct kefir_opt_memory_access_flags) {                          \
                                             .load_extension = (_extension), .volatile_access = volatile_access}, \
                                         &instr_ref));                                                            \
        REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));                        \
        REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));                                      \
    } break;

            LOAD_OP(int8_load, KEFIR_OPT_MEMORY_LOAD_NOEXTEND, KEFIR_IR_OPCODE_INT8_LOAD)
            LOAD_OP(int16_load, KEFIR_OPT_MEMORY_LOAD_NOEXTEND, KEFIR_IR_OPCODE_INT16_LOAD)
            LOAD_OP(int32_load, KEFIR_OPT_MEMORY_LOAD_NOEXTEND, KEFIR_IR_OPCODE_INT32_LOAD)
            LOAD_OP(int64_load, KEFIR_OPT_MEMORY_LOAD_NOEXTEND, KEFIR_IR_OPCODE_INT64_LOAD)
            LOAD_OP(long_double_load, KEFIR_OPT_MEMORY_LOAD_NOEXTEND, KEFIR_IR_OPCODE_LONG_DOUBLE_LOAD)
            LOAD_OP(complex_float32_load, KEFIR_OPT_MEMORY_LOAD_NOEXTEND, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_LOAD)
            LOAD_OP(complex_float64_load, KEFIR_OPT_MEMORY_LOAD_NOEXTEND, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_LOAD)
            LOAD_OP(complex_long_double_load, KEFIR_OPT_MEMORY_LOAD_NOEXTEND, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_LOAD)

#undef LOAD_OP

#define STORE_OP(_id, _opcode)                                                                               \
    case _opcode: {                                                                                          \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));                                \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));                                \
        kefir_bool_t volatile_access = (instr->arg.u64 & KEFIR_IR_MEMORY_FLAG_VOLATILE) != 0;                \
        REQUIRE_OK(kefir_opt_code_builder_##_id(                                                             \
            mem, code, current_block_id, instr_ref2, instr_ref3,                                             \
            &(const struct kefir_opt_memory_access_flags) {.load_extension = KEFIR_OPT_MEMORY_LOAD_NOEXTEND, \
                                                           .volatile_access = volatile_access},              \
            &instr_ref));                                                                                    \
        REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));                   \
    } break;

            STORE_OP(int8_store, KEFIR_IR_OPCODE_INT8_STORE)
            STORE_OP(int16_store, KEFIR_IR_OPCODE_INT16_STORE)
            STORE_OP(int32_store, KEFIR_IR_OPCODE_INT32_STORE)
            STORE_OP(int64_store, KEFIR_IR_OPCODE_INT64_STORE)
            STORE_OP(long_double_store, KEFIR_IR_OPCODE_LONG_DOUBLE_STORE)
            STORE_OP(complex_float32_store, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_STORE)
            STORE_OP(complex_float64_store, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_STORE)
            STORE_OP(complex_long_double_store, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_STORE)

#undef STORE_OP

#define OVERFLOW_ARITH(_opcode, _id)                                                                            \
    case (_opcode): {                                                                                           \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref4));                                   \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref3));                                   \
        REQUIRE_OK(kefir_opt_constructor_stack_pop(mem, state, &instr_ref2));                                   \
                                                                                                                \
        REQUIRE_OK(kefir_opt_code_builder_##_id##_overflow(mem, code, current_block_id, instr_ref2, instr_ref3, \
                                                           instr_ref4, instr->arg.u32[0], instr->arg.u32[1],    \
                                                           instr->arg.u32[2], &instr_ref));                     \
        REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));                                    \
        REQUIRE_OK(kefir_opt_code_builder_add_control(code, current_block_id, instr_ref));                      \
    } break

            OVERFLOW_ARITH(KEFIR_IR_OPCODE_ADD_OVERFLOW, add);
            OVERFLOW_ARITH(KEFIR_IR_OPCODE_SUB_OVERFLOW, sub);
            OVERFLOW_ARITH(KEFIR_IR_OPCODE_MUL_OVERFLOW, mul);

#undef OVERFLOW_ARITH

        case KEFIR_IR_OPCODE_GET_ARGUMENT:
            REQUIRE_OK(kefir_opt_code_builder_get_argument(mem, code, current_block_id, instr->arg.u64, &instr_ref));
            REQUIRE_OK(kefir_opt_constructor_stack_push(mem, state, instr_ref));
            break;

        case KEFIR_IR_OPCODE_PHI:
        case KEFIR_IR_OPCODE_ALLOC_LOCAL:
        case KEFIR_IR_OPCODE_REF_LOCAL:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR instruction opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t translate_code(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                     struct kefir_opt_constructor_state *state) {
    UNUSED(module);
    state->current_block = NULL;
    state->ir_location = 0;
    REQUIRE_OK(kefir_opt_constructor_update_current_code_block(mem, state, (kefir_size_t) 0));
    const struct kefir_irblock *ir_block = &state->function->ir_func->body;
    for (; state->ir_location < kefir_irblock_length(ir_block); state->ir_location++) {
        REQUIRE_OK(kefir_opt_constructor_update_current_code_block(mem, state, state->ir_location));

        const struct kefir_irinstr *instr = kefir_irblock_at(ir_block, state->ir_location);
        REQUIRE(instr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid IR instruction to be returned"));
        REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor(&state->function->debug_info,
                                                                             state->ir_location));
        REQUIRE_OK(translate_instruction(mem, module, &state->function->code, state, instr));
        REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor(
            &state->function->debug_info, KEFIR_OPT_CODE_DEBUG_INSTRUCTION_LOCATION_NONE));
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

        kefir_opt_instruction_ref_t existing_ref;
        kefir_result_t res =
            kefir_opt_code_container_phi_link_for(&state->function->code, phi_ref, source_block_id, &existing_ref);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            REQUIRE(existing_ref == instr_ref,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Mismatch with existing phi link for a source block"));
        } else {
            REQUIRE_OK(
                kefir_opt_code_container_phi_attach(mem, &state->function->code, phi_ref, source_block_id, instr_ref));
        }
    }
    REQUIRE(target_iter == NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to link optimizer block outputs with target block phi nodes"));
    return KEFIR_OK;
}

static kefir_result_t link_blocks_match(struct kefir_mem *mem, struct kefir_opt_constructor_state *state,
                                        const struct kefir_opt_code_block *block) {
    kefir_opt_instruction_ref_t instr_ref;
    const struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&state->function->code, block, &instr_ref));
    REQUIRE_OK(kefir_opt_code_container_instr(&state->function->code, instr_ref, &instr));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_JUMP:
            REQUIRE_OK(link_blocks_impl(mem, state, block->id, instr->operation.parameters.branch.target_block));
            break;

        case KEFIR_OPT_OPCODE_BRANCH:
        case KEFIR_OPT_OPCODE_BRANCH_COMPARE:
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
            const struct kefir_opt_inline_assembly_node *inline_asm = NULL;
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
        REQUIRE_OK(
            kefir_opt_code_container_new_phi(mem, &state->function->code, target_block_id, &phi_ref, &instr_ref));
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

    const struct kefir_opt_code_block *block = NULL;
    kefir_opt_instruction_ref_t instr_ref;
    const struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_block(&state->function->code, block_id, &block));
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&state->function->code, block, &instr_ref));
    REQUIRE_OK(kefir_opt_code_container_instr(&state->function->code, instr_ref, &instr));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_JUMP: {
            kefir_opt_block_id_t target_block = instr->operation.parameters.branch.target_block;
            REQUIRE_OK(link_blocks_equalize_stack(mem, state, block_id, target_block));
            REQUIRE_OK(link_blocks_traverse(mem, state, target_block));
        } break;

        case KEFIR_OPT_OPCODE_BRANCH:
        case KEFIR_OPT_OPCODE_BRANCH_COMPARE: {
            kefir_opt_block_id_t target_block = instr->operation.parameters.branch.target_block;
            kefir_opt_block_id_t alternative_block = instr->operation.parameters.branch.alternative_block;

            REQUIRE_OK(link_blocks_equalize_stack(mem, state, block_id, target_block));
            REQUIRE_OK(link_blocks_equalize_stack(mem, state, block_id, alternative_block));
            REQUIRE_OK(link_blocks_traverse(mem, state, target_block));
            REQUIRE_OK(link_blocks_traverse(mem, state, alternative_block));
        } break;

        case KEFIR_OPT_OPCODE_IJUMP:
        case KEFIR_OPT_OPCODE_RETURN:
        case KEFIR_OPT_OPCODE_TAIL_INVOKE:
        case KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL:
            // Intentionally left blank
            break;

        case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY: {
            const struct kefir_opt_inline_assembly_node *inline_asm = NULL;
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
