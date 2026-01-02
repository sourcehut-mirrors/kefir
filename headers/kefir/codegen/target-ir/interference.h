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

#ifndef KEFIR_CODEGEN_TARGET_IR_INTERFERENCE_H_
#define KEFIR_CODEGEN_TARGET_IR_INTERFERENCE_H_

#include "kefir/codegen/target-ir/liveness.h"
#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/core/graph.h"

typedef struct kefir_codegen_target_ir_interference {
    struct kefir_graph interference_graph;
} kefir_codegen_target_ir_interference_t;

kefir_result_t kefir_codegen_target_ir_interference_init(struct kefir_codegen_target_ir_interference *);
kefir_result_t kefir_codegen_target_ir_interference_free(struct kefir_mem *, struct kefir_codegen_target_ir_interference *);
kefir_result_t kefir_codegen_target_ir_interference_reset(struct kefir_mem *, struct kefir_codegen_target_ir_interference *);
kefir_result_t kefir_codegen_target_ir_interference_build(struct kefir_mem *, struct kefir_codegen_target_ir_interference *, const struct kefir_codegen_target_ir_control_flow *, const struct kefir_codegen_target_ir_liveness *);

kefir_result_t kefir_codegen_target_ir_interference_has(const struct kefir_codegen_target_ir_interference *, kefir_codegen_target_ir_value_ref_t, kefir_codegen_target_ir_value_ref_t, kefir_bool_t *);

typedef struct kefir_codegen_target_ir_interference_liveness_index {
    struct kefir_hashset begin_liveness;
    struct kefir_hashset end_liveness;
} kefir_codegen_target_ir_interference_liveness_index_t;

kefir_result_t kefir_codegen_target_ir_interference_build_per_block_liveness(struct kefir_mem *, const struct kefir_codegen_target_ir_control_flow *, const struct kefir_codegen_target_ir_liveness *, kefir_codegen_target_ir_block_ref_t, struct kefir_hashtree *);
kefir_result_t kefir_codegen_target_ir_interference_build_update_alive_set(struct kefir_mem *, kefir_codegen_target_ir_instruction_ref_t, struct kefir_hashtree *, struct kefir_hashset *);
kefir_result_t kefir_codegen_target_ir_interference_free_liveness_index(struct kefir_mem *, struct kefir_hashtree *, kefir_hashtree_key_t, kefir_hashtree_value_t, void *);

#endif
