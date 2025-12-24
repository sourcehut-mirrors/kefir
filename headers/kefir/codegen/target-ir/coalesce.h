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

#ifndef KEFIR_CODEGEN_TARGET_IR_COALESCE_H_
#define KEFIR_CODEGEN_TARGET_IR_COALESCE_H_

#include "kefir/codegen/target-ir/code.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/codegen/target-ir/interference.h"

typedef struct kefir_codegen_target_ir_coalesce {
    struct kefir_graph coalesce_graph;
} kefir_codegen_target_ir_coalesce_t;

kefir_result_t kefir_codegen_target_ir_coalesce_init(struct kefir_codegen_target_ir_coalesce *);
kefir_result_t kefir_codegen_target_ir_coalesce_free(struct kefir_mem *, struct kefir_codegen_target_ir_coalesce *);

typedef struct kefir_codegen_target_ir_coalesce_callback {
    kefir_result_t (*try_coalesce)(struct kefir_mem *, kefir_codegen_target_ir_value_ref_t, kefir_codegen_target_ir_value_ref_t, void *);
    void *payload;
} kefir_codegen_target_ir_coalesce_callback_t;

typedef struct kefir_codegen_target_ir_coalesce_class {
    kefir_result_t (*extract_coalesce)(struct kefir_mem *, const struct kefir_codegen_target_ir_code *, const struct kefir_codegen_target_ir_instruction *, const struct kefir_codegen_target_ir_coalesce_callback *, void *);
    void *payload;
} kefir_codegen_target_ir_coalesce_class_t;

kefir_result_t kefir_codegen_target_ir_coalesce_build(struct kefir_mem *, struct kefir_codegen_target_ir_coalesce *,
    const struct kefir_codegen_target_ir_control_flow *,
    const struct kefir_codegen_target_ir_interference *,
    const struct kefir_codegen_target_ir_coalesce_class *);

typedef struct kefir_codegen_target_ir_coalesce_iterator {
    struct kefir_graph_edge_iterator iter;
} kefir_codegen_target_ir_coalesce_iterator_t;

kefir_result_t kefir_codegen_target_ir_coalesce_iter(const struct kefir_codegen_target_ir_coalesce *,
    struct kefir_codegen_target_ir_coalesce_iterator *,
    kefir_codegen_target_ir_value_ref_t,
    kefir_codegen_target_ir_value_ref_t *);
kefir_result_t kefir_codegen_target_ir_coalesce_next(struct kefir_codegen_target_ir_coalesce_iterator *,
    kefir_codegen_target_ir_value_ref_t *);

#endif
