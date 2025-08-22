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

#include "kefir/optimizer/structure.h"
#include "kefir/optimizer/format.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct semi_nca_data {
    kefir_size_t num_of_blocks;
    kefir_int64_t counter;
    kefir_int64_t *dfs_trace;
    kefir_opt_block_id_t *reverse_dfs_trace;
    kefir_opt_block_id_t *dfs_parents;
    kefir_opt_block_id_t *semi_dominators;
    kefir_opt_block_id_t *best_candidates;
    kefir_opt_block_id_t *immediate_dominators;
};

static kefir_result_t semi_nca_dfs(struct kefir_opt_code_structure *structure, struct semi_nca_data *data,
                                   kefir_opt_block_id_t block_id) {
    REQUIRE(data->dfs_trace[block_id] == -1, KEFIR_OK);
    data->dfs_trace[block_id] = data->counter;
    data->reverse_dfs_trace[data->counter] = block_id;
    data->counter++;

    for (const struct kefir_list_entry *iter = kefir_list_head(&structure->blocks[block_id].successors); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, (kefir_uptr_t) iter->value);
        if (data->dfs_trace[successor_block_id] == -1) {
            data->dfs_parents[successor_block_id] = block_id;
            REQUIRE_OK(semi_nca_dfs(structure, data, successor_block_id));
        }
    }
    return KEFIR_OK;
}

static kefir_opt_block_id_t semi_nca_evaluate(struct semi_nca_data *data, kefir_opt_block_id_t block_id,
                                              kefir_int64_t current) {
    if (data->dfs_trace[block_id] <= current) {
        return block_id;
    }

    kefir_opt_block_id_t parent = data->dfs_parents[block_id];
    kefir_opt_block_id_t result = semi_nca_evaluate(data, parent, current);
    if (data->dfs_trace[data->best_candidates[parent]] < data->dfs_trace[data->best_candidates[block_id]]) {
        data->best_candidates[block_id] = data->best_candidates[parent];
    }
    data->dfs_parents[block_id] = result;
    return result;
}

static kefir_result_t semi_nca_impl(struct kefir_opt_code_structure *structure, struct semi_nca_data *data) {
    for (kefir_size_t i = 0; i < data->num_of_blocks; i++) {
        data->dfs_trace[i] = -1;
        data->reverse_dfs_trace[i] = KEFIR_ID_NONE;
        data->dfs_parents[i] = KEFIR_ID_NONE;
        data->semi_dominators[i] = KEFIR_ID_NONE;
        data->best_candidates[i] = i;
        data->immediate_dominators[i] = KEFIR_ID_NONE;
    }

    REQUIRE_OK(semi_nca_dfs(structure, data, structure->code->entry_point));

    for (kefir_int64_t i = data->counter; i > 0;) {
        i--;
        kefir_opt_block_id_t block_id = data->reverse_dfs_trace[i];
        data->semi_dominators[block_id] = block_id;

        for (const struct kefir_list_entry *iter = kefir_list_head(&structure->blocks[block_id].predecessors);
             iter != NULL; kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, predecessor_block_id, (kefir_uptr_t) iter->value);
            if (data->dfs_trace[predecessor_block_id] != -1) {
                semi_nca_evaluate(data, predecessor_block_id, i);
                if (data->dfs_trace[data->best_candidates[predecessor_block_id]] <
                    data->dfs_trace[data->semi_dominators[block_id]]) {
                    data->semi_dominators[block_id] = data->best_candidates[predecessor_block_id];
                }
            }
        }

        data->best_candidates[block_id] = data->semi_dominators[block_id];
        data->immediate_dominators[block_id] = data->dfs_parents[block_id];
    }

    for (kefir_int64_t i = 1; i < data->counter; i++) {
        kefir_opt_block_id_t block_id = data->reverse_dfs_trace[i];
        while (data->dfs_trace[data->immediate_dominators[block_id]] >
               data->dfs_trace[data->semi_dominators[block_id]]) {
            data->immediate_dominators[block_id] = data->immediate_dominators[data->immediate_dominators[block_id]];
        }
    }

    for (kefir_opt_block_id_t block_id = 0; block_id < data->num_of_blocks; block_id++) {
        structure->blocks[block_id].immediate_dominator = data->immediate_dominators[block_id];
    }

    return KEFIR_OK;
}

static kefir_result_t semi_nca(struct kefir_mem *mem, struct kefir_opt_code_structure *structure) {
    struct semi_nca_data data = {0};
    REQUIRE_OK(kefir_opt_code_container_block_count(structure->code, &data.num_of_blocks));
    data.dfs_trace = KEFIR_MALLOC(mem, sizeof(kefir_int64_t) * data.num_of_blocks);
    data.reverse_dfs_trace = KEFIR_MALLOC(mem, sizeof(kefir_opt_block_id_t) * data.num_of_blocks);
    data.dfs_parents = KEFIR_MALLOC(mem, sizeof(kefir_opt_block_id_t) * data.num_of_blocks);
    data.semi_dominators = KEFIR_MALLOC(mem, sizeof(kefir_opt_block_id_t) * data.num_of_blocks);
    data.best_candidates = KEFIR_MALLOC(mem, sizeof(kefir_opt_block_id_t) * data.num_of_blocks);
    data.immediate_dominators = KEFIR_MALLOC(mem, sizeof(kefir_opt_block_id_t) * data.num_of_blocks);

    kefir_result_t res = KEFIR_OK;
    if (data.dfs_trace == NULL || data.reverse_dfs_trace == NULL || data.dfs_parents == NULL ||
        data.semi_dominators == NULL || data.best_candidates == NULL || data.immediate_dominators == NULL) {
        res = KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate semi-nca dominator algorithm data");
    }
    REQUIRE_CHAIN(&res, semi_nca_impl(structure, &data));

    KEFIR_FREE(mem, data.dfs_trace);
    KEFIR_FREE(mem, data.reverse_dfs_trace);
    KEFIR_FREE(mem, data.dfs_parents);
    KEFIR_FREE(mem, data.semi_dominators);
    KEFIR_FREE(mem, data.best_candidates);
    KEFIR_FREE(mem, data.immediate_dominators);

    return res;
}

kefir_result_t kefir_opt_code_structure_find_dominators(struct kefir_mem *mem,
                                                        struct kefir_opt_code_structure *structure) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));

    REQUIRE_OK(semi_nca(mem, structure));
    return KEFIR_OK;
}
