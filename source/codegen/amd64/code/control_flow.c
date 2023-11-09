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

#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/function.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

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

static kefir_result_t map_phi_outputs(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                      struct kefir_opt_code_block *target_block,
                                      const struct kefir_opt_instruction *instruction) {
    kefir_result_t res;
    struct kefir_opt_phi_node *phi = NULL;
    for (res = kefir_opt_code_block_phi_head(&function->function->code, target_block, &phi);
         res == KEFIR_OK && phi != NULL; res = kefir_opt_phi_next_sibling(&function->function->code, phi, &phi)) {

        if (!function->function_analysis->instructions[phi->output_ref].reachable) {
            continue;
        }

        kefir_opt_instruction_ref_t source_ref, target_ref = phi->output_ref;
        REQUIRE_OK(kefir_opt_code_container_phi_link_for(&function->function->code, phi->node_id, instruction->block_id,
                                                         &source_ref));

        kefir_asmcmp_virtual_register_index_t source_vreg_idx, target_vreg_idx;
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, source_ref, &source_vreg_idx));

        const struct kefir_asmcmp_virtual_register *source_vreg, *target_vreg;
        REQUIRE_OK(kefir_asmcmp_virtual_register_get(&function->code.context, source_vreg_idx, &source_vreg));

        res = kefir_codegen_amd64_function_vreg_of(function, target_ref, &target_vreg_idx);
        if (res == KEFIR_NOT_FOUND) {
            REQUIRE_OK(
                kefir_asmcmp_virtual_register_new(mem, &function->code.context, source_vreg->type, &target_vreg_idx));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, target_ref, target_vreg_idx));
        } else {
            REQUIRE_OK(res);
            REQUIRE_OK(kefir_asmcmp_virtual_register_get(&function->code.context, target_vreg_idx, &target_vreg));
            if (target_vreg->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_UNSPECIFIED) {
                REQUIRE_OK(kefir_asmcmp_virtual_register_specify_type(&function->code.context, target_vreg_idx,
                                                                      source_vreg->type));
            }
        }

        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             target_vreg_idx, source_vreg_idx, NULL));
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

    REQUIRE_OK(map_phi_outputs(mem, function, target_block, instruction));

    if (function->function_analysis->blocks[target_block->id].linear_position !=
        function->function_analysis->blocks[source_block->id].linear_position + 1) {
        struct kefir_hashtree_node *target_label_node;
        REQUIRE_OK(kefir_hashtree_at(&function->labels, (kefir_hashtree_key_t) target_block->id, &target_label_node));
        ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, target_label, target_label_node->value);

        const char *label_symbol;
        REQUIRE_OK(kefir_codegen_amd64_function_format_label(mem, function, target_label, &label_symbol));

        REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_LABEL(label_symbol, 0), NULL));
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(branch)(struct kefir_mem *mem,
                                                            struct kefir_codegen_amd64_function *function,
                                                            const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

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
    const char *label;
    kefir_bool_t alternative_phi_outputs;
    kefir_asmcmp_label_index_t branch_label_idx;
    REQUIRE_OK(has_phi_outputs(function, alternative_block, &alternative_phi_outputs));
    if (alternative_phi_outputs) {
        REQUIRE_OK(
            kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &branch_label_idx));
        REQUIRE_OK(kefir_codegen_amd64_function_format_label(mem, function, branch_label_idx, &label));
        REQUIRE_OK(kefir_asmcmp_amd64_jz(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_LABEL(label, 0), NULL));
    } else {
        REQUIRE_OK(kefir_hashtree_at(&function->labels, (kefir_hashtree_key_t) alternative_block->id, &label_node));
        ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, target_label, label_node->value);
        REQUIRE_OK(kefir_codegen_amd64_function_format_label(mem, function, target_label, &label));
        REQUIRE_OK(kefir_asmcmp_amd64_jz(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_LABEL(label, 0), NULL));
    }

    REQUIRE_OK(map_phi_outputs(mem, function, target_block, instruction));

    if (alternative_phi_outputs || function->function_analysis->blocks[target_block->id].linear_position !=
                                       function->function_analysis->blocks[source_block->id].linear_position + 1) {
        REQUIRE_OK(kefir_hashtree_at(&function->labels, (kefir_hashtree_key_t) target_block->id, &label_node));
        ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, target_label, label_node->value);
        REQUIRE_OK(kefir_codegen_amd64_function_format_label(mem, function, target_label, &label));
        REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_LABEL(label, 0), NULL));
    }

    if (alternative_phi_outputs) {
        REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, branch_label_idx));
        REQUIRE_OK(map_phi_outputs(mem, function, alternative_block, instruction));

        if (function->function_analysis->blocks[alternative_block->id].linear_position !=
            function->function_analysis->blocks[source_block->id].linear_position + 1) {
            REQUIRE_OK(kefir_hashtree_at(&function->labels, (kefir_hashtree_key_t) alternative_block->id, &label_node));
            ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, target_label, label_node->value);
            REQUIRE_OK(kefir_codegen_amd64_function_format_label(mem, function, target_label, &label));
            REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_LABEL(label, 0), NULL));
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
