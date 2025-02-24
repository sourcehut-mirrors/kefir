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

#include "kefir/optimizer/pipeline.h"
#include "kefir/optimizer/builder.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/optimizer/structure.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t match_passthrough_block(struct kefir_opt_function *func,
                                              struct kefir_opt_code_structure *structure,
                                              kefir_opt_block_id_t pred_block_id, kefir_opt_block_id_t block_id,
                                              kefir_opt_block_id_t *passthrough) {
    *passthrough = KEFIR_ID_NONE;
    kefir_bool_t only_predecessor;
    REQUIRE_OK(kefir_opt_code_structure_block_exclusive_direct_predecessor(structure, pred_block_id, block_id,
                                                                           &only_predecessor));
    REQUIRE(only_predecessor, KEFIR_OK);

    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(&func->code, block_id, &block));

    kefir_opt_instruction_ref_t tail_instr_ref;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&func->code, block, &tail_instr_ref));
    REQUIRE(tail_instr_ref != KEFIR_ID_NONE, KEFIR_OK);

    const struct kefir_opt_instruction *tail_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, tail_instr_ref, &tail_instr));
    REQUIRE(tail_instr->operation.opcode == KEFIR_OPT_OPCODE_JUMP, KEFIR_OK);

    kefir_result_t res;
    kefir_opt_instruction_ref_t instr_ref;
    for (res = kefir_opt_code_block_instr_head(&func->code, block, &instr_ref);
         res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
         res = kefir_opt_instruction_next_sibling(&func->code, instr_ref, &instr_ref)) {
        if (instr_ref == tail_instr_ref) {
            continue;
        }
        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_ref, &instr));
        switch (instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_INT_CONST:
            case KEFIR_OPT_OPCODE_UINT_CONST:
            case KEFIR_OPT_OPCODE_FLOAT32_CONST:
            case KEFIR_OPT_OPCODE_FLOAT64_CONST:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_CONST:
            case KEFIR_OPT_OPCODE_STRING_REF:
            case KEFIR_OPT_OPCODE_BLOCK_LABEL:
                // Constants are permitted in passthrough blocks
                break;

            default:
                return KEFIR_OK;
        }
    }
    REQUIRE_OK(res);

    *passthrough = tail_instr->operation.parameters.branch.target_block;
    return KEFIR_OK;
}

