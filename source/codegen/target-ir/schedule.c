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

#include "kefir/codegen/target-ir/schedule.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_codegen_target_ir_code_schedule_init(struct kefir_codegen_target_ir_code_schedule *schedule, const struct kefir_codegen_target_ir_code *code) {
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to target IR schedule"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR code"));

    schedule->code = code;
    schedule->block_schedule = NULL;
    schedule->schedule_length = 0;
    REQUIRE_OK(kefir_hashtable_init(&schedule->blocks_by_ref, &kefir_hashtable_uint_ops));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_schedule_free(struct kefir_mem *mem, struct kefir_codegen_target_ir_code_schedule *schedule) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR schedule"));

    REQUIRE_OK(kefir_hashtable_free(mem, &schedule->blocks_by_ref));
    KEFIR_FREE(mem, schedule->block_schedule);
    memset(schedule, 0, sizeof(struct kefir_codegen_target_ir_code_schedule));
    return KEFIR_OK;
}

static kefir_result_t schedule_block(struct kefir_mem *mem, kefir_codegen_target_ir_block_ref_t block_ref, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_codegen_target_ir_code_schedule *, schedule,
        payload);
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR schedule"));
    REQUIRE(block_ref != KEFIR_ID_NONE && block_ref < kefir_codegen_target_ir_code_block_count(schedule->code), KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR block reference"));
    REQUIRE(!kefir_hashtable_has(&schedule->blocks_by_ref, (kefir_hashtable_key_t) block_ref), KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Provided target IR block has already been scheduled"));
    REQUIRE(schedule->schedule_length + 1 < kefir_codegen_target_ir_code_block_count(schedule->code), KEFIR_SET_ERROR(KEFIR_OUT_OF_SPACE, "Target IR schedule is already full"));

    schedule->block_schedule[schedule->schedule_length].block_ref = block_ref;
    schedule->block_schedule[schedule->schedule_length].linear_position = schedule->schedule_length;
    REQUIRE_OK(kefir_hashtable_insert(mem, &schedule->blocks_by_ref, (kefir_hashtable_key_t) block_ref, schedule->schedule_length));
    schedule->schedule_length++;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_target_ir_code_schedule_build(struct kefir_mem *mem, struct kefir_codegen_target_ir_code_schedule *schedule,
    const struct kefir_codegen_target_ir_code_scheduler *scheduler) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid unpopulated target IR schedule"));
    REQUIRE(schedule->block_schedule == NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid unpopulated target IR schedule"));
    REQUIRE(scheduler != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR scheduler"));

    schedule->block_schedule = KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_target_ir_block_schedule) * kefir_codegen_target_ir_code_block_count(schedule->code));
    REQUIRE(schedule->block_schedule != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate target IR block schedule"));
    REQUIRE_OK(scheduler->do_schedule(mem, schedule, &(struct kefir_codegen_target_ir_code_schedule_builder) {
        .schedule_block = schedule_block,
        .payload = schedule
    }, schedule->code->entry_block, scheduler->payload));
    return KEFIR_OK;
}

kefir_bool_t kefir_codegen_target_ir_code_schedule_has_block(const struct kefir_codegen_target_ir_code_schedule *schedule, kefir_codegen_target_ir_block_ref_t block_ref) {
    return kefir_hashtable_has(&schedule->blocks_by_ref, (kefir_hashtable_key_t) block_ref);
}

kefir_result_t kefir_codegen_target_ir_code_schedule_of_block(const struct kefir_codegen_target_ir_code_schedule *schedule,
    kefir_codegen_target_ir_block_ref_t block_ref,
    const struct kefir_codegen_target_ir_block_schedule **block_schedule_ptr) {
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR schedule"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&schedule->blocks_by_ref, (kefir_hashtable_key_t) block_ref, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find target IR block schedule");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(block_schedule_ptr, &schedule->block_schedule[table_value]);
    return KEFIR_OK;
}
