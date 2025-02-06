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

#define KEFIR_OPTIMIZER_ANALYSIS_INTERNAL
#include "kefir/optimizer/analysis.h"
#include "kefir/optimizer/format.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/bucketset.h"
#include <stdio.h>

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

static kefir_result_t semi_nca_dfs(struct kefir_opt_code_analysis *analysis, struct semi_nca_data *data,
                                   kefir_opt_block_id_t block_id) {
    REQUIRE(data->dfs_trace[block_id] == -1, KEFIR_OK);
    data->dfs_trace[block_id] = data->counter;
    data->reverse_dfs_trace[data->counter] = block_id;
    data->counter++;

    for (const struct kefir_list_entry *iter = kefir_list_head(&analysis->blocks[block_id].successors); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, (kefir_uptr_t) iter->value);
        if (data->dfs_trace[successor_block_id] == -1) {
            data->dfs_parents[successor_block_id] = block_id;
            REQUIRE_OK(semi_nca_dfs(analysis, data, successor_block_id));
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

static kefir_result_t semi_nca_impl(struct kefir_opt_code_analysis *analysis, struct semi_nca_data *data) {
    for (kefir_size_t i = 0; i < data->num_of_blocks; i++) {
        data->dfs_trace[i] = -1;
        data->reverse_dfs_trace[i] = KEFIR_ID_NONE;
        data->dfs_parents[i] = KEFIR_ID_NONE;
        data->semi_dominators[i] = KEFIR_ID_NONE;
        data->best_candidates[i] = i;
        data->immediate_dominators[i] = KEFIR_ID_NONE;
    }

    REQUIRE_OK(semi_nca_dfs(analysis, data, analysis->code->entry_point));

    for (kefir_int64_t i = data->counter; i > 0;) {
        i--;
        kefir_opt_block_id_t block_id = data->reverse_dfs_trace[i];
        data->semi_dominators[block_id] = block_id;

        for (const struct kefir_list_entry *iter = kefir_list_head(&analysis->blocks[block_id].predecessors);
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
        analysis->blocks[block_id].immediate_dominator = data->immediate_dominators[block_id];
    }

    return KEFIR_OK;
}

static kefir_result_t semi_nca(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis) {
    struct semi_nca_data data = {0};
    REQUIRE_OK(kefir_opt_code_container_block_count(analysis->code, &data.num_of_blocks));
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
    REQUIRE_CHAIN(&res, semi_nca_impl(analysis, &data));

    KEFIR_FREE(mem, data.dfs_trace);
    KEFIR_FREE(mem, data.reverse_dfs_trace);
    KEFIR_FREE(mem, data.dfs_parents);
    KEFIR_FREE(mem, data.semi_dominators);
    KEFIR_FREE(mem, data.best_candidates);
    KEFIR_FREE(mem, data.immediate_dominators);

    return res;
}

#ifndef NDEBUG
struct find_dominators_check_data {
    struct kefir_opt_code_analysis *analysis;
    struct kefir_bucketset tmp_bucketset;
    struct kefir_bucketset *block_dominators;
};

static kefir_result_t find_dominators_check_step(struct kefir_mem *mem, kefir_opt_block_id_t block_id,
                                                 struct find_dominators_check_data *data, kefir_bool_t *has_changes) {
    kefir_result_t res;
    struct kefir_bucketset_iterator iter2;
    kefir_bucketset_entry_t entry;
    kefir_bool_t first = true;
    for (const struct kefir_list_entry *iter = kefir_list_head(&data->analysis->blocks[block_id].predecessors);
         iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, pred_block_id, (kefir_uptr_t) iter->value);
        if (!kefir_bucketset_has(&data->block_dominators[pred_block_id],
                                 (kefir_bucketset_entry_t) data->analysis->code->entry_point)) {
            continue;
        }
        if (first) {
            first = false;
            REQUIRE_OK(kefir_bucketset_merge(mem, &data->tmp_bucketset, &data->block_dominators[pred_block_id]));
        } else {
            REQUIRE_OK(kefir_bucketset_intersect(mem, &data->tmp_bucketset, &data->block_dominators[pred_block_id]));
        }
    }
    REQUIRE_OK(kefir_bucketset_add(mem, &data->tmp_bucketset, (kefir_bucketset_entry_t) block_id));

    for (res = kefir_bucketset_iter(&data->block_dominators[block_id], &iter2, &entry);
         res == KEFIR_OK && !*has_changes; res = kefir_bucketset_next(&iter2, &entry)) {
        if (!kefir_bucketset_has(&data->tmp_bucketset, entry)) {
            *has_changes = true;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (res = kefir_bucketset_iter(&data->tmp_bucketset, &iter2, &entry); res == KEFIR_OK && !*has_changes;
         res = kefir_bucketset_next(&iter2, &entry)) {
        if (!kefir_bucketset_has(&data->block_dominators[block_id], entry)) {
            *has_changes = true;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    REQUIRE_OK(kefir_bucketset_clean_nofree(mem, &data->block_dominators[block_id]));
    REQUIRE_OK(kefir_bucketset_merge(mem, &data->block_dominators[block_id], &data->tmp_bucketset));
    return KEFIR_OK;
}

static kefir_result_t find_dominators_check_impl(struct kefir_mem *mem, struct find_dominators_check_data *data) {
    kefir_size_t total_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(data->analysis->code, &total_blocks));

    REQUIRE_OK(kefir_bucketset_add(mem, &data->block_dominators[data->analysis->code->entry_point],
                                   (kefir_bucketset_entry_t) data->analysis->code->entry_point));

    kefir_result_t res = KEFIR_OK;
    kefir_bool_t has_changes = true;
    while (has_changes && res == KEFIR_OK) {
        has_changes = false;
        for (kefir_opt_block_id_t i = 0; res == KEFIR_OK && i < total_blocks; i++) {
            if (i != data->analysis->code->entry_point) {
                res = find_dominators_check_step(mem, i, data, &has_changes);
                REQUIRE_CHAIN(&res, kefir_bucketset_clean_nofree(mem, &data->tmp_bucketset));
            }
        }
    }

    for (kefir_opt_block_id_t block_id = 0; block_id < total_blocks; block_id++) {
        REQUIRE_OK(kefir_bucketset_clean_nofree(mem, &data->tmp_bucketset));
        kefir_opt_block_id_t imm_dominator = block_id;
        while (imm_dominator != KEFIR_ID_NONE) {
            REQUIRE_OK(kefir_bucketset_add(mem, &data->tmp_bucketset, imm_dominator));
            imm_dominator = data->analysis->blocks[imm_dominator].immediate_dominator;
        }

        kefir_result_t res;
        struct kefir_bucketset_iterator iter2;
        kefir_bucketset_entry_t entry;
        for (res = kefir_bucketset_iter(&data->block_dominators[block_id], &iter2, &entry); res == KEFIR_OK;
             res = kefir_bucketset_next(&iter2, &entry)) {
            REQUIRE(kefir_bucketset_has(&data->tmp_bucketset, entry),
                    KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Validation of semi-nca algorithm results failed"));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        for (res = kefir_bucketset_iter(&data->tmp_bucketset, &iter2, &entry); res == KEFIR_OK;
             res = kefir_bucketset_next(&iter2, &entry)) {
            REQUIRE(kefir_bucketset_has(&data->block_dominators[block_id], entry),
                    KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Validation of semi-nca algorithm results failed"));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    return KEFIR_OK;
}

static kefir_result_t find_dominators_check(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis) {
    kefir_size_t total_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(analysis->code, &total_blocks));

    struct find_dominators_check_data data = {.analysis = analysis};
    REQUIRE_OK(kefir_bucketset_init(&data.tmp_bucketset, &kefir_bucketset_uint_ops));

    data.block_dominators = KEFIR_MALLOC(mem, sizeof(struct kefir_bucketset) * total_blocks);
    REQUIRE(data.block_dominators != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate bucketsets"));
    for (kefir_size_t i = 0; i < total_blocks; i++) {
        kefir_result_t res = kefir_bucketset_init(&data.block_dominators[i], &kefir_bucketset_uint_ops);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, data.block_dominators);
            return res;
        });
    }

    kefir_result_t res = find_dominators_check_impl(mem, &data);
    for (kefir_size_t i = 0; i < total_blocks; i++) {
        kefir_bucketset_free(mem, &data.block_dominators[i]);
    }
    KEFIR_FREE(mem, data.block_dominators);
    kefir_bucketset_free(mem, &data.tmp_bucketset);
    return res;
}
#endif

kefir_result_t kefir_opt_code_container_find_dominators(struct kefir_mem *mem,
                                                        struct kefir_opt_code_analysis *analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));

    REQUIRE_OK(semi_nca(mem, analysis));
#ifndef NDEBUG
    REQUIRE_OK(find_dominators_check(mem, analysis));  // TODO Remove the check once more confidence is gained
#endif

    return KEFIR_OK;
}
