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

#ifndef KEFIR_CODEGEN_TARGET_IR_SCHEULE_H_
#define KEFIR_CODEGEN_TARGET_IR_SCHEULE_H_

#include "kefir/codegen/target-ir/code.h"

typedef struct kefir_codegen_target_ir_block_schedule {
    kefir_codegen_target_ir_block_ref_t block_ref;
    kefir_size_t linear_position;
} kefir_codegen_target_ir_block_schedule_t;

typedef struct kefir_codegen_target_ir_code_schedule {
    const struct kefir_codegen_target_ir_code *code;
    struct kefir_codegen_target_ir_block_schedule *block_schedule;
    kefir_size_t schedule_length;
    struct kefir_hashtable blocks_by_ref;
} kefir_codegen_target_ir_code_schedule_t;

typedef struct kefir_codegen_target_ir_code_schedule_builder {
    kefir_result_t (*schedule_block)(struct kefir_mem *, kefir_codegen_target_ir_block_ref_t, void *);
    void *payload;
} kefir_codegen_target_ir_code_schedule_builder_t;

typedef struct kefir_codegen_target_ir_code_scheduler {
    kefir_result_t (*do_schedule)(struct kefir_mem *, const struct kefir_codegen_target_ir_code_schedule *,
                                  struct kefir_codegen_target_ir_code_schedule_builder *,
                                  kefir_codegen_target_ir_block_ref_t, void *);
    void *payload;
} kefir_codegen_target_ir_code_scheduler_t;

kefir_result_t kefir_codegen_target_ir_code_schedule_init(struct kefir_codegen_target_ir_code_schedule *,
                                                          const struct kefir_codegen_target_ir_code *);
kefir_result_t kefir_codegen_target_ir_code_schedule_free(struct kefir_mem *,
                                                          struct kefir_codegen_target_ir_code_schedule *);
kefir_result_t kefir_codegen_target_ir_code_schedule_build(struct kefir_mem *,
                                                           struct kefir_codegen_target_ir_code_schedule *,
                                                           const struct kefir_codegen_target_ir_code_scheduler *);

kefir_bool_t kefir_codegen_target_ir_code_schedule_has_block(const struct kefir_codegen_target_ir_code_schedule *,
                                                             kefir_codegen_target_ir_block_ref_t);
kefir_result_t kefir_codegen_target_ir_code_schedule_of_block(const struct kefir_codegen_target_ir_code_schedule *,
                                                              kefir_codegen_target_ir_block_ref_t,
                                                              const struct kefir_codegen_target_ir_block_schedule **);

#endif
