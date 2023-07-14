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

#include "kefir/optimizer/pipeline.h"
#include "kefir/optimizer/builder.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t cmp_branch_fuse_apply(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                            struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass) {
    UNUSED(pass);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct kefir_opt_code_container_iterator iter;
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(&func->code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {

        struct kefir_opt_instruction *branch_instr = NULL;
        REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&func->code, block, &branch_instr));
        if (branch_instr->operation.opcode != KEFIR_OPT_OPCODE_BRANCH) {
            continue;
        }

        struct kefir_opt_instruction *condition_instr = NULL;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, branch_instr->operation.parameters.branch.condition_ref,
                                                  &condition_instr));

        kefir_opt_instruction_ref_t replacement_ref = KEFIR_ID_NONE;
        switch (condition_instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_INT_EQUALS:
                REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                    mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_EQUALS,
                    condition_instr->operation.parameters.refs[0], condition_instr->operation.parameters.refs[1],
                    branch_instr->operation.parameters.branch.target_block,
                    branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                break;

            case KEFIR_OPT_OPCODE_INT_GREATER:
                REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                    mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_GREATER,
                    condition_instr->operation.parameters.refs[0], condition_instr->operation.parameters.refs[1],
                    branch_instr->operation.parameters.branch.target_block,
                    branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                break;

            case KEFIR_OPT_OPCODE_INT_GREATER_OR_EQUALS:
                REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                    mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_GREATER_OR_EQUALS,
                    condition_instr->operation.parameters.refs[0], condition_instr->operation.parameters.refs[1],
                    branch_instr->operation.parameters.branch.target_block,
                    branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                break;

            case KEFIR_OPT_OPCODE_INT_LESSER:
                REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                    mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_LESS,
                    condition_instr->operation.parameters.refs[0], condition_instr->operation.parameters.refs[1],
                    branch_instr->operation.parameters.branch.target_block,
                    branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                break;

            case KEFIR_OPT_OPCODE_INT_LESSER_OR_EQUALS:
                REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                    mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_LESS_OR_EQUALS,
                    condition_instr->operation.parameters.refs[0], condition_instr->operation.parameters.refs[1],
                    branch_instr->operation.parameters.branch.target_block,
                    branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                break;

            case KEFIR_OPT_OPCODE_INT_ABOVE:
                REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                    mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE,
                    condition_instr->operation.parameters.refs[0], condition_instr->operation.parameters.refs[1],
                    branch_instr->operation.parameters.branch.target_block,
                    branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                break;

            case KEFIR_OPT_OPCODE_INT_ABOVE_OR_EQUALS:
                REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                    mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE_OR_EQUALS,
                    condition_instr->operation.parameters.refs[0], condition_instr->operation.parameters.refs[1],
                    branch_instr->operation.parameters.branch.target_block,
                    branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                break;

            case KEFIR_OPT_OPCODE_INT_BELOW:
                REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                    mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_BELOW,
                    condition_instr->operation.parameters.refs[0], condition_instr->operation.parameters.refs[1],
                    branch_instr->operation.parameters.branch.target_block,
                    branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                break;

            case KEFIR_OPT_OPCODE_INT_BELOW_OR_EQUALS:
                REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                    mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_BELOW_OR_EQUALS,
                    condition_instr->operation.parameters.refs[0], condition_instr->operation.parameters.refs[1],
                    branch_instr->operation.parameters.branch.target_block,
                    branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                break;

            case KEFIR_OPT_OPCODE_BOOL_NOT: {
                struct kefir_opt_instruction *negated_condition_instr = NULL;
                REQUIRE_OK(kefir_opt_code_container_instr(&func->code, condition_instr->operation.parameters.refs[0],
                                                          &negated_condition_instr));

                switch (negated_condition_instr->operation.opcode) {
                    case KEFIR_OPT_OPCODE_INT_EQUALS:
                        REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                            mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_NOT_EQUALS,
                            negated_condition_instr->operation.parameters.refs[0],
                            negated_condition_instr->operation.parameters.refs[1],
                            branch_instr->operation.parameters.branch.target_block,
                            branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT_GREATER:
                        REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                            mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_LESS_OR_EQUALS,
                            negated_condition_instr->operation.parameters.refs[0],
                            negated_condition_instr->operation.parameters.refs[1],
                            branch_instr->operation.parameters.branch.target_block,
                            branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT_GREATER_OR_EQUALS:
                        REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                            mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_LESS,
                            negated_condition_instr->operation.parameters.refs[0],
                            negated_condition_instr->operation.parameters.refs[1],
                            branch_instr->operation.parameters.branch.target_block,
                            branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT_LESSER:
                        REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                            mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_GREATER_OR_EQUALS,
                            negated_condition_instr->operation.parameters.refs[0],
                            negated_condition_instr->operation.parameters.refs[1],
                            branch_instr->operation.parameters.branch.target_block,
                            branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT_LESSER_OR_EQUALS:
                        REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                            mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_GREATER,
                            negated_condition_instr->operation.parameters.refs[0],
                            negated_condition_instr->operation.parameters.refs[1],
                            branch_instr->operation.parameters.branch.target_block,
                            branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT_ABOVE:
                        REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                            mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_BELOW_OR_EQUALS,
                            negated_condition_instr->operation.parameters.refs[0],
                            negated_condition_instr->operation.parameters.refs[1],
                            branch_instr->operation.parameters.branch.target_block,
                            branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT_ABOVE_OR_EQUALS:
                        REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                            mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_BELOW,
                            negated_condition_instr->operation.parameters.refs[0],
                            negated_condition_instr->operation.parameters.refs[1],
                            branch_instr->operation.parameters.branch.target_block,
                            branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT_BELOW:
                        REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                            mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE_OR_EQUALS,
                            negated_condition_instr->operation.parameters.refs[0],
                            negated_condition_instr->operation.parameters.refs[1],
                            branch_instr->operation.parameters.branch.target_block,
                            branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT_BELOW_OR_EQUALS:
                        REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                            mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE,
                            negated_condition_instr->operation.parameters.refs[0],
                            negated_condition_instr->operation.parameters.refs[1],
                            branch_instr->operation.parameters.branch.target_block,
                            branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_BOOL_NOT:
                        REQUIRE_OK(kefir_opt_code_container_instr(&func->code,
                                                                  negated_condition_instr->operation.parameters.refs[0],
                                                                  &negated_condition_instr));

                        switch (negated_condition_instr->operation.opcode) {
                            case KEFIR_OPT_OPCODE_INT_EQUALS:
                                REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                                    mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_EQUALS,
                                    negated_condition_instr->operation.parameters.refs[0],
                                    negated_condition_instr->operation.parameters.refs[1],
                                    branch_instr->operation.parameters.branch.target_block,
                                    branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                                break;

                            case KEFIR_OPT_OPCODE_INT_GREATER:
                                REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                                    mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_GREATER,
                                    negated_condition_instr->operation.parameters.refs[0],
                                    negated_condition_instr->operation.parameters.refs[1],
                                    branch_instr->operation.parameters.branch.target_block,
                                    branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                                break;

                            case KEFIR_OPT_OPCODE_INT_GREATER_OR_EQUALS:
                                REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                                    mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_GREATER_OR_EQUALS,
                                    negated_condition_instr->operation.parameters.refs[0],
                                    negated_condition_instr->operation.parameters.refs[1],
                                    branch_instr->operation.parameters.branch.target_block,
                                    branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                                break;

                            case KEFIR_OPT_OPCODE_INT_LESSER:
                                REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                                    mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_LESS,
                                    negated_condition_instr->operation.parameters.refs[0],
                                    negated_condition_instr->operation.parameters.refs[1],
                                    branch_instr->operation.parameters.branch.target_block,
                                    branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                                break;

                            case KEFIR_OPT_OPCODE_INT_LESSER_OR_EQUALS:
                                REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                                    mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_LESS_OR_EQUALS,
                                    negated_condition_instr->operation.parameters.refs[0],
                                    negated_condition_instr->operation.parameters.refs[1],
                                    branch_instr->operation.parameters.branch.target_block,
                                    branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                                break;

                            case KEFIR_OPT_OPCODE_INT_ABOVE:
                                REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                                    mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE,
                                    negated_condition_instr->operation.parameters.refs[0],
                                    negated_condition_instr->operation.parameters.refs[1],
                                    branch_instr->operation.parameters.branch.target_block,
                                    branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                                break;

                            case KEFIR_OPT_OPCODE_INT_ABOVE_OR_EQUALS:
                                REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                                    mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_ABOVE_OR_EQUALS,
                                    negated_condition_instr->operation.parameters.refs[0],
                                    negated_condition_instr->operation.parameters.refs[1],
                                    branch_instr->operation.parameters.branch.target_block,
                                    branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                                break;

                            case KEFIR_OPT_OPCODE_INT_BELOW:
                                REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                                    mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_BELOW,
                                    negated_condition_instr->operation.parameters.refs[0],
                                    negated_condition_instr->operation.parameters.refs[1],
                                    branch_instr->operation.parameters.branch.target_block,
                                    branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                                break;

                            case KEFIR_OPT_OPCODE_INT_BELOW_OR_EQUALS:
                                REQUIRE_OK(kefir_opt_code_builder_compare_branch(
                                    mem, &func->code, block->id, KEFIR_OPT_COMPARE_BRANCH_INT_BELOW_OR_EQUALS,
                                    negated_condition_instr->operation.parameters.refs[0],
                                    negated_condition_instr->operation.parameters.refs[1],
                                    branch_instr->operation.parameters.branch.target_block,
                                    branch_instr->operation.parameters.branch.alternative_block, &replacement_ref));
                                break;

                            default:
                                // Intentionally left blank
                                break;
                        }
                        break;

                    default:
                        // Intentionally left blank
                        break;
                }
            } break;

            default:
                // Intentionally left blank
                break;
        }

        if (replacement_ref != KEFIR_ID_NONE) {
            REQUIRE_OK(kefir_opt_code_container_replace_references(&func->code, replacement_ref, branch_instr->id));
            REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, branch_instr->id));
            REQUIRE_OK(kefir_opt_code_container_add_control(&func->code, block, replacement_ref));
        }
    }
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassCompareBranchFuse = {
    .name = "compare-branch-fuse", .apply = cmp_branch_fuse_apply, .payload = NULL};
