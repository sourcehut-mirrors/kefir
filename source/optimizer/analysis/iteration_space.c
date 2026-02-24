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
#define CASE(_cmp, _width, _reversed, _signed, _inclusive)                                                         \
    case KEFIR_OPT_COMPARISON_INT##_width##_##_cmp:                                                                \
        REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block) == \
                    !(_reversed),                                                                                  \
                NO_MATCH);                                                                                         \
        REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks,                                                          \
                                      entry_tail->operation.parameters.branch.alternative_block) == (_reversed),   \
                NO_MATCH);                                                                                         \
        reversed = (_reversed);                                                                                    \
        signed_comparison = (_signed);                                                                             \
        inclusive = (_inclusive);                                                                                  \
        width = (_width);                                                                                          \
        break

#define WIDTH_CASE(_width)                             \
    CASE(BELOW, _width, false, false, false);          \
    CASE(BELOW_OR_EQUALS, _width, false, false, true); \
    CASE(LESSER, _width, false, true, false);          \
    CASE(LESSER_OR_EQUALS, _width, false, true, true); \
    CASE(ABOVE, _width, true, false, true);            \
    CASE(ABOVE_OR_EQUALS, _width, true, false, false); \
    CASE(GREATER, _width, true, true, true);           \
    CASE(GREATER_OR_EQUALS, _width, true, true, false)

        WIDTH_CASE(8);
        WIDTH_CASE(16);
        WIDTH_CASE(32);
        WIDTH_CASE(64);

#undef WIDTH_CASE
#undef CASE

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
    REQUIRE(kefir_hashtreeset_has(&loop->loop_blocks, stride_op->block_id), NO_MATCH);

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
    if (stride != NULL) {
        REQUIRE(stride != NULL, NO_MATCH);
        iteration_space->type = KEFIR_OPT_LOOP_ITERATION_SPACE_STRIDED_RANGE;
        iteration_space->range.stride_ref = stride->id;
    } else {
        iteration_space->type = KEFIR_OPT_LOOP_ITERATION_SPACE_GENERAL_RANGE;
        iteration_space->range.stride_ref = stride_op->id;
    }

    iteration_space->range.index_ref = index->id;
    iteration_space->range.lower_bound_ref = lower_bound->id;
    iteration_space->range.upper_bound_ref = upper_bound->id;
    iteration_space->range.ascending = asceding;
    iteration_space->range.comparison_width = width;
    iteration_space->range.signed_comparison = signed_comparison;
    iteration_space->range.inclusive = inclusive;
    return KEFIR_OK;
}

