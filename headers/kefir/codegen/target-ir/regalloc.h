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

#ifndef KEFIR_CODEGEN_TARGET_IR_REGALLOC_H_
#define KEFIR_CODEGEN_TARGET_IR_REGALLOC_H_

#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/codegen/target-ir/liveness.h"
#include "kefir/codegen/target-ir/interference.h"
#include "kefir/core/hashtable.h"

typedef kefir_uint64_t kefir_codegen_target_ir_regalloc_allocation_t;

typedef struct kefir_codegen_target_ir_regalloc {
    struct kefir_hashtable allocation;
} kefir_codegen_target_ir_regalloc_t;

kefir_result_t kefir_codegen_target_ir_regalloc_init(struct kefir_codegen_target_ir_regalloc *);
kefir_result_t kefir_codegen_target_ir_regalloc_free(struct kefir_mem *, struct kefir_codegen_target_ir_regalloc *);

typedef struct kefir_codegen_target_ir_regalloc_class {
    kefir_result_t (*decode_constraint)(const struct kefir_codegen_target_ir_value_type *, kefir_codegen_target_ir_regalloc_allocation_t *, void *);
    kefir_result_t (*do_allocate)(struct kefir_mem *, const struct kefir_codegen_target_ir_value_type *, const struct kefir_hashset *, const struct kefir_hashset *, kefir_codegen_target_ir_regalloc_allocation_t *, void *);
    void *payload;
} kefir_codegen_target_ir_regalloc_class_t;

kefir_result_t kefir_codegen_target_ir_regalloc_run(struct kefir_mem *, struct kefir_codegen_target_ir_regalloc *,
    const struct kefir_codegen_target_ir_control_flow *, const struct kefir_codegen_target_ir_interference *,
    const struct kefir_codegen_target_ir_regalloc_class *);

#endif
