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

#ifndef KEFIR_IR_INSTR_H_
#define KEFIR_IR_INSTR_H_

#include "kefir/core/basic-types.h"
#include "kefir/ir/opcodes.h"
#include "kefir/core/vector.h"
#include "kefir/core/mem.h"
#include "kefir/core/hashtree.h"
#include "kefir/core/string_pool.h"

#define KEFIR_IR_MEMORY_FLAG_NONE 0
#define KEFIR_IR_MEMORY_FLAG_VOLATILE 1

#define KEFIR_IR_MEMORY_ORDER_SEQ_CST 1

enum {
    KEFIR_IR_BRANCH_CONDITION_8BIT,
    KEFIR_IR_BRANCH_CONDITION_16BIT,
    KEFIR_IR_BRANCH_CONDITION_32BIT,
    KEFIR_IR_BRANCH_CONDITION_64BIT
};

enum {
    KEFIR_IR_COMPARE_INT8_EQUALS,
    KEFIR_IR_COMPARE_INT16_EQUALS,
    KEFIR_IR_COMPARE_INT32_EQUALS,
    KEFIR_IR_COMPARE_INT64_EQUALS,
    KEFIR_IR_COMPARE_INT8_GREATER,
    KEFIR_IR_COMPARE_INT16_GREATER,
    KEFIR_IR_COMPARE_INT32_GREATER,
    KEFIR_IR_COMPARE_INT64_GREATER,
    KEFIR_IR_COMPARE_INT8_LESSER,
    KEFIR_IR_COMPARE_INT16_LESSER,
    KEFIR_IR_COMPARE_INT32_LESSER,
    KEFIR_IR_COMPARE_INT64_LESSER,
    KEFIR_IR_COMPARE_INT8_ABOVE,
    KEFIR_IR_COMPARE_INT16_ABOVE,
    KEFIR_IR_COMPARE_INT32_ABOVE,
    KEFIR_IR_COMPARE_INT64_ABOVE,
    KEFIR_IR_COMPARE_INT8_BELOW,
    KEFIR_IR_COMPARE_INT16_BELOW,
    KEFIR_IR_COMPARE_INT32_BELOW,
    KEFIR_IR_COMPARE_INT64_BELOW,
    KEFIR_IR_COMPARE_FLOAT32_EQUALS,
    KEFIR_IR_COMPARE_FLOAT32_GREATER,
    KEFIR_IR_COMPARE_FLOAT32_LESSER,
    KEFIR_IR_COMPARE_FLOAT64_EQUALS,
    KEFIR_IR_COMPARE_FLOAT64_GREATER,
    KEFIR_IR_COMPARE_FLOAT64_LESSER
};

typedef struct kefir_irinstr {
    kefir_iropcode_t opcode;
    union {
        kefir_int64_t i64;
        kefir_uint64_t u64;
        kefir_uint64_t u64_2[2];
        kefir_int32_t i32[4];
        kefir_uint32_t u32[4];
        kefir_float64_t f64;
        kefir_float32_t f32[4];
        kefir_long_double_t long_double;
    } arg;
} kefir_irinstr_t;

typedef struct kefir_irblock {
    struct kefir_vector content;
    struct kefir_hashtree public_labels;
} kefir_irblock_t;

kefir_size_t kefir_irblock_available(const struct kefir_irblock *);
kefir_size_t kefir_irblock_length(const struct kefir_irblock *);
struct kefir_irinstr *kefir_irblock_at(const struct kefir_irblock *, kefir_size_t);
kefir_result_t kefir_irblock_public_labels_iter(const struct kefir_irblock *, struct kefir_hashtree_node_iterator *,
                                                const char **, kefir_size_t *);
kefir_result_t kefir_irblock_public_labels_next(struct kefir_hashtree_node_iterator *, const char **, kefir_size_t *);
kefir_result_t kefir_irblock_appendi64(struct kefir_irblock *, kefir_iropcode_t, kefir_int64_t);
kefir_result_t kefir_irblock_appendu64(struct kefir_irblock *, kefir_iropcode_t, kefir_uint64_t);
kefir_result_t kefir_irblock_appendu64_2(struct kefir_irblock *, kefir_iropcode_t, kefir_uint64_t, kefir_uint64_t);
kefir_result_t kefir_irblock_appendi32(struct kefir_irblock *, kefir_iropcode_t, kefir_int32_t, kefir_int32_t);
kefir_result_t kefir_irblock_appendu32(struct kefir_irblock *, kefir_iropcode_t, kefir_uint32_t, kefir_uint32_t);
kefir_result_t kefir_irblock_appendu32_4(struct kefir_irblock *, kefir_iropcode_t, kefir_uint32_t, kefir_uint32_t,
                                         kefir_uint32_t, kefir_uint32_t);
kefir_result_t kefir_irblock_appendf64(struct kefir_irblock *, kefir_iropcode_t, kefir_float64_t);
kefir_result_t kefir_irblock_appendf32(struct kefir_irblock *, kefir_iropcode_t, kefir_float32_t, kefir_float32_t);
kefir_result_t kefir_irblock_append_ldouble(struct kefir_irblock *, kefir_iropcode_t, kefir_long_double_t);
kefir_result_t kefir_irblock_add_public_label(struct kefir_mem *, struct kefir_irblock *, struct kefir_string_pool *,
                                              const char *, kefir_size_t);
kefir_result_t kefir_irblock_copy(struct kefir_irblock *, const struct kefir_irblock *);
kefir_result_t kefir_irblock_alloc(struct kefir_mem *, kefir_size_t, struct kefir_irblock *);
kefir_result_t kefir_irblock_realloc(struct kefir_mem *, kefir_size_t, struct kefir_irblock *);
kefir_result_t kefir_irblock_free(struct kefir_mem *, struct kefir_irblock *);

kefir_uint64_t kefir_ir_long_double_upper_half(kefir_long_double_t);
kefir_uint64_t kefir_ir_long_double_lower_half(kefir_long_double_t);
kefir_long_double_t kefir_ir_long_double_construct(kefir_uint64_t, kefir_uint64_t);

#endif