static kefir_result_t match_branch(const struct kefir_opt_code_container *code, const struct kefir_opt_code_loop *loop,
                                   const struct kefir_opt_instruction *entry_tail,
                                   struct kefir_opt_loop_iteration_space *iteration_space) {
    kefir_bool_t has_target_block =
        kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.target_block);
    kefir_bool_t has_alternative_block =
        kefir_hashtreeset_has(&loop->loop_blocks, entry_tail->operation.parameters.branch.alternative_block);
    kefir_bool_t invert = false;
    switch (entry_tail->operation.parameters.branch.condition_variant) {
        case KEFIR_OPT_BRANCH_CONDITION_8BIT:
        case KEFIR_OPT_BRANCH_CONDITION_16BIT:
        case KEFIR_OPT_BRANCH_CONDITION_32BIT:
        case KEFIR_OPT_BRANCH_CONDITION_64BIT:
            if (has_target_block && !has_alternative_block) {
                // Intentionally left blank
            } else if (!has_target_block && has_alternative_block) {
                invert = true;
            } else {
                return NO_MATCH;
            }
            break;

        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_8BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_16BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_32BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_64BIT:
            if (!has_target_block && has_alternative_block) {
                // Intentionally left blank
            } else if (has_target_block && !has_alternative_block) {
                invert = true;
            } else {
                return NO_MATCH;
            }
            break;
    }

    const struct kefir_opt_instruction *cond_instr;
    REQUIRE_OK(
        kefir_opt_code_container_instr(code, entry_tail->operation.parameters.branch.condition_ref, &cond_instr));
    REQUIRE(cond_instr->block_id == loop->loop_entry_block_id, NO_MATCH);
    REQUIRE(cond_instr->operation.opcode == KEFIR_OPT_OPCODE_PHI, NO_MATCH);

    const struct kefir_opt_phi_node *cond_phi;
    REQUIRE_OK(kefir_opt_code_container_phi(code, cond_instr->operation.parameters.phi_ref, &cond_phi));
    REQUIRE(cond_phi->number_of_links == 2, KEFIR_NO_MATCH);

    const struct kefir_opt_instruction *init_instr = NULL, *next_instr = NULL;

    kefir_result_t res;
    struct kefir_opt_phi_node_link_iterator link_iter;
    kefir_opt_block_id_t link_block_id;
    kefir_opt_instruction_ref_t link_instr_ref;
    for (res = kefir_opt_phi_node_link_iter(code, cond_phi->output_ref, &link_iter, &link_block_id, &link_instr_ref);
         res == KEFIR_OK; res = kefir_opt_phi_node_link_next(&link_iter, &link_block_id, &link_instr_ref)) {
        if (kefir_hashtreeset_has(&loop->loop_blocks, (kefir_hashtreeset_entry_t) link_block_id)) {
            REQUIRE_OK(kefir_opt_code_container_instr(code, link_instr_ref, &next_instr));
        } else {
            REQUIRE_OK(kefir_opt_code_container_instr(code, link_instr_ref, &init_instr));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    REQUIRE(init_instr != NULL, NO_MATCH);
    REQUIRE(next_instr != NULL, NO_MATCH);
    REQUIRE(!kefir_hashtreeset_has(&loop->loop_blocks, init_instr->block_id), NO_MATCH);

    iteration_space->type = KEFIR_OPT_LOOP_ITERATION_SPACE_GENERAL;
    iteration_space->general.init_ref = init_instr->id;
    iteration_space->general.condition_ref = cond_instr->id;
    iteration_space->general.next_ref = next_instr->id;
    iteration_space->general.variant = entry_tail->operation.parameters.branch.condition_variant;
    iteration_space->general.invert = invert;
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

        case KEFIR_OPT_OPCODE_BRANCH:
            REQUIRE_OK(match_branch(code, loop, entry_tail, iteration_space));
            break;

        default:
            return NO_MATCH;
    }
    return KEFIR_OK;
}

#undef NO_MATCH

static kefir_result_t range_may_execute(const struct kefir_opt_code_container *code,
                                        const struct kefir_opt_loop_iteration_space *iteration_space,
                                        kefir_bool_t *may_execute_ptr) {
    REQUIRE(iteration_space->range.ascending, KEFIR_OK);

    const struct kefir_opt_instruction *lower_bound_instr, *upper_bound_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, iteration_space->range.lower_bound_ref, &lower_bound_instr));
    REQUIRE_OK(kefir_opt_code_container_instr(code, iteration_space->range.upper_bound_ref, &upper_bound_instr));

    REQUIRE(lower_bound_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                lower_bound_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST,
            KEFIR_OK);
    REQUIRE(upper_bound_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                upper_bound_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST,
            KEFIR_OK);

