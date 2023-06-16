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

#include "kefir/optimizer/liveness.h"
#include "kefir/optimizer/analysis.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t add_liveness_range(struct kefir_opt_code_liveness_intervals *intervals,
                                         kefir_opt_instruction_ref_t instr_ref, kefir_size_t from, kefir_size_t to) {
    struct kefir_opt_code_analysis_instruction_properties *instr_props = &intervals->analysis->instructions[instr_ref];
    REQUIRE(instr_props->reachable,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Unable to add liveness range to an unreachable instruction"));

    from = MAX(instr_props->linear_position, from);

    struct kefir_opt_instruction_liveness_interval *liveness = &intervals->intervals[instr_props->linear_position];

    if (liveness->range.begin != ~(kefir_size_t) 0ull) {
        liveness->range.begin = MIN(liveness->range.begin, from);
    } else {
        liveness->range.begin = from;
    }

    if (liveness->range.end != ~(kefir_size_t) 0ull) {
        liveness->range.end = MAX(liveness->range.end, to);
    } else {
        liveness->range.end = to;
    }
    return KEFIR_OK;
}

struct mark_instr_inputs_param {
    struct kefir_mem *mem;
    struct kefir_opt_code_liveness_intervals *intervals;
    struct kefir_opt_code_analysis_block_properties *block_props;
    struct kefir_hashtreeset *live;
    kefir_size_t range_end;
};

static kefir_result_t mark_instr_inputs(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct mark_instr_inputs_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction liveness mark payload"));

    REQUIRE_OK(add_liveness_range(param->intervals, instr_ref, param->block_props->linear_range.begin_index,
                                  param->range_end));
    REQUIRE_OK(kefir_hashtreeset_add(param->mem, param->live, (kefir_hashtreeset_entry_t) instr_ref));
    return KEFIR_OK;
}

static kefir_result_t build_block_intervals(struct kefir_mem *mem, struct kefir_opt_code_liveness_intervals *intervals,
                                            struct kefir_opt_code_analysis_block_properties *block_props,
                                            struct kefir_hashtreeset *live) {
    kefir_result_t res;

    // Merge successor phis
    for (const struct kefir_list_entry *iter = kefir_list_head(&block_props->successors); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, (kefir_uptr_t) iter->value);

        struct kefir_opt_code_block *successor_block = NULL;
        REQUIRE_OK(kefir_opt_code_container_block(intervals->analysis->code, successor_block_id, &successor_block));

        const struct kefir_opt_phi_node *successor_phi = NULL;
        for (res = kefir_opt_code_block_phi_head(intervals->analysis->code, successor_block, &successor_phi);
             res == KEFIR_OK && successor_phi != NULL;
             res = kefir_opt_phi_next_sibling(intervals->analysis->code, successor_phi, &successor_phi)) {
            kefir_opt_instruction_ref_t link_ref;
            REQUIRE_OK(kefir_opt_code_container_phi_link_for(intervals->analysis->code, successor_phi->node_id,
                                                             block_props->block_id, &link_ref));

            if (intervals->analysis->instructions[link_ref].reachable) {
                REQUIRE_OK(kefir_hashtreeset_add(mem, live, (kefir_hashtreeset_entry_t) link_ref));

                if (successor_phi->number_of_links == 1 &&
                    intervals->analysis->instructions[successor_phi->output_ref].reachable) {
                    kefir_size_t successor_output =
                        intervals->analysis->instructions[successor_phi->output_ref].linear_position;
                    intervals->intervals[successor_output].alias_ref = link_ref;
                }
            }
        }
        REQUIRE_OK(res);
    }

    // Update liveness ranges for live set instructions
    struct kefir_hashtreeset_iterator live_iter;
    for (res = kefir_hashtreeset_iter(live, &live_iter); res == KEFIR_OK; res = kefir_hashtreeset_next(&live_iter)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, live_iter.entry);

        REQUIRE_OK(add_liveness_range(intervals, instr_ref, block_props->linear_range.begin_index,
                                      block_props->linear_range.end_index));
    }
    REQUIRE(res == KEFIR_ITERATOR_END, res);

    // Iterate reachable instructions in the block and update their liveness
    for (kefir_size_t instr_idx = block_props->linear_range.end_index;
         instr_idx > block_props->linear_range.begin_index; instr_idx--) {
        struct kefir_opt_code_analysis_instruction_properties *instr_props =
            intervals->analysis->linearization[instr_idx - 1];

        struct kefir_opt_instruction *instr = NULL;
        REQUIRE_OK(kefir_opt_code_container_instr(intervals->analysis->code, instr_props->instr_ref, &instr));

        REQUIRE_OK(add_liveness_range(intervals, instr_props->instr_ref, instr_props->linear_position,
                                      instr_props->linear_position + 1));
        REQUIRE_OK(kefir_hashtreeset_delete(mem, live, (kefir_hashtreeset_entry_t) instr_props->instr_ref));

        struct mark_instr_inputs_param inputs_param = {
            .mem = mem, .intervals = intervals, .block_props = block_props, .live = live, .range_end = instr_idx};
        REQUIRE_OK(
            kefir_opt_instruction_extract_inputs(intervals->analysis->code, instr, mark_instr_inputs, &inputs_param));
    }

    // Remove phi nodes from live set
    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(kefir_opt_code_container_block(intervals->analysis->code, block_props->block_id, &block));
    const struct kefir_opt_phi_node *phi = NULL;
    for (res = kefir_opt_code_block_phi_head(intervals->analysis->code, block, &phi); res == KEFIR_OK && phi != NULL;
         res = kefir_opt_phi_next_sibling(intervals->analysis->code, phi, &phi)) {
        REQUIRE_OK(kefir_hashtreeset_delete(mem, live, (kefir_hashtreeset_entry_t) phi->output_ref));
    }
    REQUIRE_OK(res);

    REQUIRE(kefir_hashtreeset_empty(live),
            KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected optimizer liveness set contents"));
    return KEFIR_OK;
}

