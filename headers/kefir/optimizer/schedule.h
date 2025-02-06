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

#ifndef KEFIR_OPTIMIZER_SCHEDULE_H_
#define KEFIR_OPTIMIZER_SCHEDULE_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/list.h"
#include "kefir/core/hashtree.h"
#include "kefir/optimizer/code.h"
#include "kefir/optimizer/analysis.h"

typedef struct kefir_opt_code_instruction_schedule {
    kefir_size_t linear_index;
    struct {
        kefir_size_t begin_index;
        kefir_size_t end_index;
    } liveness_range;
} kefir_opt_code_instruction_schedule_t;

typedef struct kefir_opt_code_block_schedule {
    kefir_size_t linear_index;
    struct kefir_list instructions;
} kefir_opt_code_block_schedule_t;

typedef kefir_result_t (*kefir_opt_code_instruction_scheduler_dependency_callback_t)(kefir_opt_instruction_ref_t,
                                                                                     void *);

typedef struct kefir_opt_code_instruction_scheduler {
    kefir_result_t (*try_schedule)(kefir_opt_instruction_ref_t,
                                   kefir_opt_code_instruction_scheduler_dependency_callback_t, void *, kefir_bool_t *,
                                   void *);
    void *payload;
} kefir_opt_code_instruction_scheduler_t;

typedef struct kefir_opt_code_schedule {
    struct kefir_hashtree instructions;
    struct kefir_hashtree blocks;
    struct kefir_hashtree blocks_by_index;
    kefir_size_t next_block_index;
    kefir_size_t next_instruction_index;
} kefir_opt_code_schedule_t;

kefir_result_t kefir_opt_code_schedule_init(struct kefir_opt_code_schedule *);
kefir_result_t kefir_opt_code_schedule_free(struct kefir_mem *, struct kefir_opt_code_schedule *);

kefir_result_t kefir_opt_code_schedule_run(struct kefir_mem *, struct kefir_opt_code_schedule *,
                                           const struct kefir_opt_code_container *,
                                           const struct kefir_opt_code_analysis *,
                                           const struct kefir_opt_code_instruction_scheduler *);

kefir_result_t kefir_opt_code_schedule_of_block(const struct kefir_opt_code_schedule *, kefir_opt_block_id_t,
                                                const struct kefir_opt_code_block_schedule **);
kefir_result_t kefir_opt_code_schedule_of(const struct kefir_opt_code_schedule *, kefir_opt_instruction_ref_t,
                                          const struct kefir_opt_code_instruction_schedule **);
kefir_bool_t kefir_opt_code_schedule_has(const struct kefir_opt_code_schedule *, kefir_opt_instruction_ref_t);
kefir_bool_t kefir_opt_code_schedule_has_block(const struct kefir_opt_code_schedule *, kefir_opt_block_id_t);

kefir_size_t kefir_opt_code_schedule_num_of_blocks(const struct kefir_opt_code_schedule *);
kefir_result_t kefir_opt_code_schedule_block_by_index(const struct kefir_opt_code_schedule *, kefir_size_t,
                                                      kefir_opt_block_id_t *);

typedef struct kefir_opt_code_block_schedule_iterator {
    const struct kefir_list_entry *iter;
    const struct kefir_opt_code_block_schedule *block_schedule;
    kefir_opt_instruction_ref_t instr_ref;
} kefir_opt_code_block_schedule_iterator_t;

kefir_result_t kefir_opt_code_block_schedule_iter(const struct kefir_opt_code_schedule *, kefir_opt_block_id_t,
                                                  struct kefir_opt_code_block_schedule_iterator *);
kefir_result_t kefir_opt_code_block_schedule_next(struct kefir_opt_code_block_schedule_iterator *);

kefir_result_t kefir_opt_code_instruction_scheduler_default_schedule(
    kefir_opt_instruction_ref_t, kefir_opt_code_instruction_scheduler_dependency_callback_t, void *, kefir_bool_t *,
    void *);

#define KEFIR_OPT_CODE_INSTRUCTION_DEFAULT_SCHEDULE_INIT(_code) \
    {.try_schedule = kefir_opt_code_instruction_scheduler_default_schedule, .payload = (void *) (_code)}

#endif
