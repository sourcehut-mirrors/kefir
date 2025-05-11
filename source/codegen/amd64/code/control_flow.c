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

static kefir_result_t new_virtual_register_of_type(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                                   kefir_asmcmp_virtual_register_index_t vreg_idx,
                                                   kefir_asmcmp_virtual_register_index_t *new_vreg_idx) {
    const struct kefir_asmcmp_virtual_register *vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_get(&function->code.context, vreg_idx, &vreg));

    switch (vreg->type) {
        case KEFIR_ASMCMP_VIRTUAL_REGISTER_UNSPECIFIED:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, vreg->type, new_vreg_idx));
            REQUIRE_OK(kefir_asmcmp_virtual_register_specify_type_dependent(mem, &function->code.context, *new_vreg_idx,
                                                                            vreg_idx));
            break;

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE:
        case KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, vreg->type, new_vreg_idx));
            break;

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_SPILL_SPACE:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(
                mem, &function->code.context, vreg->parameters.spill_space_allocation.length,
                vreg->parameters.spill_space_allocation.alignment, new_vreg_idx));
            break;

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_LOCAL_VARIABLE:
        case KEFIR_ASMCMP_VIRTUAL_REGISTER_IMMEDIATE_INTEGER:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, new_vreg_idx));
            break;

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_EXTERNAL_MEMORY:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, new_vreg_idx));
            break;

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR: {
            kefir_asmcmp_virtual_register_index_t first, second;
            REQUIRE_OK(new_virtual_register_of_type(mem, function, vreg->parameters.pair.virtual_registers[0], &first));
            REQUIRE_OK(
                new_virtual_register_of_type(mem, function, vreg->parameters.pair.virtual_registers[1], &second));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context, vreg->parameters.pair.type,
                                                              first, second, new_vreg_idx));
        } break;
    }

    return KEFIR_OK;
}

