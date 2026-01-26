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

#ifndef KEFIR_CODEGEN_TARGET_IR_INTERFERENCE_H_
#define KEFIR_CODEGEN_TARGET_IR_INTERFERENCE_H_

#include "kefir/codegen/target-ir/liveness.h"
#include "kefir/codegen/target-ir/control_flow.h"

typedef struct kefir_codegen_target_ir_instruction_interference {
    struct kefir_hashtable value_entries;
} kefir_codegen_target_ir_instruction_interference_t;

typedef struct kefir_codegen_target_ir_interference {
    kefir_size_t length;
    struct kefir_codegen_target_ir_instruction_interference *interference;
} kefir_codegen_target_ir_interference_t;

kefir_result_t kefir_codegen_target_ir_interference_init(struct kefir_codegen_target_ir_interference *);
kefir_result_t kefir_codegen_target_ir_interference_free(struct kefir_mem *, struct kefir_codegen_target_ir_interference *);
kefir_result_t kefir_codegen_target_ir_interference_reset(struct kefir_mem *, struct kefir_codegen_target_ir_interference *);
kefir_result_t kefir_codegen_target_ir_interference_build(struct kefir_mem *, struct kefir_codegen_target_ir_interference *, const struct kefir_codegen_target_ir_control_flow *, const struct kefir_codegen_target_ir_liveness *);

kefir_result_t kefir_codegen_target_ir_interference_has(const struct kefir_codegen_target_ir_interference *, kefir_codegen_target_ir_value_ref_t, kefir_codegen_target_ir_value_ref_t, kefir_bool_t *);

typedef struct kefir_codegen_target_ir_interference_iterator {
    void *entry;
    kefir_size_t index;
} kefir_codegen_target_ir_interference_iterator_t;

kefir_result_t kefir_codegen_target_ir_interference_iter(const struct kefir_codegen_target_ir_interference *, struct kefir_codegen_target_ir_interference_iterator *, kefir_codegen_target_ir_value_ref_t, kefir_codegen_target_ir_value_ref_t *);
kefir_result_t kefir_codegen_target_ir_interference_next(struct kefir_codegen_target_ir_interference_iterator *, kefir_codegen_target_ir_value_ref_t *);

#endif
