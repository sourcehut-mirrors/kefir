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

#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/function.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_FUSION_IMPL(branch)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction, kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
    void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));
    REQUIRE(callback != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen instruction fusion callback"));

    struct kefir_codegen_amd64_comparison_match_op fused_comparison_op;
    REQUIRE_OK(kefir_codegen_amd64_match_comparison_op(
        &function->function->code, instruction->operation.parameters.branch.condition_ref, &fused_comparison_op));
    switch (fused_comparison_op.type) {
        case KEFIR_CODEGEN_AMD64_COMPARISON_NONE:
            REQUIRE_OK(callback(instruction->operation.parameters.branch.condition_ref, payload));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL:
            REQUIRE_OK(callback(fused_comparison_op.refs[0], payload));
            REQUIRE_OK(callback(fused_comparison_op.refs[1], payload));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST:
            REQUIRE_OK(callback(fused_comparison_op.refs[0], payload));
            break;
    }
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(phi)(struct kefir_mem *mem,
                                                         struct kefir_codegen_amd64_function *function,
                                                         const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t vreg;
    kefir_result_t res = kefir_codegen_amd64_function_vreg_of(function, instruction->id, &vreg);
    if (res == KEFIR_NOT_FOUND) {
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_UNSPECIFIED, &vreg));
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, vreg));
    } else {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_map_phi_outputs(struct kefir_mem *mem,
                                                            struct kefir_codegen_amd64_function *function,
                                                            kefir_opt_block_id_t target_block_ref,
                                                            kefir_opt_block_id_t source_block_ref) {
    struct kefir_opt_code_block *target_block;
    REQUIRE_OK(kefir_opt_code_container_block(&function->function->code, target_block_ref, &target_block));

    kefir_result_t res;
    struct kefir_opt_phi_node *phi = NULL;
    for (res = kefir_opt_code_block_phi_head(&function->function->code, target_block, &phi);
         res == KEFIR_OK && phi != NULL; res = kefir_opt_phi_next_sibling(&function->function->code, phi, &phi)) {

        if (!function->function_analysis->instructions[phi->output_ref].reachable) {
            continue;
        }

        kefir_opt_instruction_ref_t source_ref, target_ref = phi->output_ref;
        REQUIRE_OK(kefir_opt_code_container_phi_link_for(&function->function->code, phi->node_id, source_block_ref,
                                                         &source_ref));

        kefir_asmcmp_virtual_register_index_t source_vreg_idx, target_vreg_idx;
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, source_ref, &source_vreg_idx));

        const struct kefir_asmcmp_virtual_register *source_vreg, *target_vreg;
        REQUIRE_OK(kefir_asmcmp_virtual_register_get(&function->code.context, source_vreg_idx, &source_vreg));

        res = kefir_codegen_amd64_function_vreg_of(function, target_ref, &target_vreg_idx);
        if (res == KEFIR_NOT_FOUND) {
            switch (source_vreg->type) {
                case KEFIR_ASMCMP_VIRTUAL_REGISTER_UNSPECIFIED:
                case KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE:
                case KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT:
                    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, source_vreg->type,
                                                                 &target_vreg_idx));
                    break;

                case KEFIR_ASMCMP_VIRTUAL_REGISTER_DIRECT_SPILL_SPACE:
                    REQUIRE_OK(kefir_asmcmp_virtual_register_new_direct_spill_space_allocation(
                        mem, &function->code.context, source_vreg->parameters.spill_space_allocation.length,
                        source_vreg->parameters.spill_space_allocation.alignment, &target_vreg_idx));
                    break;

                case KEFIR_ASMCMP_VIRTUAL_REGISTER_INDIRECT_SPILL_SPACE:
                    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
                        mem, &function->code.context, source_vreg->parameters.spill_space_allocation.length,
                        source_vreg->parameters.spill_space_allocation.alignment, &target_vreg_idx));
                    break;

                case KEFIR_ASMCMP_VIRTUAL_REGISTER_EXTERNAL_MEMORY:
                    REQUIRE_OK(kefir_asmcmp_virtual_register_new_memory_pointer(
                        mem, &function->code.context, source_vreg->parameters.memory.base_reg,
                        source_vreg->parameters.memory.offset, &target_vreg_idx));
                    REQUIRE_OK(kefir_codegen_amd64_stack_frame_require_frame_pointer(&function->stack_frame));
                    break;
            }
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, target_ref, target_vreg_idx));
        } else {
            REQUIRE_OK(res);
            REQUIRE_OK(kefir_asmcmp_virtual_register_get(&function->code.context, target_vreg_idx, &target_vreg));
            if (target_vreg->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_UNSPECIFIED) {
                REQUIRE_OK(kefir_asmcmp_virtual_register_specify_type(&function->code.context, target_vreg_idx,
                                                                      source_vreg->type));
            }
        }

        if (source_block_ref != target_block_ref) {
            REQUIRE_OK(kefir_asmcmp_amd64_vreg_lifetime_range_begin(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), target_vreg_idx, NULL));
        }
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             target_vreg_idx, source_vreg_idx, NULL));
    }
    REQUIRE_OK(res);

    for (res = kefir_opt_code_block_phi_head(&function->function->code, target_block, &phi);
         res == KEFIR_OK && phi != NULL; res = kefir_opt_phi_next_sibling(&function->function->code, phi, &phi)) {

        if (!function->function_analysis->instructions[phi->output_ref].reachable) {
            continue;
        }

        kefir_opt_instruction_ref_t target_ref = phi->output_ref;

        kefir_asmcmp_virtual_register_index_t target_vreg_idx;
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, target_ref, &target_vreg_idx));

        REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), target_vreg_idx, NULL));
        if (target_block_ref != source_block_ref) {
            REQUIRE_OK(kefir_asmcmp_amd64_vreg_lifetime_range_end(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), target_vreg_idx, NULL));
        }
    }
    REQUIRE_OK(res);

    return KEFIR_OK;
}

