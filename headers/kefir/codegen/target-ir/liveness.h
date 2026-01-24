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

#ifndef KEFIR_CODEGEN_TARGET_IR_LIVENESS_H_
#define KEFIR_CODEGEN_TARGET_IR_LIVENESS_H_

#include "kefir/codegen/target-ir/control_flow.h"
#include "kefir/codegen/target-ir/numbering.h"

typedef struct kefir_codegen_target_ir_liveness_entry {
    kefir_codegen_target_ir_value_ref_t *content;
    kefir_size_t length;
    kefir_size_t capacity;
} kefir_codegen_target_ir_liveness_entry_t;

typedef struct kefir_codegen_target_ir_liveness_index {
    struct kefir_hashset begin_liveness;
    struct kefir_hashset end_liveness;
} kefir_codegen_target_ir_liveness_index_t;
typedef struct kefir_codegen_target_ir_liveness_value_block_ranges {
    kefir_size_t length;
    struct kefir_codegen_target_ir_liveness_index null_index;
    struct kefir_codegen_target_ir_liveness_index *indices;
} kefir_codegen_target_ir_liveness_value_block_ranges_t;

typedef struct kefir_codegen_target_ir_block_liveness {
    struct kefir_codegen_target_ir_liveness_entry live_in;
    struct kefir_codegen_target_ir_liveness_entry live_out;
    kefir_bool_t value_liveness_ranges_ready;
    struct kefir_codegen_target_ir_liveness_value_block_ranges value_liveness_ranges;
} kefir_codegen_target_ir_block_liveness_t;

typedef struct kefir_codegen_target_ir_value_liveness {
    struct kefir_hashtable per_block;
} kefir_codegen_target_ir_value_liveness_t;

typedef struct kefir_codegen_target_ir_liveness {
    const struct kefir_codegen_target_ir_code *code;
    struct kefir_codegen_target_ir_numbering numbering;
    struct kefir_codegen_target_ir_block_liveness *blocks;
    struct kefir_hashtable values;
} kefir_codegen_target_ir_liveness_t;

kefir_result_t kefir_codegen_target_ir_liveness_init(struct kefir_codegen_target_ir_liveness *);
kefir_result_t kefir_codegen_target_ir_liveness_free(struct kefir_mem *, struct kefir_codegen_target_ir_liveness *);
kefir_result_t kefir_codegen_target_ir_liveness_reset(struct kefir_mem *, struct kefir_codegen_target_ir_liveness *);

typedef enum kefir_codegen_target_ir_liveness_class {
    KEFIR_CODEGEN_TARGET_IR_LIVENESS_NORMAL = 0,
    KEFIR_CODEGEN_TARGET_IR_LIVENESS_UPSILON,
    KEFIR_CODEGEN_TARGET_IR_LIVENESS_END
} kefir_codegen_target_ir_liveness_class_t;

kefir_result_t kefir_codegen_target_ir_liveness_build(struct kefir_mem *, const struct kefir_codegen_target_ir_control_flow *, struct kefir_codegen_target_ir_liveness *);
kefir_result_t kefir_codegen_target_ir_liveness_value_at(const struct kefir_codegen_target_ir_liveness *, kefir_codegen_target_ir_value_ref_t, kefir_codegen_target_ir_block_ref_t, kefir_codegen_target_ir_liveness_class_t, kefir_codegen_target_ir_instruction_ref_t *, kefir_codegen_target_ir_instruction_ref_t *);
kefir_result_t kefir_codegen_target_ir_liveness_value_ranges(struct kefir_mem *, const struct kefir_codegen_target_ir_control_flow *, const struct kefir_codegen_target_ir_liveness *, kefir_codegen_target_ir_block_ref_t, const struct kefir_codegen_target_ir_liveness_value_block_ranges **);
kefir_result_t kefir_codegen_target_ir_liveness_build_update_alive_set(struct kefir_mem *, const struct kefir_codegen_target_ir_liveness *, kefir_codegen_target_ir_instruction_ref_t, const struct kefir_codegen_target_ir_liveness_value_block_ranges *, struct kefir_hashset *);
kefir_result_t kefir_codegen_target_ir_liveness_range_get(const struct kefir_codegen_target_ir_liveness *, const struct kefir_codegen_target_ir_liveness_value_block_ranges *,
    kefir_codegen_target_ir_instruction_ref_t, const struct kefir_codegen_target_ir_liveness_index **);

typedef struct kefir_codegen_target_ir_value_liveness_block_iterator {
    const struct kefir_codegen_target_ir_liveness *liveness;
    kefir_codegen_target_ir_value_ref_t value_ref;
    kefir_codegen_target_ir_block_ref_t block_ref;
    kefir_uint32_t klass;
} kefir_codegen_target_ir_value_liveness_block_iterator_t;

kefir_result_t kefir_codegen_target_ir_value_liveness_at(const struct kefir_codegen_target_ir_liveness *, struct kefir_codegen_target_ir_value_liveness_block_iterator *, kefir_codegen_target_ir_value_ref_t, kefir_codegen_target_ir_block_ref_t, kefir_codegen_target_ir_instruction_ref_t *, kefir_codegen_target_ir_instruction_ref_t *);
kefir_result_t kefir_codegen_target_ir_value_liveness_at_next(struct kefir_codegen_target_ir_value_liveness_block_iterator *, kefir_codegen_target_ir_instruction_ref_t *, kefir_codegen_target_ir_instruction_ref_t *);

typedef struct kefir_codegen_target_ir_value_liveness_iterator {
    struct kefir_hashtable_iterator iter;
} kefir_codegen_target_ir_value_liveness_iterator_t;

kefir_result_t kefir_codegen_target_ir_value_liveness_iter(const struct kefir_codegen_target_ir_liveness *, struct kefir_codegen_target_ir_value_liveness_iterator *, kefir_codegen_target_ir_value_ref_t, kefir_codegen_target_ir_block_ref_t *, kefir_codegen_target_ir_instruction_ref_t *, kefir_codegen_target_ir_instruction_ref_t *);
kefir_result_t kefir_codegen_target_ir_value_liveness_next(struct kefir_codegen_target_ir_value_liveness_iterator *, kefir_codegen_target_ir_block_ref_t *, kefir_codegen_target_ir_instruction_ref_t *, kefir_codegen_target_ir_instruction_ref_t *);

#endif