#define CHECK(_width)                                                                                              \
    if (iteration_space->range.comparison_width == (_width) && iteration_space->range.signed_comparison &&         \
        iteration_space->range.inclusive) {                                                                        \
        *may_execute_ptr = ((kefir_int##_width##_t) lower_bound_instr->operation.parameters.imm.integer) <=        \
                           ((kefir_int##_width##_t) upper_bound_instr->operation.parameters.imm.integer);          \
    } else if (iteration_space->range.comparison_width == (_width) && iteration_space->range.signed_comparison &&  \
               !iteration_space->range.inclusive) {                                                                \
        *may_execute_ptr = ((kefir_int##_width##_t) lower_bound_instr->operation.parameters.imm.integer) <         \
                           ((kefir_int##_width##_t) upper_bound_instr->operation.parameters.imm.integer);          \
    } else if (iteration_space->range.comparison_width == (_width) && !iteration_space->range.signed_comparison && \
               iteration_space->range.inclusive) {                                                                 \
        *may_execute_ptr = ((kefir_uint##_width##_t) lower_bound_instr->operation.parameters.imm.integer) <=       \
                           ((kefir_uint##_width##_t) upper_bound_instr->operation.parameters.imm.integer);         \
    } else if (iteration_space->range.comparison_width == (_width) && !iteration_space->range.signed_comparison && \
               !iteration_space->range.inclusive) {                                                                \
        *may_execute_ptr = ((kefir_uint##_width##_t) lower_bound_instr->operation.parameters.imm.integer) <        \
                           ((kefir_uint##_width##_t) upper_bound_instr->operation.parameters.imm.integer);         \
    }

    // clang-format off
    CHECK(8)
    else CHECK(16)
    else CHECK(32)
    else CHECK(64)
#undef CHECK
    return KEFIR_OK;
    // clang-format on
}

static kefir_result_t general_may_execute(const struct kefir_opt_code_container *code,
                                          const struct kefir_opt_loop_iteration_space *iteration_space,
                                          kefir_bool_t *may_execute_ptr) {
    const struct kefir_opt_instruction *init_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, iteration_space->general.init_ref, &init_instr));

    REQUIRE(init_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                init_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST,
            KEFIR_OK);

    kefir_bool_t init_holds = true;
    switch (iteration_space->general.variant) {
        case KEFIR_OPT_BRANCH_CONDITION_8BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_8BIT:
            init_holds = (kefir_uint8_t) init_instr->operation.parameters.imm.uinteger;
            break;

        case KEFIR_OPT_BRANCH_CONDITION_16BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_16BIT:
            init_holds = (kefir_uint16_t) init_instr->operation.parameters.imm.uinteger;
            break;

        case KEFIR_OPT_BRANCH_CONDITION_32BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_32BIT:
            init_holds = (kefir_uint32_t) init_instr->operation.parameters.imm.uinteger;
            break;

        case KEFIR_OPT_BRANCH_CONDITION_64BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_64BIT:
            init_holds = (kefir_uint64_t) init_instr->operation.parameters.imm.uinteger;
            break;
    }
    if (iteration_space->general.invert) {
        init_holds = !init_holds;
    }

    *may_execute_ptr = init_holds;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_loop_may_execute(const struct kefir_opt_code_container *code,
                                          const struct kefir_opt_loop_iteration_space *iteration_space,
                                          kefir_bool_t *may_execute_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(iteration_space != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer iteration space"));
    REQUIRE(may_execute_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    *may_execute_ptr = true;
    switch (iteration_space->type) {
        case KEFIR_OPT_LOOP_ITERATION_SPACE_STRIDED_RANGE:
        case KEFIR_OPT_LOOP_ITERATION_SPACE_GENERAL_RANGE:
            REQUIRE_OK(range_may_execute(code, iteration_space, may_execute_ptr));
            break;

        case KEFIR_OPT_LOOP_ITERATION_SPACE_GENERAL:
            REQUIRE_OK(general_may_execute(code, iteration_space, may_execute_ptr));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t range_must_execute(const struct kefir_opt_code_container *code,
                                         const struct kefir_opt_loop_iteration_space *iteration_space,
                                         kefir_bool_t *must_execute_ptr) {
    REQUIRE(iteration_space->range.ascending, KEFIR_OK);

    const struct kefir_opt_instruction *lower_bound_instr, *upper_bound_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, iteration_space->range.lower_bound_ref, &lower_bound_instr));
    REQUIRE_OK(kefir_opt_code_container_instr(code, iteration_space->range.upper_bound_ref, &upper_bound_instr));

    REQUIRE(lower_bound_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                lower_bound_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST,
            KEFIR_OK);
    REQUIRE(upper_bound_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                upper_bound_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST,
            KEFIR_OK);

#define CHECK(_width)                                                                                              \
    if (iteration_space->range.comparison_width == (_width) && iteration_space->range.signed_comparison &&         \
        iteration_space->range.inclusive) {                                                                        \
        *must_execute_ptr = ((kefir_int##_width##_t) lower_bound_instr->operation.parameters.imm.integer) <=       \
                            ((kefir_int##_width##_t) upper_bound_instr->operation.parameters.imm.integer);         \
    } else if (iteration_space->range.comparison_width == (_width) && iteration_space->range.signed_comparison &&  \
               !iteration_space->range.inclusive) {                                                                \
        *must_execute_ptr = ((kefir_int##_width##_t) lower_bound_instr->operation.parameters.imm.integer) <        \
                            ((kefir_int##_width##_t) upper_bound_instr->operation.parameters.imm.integer);         \
    } else if (iteration_space->range.comparison_width == (_width) && !iteration_space->range.signed_comparison && \
               iteration_space->range.inclusive) {                                                                 \
        *must_execute_ptr = ((kefir_uint##_width##_t) lower_bound_instr->operation.parameters.imm.integer) <=      \
                            ((kefir_uint##_width##_t) upper_bound_instr->operation.parameters.imm.integer);        \
    } else if (iteration_space->range.comparison_width == (_width) && !iteration_space->range.signed_comparison && \
               !iteration_space->range.inclusive) {                                                                \
        *must_execute_ptr = ((kefir_uint##_width##_t) lower_bound_instr->operation.parameters.imm.integer) <       \
                            ((kefir_uint##_width##_t) upper_bound_instr->operation.parameters.imm.integer);        \
    }

    // clang-format off
    CHECK(8)
    else CHECK(16)
    else CHECK(32)
    else CHECK(64)
#undef CHECK
    return KEFIR_OK;
    // clang-format on
}

static kefir_result_t general_must_execute(const struct kefir_opt_code_container *code,
                                           const struct kefir_opt_loop_iteration_space *iteration_space,
                                           kefir_bool_t *must_execute_ptr) {
    const struct kefir_opt_instruction *init_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, iteration_space->general.init_ref, &init_instr));

    REQUIRE(init_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                init_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST,
            KEFIR_OK);

    kefir_bool_t init_holds = true;
    switch (iteration_space->general.variant) {
        case KEFIR_OPT_BRANCH_CONDITION_8BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_8BIT:
            init_holds = (kefir_uint8_t) init_instr->operation.parameters.imm.uinteger;
            break;

        case KEFIR_OPT_BRANCH_CONDITION_16BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_16BIT:
            init_holds = (kefir_uint16_t) init_instr->operation.parameters.imm.uinteger;
            break;

        case KEFIR_OPT_BRANCH_CONDITION_32BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_32BIT:
            init_holds = (kefir_uint32_t) init_instr->operation.parameters.imm.uinteger;
            break;

        case KEFIR_OPT_BRANCH_CONDITION_64BIT:
        case KEFIR_OPT_BRANCH_CONDITION_NEGATED_64BIT:
            init_holds = (kefir_uint64_t) init_instr->operation.parameters.imm.uinteger;
            break;
    }
    if (iteration_space->general.invert) {
        init_holds = !init_holds;
    }

    *must_execute_ptr = init_holds;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_loop_must_execute(const struct kefir_opt_code_container *code,
                                           const struct kefir_opt_loop_iteration_space *iteration_space,
                                           kefir_bool_t *must_execute_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(iteration_space != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer iteration space"));
    REQUIRE(must_execute_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    *must_execute_ptr = false;
    switch (iteration_space->type) {
        case KEFIR_OPT_LOOP_ITERATION_SPACE_STRIDED_RANGE:
        case KEFIR_OPT_LOOP_ITERATION_SPACE_GENERAL_RANGE:
            REQUIRE_OK(range_must_execute(code, iteration_space, must_execute_ptr));
            break;

        case KEFIR_OPT_LOOP_ITERATION_SPACE_GENERAL:
            REQUIRE_OK(general_must_execute(code, iteration_space, must_execute_ptr));
            break;
    }

    return KEFIR_OK;
}