static kefir_result_t has_phi_outputs(struct kefir_codegen_amd64_function *function,
                                      struct kefir_opt_code_block *target_block, kefir_bool_t *has_outputs) {
    struct kefir_opt_phi_node *phi = NULL;
    kefir_result_t res;
    *has_outputs = false;
    for (res = kefir_opt_code_block_phi_head(&function->function->code, target_block, &phi);
         res == KEFIR_OK && phi != NULL; res = kefir_opt_phi_next_sibling(&function->function->code, phi, &phi)) {

        if (!function->function_analysis->instructions[phi->output_ref].reachable) {
            continue;
        }

        *has_outputs = true;
        break;
    }
    REQUIRE_OK(res);
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(jump)(struct kefir_mem *mem,
                                                          struct kefir_codegen_amd64_function *function,
                                                          const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    struct kefir_opt_code_block *target_block, *source_block;
    REQUIRE_OK(kefir_opt_code_container_block(&function->function->code,
                                              instruction->operation.parameters.branch.target_block, &target_block));
    REQUIRE_OK(kefir_opt_code_container_block(&function->function->code, instruction->block_id, &source_block));

    REQUIRE_OK(kefir_codegen_amd64_function_map_phi_outputs(mem, function, target_block->id, instruction->block_id));

    if (function->function_analysis->blocks[target_block->id].linear_position !=
        function->function_analysis->blocks[source_block->id].linear_position + 1) {
        struct kefir_hashtree_node *target_label_node;
        REQUIRE_OK(kefir_hashtree_at(&function->labels, (kefir_hashtree_key_t) target_block->id, &target_label_node));
        ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, target_label, target_label_node->value);

        REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(target_label), NULL));
    }

    return KEFIR_OK;
}

