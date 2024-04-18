/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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
#include <string.h>

static kefir_result_t linearize_impl(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                     struct kefir_list *queue) {
    kefir_size_t block_index = 0;
    kefir_size_t linear_index = 0;
    kefir_result_t res = KEFIR_OK;
    for (struct kefir_list_entry *head_entry = kefir_list_head(queue); res == KEFIR_OK && head_entry != NULL;
         res = kefir_list_pop(mem, queue, head_entry), head_entry = kefir_list_head(queue)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) head_entry->value);

        if (!analysis->blocks[block_id].reachable) {
            continue;
        }

        analysis->block_linearization[block_index] = &analysis->blocks[block_id];
        analysis->blocks[block_id].linear_position = ++block_index;
        analysis->blocks[block_id].linear_range.begin_index = linear_index;

        const struct kefir_opt_code_block *block = NULL;
        REQUIRE_OK(kefir_opt_code_container_block(analysis->code, block_id, &block));
        
        kefir_opt_instruction_ref_t instr_ref;
        for (res = kefir_opt_code_block_instr_head(analysis->code, block, &instr_ref); res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
             res = kefir_opt_instruction_next_sibling(analysis->code, instr_ref, &instr_ref)) {

            if (!analysis->instructions[instr_ref].reachable) {
                continue;
            }

            analysis->linearization[linear_index] = &analysis->instructions[instr_ref];
            analysis->instructions[instr_ref].linear_position = linear_index++;
        }
        REQUIRE_OK(res);

        analysis->blocks[block_id].linear_range.end_index = linear_index;
    }
    REQUIRE_OK(res);

    return KEFIR_OK;
}

struct scheduler_callback_payload {
    struct kefir_mem *mem;
    struct kefir_list *queue;
};

static kefir_result_t scheduler_callback(kefir_opt_block_id_t block_id, void *payload) {
    REQUIRE(payload != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer scheduler callback payload"));
    ASSIGN_DECL_CAST(struct scheduler_callback_payload *, param, payload);

    REQUIRE_OK(kefir_list_insert_after(param->mem, param->queue, kefir_list_tail(param->queue),
                                       (void *) (kefir_uptr_t) block_id));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_analyze_linearize(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis,
                                                const struct kefir_opt_code_analyze_block_scheduler *block_scheduler) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));
    REQUIRE(block_scheduler != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer analysis block scheduler"));

    analysis->linearization = KEFIR_MALLOC(
        mem, sizeof(struct kefir_opt_code_analysis_instruction_properties *) * analysis->linearization_length);
    REQUIRE(analysis->linearization != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer analysis linearization"));

    analysis->block_linearization = KEFIR_MALLOC(
        mem, sizeof(struct kefir_opt_code_analysis_block_properties *) * analysis->block_linearization_length);
    REQUIRE(analysis->block_linearization != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer analysis linearization"));

    struct kefir_list queue;
    REQUIRE_OK(kefir_list_init(&queue));
    kefir_result_t res = block_scheduler->schedule(mem, block_scheduler, analysis->code, scheduler_callback,
                                                   &(struct scheduler_callback_payload){.mem = mem, .queue = &queue});
    REQUIRE_CHAIN(&res, linearize_impl(mem, analysis, &queue));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &queue));
    return KEFIR_OK;
}
