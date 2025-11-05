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

#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct semi_nca_data {
    kefir_size_t num_of_blocks;
    kefir_int64_t counter;
    kefir_int64_t *dfs_trace;
    kefir_codegen_target_ir_block_ref_t *reverse_dfs_trace;
    kefir_codegen_target_ir_block_ref_t *dfs_parents;
    kefir_codegen_target_ir_block_ref_t *semi_dominators;
    kefir_codegen_target_ir_block_ref_t *best_candidates;
    kefir_codegen_target_ir_block_ref_t *immediate_dominators;
};

static kefir_result_t semi_nca_dfs(struct kefir_codegen_target_ir_control_flow *control_flow, struct semi_nca_data *data,
                                   kefir_codegen_target_ir_block_ref_t block_ref) {
    REQUIRE(data->dfs_trace[block_ref] == -1, KEFIR_OK);
    data->dfs_trace[block_ref] = data->counter;
    data->reverse_dfs_trace[data->counter] = block_ref;
    data->counter++;

    kefir_result_t res;
    struct kefir_hashtreeset_iterator iter;
    for (res = kefir_hashtreeset_iter(&control_flow->blocks[block_ref].successors, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, successor_block_ref, (kefir_uptr_t) iter.entry);
        if (data->dfs_trace[successor_block_ref] == -1) {
            data->dfs_parents[successor_block_ref] = block_ref;
            REQUIRE_OK(semi_nca_dfs(control_flow, data, successor_block_ref));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_codegen_target_ir_block_ref_t semi_nca_evaluate(struct semi_nca_data *data, kefir_codegen_target_ir_block_ref_t block_ref,
                                              kefir_int64_t current) {
    if (data->dfs_trace[block_ref] <= current) {
        return block_ref;
    }

    kefir_codegen_target_ir_block_ref_t parent = data->dfs_parents[block_ref];
    kefir_codegen_target_ir_block_ref_t result = semi_nca_evaluate(data, parent, current);
    if (data->dfs_trace[data->best_candidates[parent]] < data->dfs_trace[data->best_candidates[block_ref]]) {
        data->best_candidates[block_ref] = data->best_candidates[parent];
    }
    data->dfs_parents[block_ref] = result;
    return result;
}

static kefir_result_t semi_nca_impl(struct kefir_codegen_target_ir_control_flow *control_flow, struct semi_nca_data *data) {
    for (kefir_size_t i = 0; i < data->num_of_blocks; i++) {
        data->dfs_trace[i] = -1;
        data->reverse_dfs_trace[i] = KEFIR_ID_NONE;
        data->dfs_parents[i] = KEFIR_ID_NONE;
        data->semi_dominators[i] = KEFIR_ID_NONE;
        data->best_candidates[i] = i;
        data->immediate_dominators[i] = KEFIR_ID_NONE;
    }

    REQUIRE_OK(semi_nca_dfs(control_flow, data, control_flow->code->entry_block));

    for (kefir_int64_t i = data->counter; i > 0;) {
        i--;
        kefir_codegen_target_ir_block_ref_t block_ref = data->reverse_dfs_trace[i];
        data->semi_dominators[block_ref] = block_ref;

        kefir_result_t res;
        struct kefir_hashtreeset_iterator iter;
        for (res = kefir_hashtreeset_iter(&control_flow->blocks[block_ref].predecessors, &iter); res == KEFIR_OK;
            res = kefir_hashtreeset_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_codegen_target_ir_block_ref_t, predecessor_block_ref, (kefir_uptr_t) iter.entry);
            if (data->dfs_trace[predecessor_block_ref] != -1) {
                semi_nca_evaluate(data, predecessor_block_ref, i);
                if (data->dfs_trace[data->best_candidates[predecessor_block_ref]] <
                    data->dfs_trace[data->semi_dominators[block_ref]]) {
                    data->semi_dominators[block_ref] = data->best_candidates[predecessor_block_ref];
                }
            }
        }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

        data->best_candidates[block_ref] = data->semi_dominators[block_ref];
        data->immediate_dominators[block_ref] = data->dfs_parents[block_ref];
    }

    for (kefir_int64_t i = 1; i < data->counter; i++) {
        kefir_codegen_target_ir_block_ref_t block_re = data->reverse_dfs_trace[i];
        while (data->dfs_trace[data->immediate_dominators[block_re]] >
               data->dfs_trace[data->semi_dominators[block_re]]) {
            data->immediate_dominators[block_re] = data->immediate_dominators[data->immediate_dominators[block_re]];
        }
    }

    for (kefir_codegen_target_ir_block_ref_t block_ref = 0; block_ref < data->num_of_blocks; block_ref++) {
        control_flow->blocks[block_ref].immediate_dominator = data->immediate_dominators[block_ref];
    }

    return KEFIR_OK;
}

static kefir_result_t semi_nca(struct kefir_mem *mem, struct kefir_codegen_target_ir_control_flow *control_flow) {
    struct semi_nca_data data = {
        .num_of_blocks = kefir_codegen_target_ir_code_block_count(control_flow->code)
    };
    data.dfs_trace = KEFIR_MALLOC(mem, sizeof(kefir_int64_t) * data.num_of_blocks);
    data.reverse_dfs_trace = KEFIR_MALLOC(mem, sizeof(kefir_codegen_target_ir_block_ref_t) * data.num_of_blocks);
    data.dfs_parents = KEFIR_MALLOC(mem, sizeof(kefir_codegen_target_ir_block_ref_t) * data.num_of_blocks);
    data.semi_dominators = KEFIR_MALLOC(mem, sizeof(kefir_codegen_target_ir_block_ref_t) * data.num_of_blocks);
    data.best_candidates = KEFIR_MALLOC(mem, sizeof(kefir_codegen_target_ir_block_ref_t) * data.num_of_blocks);
    data.immediate_dominators = KEFIR_MALLOC(mem, sizeof(kefir_codegen_target_ir_block_ref_t) * data.num_of_blocks);

    kefir_result_t res = KEFIR_OK;
    if (data.dfs_trace == NULL || data.reverse_dfs_trace == NULL || data.dfs_parents == NULL ||
        data.semi_dominators == NULL || data.best_candidates == NULL || data.immediate_dominators == NULL) {
        res = KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate semi-nca dominator algorithm data");
    }
    REQUIRE_CHAIN(&res, semi_nca_impl(control_flow, &data));

    KEFIR_FREE(mem, data.dfs_trace);
    KEFIR_FREE(mem, data.reverse_dfs_trace);
    KEFIR_FREE(mem, data.dfs_parents);
    KEFIR_FREE(mem, data.semi_dominators);
    KEFIR_FREE(mem, data.best_candidates);
    KEFIR_FREE(mem, data.immediate_dominators);

    return res;
}

kefir_result_t kefir_codegen_target_ir_control_flow_find_dominators(struct kefir_mem *mem,
                                                        struct kefir_codegen_target_ir_control_flow *control_flow) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));

    REQUIRE_OK(semi_nca(mem, control_flow));
    return KEFIR_OK;
}
