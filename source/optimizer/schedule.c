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

#include "kefir/optimizer/schedule.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_opt_code_schedule_init(struct kefir_opt_code_schedule *schedule) {
    REQUIRE(schedule != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code schedule"));

    schedule->blocks = NULL;
    schedule->blocks_length = 0;
    schedule->blocks_capacity = 0;
    REQUIRE_OK(kefir_hashtable_init(&schedule->instructions_by_ref, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&schedule->blocks_by_ref, &kefir_hashtable_uint_ops));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_schedule_free(struct kefir_mem *mem, struct kefir_opt_code_schedule *schedule) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code schedule"));

    for (kefir_size_t i = 0; i < schedule->blocks_length; i++) {
        KEFIR_FREE(mem, schedule->blocks[i].instructions);
    }
    KEFIR_FREE(mem, schedule->blocks);
    REQUIRE_OK(kefir_hashtable_free(mem, &schedule->instructions_by_ref));
    REQUIRE_OK(kefir_hashtable_free(mem, &schedule->blocks_by_ref));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_schedule_clear(struct kefir_mem *mem, struct kefir_opt_code_schedule *schedule) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code schedule"));

    for (kefir_size_t i = 0; i < schedule->blocks_length; i++) {
        KEFIR_FREE(mem, schedule->blocks[i].instructions);
    }
    KEFIR_FREE(mem, schedule->blocks);
    schedule->blocks = NULL;
    schedule->blocks_length = 0;
    schedule->blocks_capacity = 0;
    REQUIRE_OK(kefir_hashtable_clear(mem, &schedule->instructions_by_ref));
    REQUIRE_OK(kefir_hashtable_clear(mem, &schedule->blocks_by_ref));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_schedule_of_block(const struct kefir_opt_code_schedule *schedule,
                                                kefir_opt_block_id_t block_id,
                                                const struct kefir_opt_code_block_schedule **block_schedule) {
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code schedule"));
    REQUIRE(block_schedule != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer block schedule"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&schedule->blocks_by_ref, (kefir_hashtable_key_t) block_id, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested optimizer block schedule");
    }
    REQUIRE_OK(res);

    *block_schedule = &schedule->blocks[table_value];
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_schedule_of(const struct kefir_opt_code_schedule *schedule,
                                          kefir_opt_instruction_ref_t instr_ref,
                                          const struct kefir_opt_code_instruction_schedule **instr_schedule) {
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code schedule"));
    REQUIRE(instr_schedule != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction schedule"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&schedule->instructions_by_ref, (kefir_hashtable_key_t) instr_ref, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested optimizer instruction schedule");
    }
    REQUIRE_OK(res);

    kefir_uint32_t block_linear_position = table_value >> 32;
    kefir_uint32_t linear_position = (kefir_uint32_t) table_value;

    *instr_schedule = &schedule->blocks[block_linear_position].instructions[linear_position];
    return KEFIR_OK;
}


