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
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t find_dominators_step(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                           kefir_opt_block_id_t block_id, struct kefir_bucketset *tmp_dominators,
                                           kefir_bool_t *has_changes) {
    kefir_result_t res;
    struct kefir_bucketset_iterator iter2;
    kefir_bucketset_entry_t entry;
    for (const struct kefir_list_entry *iter = kefir_list_head(&analysis->blocks[block_id].predecessors); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, pred_block_id, (kefir_uptr_t) iter->value);
        if (iter == kefir_list_head(&analysis->blocks[block_id].predecessors)) {
            REQUIRE_OK(kefir_bucketset_merge(mem, tmp_dominators, &analysis->blocks[pred_block_id].dominators));
        } else {
            REQUIRE_OK(kefir_bucketset_intersect(mem, tmp_dominators, &analysis->blocks[pred_block_id].dominators));
        }
    }
    REQUIRE_OK(kefir_bucketset_add(mem, tmp_dominators, (kefir_bucketset_entry_t) block_id));

    for (res = kefir_bucketset_iter(&analysis->blocks[block_id].dominators, &iter2, &entry);
         res == KEFIR_OK && !*has_changes; res = kefir_bucketset_next(&iter2, &entry)) {
        if (!kefir_bucketset_has(tmp_dominators, entry)) {
            *has_changes = true;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    REQUIRE_OK(kefir_bucketset_clean_nofree(mem, &analysis->blocks[block_id].dominators));
    REQUIRE_OK(kefir_bucketset_merge(mem, &analysis->blocks[block_id].dominators, tmp_dominators));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_find_dominators(struct kefir_mem *mem,
                                                        struct kefir_opt_code_analysis *analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));

    kefir_size_t total_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(analysis->code, &total_blocks));

    REQUIRE_OK(kefir_bucketset_add(mem, &analysis->blocks[analysis->code->entry_point].dominators,
                                   (kefir_bucketset_entry_t) analysis->code->entry_point));
    for (kefir_opt_block_id_t i = 0; i < total_blocks; i++) {
        if (i != analysis->code->entry_point) {
            for (kefir_opt_block_id_t j = 0; j < total_blocks; j++) {
                REQUIRE_OK(kefir_bucketset_add(mem, &analysis->blocks[i].dominators, (kefir_bucketset_entry_t) j));
            }
        }
    }

    kefir_result_t res = KEFIR_OK;
    kefir_bool_t has_changes = true;
    struct kefir_bucketset tmp_dominators;
    REQUIRE_OK(kefir_bucketset_init(&tmp_dominators, &kefir_bucketset_uint_ops));
    while (has_changes && res == KEFIR_OK) {
        has_changes = false;
        for (kefir_opt_block_id_t i = 0; res == KEFIR_OK && i < total_blocks; i++) {
            if (i != analysis->code->entry_point) {
                res = find_dominators_step(mem, analysis, i, &tmp_dominators, &has_changes);
                REQUIRE_CHAIN(&res, kefir_bucketset_clean_nofree(mem, &tmp_dominators));
            }
        }
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_bucketset_free(mem, &tmp_dominators);
        return res;
    });
    REQUIRE_OK(kefir_bucketset_free(mem, &tmp_dominators));

    return KEFIR_OK;
}