static kefir_result_t block_merge_impl(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       struct kefir_opt_code_structure *structure) {
    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(&func->code, &num_of_blocks));
    kefir_bool_t fixpoint_reached = false;
    while (!fixpoint_reached) {
        fixpoint_reached = true;
        for (kefir_opt_block_id_t block_id = 0; block_id < num_of_blocks; block_id++) {
            kefir_bool_t reachable;
            REQUIRE_OK(kefir_opt_code_structure_is_reachable_from_entry(structure, block_id, &reachable));
            if (!reachable) {
                continue;
            }

            const struct kefir_opt_code_block *block;
            REQUIRE_OK(kefir_opt_code_container_block(&func->code, block_id, &block));

            kefir_opt_instruction_ref_t tail_instr_ref;
            REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&func->code, block, &tail_instr_ref));
            if (tail_instr_ref == KEFIR_ID_NONE) {
                continue;
            }

            const struct kefir_opt_instruction *tail_instr;
            REQUIRE_OK(kefir_opt_code_container_instr(&func->code, tail_instr_ref, &tail_instr));

            switch (tail_instr->operation.opcode) {
                case KEFIR_OPT_OPCODE_JUMP: {
                    const kefir_opt_block_id_t target_block_id = tail_instr->operation.parameters.branch.target_block;
                    const struct kefir_opt_code_block *target_block;
                    REQUIRE_OK(kefir_opt_code_container_block(&func->code, target_block_id, &target_block));

                    kefir_bool_t only_predecessor;
                    REQUIRE_OK(kefir_opt_code_structure_block_exclusive_direct_predecessor(
                        structure, block_id, target_block_id, &only_predecessor));
                    if (only_predecessor) {
                        for (const struct kefir_list_entry *iter =
                                 kefir_list_head(&structure->blocks[target_block_id].successors);
                             iter != NULL; kefir_list_next(&iter)) {
                            ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, (kefir_uptr_t) iter->value);
                            REQUIRE_OK(kefir_opt_code_block_redirect_phi_links(mem, &func->code, target_block_id,
                                                                               block_id, successor_block_id));
                        }

                        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, tail_instr_ref));
                        REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, tail_instr_ref));

                        REQUIRE_OK(kefir_opt_code_block_merge_into(mem, &func->code, &func->debug_info, block_id,
                                                                   target_block_id, true));

                        REQUIRE_OK(kefir_opt_code_structure_redirect_edges(mem, structure, target_block_id, block_id));
                        fixpoint_reached = false;
                    }
                } break;

                case KEFIR_OPT_OPCODE_BRANCH: {
                    const kefir_opt_branch_condition_variant_t condition_variant =
                        tail_instr->operation.parameters.branch.condition_variant;
                    const kefir_opt_instruction_ref_t condition_ref =
                        tail_instr->operation.parameters.branch.condition_ref;
                    const kefir_opt_block_id_t target_block_id = tail_instr->operation.parameters.branch.target_block;
                    const kefir_opt_block_id_t alternative_block_id =
                        tail_instr->operation.parameters.branch.alternative_block;
                    kefir_opt_block_id_t target_passthrough_block_id, alternative_passthrough_block_id;
                    REQUIRE_OK(match_passthrough_block(func, structure, block_id, target_block_id,
                                                       &target_passthrough_block_id));
                    REQUIRE_OK(match_passthrough_block(func, structure, block_id, alternative_block_id,
                                                       &alternative_passthrough_block_id));

                    const kefir_opt_block_id_t resolved_target_block_id =
                        target_passthrough_block_id != KEFIR_ID_NONE ? target_passthrough_block_id : target_block_id;
                    const kefir_opt_block_id_t resolved_alternative_block_id =
                        alternative_passthrough_block_id != KEFIR_ID_NONE ? alternative_passthrough_block_id
                                                                          : alternative_block_id;
                    if ((target_passthrough_block_id != KEFIR_ID_NONE ||
                         alternative_passthrough_block_id != KEFIR_ID_NONE) &&
                        resolved_target_block_id != resolved_alternative_block_id) {
                        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, tail_instr_ref));
                        REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, tail_instr_ref));

                        if (target_passthrough_block_id != KEFIR_ID_NONE) {
                            REQUIRE_OK(kefir_opt_code_block_merge_into(mem, &func->code, &func->debug_info, block_id,
                                                                       target_block_id, false));
                            REQUIRE_OK(kefir_opt_code_block_redirect_phi_links(mem, &func->code, target_block_id,
                                                                               block_id, target_passthrough_block_id));
                        }
                        if (alternative_passthrough_block_id != KEFIR_ID_NONE) {
                            REQUIRE_OK(kefir_opt_code_block_merge_into(mem, &func->code, &func->debug_info, block_id,
                                                                       alternative_block_id, false));
                            REQUIRE_OK(kefir_opt_code_block_redirect_phi_links(
                                mem, &func->code, alternative_block_id, block_id, alternative_passthrough_block_id));
                        }

                        REQUIRE_OK(kefir_opt_code_builder_finalize_branch(mem, &func->code, block_id, condition_variant,
                                                                          condition_ref, resolved_target_block_id,
                                                                          resolved_alternative_block_id, NULL));

                        if (target_passthrough_block_id != KEFIR_ID_NONE) {
                            REQUIRE_OK(
                                kefir_opt_code_structure_redirect_edges(mem, structure, target_block_id, block_id));
                        }
                        if (alternative_passthrough_block_id != KEFIR_ID_NONE) {
                            REQUIRE_OK(kefir_opt_code_structure_redirect_edges(mem, structure, alternative_block_id,
                                                                               block_id));
                        }
                        fixpoint_reached = false;
                    }
                } break;

                case KEFIR_OPT_OPCODE_BRANCH_COMPARE: {
                    const kefir_opt_comparison_operation_t comparison_op =
                        tail_instr->operation.parameters.branch.comparison.operation;
                    const kefir_opt_instruction_ref_t arg1_reg = tail_instr->operation.parameters.refs[0],
                                                      arg2_reg = tail_instr->operation.parameters.refs[1];
                    const kefir_opt_block_id_t target_block_id = tail_instr->operation.parameters.branch.target_block;
                    const kefir_opt_block_id_t alternative_block_id =
                        tail_instr->operation.parameters.branch.alternative_block;
                    kefir_opt_block_id_t target_passthrough_block_id, alternative_passthrough_block_id;
                    REQUIRE_OK(match_passthrough_block(func, structure, block_id, target_block_id,
                                                       &target_passthrough_block_id));
                    REQUIRE_OK(match_passthrough_block(func, structure, block_id, alternative_block_id,
                                                       &alternative_passthrough_block_id));

                    const kefir_opt_block_id_t resolved_target_block_id =
                        target_passthrough_block_id != KEFIR_ID_NONE ? target_passthrough_block_id : target_block_id;
                    const kefir_opt_block_id_t resolved_alternative_block_id =
                        alternative_passthrough_block_id != KEFIR_ID_NONE ? alternative_passthrough_block_id
                                                                          : alternative_block_id;
                    if ((target_passthrough_block_id != KEFIR_ID_NONE ||
                         alternative_passthrough_block_id != KEFIR_ID_NONE) &&
                        resolved_target_block_id != resolved_alternative_block_id) {
                        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, tail_instr_ref));
                        REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, tail_instr_ref));

                        if (target_passthrough_block_id != KEFIR_ID_NONE) {
                            REQUIRE_OK(kefir_opt_code_block_merge_into(mem, &func->code, &func->debug_info, block_id,
                                                                       target_block_id, false));
                            REQUIRE_OK(kefir_opt_code_block_redirect_phi_links(mem, &func->code, target_block_id,
                                                                               block_id, target_passthrough_block_id));
                        }
                        if (alternative_passthrough_block_id != KEFIR_ID_NONE) {
                            REQUIRE_OK(kefir_opt_code_block_merge_into(mem, &func->code, &func->debug_info, block_id,
                                                                       alternative_block_id, false));
                            REQUIRE_OK(kefir_opt_code_block_redirect_phi_links(
                                mem, &func->code, alternative_block_id, block_id, alternative_passthrough_block_id));
                        }

                        REQUIRE_OK(kefir_opt_code_builder_finalize_branch_compare(
                            mem, &func->code, block_id, comparison_op, arg1_reg, arg2_reg, resolved_target_block_id,
                            resolved_alternative_block_id, NULL));

                        if (target_passthrough_block_id != KEFIR_ID_NONE) {
                            REQUIRE_OK(
                                kefir_opt_code_structure_redirect_edges(mem, structure, target_block_id, block_id));
                        }
                        if (alternative_passthrough_block_id != KEFIR_ID_NONE) {
                            REQUIRE_OK(kefir_opt_code_structure_redirect_edges(mem, structure, alternative_block_id,
                                                                               block_id));
                        }
                        fixpoint_reached = false;
                    }
                } break;

                default:
                    // Intentionally left blank
                    break;
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t block_merge_apply(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                        struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass) {
    UNUSED(pass);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct kefir_opt_code_structure structure;
    REQUIRE_OK(kefir_opt_code_structure_init(&structure));
    kefir_result_t res = kefir_opt_code_structure_build(mem, &structure, &func->code);
    REQUIRE_CHAIN(&res, block_merge_impl(mem, func, &structure));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_structure_free(mem, &structure);
        return res;
    });
    REQUIRE_OK(kefir_opt_code_structure_free(mem, &structure));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassBlockMerge = {
    .name = "block-merge", .apply = block_merge_apply, .payload = NULL};