static kefir_result_t map_phi_outputs_impl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                           kefir_opt_block_id_t target_block_ref, kefir_opt_block_id_t source_block_ref,
                                           struct kefir_hashtreeset *transferred_vregs,
                                           struct kefir_hashtreeset *used_target_vregs,
                                           struct kefir_hashtreeset *used_source_vregs,
                                           struct kefir_hashtree *deferred_target_vregs) {
    const struct kefir_opt_code_block *source_block;
    REQUIRE_OK(kefir_opt_code_container_block(&function->function->code, source_block_ref, &source_block));
    const struct kefir_opt_code_block *target_block;
    REQUIRE_OK(kefir_opt_code_container_block(&function->function->code, target_block_ref, &target_block));

    kefir_result_t res;
    kefir_opt_phi_id_t phi_ref;
    kefir_opt_instruction_ref_t source_ref, target_ref;
    kefir_asmcmp_virtual_register_index_t source_vreg_idx, target_vreg_idx, deferred_target_vreg_idx;
    const struct kefir_opt_phi_node *phi = NULL, *source_phi = NULL;
    for (res = kefir_opt_code_block_phi_head(&function->function->code, target_block, &phi_ref);
         res == KEFIR_OK && phi_ref != KEFIR_ID_NONE;
         res = kefir_opt_phi_next_sibling(&function->function->code, phi_ref, &phi_ref)) {

        REQUIRE_OK(kefir_opt_code_container_phi(&function->function->code, phi_ref, &phi));
        if (!kefir_opt_code_schedule_has(&function->schedule, phi->output_ref) ||
            !kefir_hashtreeset_has(&function->translated_instructions, (kefir_hashtreeset_entry_t) phi->output_ref)) {
            continue;
        }

        REQUIRE_OK(kefir_opt_code_container_phi_link_for(&function->function->code, phi->node_id, source_block_ref,
                                                         &source_ref));
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, source_ref, &source_vreg_idx));
        kefir_result_t res = kefir_codegen_amd64_function_vreg_of(function, phi->output_ref, &target_vreg_idx);
        if (res == KEFIR_NOT_FOUND) {
            REQUIRE_OK(kefir_hashtreeset_add(mem, used_source_vregs, (kefir_hashtreeset_entry_t) source_vreg_idx));
        } else {
            REQUIRE_OK(res);
            if (source_vreg_idx != target_vreg_idx) {
                REQUIRE_OK(kefir_hashtreeset_add(mem, used_source_vregs, (kefir_hashtreeset_entry_t) source_vreg_idx));
            }
        }
    }

    for (res = kefir_opt_code_block_phi_head(&function->function->code, target_block, &phi_ref);
         res == KEFIR_OK && phi_ref != KEFIR_ID_NONE;
         res = kefir_opt_phi_next_sibling(&function->function->code, phi_ref, &phi_ref)) {

        REQUIRE_OK(kefir_opt_code_container_phi(&function->function->code, phi_ref, &phi));
        if (!kefir_opt_code_schedule_has(&function->schedule, phi->output_ref) ||
            !kefir_hashtreeset_has(&function->translated_instructions, (kefir_hashtreeset_entry_t) phi->output_ref)) {
            continue;
        }

        target_ref = phi->output_ref;
        REQUIRE_OK(kefir_opt_code_container_phi_link_for(&function->function->code, phi->node_id, source_block_ref,
                                                         &source_ref));

        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, source_ref, &source_vreg_idx));

        const struct kefir_asmcmp_virtual_register *source_vreg, *target_vreg;
        REQUIRE_OK(kefir_asmcmp_virtual_register_get(&function->code.context, source_vreg_idx, &source_vreg));

        res = kefir_codegen_amd64_function_vreg_of(function, target_ref, &target_vreg_idx);

        if (res == KEFIR_OK && kefir_hashtreeset_has(used_source_vregs, (kefir_hashtreeset_entry_t) target_vreg_idx)) {
            deferred_target_vreg_idx = target_vreg_idx;
        } else {
            deferred_target_vreg_idx = KEFIR_ASMCMP_INDEX_NONE;
        }
        if (res == KEFIR_NOT_FOUND || deferred_target_vreg_idx != KEFIR_ASMCMP_INDEX_NONE) {
            if (deferred_target_vreg_idx == KEFIR_ASMCMP_INDEX_NONE && phi->number_of_links == 1 &&
                !kefir_hashtreeset_has(transferred_vregs, (kefir_hashtreeset_entry_t) source_vreg_idx)) {
                const struct kefir_opt_instruction *source_instr;
                REQUIRE_OK(kefir_opt_code_container_instr(&function->function->code, source_ref, &source_instr));
                if (source_instr->operation.opcode == KEFIR_OPT_OPCODE_PHI) {
                    REQUIRE_OK(kefir_opt_code_container_phi(&function->function->code,
                                                            source_instr->operation.parameters.phi_ref, &source_phi));
                }

                if (source_phi == NULL || source_phi->number_of_links == 1) {
                    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, target_ref, source_vreg_idx));
                    REQUIRE_OK(
                        kefir_hashtreeset_add(mem, transferred_vregs, (kefir_hashtreeset_entry_t) source_vreg_idx));
                    continue;
                }
            }

            REQUIRE_OK(new_virtual_register_of_type(mem, function, source_vreg_idx, &target_vreg_idx));

            if (deferred_target_vreg_idx != KEFIR_ASMCMP_INDEX_NONE) {
                REQUIRE_OK(kefir_hashtree_insert(mem, deferred_target_vregs,
                                                 (kefir_hashtree_key_t) deferred_target_vreg_idx,
                                                 (kefir_hashtree_value_t) target_vreg_idx));
            } else {
                REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, target_ref, target_vreg_idx));
            }
        } else {
            REQUIRE_OK(res);
            REQUIRE_OK(kefir_asmcmp_virtual_register_get(&function->code.context, target_vreg_idx, &target_vreg));
            if (target_vreg->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_UNSPECIFIED) {
                REQUIRE_OK(kefir_asmcmp_virtual_register_specify_type_dependent(mem, &function->code.context,
                                                                                target_vreg_idx, source_vreg_idx));
            }
        }

        if (deferred_target_vreg_idx != KEFIR_ASMCMP_INDEX_NONE ||
            (source_block_ref != target_block_ref && source_vreg_idx != target_vreg_idx &&
             !kefir_bucketset_has(&function->function_analysis.liveness.blocks[source_block_ref].alive_instr,
                                  (kefir_bucketset_entry_t) target_ref) &&
             !kefir_hashtreeset_has(used_source_vregs, (kefir_hashtreeset_entry_t) target_vreg_idx))) {
            REQUIRE_OK(kefir_hashtreeset_add(mem, used_target_vregs, (kefir_hashtreeset_entry_t) target_vreg_idx));
        }
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             target_vreg_idx, source_vreg_idx, NULL));
    }
    REQUIRE_OK(res);

    struct kefir_hashtree_node_iterator deferred_iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(deferred_target_vregs, &deferred_iter); node != NULL;
         node = kefir_hashtree_next(&deferred_iter)) {
        ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, deferred_target_vreg, node->key);
        ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, temporary_target_vreg, node->value);
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             deferred_target_vreg, temporary_target_vreg, NULL));
    }

    for (res = kefir_opt_code_block_phi_head(&function->function->code, target_block, &phi_ref);
         res == KEFIR_OK && phi_ref != KEFIR_ID_NONE;
         res = kefir_opt_phi_next_sibling(&function->function->code, phi_ref, &phi_ref)) {

        REQUIRE_OK(kefir_opt_code_container_phi(&function->function->code, phi_ref, &phi));
        if (!kefir_opt_code_schedule_has(&function->schedule, phi->output_ref) ||
            !kefir_hashtreeset_has(&function->translated_instructions, (kefir_hashtreeset_entry_t) phi->output_ref)) {
            continue;
        }

        kefir_opt_instruction_ref_t target_ref = phi->output_ref;

        kefir_asmcmp_virtual_register_index_t target_vreg_idx;
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, target_ref, &target_vreg_idx));

        REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), target_vreg_idx, NULL));
    }
    REQUIRE_OK(res);

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_map_phi_outputs(struct kefir_mem *mem,
                                                            struct kefir_codegen_amd64_function *function,
                                                            kefir_opt_block_id_t target_block_ref,
                                                            kefir_opt_block_id_t source_block_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(target_block_ref != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target block reference"));
    REQUIRE(source_block_ref != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source block reference"));

    struct kefir_hashtreeset transferred_vregs;
    struct kefir_hashtreeset used_target_vregs;
    struct kefir_hashtreeset used_source_vregs;
    struct kefir_hashtree deferred_target_vregs;
    REQUIRE_OK(kefir_hashtreeset_init(&transferred_vregs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&used_target_vregs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&used_source_vregs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&deferred_target_vregs, &kefir_hashtree_uint_ops));
    kefir_result_t res = map_phi_outputs_impl(mem, function, target_block_ref, source_block_ref, &transferred_vregs,
                                              &used_target_vregs, &used_source_vregs, &deferred_target_vregs);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &deferred_target_vregs);
        kefir_hashtreeset_free(mem, &used_source_vregs);
        kefir_hashtreeset_free(mem, &used_target_vregs);
        kefir_hashtreeset_free(mem, &transferred_vregs);
        return res;
    });
    res = kefir_hashtree_free(mem, &deferred_target_vregs);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &used_source_vregs);
        kefir_hashtreeset_free(mem, &used_target_vregs);
        kefir_hashtreeset_free(mem, &transferred_vregs);
        return res;
    });
    res = kefir_hashtreeset_free(mem, &used_source_vregs);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &used_target_vregs);
        kefir_hashtreeset_free(mem, &transferred_vregs);
        return res;
    });
    res = kefir_hashtreeset_free(mem, &used_target_vregs);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &transferred_vregs);
        return res;
    });
    REQUIRE_OK(kefir_hashtreeset_free(mem, &transferred_vregs));
    return KEFIR_OK;
}