static kefir_result_t fused_comparison_match_impl(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction,
    const struct kefir_codegen_amd64_comparison_match_op *fused_comparison_op) {
    struct kefir_opt_code_block *target_block, *alternative_block, *source_block;
    REQUIRE_OK(kefir_opt_code_container_block(&function->function->code,
                                              instruction->operation.parameters.branch.target_block, &target_block));
    REQUIRE_OK(kefir_opt_code_container_block(
        &function->function->code, instruction->operation.parameters.branch.alternative_block, &alternative_block));
    REQUIRE_OK(kefir_opt_code_container_block(&function->function->code, instruction->block_id, &source_block));

    kefir_asmcmp_virtual_register_index_t arg1_vreg_idx, arg2_vreg_idx;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, fused_comparison_op->refs[0], &arg1_vreg_idx));

    switch (fused_comparison_op->type) {
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL:
            REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, fused_comparison_op->refs[1], &arg2_vreg_idx));
            REQUIRE_OK(kefir_asmcmp_amd64_cmp(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg_idx), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg_idx), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL_CONST:
            if (fused_comparison_op->int_value >= KEFIR_INT32_MIN &&
                fused_comparison_op->int_value <= KEFIR_INT32_MAX) {
                REQUIRE_OK(kefir_asmcmp_amd64_cmp(mem, &function->code,
                                                  kefir_asmcmp_context_instr_tail(&function->code.context),
                                                  &KEFIR_ASMCMP_MAKE_VREG64(arg1_vreg_idx),
                                                  &KEFIR_ASMCMP_MAKE_INT(fused_comparison_op->int_value), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_virtual_register_new(
                    mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg2_vreg_idx));
                REQUIRE_OK(kefir_asmcmp_amd64_movabs(mem, &function->code,
                                                     kefir_asmcmp_context_instr_tail(&function->code.context),
                                                     &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg_idx),
                                                     &KEFIR_ASMCMP_MAKE_INT(fused_comparison_op->int_value), NULL));
                REQUIRE_OK(kefir_asmcmp_amd64_cmp(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg_idx), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg_idx), NULL));
            }
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL:
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, fused_comparison_op->refs[1], &arg2_vreg_idx));
            REQUIRE_OK(kefir_asmcmp_amd64_ucomiss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg_idx), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg_idx), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL:
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, fused_comparison_op->refs[1], &arg2_vreg_idx));
            REQUIRE_OK(kefir_asmcmp_amd64_ucomisd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg_idx), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg_idx), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST: {
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            kefir_asmcmp_label_index_t label;
            REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label));

            REQUIRE_OK(kefir_hashtree_insert(mem, &function->constants, (kefir_hashtree_key_t) label,
                                             (kefir_hashtree_value_t) fused_comparison_op->refs[1]));

            REQUIRE_OK(kefir_asmcmp_amd64_ucomiss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg_idx),
                &KEFIR_ASMCMP_MAKE_INDIRECT_INTERNAL_LABEL(label, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
        } break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST: {
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            kefir_asmcmp_label_index_t label;
            REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label));

            REQUIRE_OK(kefir_hashtree_insert(mem, &function->constants, (kefir_hashtree_key_t) label,
                                             (kefir_hashtree_value_t) fused_comparison_op->refs[1]));

            REQUIRE_OK(kefir_asmcmp_amd64_ucomisd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg_idx),
                &KEFIR_ASMCMP_MAKE_INDIRECT_INTERNAL_LABEL(label, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
        } break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_NONE:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected fused branch type");
    }

    struct kefir_hashtree_node *label_node;

    kefir_bool_t alternative_phi_outputs;
    kefir_asmcmp_label_index_t branch_label_idx;
    REQUIRE_OK(has_phi_outputs(function, alternative_block, &alternative_phi_outputs));
    if (alternative_phi_outputs) {
        REQUIRE_OK(
            kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &branch_label_idx));
    } else {
        REQUIRE_OK(kefir_hashtree_at(&function->labels, (kefir_hashtree_key_t) alternative_block->id, &label_node));
        ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, target_label, label_node->value);
        branch_label_idx = target_label;
    }

    switch (fused_comparison_op->type) {
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_jne(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_NOT_EQUAL_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_je(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_jle(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_jl(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_jge(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_jg(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_jbe(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_jb(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_jae(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_ja(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_jp(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_jne(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL_CONST: {
            kefir_asmcmp_label_index_t p_label;
            REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &p_label));

            REQUIRE_OK(kefir_asmcmp_amd64_jp(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(p_label), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_je(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));

            REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, p_label));
        } break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_jbe(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_jb(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_jae(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_ja(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_NONE:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected fused branch type");
    }

    REQUIRE_OK(kefir_codegen_amd64_function_map_phi_outputs(mem, function, target_block->id, instruction->block_id));

    if (alternative_phi_outputs || function->function_analysis->blocks[target_block->id].linear_position !=
                                       function->function_analysis->blocks[source_block->id].linear_position + 1) {
        REQUIRE_OK(kefir_hashtree_at(&function->labels, (kefir_hashtree_key_t) target_block->id, &label_node));
        ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, target_label, label_node->value);
        REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(target_label), NULL));
    }

    if (alternative_phi_outputs) {
        REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, branch_label_idx));
        REQUIRE_OK(
            kefir_codegen_amd64_function_map_phi_outputs(mem, function, alternative_block->id, instruction->block_id));

        if (function->function_analysis->blocks[alternative_block->id].linear_position !=
            function->function_analysis->blocks[source_block->id].linear_position + 1) {
            REQUIRE_OK(kefir_hashtree_at(&function->labels, (kefir_hashtree_key_t) alternative_block->id, &label_node));
            ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, target_label, label_node->value);
            REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(target_label), NULL));
        }
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(branch)(struct kefir_mem *mem,
                                                            struct kefir_codegen_amd64_function *function,
                                                            const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    struct kefir_codegen_amd64_comparison_match_op fused_comparison_op;
    REQUIRE_OK(kefir_codegen_amd64_match_comparison_op(
        &function->function->code, instruction->operation.parameters.branch.condition_ref, &fused_comparison_op));
    if (fused_comparison_op.type != KEFIR_CODEGEN_AMD64_COMPARISON_NONE) {
        REQUIRE_OK(fused_comparison_match_impl(mem, function, instruction, &fused_comparison_op));
        return KEFIR_OK;
    }

    struct kefir_opt_code_block *target_block, *alternative_block, *source_block;
    REQUIRE_OK(kefir_opt_code_container_block(&function->function->code,
                                              instruction->operation.parameters.branch.target_block, &target_block));
    REQUIRE_OK(kefir_opt_code_container_block(
        &function->function->code, instruction->operation.parameters.branch.alternative_block, &alternative_block));
    REQUIRE_OK(kefir_opt_code_container_block(&function->function->code, instruction->block_id, &source_block));

    kefir_asmcmp_virtual_register_index_t condition_vreg_idx;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.branch.condition_ref,
                                                    &condition_vreg_idx));

    REQUIRE_OK(kefir_asmcmp_amd64_test(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(condition_vreg_idx),
                                       &KEFIR_ASMCMP_MAKE_VREG(condition_vreg_idx), NULL));

    struct kefir_hashtree_node *label_node;
    kefir_bool_t alternative_phi_outputs;
    kefir_asmcmp_label_index_t branch_label_idx;
    REQUIRE_OK(has_phi_outputs(function, alternative_block, &alternative_phi_outputs));
    if (alternative_phi_outputs) {
        REQUIRE_OK(
            kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &branch_label_idx));
        REQUIRE_OK(kefir_asmcmp_amd64_jz(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
    } else {
        REQUIRE_OK(kefir_hashtree_at(&function->labels, (kefir_hashtree_key_t) alternative_block->id, &label_node));
        ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, target_label, label_node->value);
        REQUIRE_OK(kefir_asmcmp_amd64_jz(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(target_label), NULL));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_map_phi_outputs(mem, function, target_block->id, instruction->block_id));

    if (alternative_phi_outputs || function->function_analysis->blocks[target_block->id].linear_position !=
                                       function->function_analysis->blocks[source_block->id].linear_position + 1) {
        REQUIRE_OK(kefir_hashtree_at(&function->labels, (kefir_hashtree_key_t) target_block->id, &label_node));
        ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, target_label, label_node->value);
        REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(target_label), NULL));
    }

    if (alternative_phi_outputs) {
        REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, branch_label_idx));
        REQUIRE_OK(
            kefir_codegen_amd64_function_map_phi_outputs(mem, function, alternative_block->id, instruction->block_id));

        if (function->function_analysis->blocks[alternative_block->id].linear_position !=
            function->function_analysis->blocks[source_block->id].linear_position + 1) {
            REQUIRE_OK(kefir_hashtree_at(&function->labels, (kefir_hashtree_key_t) alternative_block->id, &label_node));
            ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, target_label, label_node->value);
            REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(target_label), NULL));
        }
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(ijump)(struct kefir_mem *mem,
                                                           struct kefir_codegen_amd64_function *function,
                                                           const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t target_vreg_idx;
    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &target_vreg_idx));

    REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(target_vreg_idx), NULL));

    return KEFIR_OK;
}