static kefir_result_t intervals_build_impl(struct kefir_mem *mem, struct kefir_opt_code_liveness_intervals *intervals) {
    for (kefir_size_t block_rev_idx = 0; block_rev_idx < intervals->analysis->block_linearization_length;
         block_rev_idx++) {
        struct kefir_opt_code_analysis_block_properties *block_props =
            intervals->analysis
                ->block_linearization[intervals->analysis->block_linearization_length - (block_rev_idx + 1)];

        struct kefir_hashtreeset live_set;
        REQUIRE_OK(kefir_hashtreeset_init(&live_set, &kefir_hashtree_uint_ops));

        kefir_result_t res = build_block_intervals(mem, intervals, block_props, &live_set);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_hashtreeset_free(mem, &live_set);
            return res;
        });

        REQUIRE_OK(kefir_hashtreeset_free(mem, &live_set));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_liveness_intervals_build(struct kefir_mem *mem,
                                                       const struct kefir_opt_code_analysis *analysis,
                                                       struct kefir_opt_code_liveness_intervals *intervals) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));
    REQUIRE(intervals != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer optimizer liveness intervals"));

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(analysis->code, &num_of_blocks));

    intervals->analysis = analysis;
    intervals->intervals =
        KEFIR_MALLOC(mem, sizeof(struct kefir_opt_instruction_liveness_interval) * analysis->linearization_length);
    REQUIRE(intervals->intervals != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer liveness intervals"));
    for (kefir_size_t i = 0; i < analysis->linearization_length; i++) {
        intervals->intervals[i].instr_ref = intervals->analysis->linearization[i]->instr_ref;
        intervals->intervals[i].alias_ref = KEFIR_ID_NONE;
        intervals->intervals[i].range.begin = ~(kefir_size_t) 0ull;
        intervals->intervals[i].range.end = ~(kefir_size_t) 0ull;
    }

    kefir_result_t res = intervals_build_impl(mem, intervals);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, intervals->intervals);
        memset(intervals, 0, sizeof(struct kefir_opt_code_liveness_intervals));
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_liveness_intervals_free(struct kefir_mem *mem,
                                                      struct kefir_opt_code_liveness_intervals *intervals) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(intervals != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer liveness intervals"));

    memset(intervals->intervals, 0,
           sizeof(struct kefir_opt_instruction_liveness_interval) * intervals->analysis->linearization_length);
    KEFIR_FREE(mem, intervals->intervals);

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(intervals->analysis->code, &num_of_blocks));
    memset(intervals, 0, sizeof(struct kefir_opt_code_liveness_intervals));
    return KEFIR_OK;
}