static kefir_result_t has_phi_outputs(struct kefir_codegen_amd64_function *function,
                                      const struct kefir_opt_code_block *target_block, kefir_bool_t *has_outputs) {
    kefir_opt_phi_id_t phi_ref;
    const struct kefir_opt_phi_node *phi = NULL;
    kefir_result_t res;
    *has_outputs = false;
    for (res = kefir_opt_code_block_phi_head(&function->function->code, target_block, &phi_ref);
         res == KEFIR_OK && phi_ref != KEFIR_ID_NONE;
         res = kefir_opt_phi_next_sibling(&function->function->code, phi_ref, &phi_ref)) {

        REQUIRE_OK(kefir_opt_code_container_phi(&function->function->code, phi_ref, &phi));
        if (!kefir_opt_code_schedule_has(&function->schedule, phi->output_ref) ||
            !kefir_hashtreeset_has(&function->translated_instructions, (kefir_hashtreeset_entry_t) phi->output_ref)) {
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

    REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));

    const struct kefir_opt_code_block *target_block, *source_block;
    REQUIRE_OK(kefir_opt_code_container_block(&function->function->code,
                                              instruction->operation.parameters.branch.target_block, &target_block));
    REQUIRE_OK(kefir_opt_code_container_block(&function->function->code, instruction->block_id, &source_block));

    REQUIRE_OK(kefir_codegen_amd64_function_map_phi_outputs(mem, function, target_block->id, instruction->block_id));

    const struct kefir_opt_code_block_schedule *target_block_schedule, *source_block_schedule;
    REQUIRE_OK(kefir_opt_code_schedule_of_block(&function->schedule, target_block->id, &target_block_schedule));
    REQUIRE_OK(kefir_opt_code_schedule_of_block(&function->schedule, source_block->id, &source_block_schedule));
    if (target_block_schedule->linear_index != source_block_schedule->linear_index + 1) {
        struct kefir_hashtree_node *target_label_node;
        REQUIRE_OK(kefir_hashtree_at(&function->labels, (kefir_hashtree_key_t) target_block->id, &target_label_node));
        ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, target_label, target_label_node->value);

        REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(target_label), NULL));
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(branch_compare)(struct kefir_mem *mem,
                                                                    struct kefir_codegen_amd64_function *function,
                                                                    const struct kefir_opt_instruction *instruction) {
    REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));

    const struct kefir_opt_code_block *target_block, *alternative_block, *source_block;
    REQUIRE_OK(kefir_opt_code_container_block(&function->function->code,
                                              instruction->operation.parameters.branch.target_block, &target_block));
    REQUIRE_OK(kefir_opt_code_container_block(
        &function->function->code, instruction->operation.parameters.branch.alternative_block, &alternative_block));
    REQUIRE_OK(kefir_opt_code_container_block(&function->function->code, instruction->block_id, &source_block));

    kefir_asmcmp_virtual_register_index_t arg1_vreg_idx, arg2_vreg_idx;
    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg_idx));

    switch (instruction->operation.parameters.branch.comparison.operation) {
#define OP(_variant)                                                                                         \
    do {                                                                                                     \
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], \
                                                        &arg2_vreg_idx));                                    \
        REQUIRE_OK(kefir_asmcmp_amd64_cmp(mem, &function->code,                                              \
                                          kefir_asmcmp_context_instr_tail(&function->code.context),          \
                                          &KEFIR_ASMCMP_MAKE_VREG##_variant(arg1_vreg_idx),                  \
                                          &KEFIR_ASMCMP_MAKE_VREG##_variant(arg2_vreg_idx), NULL));          \
    } while (0)

        case KEFIR_OPT_COMPARISON_INT8_EQUALS:
        case KEFIR_OPT_COMPARISON_INT8_NOT_EQUALS:
        case KEFIR_OPT_COMPARISON_INT8_GREATER:
        case KEFIR_OPT_COMPARISON_INT8_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT8_LESSER:
        case KEFIR_OPT_COMPARISON_INT8_LESSER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT8_ABOVE:
        case KEFIR_OPT_COMPARISON_INT8_ABOVE_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT8_BELOW:
        case KEFIR_OPT_COMPARISON_INT8_BELOW_OR_EQUALS:
            OP(8);
            break;

        case KEFIR_OPT_COMPARISON_INT16_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_NOT_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_GREATER:
        case KEFIR_OPT_COMPARISON_INT16_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_LESSER:
        case KEFIR_OPT_COMPARISON_INT16_LESSER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_ABOVE:
        case KEFIR_OPT_COMPARISON_INT16_ABOVE_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_BELOW:
        case KEFIR_OPT_COMPARISON_INT16_BELOW_OR_EQUALS:
            OP(16);
            break;

        case KEFIR_OPT_COMPARISON_INT32_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_NOT_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_GREATER:
        case KEFIR_OPT_COMPARISON_INT32_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_LESSER:
        case KEFIR_OPT_COMPARISON_INT32_LESSER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_ABOVE:
        case KEFIR_OPT_COMPARISON_INT32_ABOVE_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_BELOW:
        case KEFIR_OPT_COMPARISON_INT32_BELOW_OR_EQUALS:
            OP(32);
            break;

        case KEFIR_OPT_COMPARISON_INT64_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_NOT_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_GREATER:
        case KEFIR_OPT_COMPARISON_INT64_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_LESSER:
        case KEFIR_OPT_COMPARISON_INT64_LESSER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_ABOVE:
        case KEFIR_OPT_COMPARISON_INT64_ABOVE_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_BELOW:
        case KEFIR_OPT_COMPARISON_INT64_BELOW_OR_EQUALS:
            OP(64);
            break;
#undef OP
        case KEFIR_OPT_COMPARISON_FLOAT32_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT32_GREATER:
        case KEFIR_OPT_COMPARISON_FLOAT32_GREATER_OR_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT32_LESSER:
        case KEFIR_OPT_COMPARISON_FLOAT32_LESSER_OR_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER:
        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER_OR_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER:
        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER_OR_EQUAL: {
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

            const struct kefir_opt_instruction *arg2_instr;
            REQUIRE_OK(kefir_opt_code_container_instr(&function->function->code,
                                                      instruction->operation.parameters.refs[1], &arg2_instr));
            if (arg2_instr->operation.opcode != KEFIR_OPT_OPCODE_FLOAT32_CONST) {
                REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1],
                                                                &arg2_vreg_idx));
                REQUIRE_OK(kefir_asmcmp_amd64_ucomiss(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg_idx), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg_idx), NULL));
            } else {
                kefir_asmcmp_label_index_t label;
                REQUIRE_OK(
                    kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label));

                REQUIRE_OK(kefir_hashtree_insert(mem, &function->constants, (kefir_hashtree_key_t) label,
                                                 (kefir_hashtree_value_t) arg2_instr->id));

                if (function->codegen->config->position_independent_code) {
                    REQUIRE_OK(kefir_asmcmp_amd64_ucomiss(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg_idx),
                        &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_INTERNAL(label, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
                } else {
                    REQUIRE_OK(kefir_asmcmp_amd64_ucomiss(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg_idx),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_INTERNAL_LABEL(label, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
                }
            }
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT64_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT64_GREATER:
        case KEFIR_OPT_COMPARISON_FLOAT64_GREATER_OR_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT64_LESSER:
        case KEFIR_OPT_COMPARISON_FLOAT64_LESSER_OR_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER:
        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER_OR_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER:
        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER_OR_EQUAL:
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            const struct kefir_opt_instruction *arg2_instr;
            REQUIRE_OK(kefir_opt_code_container_instr(&function->function->code,
                                                      instruction->operation.parameters.refs[1], &arg2_instr));
            if (arg2_instr->operation.opcode != KEFIR_OPT_OPCODE_FLOAT64_CONST) {
                REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1],
                                                                &arg2_vreg_idx));
                REQUIRE_OK(kefir_asmcmp_amd64_ucomisd(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg_idx), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg_idx), NULL));
            } else {
                kefir_asmcmp_label_index_t label;
                REQUIRE_OK(
                    kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label));

                REQUIRE_OK(kefir_hashtree_insert(mem, &function->constants, (kefir_hashtree_key_t) label,
                                                 (kefir_hashtree_value_t) arg2_instr->id));

                if (function->codegen->config->position_independent_code) {
                    REQUIRE_OK(kefir_asmcmp_amd64_ucomisd(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg_idx),
                        &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_INTERNAL(label, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
                } else {
                    REQUIRE_OK(kefir_asmcmp_amd64_ucomisd(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg_idx),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_INTERNAL_LABEL(label, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
                }
            }
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer comparison operation");
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

    switch (instruction->operation.parameters.branch.comparison.operation) {
        case KEFIR_OPT_COMPARISON_INT8_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_EQUALS:
            REQUIRE_OK(kefir_asmcmp_amd64_jne(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT8_NOT_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_NOT_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_NOT_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_NOT_EQUALS:
            REQUIRE_OK(kefir_asmcmp_amd64_je(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT8_GREATER:
        case KEFIR_OPT_COMPARISON_INT16_GREATER:
        case KEFIR_OPT_COMPARISON_INT32_GREATER:
        case KEFIR_OPT_COMPARISON_INT64_GREATER:
            REQUIRE_OK(kefir_asmcmp_amd64_jle(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT8_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_GREATER_OR_EQUALS:
            REQUIRE_OK(kefir_asmcmp_amd64_jl(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT8_LESSER:
        case KEFIR_OPT_COMPARISON_INT16_LESSER:
        case KEFIR_OPT_COMPARISON_INT32_LESSER:
        case KEFIR_OPT_COMPARISON_INT64_LESSER:
            REQUIRE_OK(kefir_asmcmp_amd64_jge(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT8_LESSER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_LESSER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_LESSER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_LESSER_OR_EQUALS:
            REQUIRE_OK(kefir_asmcmp_amd64_jg(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT8_ABOVE:
        case KEFIR_OPT_COMPARISON_INT16_ABOVE:
        case KEFIR_OPT_COMPARISON_INT32_ABOVE:
        case KEFIR_OPT_COMPARISON_INT64_ABOVE:
            REQUIRE_OK(kefir_asmcmp_amd64_jbe(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT8_ABOVE_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_ABOVE_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_ABOVE_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_ABOVE_OR_EQUALS:
            REQUIRE_OK(kefir_asmcmp_amd64_jb(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT8_BELOW:
        case KEFIR_OPT_COMPARISON_INT16_BELOW:
        case KEFIR_OPT_COMPARISON_INT32_BELOW:
        case KEFIR_OPT_COMPARISON_INT64_BELOW:
            REQUIRE_OK(kefir_asmcmp_amd64_jae(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT8_BELOW_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_BELOW_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_BELOW_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_BELOW_OR_EQUALS:
            REQUIRE_OK(kefir_asmcmp_amd64_ja(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT64_EQUAL:
            REQUIRE_OK(kefir_asmcmp_amd64_jp(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_jne(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_EQUAL: {
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

        case KEFIR_OPT_COMPARISON_FLOAT32_GREATER:
        case KEFIR_OPT_COMPARISON_FLOAT64_GREATER:
            REQUIRE_OK(kefir_asmcmp_amd64_jp(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_jbe(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER:
        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER: {
            kefir_asmcmp_label_index_t p_label;
            REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &p_label));

            REQUIRE_OK(kefir_asmcmp_amd64_jp(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(p_label), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_ja(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));

            REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, p_label));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT32_GREATER_OR_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT64_GREATER_OR_EQUAL:
            REQUIRE_OK(kefir_asmcmp_amd64_jp(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_jb(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER_OR_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER_OR_EQUAL: {
            kefir_asmcmp_label_index_t p_label;
            REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &p_label));

            REQUIRE_OK(kefir_asmcmp_amd64_jp(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(p_label), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_jae(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));

            REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, p_label));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT32_LESSER:
        case KEFIR_OPT_COMPARISON_FLOAT64_LESSER:
            REQUIRE_OK(kefir_asmcmp_amd64_jp(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_jae(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER:
        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER: {
            kefir_asmcmp_label_index_t p_label;
            REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &p_label));

            REQUIRE_OK(kefir_asmcmp_amd64_jp(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(p_label), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_jb(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));

            REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, p_label));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT32_LESSER_OR_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT64_LESSER_OR_EQUAL:
            REQUIRE_OK(kefir_asmcmp_amd64_jp(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_ja(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER_OR_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER_OR_EQUAL: {
            kefir_asmcmp_label_index_t p_label;
            REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &p_label));

            REQUIRE_OK(kefir_asmcmp_amd64_jp(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(p_label), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_jbe(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));

            REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, p_label));
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer comparison operation");
    }

    REQUIRE_OK(kefir_codegen_amd64_function_map_phi_outputs(mem, function, target_block->id, instruction->block_id));

    const struct kefir_opt_code_block_schedule *target_block_schedule, *source_block_schedule;
    REQUIRE_OK(kefir_opt_code_schedule_of_block(&function->schedule, target_block->id, &target_block_schedule));
    REQUIRE_OK(kefir_opt_code_schedule_of_block(&function->schedule, source_block->id, &source_block_schedule));
    if (alternative_phi_outputs || target_block_schedule->linear_index != source_block_schedule->linear_index + 1) {
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

        const struct kefir_opt_code_block_schedule *alternative_block_schedule;
        REQUIRE_OK(
            kefir_opt_code_schedule_of_block(&function->schedule, alternative_block->id, &alternative_block_schedule));
        if (alternative_block_schedule->linear_index != source_block_schedule->linear_index + 1) {
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

    REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));

    const struct kefir_opt_code_block *target_block, *alternative_block, *source_block;
    REQUIRE_OK(kefir_opt_code_container_block(&function->function->code,
                                              instruction->operation.parameters.branch.target_block, &target_block));
    REQUIRE_OK(kefir_opt_code_container_block(
        &function->function->code, instruction->operation.parameters.branch.alternative_block, &alternative_block));
    REQUIRE_OK(kefir_opt_code_container_block(&function->function->code, instruction->block_id, &source_block));

    const kefir_bool_t invert_condition =
        KEFIR_OPT_BRANCH_CONDITION_VARIANT_IS_NEGATED(instruction->operation.parameters.branch.condition_variant);
    const struct kefir_opt_instruction *condition_instr;
    kefir_asmcmp_virtual_register_index_t condition_vreg_idx;
    REQUIRE_OK(kefir_opt_code_container_instr(
        &function->function->code, instruction->operation.parameters.branch.condition_ref, &condition_instr));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.branch.condition_ref,
                                                    &condition_vreg_idx));

    switch (instruction->operation.parameters.branch.condition_variant) {
        case KEFIR_OPT_BRANCH_CONDITION_8BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_8BIT:
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG8(condition_vreg_idx), &KEFIR_ASMCMP_MAKE_VREG8(condition_vreg_idx), NULL));
            break;

        case KEFIR_OPT_BRANCH_CONDITION_16BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_16BIT:
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG16(condition_vreg_idx), &KEFIR_ASMCMP_MAKE_VREG16(condition_vreg_idx), NULL));
            break;

        case KEFIR_OPT_BRANCH_CONDITION_32BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_32BIT:
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(condition_vreg_idx), &KEFIR_ASMCMP_MAKE_VREG32(condition_vreg_idx), NULL));
            break;

        case KEFIR_OPT_BRANCH_CONDITION_64BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_64BIT:
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(condition_vreg_idx), &KEFIR_ASMCMP_MAKE_VREG64(condition_vreg_idx), NULL));
            break;
    }

    struct kefir_hashtree_node *label_node;
    kefir_bool_t alternative_phi_outputs;
    kefir_asmcmp_label_index_t branch_label_idx;
    REQUIRE_OK(has_phi_outputs(function, alternative_block, &alternative_phi_outputs));
    if (alternative_phi_outputs) {
        REQUIRE_OK(
            kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &branch_label_idx));
        if (invert_condition) {
            REQUIRE_OK(kefir_asmcmp_amd64_jnz(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_jz(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(branch_label_idx), NULL));
        }
    } else {
        REQUIRE_OK(kefir_hashtree_at(&function->labels, (kefir_hashtree_key_t) alternative_block->id, &label_node));
        ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, target_label, label_node->value);
        if (invert_condition) {
            REQUIRE_OK(kefir_asmcmp_amd64_jnz(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(target_label), NULL));
        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_jz(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(target_label), NULL));
        }
    }

    REQUIRE_OK(kefir_codegen_amd64_function_map_phi_outputs(mem, function, target_block->id, instruction->block_id));

    const struct kefir_opt_code_block_schedule *target_block_schedule, *source_block_schedule;
    REQUIRE_OK(kefir_opt_code_schedule_of_block(&function->schedule, target_block->id, &target_block_schedule));
    REQUIRE_OK(kefir_opt_code_schedule_of_block(&function->schedule, source_block->id, &source_block_schedule));
    if (alternative_phi_outputs || target_block_schedule->linear_index != source_block_schedule->linear_index + 1) {
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

        const struct kefir_opt_code_block_schedule *alternative_block_schedule;
        REQUIRE_OK(
            kefir_opt_code_schedule_of_block(&function->schedule, alternative_block->id, &alternative_block_schedule));
        if (alternative_block_schedule->linear_index != source_block_schedule->linear_index + 1) {
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

    REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));
    REQUIRE_OK(kefir_codegen_local_variable_allocator_mark_all_global(&function->variable_allocator));

    kefir_asmcmp_virtual_register_index_t target_vreg_idx;
    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &target_vreg_idx));

    REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(target_vreg_idx), NULL));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(select_compare)(struct kefir_mem *mem,
                                                                    struct kefir_codegen_amd64_function *function,
                                                                    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t condition1_vreg, condition2_vreg, arg1_vreg, arg2_vreg, arg2_placement_vreg,
        result_vreg, result_placement_vreg;
    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &condition1_vreg));
    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &condition2_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[2], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[3], &arg2_vreg));

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg2_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_placement_vreg, arg1_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         arg2_placement_vreg, arg2_vreg, NULL));

    switch (instruction->operation.parameters.comparison) {
        case KEFIR_OPT_COMPARISON_INT8_EQUALS:
        case KEFIR_OPT_COMPARISON_INT8_NOT_EQUALS:
        case KEFIR_OPT_COMPARISON_INT8_GREATER:
        case KEFIR_OPT_COMPARISON_INT8_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT8_LESSER:
        case KEFIR_OPT_COMPARISON_INT8_LESSER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT8_ABOVE:
        case KEFIR_OPT_COMPARISON_INT8_ABOVE_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT8_BELOW:
        case KEFIR_OPT_COMPARISON_INT8_BELOW_OR_EQUALS:
            REQUIRE_OK(kefir_asmcmp_amd64_cmp(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG8(condition1_vreg), &KEFIR_ASMCMP_MAKE_VREG8(condition2_vreg), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT16_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_NOT_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_GREATER:
        case KEFIR_OPT_COMPARISON_INT16_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_LESSER:
        case KEFIR_OPT_COMPARISON_INT16_LESSER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_ABOVE:
        case KEFIR_OPT_COMPARISON_INT16_ABOVE_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_BELOW:
        case KEFIR_OPT_COMPARISON_INT16_BELOW_OR_EQUALS:
            REQUIRE_OK(kefir_asmcmp_amd64_cmp(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG16(condition1_vreg), &KEFIR_ASMCMP_MAKE_VREG16(condition2_vreg), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT32_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_NOT_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_GREATER:
        case KEFIR_OPT_COMPARISON_INT32_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_LESSER:
        case KEFIR_OPT_COMPARISON_INT32_LESSER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_ABOVE:
        case KEFIR_OPT_COMPARISON_INT32_ABOVE_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_BELOW:
        case KEFIR_OPT_COMPARISON_INT32_BELOW_OR_EQUALS:
            REQUIRE_OK(kefir_asmcmp_amd64_cmp(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(condition1_vreg), &KEFIR_ASMCMP_MAKE_VREG32(condition2_vreg), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT64_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_NOT_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_GREATER:
        case KEFIR_OPT_COMPARISON_INT64_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_LESSER:
        case KEFIR_OPT_COMPARISON_INT64_LESSER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_ABOVE:
        case KEFIR_OPT_COMPARISON_INT64_ABOVE_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_BELOW:
        case KEFIR_OPT_COMPARISON_INT64_BELOW_OR_EQUALS:
            REQUIRE_OK(kefir_asmcmp_amd64_cmp(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(condition1_vreg), &KEFIR_ASMCMP_MAKE_VREG64(condition2_vreg), NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected comparison operation");
    }

    switch (instruction->operation.parameters.comparison) {
        case KEFIR_OPT_COMPARISON_INT8_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_EQUALS:
            REQUIRE_OK(kefir_asmcmp_amd64_cmovne(mem, &function->code,
                                                 kefir_asmcmp_context_instr_tail(&function->code.context),
                                                 &KEFIR_ASMCMP_MAKE_VREG64(result_placement_vreg),
                                                 &KEFIR_ASMCMP_MAKE_VREG64(arg2_placement_vreg), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT8_NOT_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_NOT_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_NOT_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_NOT_EQUALS:
            REQUIRE_OK(kefir_asmcmp_amd64_cmove(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG64(result_placement_vreg),
                                                &KEFIR_ASMCMP_MAKE_VREG64(arg2_placement_vreg), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT8_GREATER:
        case KEFIR_OPT_COMPARISON_INT16_GREATER:
        case KEFIR_OPT_COMPARISON_INT32_GREATER:
        case KEFIR_OPT_COMPARISON_INT64_GREATER:
            REQUIRE_OK(kefir_asmcmp_amd64_cmovle(mem, &function->code,
                                                 kefir_asmcmp_context_instr_tail(&function->code.context),
                                                 &KEFIR_ASMCMP_MAKE_VREG64(result_placement_vreg),
                                                 &KEFIR_ASMCMP_MAKE_VREG64(arg2_placement_vreg), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT8_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_GREATER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_GREATER_OR_EQUALS:
            REQUIRE_OK(kefir_asmcmp_amd64_cmovl(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG64(result_placement_vreg),
                                                &KEFIR_ASMCMP_MAKE_VREG64(arg2_placement_vreg), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT8_LESSER:
        case KEFIR_OPT_COMPARISON_INT16_LESSER:
        case KEFIR_OPT_COMPARISON_INT32_LESSER:
        case KEFIR_OPT_COMPARISON_INT64_LESSER:
            REQUIRE_OK(kefir_asmcmp_amd64_cmovge(mem, &function->code,
                                                 kefir_asmcmp_context_instr_tail(&function->code.context),
                                                 &KEFIR_ASMCMP_MAKE_VREG64(result_placement_vreg),
                                                 &KEFIR_ASMCMP_MAKE_VREG64(arg2_placement_vreg), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT8_LESSER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_LESSER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_LESSER_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_LESSER_OR_EQUALS:
            REQUIRE_OK(kefir_asmcmp_amd64_cmovg(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG64(result_placement_vreg),
                                                &KEFIR_ASMCMP_MAKE_VREG64(arg2_placement_vreg), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT8_ABOVE:
        case KEFIR_OPT_COMPARISON_INT16_ABOVE:
        case KEFIR_OPT_COMPARISON_INT32_ABOVE:
        case KEFIR_OPT_COMPARISON_INT64_ABOVE:
            REQUIRE_OK(kefir_asmcmp_amd64_cmovbe(mem, &function->code,
                                                 kefir_asmcmp_context_instr_tail(&function->code.context),
                                                 &KEFIR_ASMCMP_MAKE_VREG64(result_placement_vreg),
                                                 &KEFIR_ASMCMP_MAKE_VREG64(arg2_placement_vreg), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT8_ABOVE_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_ABOVE_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_ABOVE_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_ABOVE_OR_EQUALS:
            REQUIRE_OK(kefir_asmcmp_amd64_cmovb(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG64(result_placement_vreg),
                                                &KEFIR_ASMCMP_MAKE_VREG64(arg2_placement_vreg), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT8_BELOW:
        case KEFIR_OPT_COMPARISON_INT16_BELOW:
        case KEFIR_OPT_COMPARISON_INT32_BELOW:
        case KEFIR_OPT_COMPARISON_INT64_BELOW:
            REQUIRE_OK(kefir_asmcmp_amd64_cmovae(mem, &function->code,
                                                 kefir_asmcmp_context_instr_tail(&function->code.context),
                                                 &KEFIR_ASMCMP_MAKE_VREG64(result_placement_vreg),
                                                 &KEFIR_ASMCMP_MAKE_VREG64(arg2_placement_vreg), NULL));
            break;

        case KEFIR_OPT_COMPARISON_INT8_BELOW_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT16_BELOW_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT32_BELOW_OR_EQUALS:
        case KEFIR_OPT_COMPARISON_INT64_BELOW_OR_EQUALS:
            REQUIRE_OK(kefir_asmcmp_amd64_cmova(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG64(result_placement_vreg),
                                                &KEFIR_ASMCMP_MAKE_VREG64(arg2_placement_vreg), NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected comparison operation");
    }

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_vreg, result_placement_vreg, NULL));
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(select)(struct kefir_mem *mem,
                                                            struct kefir_codegen_amd64_function *function,
                                                            const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t condition_vreg, arg1_vreg, arg2_vreg, arg2_placement_vreg, result_vreg;
    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &condition_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[2], &arg2_vreg));

    const struct kefir_asmcmp_virtual_register *arg1_asmcmp_vreg, *arg2_asmcmp_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_get(&function->code.context, arg1_vreg, &arg1_asmcmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_get(&function->code.context, arg2_vreg, &arg2_asmcmp_vreg));

    kefir_bool_t complex_float_select = false;
    if (arg1_asmcmp_vreg->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR ||
        arg2_asmcmp_vreg->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR) {
        REQUIRE(arg1_asmcmp_vreg->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR &&
                    arg2_asmcmp_vreg->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR &&
                    arg1_asmcmp_vreg->parameters.pair.type == arg2_asmcmp_vreg->parameters.pair.type,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected both virtual registers to be pairs of the same type"));

        kefir_asmcmp_virtual_register_index_t arg2_placement_real_vreg, arg2_placement_imag_vreg, result_real_vreg,
            result_imag_vreg;
        switch (arg1_asmcmp_vreg->parameters.pair.type) {
            case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_SINGLE:
            case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_DOUBLE:
                complex_float_select = true;
                REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                             KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT,
                                                             &arg2_placement_real_vreg));
                REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                             KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT,
                                                             &arg2_placement_imag_vreg));
                REQUIRE_OK(kefir_asmcmp_virtual_register_new(
                    mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_real_vreg));
                REQUIRE_OK(kefir_asmcmp_virtual_register_new(
                    mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_imag_vreg));
                REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(
                    mem, &function->code.context, arg1_asmcmp_vreg->parameters.pair.type, arg2_placement_real_vreg,
                    arg2_placement_imag_vreg, &arg2_placement_vreg));
                REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context,
                                                                  arg1_asmcmp_vreg->parameters.pair.type,
                                                                  result_real_vreg, result_imag_vreg, &result_vreg));
                break;
        }
    } else {
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg2_placement_vreg));
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg, arg1_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         arg2_placement_vreg, arg2_vreg, NULL));

    const kefir_bool_t invert_condition =
        KEFIR_OPT_BRANCH_CONDITION_VARIANT_IS_NEGATED(instruction->operation.parameters.condition_variant);
    switch (instruction->operation.parameters.condition_variant) {
        case KEFIR_OPT_BRANCH_CONDITION_8BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_8BIT:
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG8(condition_vreg), &KEFIR_ASMCMP_MAKE_VREG8(condition_vreg), NULL));
            break;

        case KEFIR_OPT_BRANCH_CONDITION_16BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_16BIT:
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG16(condition_vreg), &KEFIR_ASMCMP_MAKE_VREG16(condition_vreg), NULL));
            break;

        case KEFIR_OPT_BRANCH_CONDITION_32BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_32BIT:
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(condition_vreg), &KEFIR_ASMCMP_MAKE_VREG32(condition_vreg), NULL));
            break;

        case KEFIR_OPT_BRANCH_CONDITION_64BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_64BIT:
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(condition_vreg), &KEFIR_ASMCMP_MAKE_VREG64(condition_vreg), NULL));
            break;
    }

    if (complex_float_select) {
        kefir_asmcmp_label_index_t no_arg2_move_label;
        REQUIRE_OK(
            kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &no_arg2_move_label));
        if (invert_condition) {
            REQUIRE_OK(kefir_asmcmp_amd64_jz(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(no_arg2_move_label), NULL));
        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_jnz(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(no_arg2_move_label), NULL));
        }
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             result_vreg, arg2_placement_vreg, NULL));

        REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, no_arg2_move_label));
    } else {
        if (invert_condition) {
            REQUIRE_OK(kefir_asmcmp_amd64_cmovnz(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_placement_vreg), NULL));
        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_cmovz(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_placement_vreg), NULL));
        }
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}
