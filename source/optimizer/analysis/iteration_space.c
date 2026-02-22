/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#include "kefir/optimizer/iteration_space.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

#define NO_MATCH KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match optimizer loop iteration space")

static kefir_result_t match_branch_compare(const struct kefir_opt_code_container *code,
                                           const struct kefir_opt_code_loop *loop,
                                           const struct kefir_opt_instruction *entry_tail,
                                           struct kefir_opt_loop_iteration_space *iteration_space) {

    kefir_int_t width = 0;
    kefir_bool_t reversed = false;
    kefir_bool_t signed_comparison = false;
    kefir_bool_t inclusive = false;
    switch (entry_tail->operation.parameters.branch.comparison.operation) {
        case KEFIR_OPT_COMPARISON_INT8_BELOW:
            REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                !kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            width = 8;
            break;

        case KEFIR_OPT_COMPARISON_INT8_BELOW_OR_EQUALS:
            REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                !kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            inclusive = true;
            width = 8;
            break;

        case KEFIR_OPT_COMPARISON_INT8_LESSER:
            REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                !kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            signed_comparison = true;
            width = 8;
            break;

        case KEFIR_OPT_COMPARISON_INT8_LESSER_OR_EQUALS:
            REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                !kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            inclusive = true;
            signed_comparison = true;
            width = 8;
            break;

        case KEFIR_OPT_COMPARISON_INT8_ABOVE:
            REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            width = 8;
            reversed = true;
            break;

        case KEFIR_OPT_COMPARISON_INT8_ABOVE_OR_EQUALS:
            REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            inclusive = true;
            width = 8;
            reversed = true;
            break;

        case KEFIR_OPT_COMPARISON_INT8_GREATER:
            REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            signed_comparison = true;
            width = 8;
            reversed = true;
            break;

        case KEFIR_OPT_COMPARISON_INT8_GREATER_OR_EQUALS:
            REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            inclusive = true;
            signed_comparison = true;
            width = 8;
            reversed = true;
            break;

        case KEFIR_OPT_COMPARISON_INT16_BELOW:
            REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                !kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            width = 16;
            break;

        case KEFIR_OPT_COMPARISON_INT16_BELOW_OR_EQUALS:
            REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                !kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            inclusive = true;
            width = 16;
            break;

        case KEFIR_OPT_COMPARISON_INT16_LESSER:
            REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                !kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            signed_comparison = true;
            width = 16;
            break;

        case KEFIR_OPT_COMPARISON_INT16_LESSER_OR_EQUALS:
            REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                !kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            inclusive = true;
            signed_comparison = true;
            width = 16;
            break;

        case KEFIR_OPT_COMPARISON_INT16_ABOVE:
            REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            width = 16;
            reversed = true;
            break;

        case KEFIR_OPT_COMPARISON_INT16_ABOVE_OR_EQUALS:
            REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            inclusive = true;
            width = 16;
            reversed = true;
            break;

        case KEFIR_OPT_COMPARISON_INT16_GREATER:
            REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            signed_comparison = true;
            width = 16;
            reversed = true;
            break;

        case KEFIR_OPT_COMPARISON_INT16_GREATER_OR_EQUALS:
            REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            inclusive = true;
            signed_comparison = true;
            width = 16;
            reversed = true;
            break;

        case KEFIR_OPT_COMPARISON_INT32_BELOW:
            REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                !kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            width = 32;
            break;

        case KEFIR_OPT_COMPARISON_INT32_BELOW_OR_EQUALS:
            REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                !kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            inclusive = true;
            width = 32;
            break;

        case KEFIR_OPT_COMPARISON_INT32_LESSER:
            REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                !kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            signed_comparison = true;
            width = 32;
            break;

        case KEFIR_OPT_COMPARISON_INT32_LESSER_OR_EQUALS:
            REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                !kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            inclusive = true;
            signed_comparison = true;
            width = 32;
            break;

        case KEFIR_OPT_COMPARISON_INT32_ABOVE:
            REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            width = 32;
            reversed = true;
            break;

        case KEFIR_OPT_COMPARISON_INT32_ABOVE_OR_EQUALS:
            REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            inclusive = true;
            width = 32;
            reversed = true;
            break;

        case KEFIR_OPT_COMPARISON_INT32_GREATER:
            REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            signed_comparison = true;
            width = 32;
            reversed = true;
            break;

        case KEFIR_OPT_COMPARISON_INT32_GREATER_OR_EQUALS:
            REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            inclusive = true;
            signed_comparison = true;
            width = 32;
            reversed = true;
            break;

        case KEFIR_OPT_COMPARISON_INT64_BELOW:
            REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                !kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            width = 64;
            break;

        case KEFIR_OPT_COMPARISON_INT64_BELOW_OR_EQUALS:
            REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                !kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            inclusive = true;
            width = 64;
            break;

        case KEFIR_OPT_COMPARISON_INT64_LESSER:
            REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                !kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            signed_comparison = true;
            width = 64;
            break;

        case KEFIR_OPT_COMPARISON_INT64_LESSER_OR_EQUALS:
            REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                !kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            inclusive = true;
            signed_comparison = true;
            width = 64;
            break;

        case KEFIR_OPT_COMPARISON_INT64_ABOVE:
            REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            width = 64;
            reversed = true;
            break;

        case KEFIR_OPT_COMPARISON_INT64_ABOVE_OR_EQUALS:
            REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            inclusive = true;
            width = 64;
            reversed = true;
            break;

        case KEFIR_OPT_COMPARISON_INT64_GREATER:
            REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            signed_comparison = true;
            width = 64;
            reversed = true;
            break;

        case KEFIR_OPT_COMPARISON_INT64_GREATER_OR_EQUALS:
            REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block),
                    NO_MATCH);
            REQUIRE(
                kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block),
                NO_MATCH);
            inclusive = true;
            signed_comparison = true;
            width = 64;
            reversed = true;
            break;

        default:
            return NO_MATCH;
    }

    const struct kefir_opt_instruction *upper_bound, *index;

    if (reversed) {
        REQUIRE_OK(kefir_opt_code_container_instr(code, entry_tail->operation.parameters.refs[1], &upper_bound));
        REQUIRE_OK(kefir_opt_code_container_instr(code, entry_tail->operation.parameters.refs[0], &index));
    } else {
        REQUIRE_OK(kefir_opt_code_container_instr(code, entry_tail->operation.parameters.refs[1], &index));
        REQUIRE_OK(kefir_opt_code_container_instr(code, entry_tail->operation.parameters.refs[0], &upper_bound));
    }
    REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, upper_bound->block_id), NO_MATCH);
    REQUIRE(index->block_id == loop->loop_entry_block_id, NO_MATCH);
    REQUIRE(index->operation.opcode == KEFIR_OPT_OPCODE_PHI, NO_MATCH);

    const struct kefir_opt_phi_node *index_phi;
    REQUIRE_OK(kefir_opt_code_container_phi(code, index->operation.parameters.phi_ref, &index_phi));
    REQUIRE(index_phi->number_of_links == 2, KEFIR_NO_MATCH);

    const struct kefir_opt_instruction *lower_bound = NULL, *stride_op = NULL, *stride = NULL;

    kefir_result_t res;
    struct kefir_opt_phi_node_link_iterator link_iter;
    kefir_opt_block_id_t link_block_id;
    kefir_opt_instruction_ref_t link_instr_ref;
    for (res = kefir_opt_phi_node_link_iter(code, index_phi->output_ref, &link_iter, &link_block_id, &link_instr_ref);
         res == KEFIR_OK; res = kefir_opt_phi_node_link_next(&link_iter, &link_block_id, &link_instr_ref)) {
        if (kefir_hashtreeset_has(&loop->loop_blocks, (kefir_hashtreeset_entry_t) link_block_id)) {
            REQUIRE_OK(kefir_opt_code_container_instr(code, link_instr_ref, &stride_op));
        } else {
            REQUIRE_OK(kefir_opt_code_container_instr(code, link_instr_ref, &lower_bound));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    REQUIRE(lower_bound != NULL, NO_MATCH);
    REQUIRE(stride_op != NULL, NO_MATCH);

    kefir_bool_t asceding = true;
    if (((stride_op->operation.opcode == KEFIR_OPT_OPCODE_INT8_ADD && width == 8) ||
         (stride_op->operation.opcode == KEFIR_OPT_OPCODE_INT16_ADD && width == 16) ||
         (stride_op->operation.opcode == KEFIR_OPT_OPCODE_INT32_ADD && width == 32) ||
         (stride_op->operation.opcode == KEFIR_OPT_OPCODE_INT64_ADD && width == 64))) {
        if (stride_op->operation.parameters.refs[0] == index->id) {
            REQUIRE_OK(kefir_opt_code_container_instr(code, stride_op->operation.parameters.refs[1], &stride));
        } else if (stride_op->operation.parameters.refs[1] == index->id) {
            REQUIRE_OK(kefir_opt_code_container_instr(code, stride_op->operation.parameters.refs[0], &stride));
        }
    }
    REQUIRE(stride != NULL, NO_MATCH);
    REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, stride->block_id), NO_MATCH);

    iteration_space->type = KEFIR_OPT_LOOP_ITERATION_SPACE_RANGE;
    iteration_space->strided_range.index_ref = index->id;
    iteration_space->strided_range.lower_bound_ref = lower_bound->id;
    iteration_space->strided_range.upper_bound_ref = upper_bound->id;
    iteration_space->strided_range.stride_ref = stride->id;
    iteration_space->strided_range.ascending = asceding;
    iteration_space->strided_range.comparison_width = width;
    iteration_space->strided_range.signed_comparison = signed_comparison;
    iteration_space->strided_range.inclusive = inclusive;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_loop_match_iteration_space(const struct kefir_opt_code_container *code,
                                                    const struct kefir_opt_code_loop *loop,
                                                    struct kefir_opt_loop_iteration_space *iteration_space) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(loop != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer loop"));
    REQUIRE(iteration_space != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer iteration space"));

    kefir_opt_instruction_ref_t entry_tail_ref;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(code, loop->loop_entry_block_id, &entry_tail_ref));
    REQUIRE(entry_tail_ref != KEFIR_ID_NONE, NO_MATCH);

    const struct kefir_opt_instruction *entry_tail;
    REQUIRE_OK(kefir_opt_code_container_instr(code, entry_tail_ref, &entry_tail));
    REQUIRE(entry_tail != NULL, NO_MATCH);

    switch (entry_tail->operation.opcode) {
        case KEFIR_OPT_OPCODE_BRANCH_COMPARE:
            REQUIRE_OK(match_branch_compare(code, loop, entry_tail, iteration_space));
            break;

        default:
            return NO_MATCH;
    }
    return KEFIR_OK;
}