kefir_result_t kefir_opt_code_schedule_at(const struct kefir_opt_code_schedule *schedule, kefir_uint32_t block_linear_position, kefir_uint32_t linear_position,
                                          kefir_opt_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code schedule"));
    REQUIRE(block_linear_position < schedule->blocks_length, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested optimizer instruction schedule"));
    REQUIRE(linear_position < schedule->blocks[block_linear_position].instructions_length, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested optimizer instruction schedule"));
    REQUIRE(instr_ref_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction reference"));

    *instr_ref_ptr = schedule->blocks[block_linear_position].instructions[linear_position].instr_ref;
    return KEFIR_OK;
}

kefir_bool_t kefir_opt_code_schedule_has(const struct kefir_opt_code_schedule *schedule,
                                         kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(schedule != NULL, false);
    return kefir_hashtable_has(&schedule->instructions_by_ref, (kefir_hashtable_key_t) instr_ref);
}

kefir_bool_t kefir_opt_code_schedule_has_block(const struct kefir_opt_code_schedule *schedule,
                                               kefir_opt_block_id_t block_id) {
    REQUIRE(schedule != NULL, false);
    return kefir_hashtable_has(&schedule->blocks_by_ref, (kefir_hashtable_key_t) block_id);
}

kefir_size_t kefir_opt_code_schedule_num_of_blocks(const struct kefir_opt_code_schedule *schedule) {
    REQUIRE(schedule != NULL, 0);
    return schedule->blocks_length;
}

kefir_result_t kefir_opt_code_schedule_block_by_index(const struct kefir_opt_code_schedule *schedule,
                                                      kefir_size_t index, kefir_opt_block_id_t *block_id_ptr) {
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code schedule"));
    REQUIRE(index < schedule->blocks_length, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested optimizer schedule block"));

    ASSIGN_PTR(block_id_ptr, schedule->blocks[index].block_id);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_schedule_iter(const struct kefir_opt_code_schedule *schedule,
                                                  kefir_opt_block_id_t block_id,
                                                  struct kefir_opt_code_block_schedule_iterator *iter) {
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code schedule"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                          "Expected valid pointer to optimizer code block schedule iterator"));

    REQUIRE_OK(kefir_opt_code_schedule_of_block(schedule, block_id, &iter->block_schedule));
    iter->index = 0;
    if (iter->index < iter->block_schedule->instructions_length) {
        iter->instr_ref = (kefir_opt_instruction_ref_t) iter->block_schedule->instructions[iter->index].instr_ref;
    } else {
        iter->instr_ref = KEFIR_ID_NONE;
        return KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of optimizer block schedule iterator");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_schedule_next(struct kefir_opt_code_block_schedule_iterator *iter) {
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block schedule iterator"));

    iter->index++;
    if (iter->index < iter->block_schedule->instructions_length) {
        iter->instr_ref = (kefir_opt_instruction_ref_t) iter->block_schedule->instructions[iter->index].instr_ref;
    } else {
        iter->instr_ref = KEFIR_ID_NONE;
        return KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of optimizer block schedule iterator");
    }
    return KEFIR_OK;
}

struct builder_payload {
    struct kefir_opt_code_schedule *schedule;
    const struct kefir_opt_code_container *code;
    const struct kefir_opt_code_analysis *code_analysis;
};

static kefir_result_t builder_schedule_block(struct kefir_mem *mem, kefir_opt_block_id_t block_id, kefir_uint32_t *linear_index_ptr, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct builder_payload  *, builder_payload, payload);
    REQUIRE(builder_payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer schedule builder payload"));

    REQUIRE(!kefir_hashtable_has(&builder_payload->schedule->blocks_by_ref, (kefir_hashtable_key_t) block_id), KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Optimizer block has altrady been scheduled"));

    if (builder_payload->schedule->blocks_length >= builder_payload->schedule->blocks_capacity) {
        kefir_size_t new_capacity = MAX(builder_payload->schedule->blocks_capacity * 2, 128);
        struct kefir_opt_code_block_schedule *new_blocks = KEFIR_REALLOC(mem, builder_payload->schedule->blocks, sizeof(struct kefir_opt_code_block_schedule) * new_capacity);
        REQUIRE(new_blocks != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer block schedule"));
        builder_payload->schedule->blocks = new_blocks;
        builder_payload->schedule->blocks_capacity = new_capacity;
    }
    
    struct kefir_opt_code_block_schedule *block_schedule = &builder_payload->schedule->blocks[builder_payload->schedule->blocks_length];
    block_schedule->block_id = block_id;
    block_schedule->linear_position = builder_payload->schedule->blocks_length;
    block_schedule->instructions = NULL;
    block_schedule->instructions_length = 0;
    block_schedule->instructions_capacity = 0;

    REQUIRE_OK(kefir_hashtable_insert(mem, &builder_payload->schedule->blocks_by_ref, (kefir_hashtable_key_t) block_id, (kefir_hashtable_value_t) block_schedule->linear_position));
    builder_payload->schedule->blocks_length++;
    ASSIGN_PTR(linear_index_ptr, block_schedule->linear_position);
    return KEFIR_OK;
}

static kefir_result_t builder_schedule_instruction(struct kefir_mem *mem, kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t instr_ref, kefir_uint32_t *linear_index_ptr, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct builder_payload  *, builder_payload, payload);
    REQUIRE(builder_payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer schedule builder payload"));

    REQUIRE(!kefir_hashtable_has(&builder_payload->schedule->instructions_by_ref, (kefir_hashtable_key_t) instr_ref), KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Optimizer instruction has altrady been scheduled"));
    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&builder_payload->schedule->blocks_by_ref, (kefir_hashtable_key_t) block_id, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find optimizer block schedule");
    }
    REQUIRE_OK(res);
    struct kefir_opt_code_block_schedule *block_schedule = &builder_payload->schedule->blocks[table_value];

    if (block_schedule->instructions_length >= block_schedule->instructions_capacity) {
        kefir_size_t new_capacity = MAX(block_schedule->instructions_capacity * 2, 128);
        struct kefir_opt_code_instruction_schedule *new_instructions = KEFIR_REALLOC(mem, block_schedule->instructions, sizeof(struct kefir_opt_code_instruction_schedule) * new_capacity);
        REQUIRE(new_instructions != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer instruction schedule"));
        block_schedule->instructions = new_instructions;
        block_schedule->instructions_capacity = new_capacity;
    }
    
    struct kefir_opt_code_instruction_schedule *instruction_schedule = &block_schedule->instructions[block_schedule->instructions_length];
    instruction_schedule->block_position = block_schedule->linear_position;
    instruction_schedule->linear_position = block_schedule->instructions_length;
    instruction_schedule->instr_ref = instr_ref;

    table_value = (((kefir_uint64_t) block_schedule->linear_position) << 32) | (kefir_uint32_t) instruction_schedule->linear_position;
    REQUIRE_OK(kefir_hashtable_insert(mem, &builder_payload->schedule->instructions_by_ref, (kefir_hashtable_key_t) instr_ref, table_value));
    block_schedule->instructions_length++;
    ASSIGN_PTR(linear_index_ptr, instruction_schedule->linear_position);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_schedule_run(struct kefir_mem *mem, struct kefir_opt_code_schedule *schedule,
                                           const struct kefir_opt_code_container *code,
                                           const struct kefir_opt_code_analysis *code_analysis,
                                           const struct kefir_opt_code_scheduler *scheduler) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(schedule != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code schedule"));
    REQUIRE(schedule->blocks == NULL, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Schedule has already been constructed"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(code_analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));
    REQUIRE(scheduler != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code scheduler"));

    struct builder_payload payload = {
        .code = code,
        .code_analysis = code_analysis,
        .schedule = schedule
    };
    struct kefir_opt_code_schedule_builder builder = {
        .schedule_block = builder_schedule_block,
        .schedule_instruction = builder_schedule_instruction,
        .payload = &payload
    };
    REQUIRE_OK(scheduler->do_schedule(mem, code, code_analysis, schedule, &builder, scheduler->payload));
    return KEFIR_OK;
}
