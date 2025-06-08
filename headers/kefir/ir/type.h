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

#ifndef KEFIR_IR_TYPE_H_
#define KEFIR_IR_TYPE_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/vector.h"
#include "kefir/core/mem.h"

typedef enum kefir_ir_typecode {
    // Aggregate types
    KEFIR_IR_TYPE_STRUCT,
    KEFIR_IR_TYPE_ARRAY,
    KEFIR_IR_TYPE_UNION,
    // Fixed scalars
    KEFIR_IR_TYPE_INT8,
    KEFIR_IR_TYPE_INT16,
    KEFIR_IR_TYPE_INT32,
    KEFIR_IR_TYPE_INT64,
    KEFIR_IR_TYPE_FLOAT32,
    KEFIR_IR_TYPE_FLOAT64,
    // Fixed complex numbers
    KEFIR_IR_TYPE_COMPLEX_FLOAT32,
    KEFIR_IR_TYPE_COMPLEX_FLOAT64,
    // Platform-dependent scalars
    KEFIR_IR_TYPE_BOOL,
    KEFIR_IR_TYPE_CHAR,
    KEFIR_IR_TYPE_SHORT,
    KEFIR_IR_TYPE_INT,
    KEFIR_IR_TYPE_LONG,
    KEFIR_IR_TYPE_WORD,
    KEFIR_IR_TYPE_LONG_DOUBLE,
    // Platform-dependent complex numbers
    KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE,
    // Bit-fields
    KEFIR_IR_TYPE_BITFIELD,
    // > 64-bit scalars are not supported yet
    KEFIR_IR_TYPE_NONE,
    KEFIR_IR_TYPE_COUNT,  // Auxilary
} kefir_ir_typecode_t;

typedef struct kefir_ir_typeentry {
    kefir_ir_typecode_t typecode;
    kefir_uint32_t alignment : 8;
    kefir_bool_t atomic;
    kefir_int64_t param;
} kefir_ir_typeentry_t;

typedef struct kefir_ir_type {
    struct kefir_vector vector;
} kefir_ir_type_t;

kefir_result_t kefir_ir_type_init(struct kefir_ir_type *, void *, kefir_size_t);
kefir_result_t kefir_ir_type_alloc(struct kefir_mem *, kefir_size_t, struct kefir_ir_type *);
kefir_result_t kefir_ir_type_realloc(struct kefir_mem *, kefir_size_t, struct kefir_ir_type *);
kefir_result_t kefir_ir_type_free(struct kefir_mem *, struct kefir_ir_type *);

kefir_result_t kefir_ir_type_append_entry(struct kefir_ir_type *, const struct kefir_ir_typeentry *);
kefir_result_t kefir_ir_type_append(struct kefir_ir_type *, kefir_ir_typecode_t, kefir_uint32_t, kefir_int64_t);
kefir_result_t kefir_ir_type_append_from(struct kefir_ir_type *, const struct kefir_ir_type *, kefir_size_t);

struct kefir_ir_typeentry *kefir_ir_type_at(const struct kefir_ir_type *, kefir_size_t);
kefir_size_t kefir_ir_type_available_entries(const struct kefir_ir_type *);
kefir_size_t kefir_ir_type_length(const struct kefir_ir_type *);
kefir_size_t kefir_ir_type_children(const struct kefir_ir_type *);
kefir_size_t kefir_ir_type_child_index(const struct kefir_ir_type *, kefir_size_t);
kefir_size_t kefir_ir_type_length_of(const struct kefir_ir_type *, kefir_size_t);
kefir_size_t kefir_ir_type_slots_of(const struct kefir_ir_type *, kefir_size_t);
kefir_size_t kefir_ir_type_slots(const struct kefir_ir_type *);
kefir_result_t kefir_ir_type_slot_of(const struct kefir_ir_type *, kefir_size_t, kefir_size_t *);

kefir_int_t kefir_ir_typeentry_compare(const struct kefir_ir_typeentry *, const struct kefir_ir_typeentry *);
kefir_result_t kefir_ir_type_same(const struct kefir_ir_type *, kefir_size_t, const struct kefir_ir_type *,
                                  kefir_size_t, kefir_bool_t *);

typedef kefir_result_t (*kefir_ir_type_visitor_callback_t)(const struct kefir_ir_type *, kefir_size_t,
                                                           const struct kefir_ir_typeentry *, void *);

typedef kefir_ir_type_visitor_callback_t kefir_ir_type_visitor_hook_t;

typedef struct kefir_ir_type_visitor {
    kefir_ir_type_visitor_callback_t visit[KEFIR_IR_TYPE_COUNT];
    kefir_ir_type_visitor_hook_t prehook;
    kefir_ir_type_visitor_hook_t posthook;
} kefir_ir_type_visitor_t;

kefir_result_t kefir_ir_type_visitor_init(struct kefir_ir_type_visitor *, kefir_ir_type_visitor_callback_t);
kefir_result_t kefir_ir_type_visitor_list_nodes(const struct kefir_ir_type *, const struct kefir_ir_type_visitor *,
                                                void *, kefir_size_t, kefir_size_t);

#define KEFIR_IR_TYPE_VISITOR_INIT_FIXED_INTEGERS(visitor, callback) \
    do {                                                             \
        (visitor)->visit[KEFIR_IR_TYPE_INT8] = (callback);           \
        (visitor)->visit[KEFIR_IR_TYPE_INT16] = (callback);          \
        (visitor)->visit[KEFIR_IR_TYPE_INT32] = (callback);          \
        (visitor)->visit[KEFIR_IR_TYPE_INT64] = (callback);          \
    } while (0)
#define KEFIR_IR_TYPE_VISITOR_INIT_ALIASED_INTEGERS(visitor, callback) \
    do {                                                               \
        (visitor)->visit[KEFIR_IR_TYPE_BOOL] = (callback);             \
        (visitor)->visit[KEFIR_IR_TYPE_CHAR] = (callback);             \
        (visitor)->visit[KEFIR_IR_TYPE_SHORT] = (callback);            \
        (visitor)->visit[KEFIR_IR_TYPE_INT] = (callback);              \
        (visitor)->visit[KEFIR_IR_TYPE_LONG] = (callback);             \
        (visitor)->visit[KEFIR_IR_TYPE_WORD] = (callback);             \
        (visitor)->visit[KEFIR_IR_TYPE_BITFIELD] = (callback);         \
    } while (0)
#define KEFIR_IR_TYPE_VISITOR_INIT_INTEGERS(visitor, callback)              \
    do {                                                                    \
        KEFIR_IR_TYPE_VISITOR_INIT_FIXED_INTEGERS((visitor), (callback));   \
        KEFIR_IR_TYPE_VISITOR_INIT_ALIASED_INTEGERS((visitor), (callback)); \
    } while (0)
#define KEFIR_IR_TYPE_VISITOR_INIT_FIXED_FP(visitor, callback) \
    do {                                                       \
        (visitor)->visit[KEFIR_IR_TYPE_FLOAT32] = (callback);  \
        (visitor)->visit[KEFIR_IR_TYPE_FLOAT64] = (callback);  \
    } while (0)
#define KEFIR_IR_TYPE_VISITOR_INIT_SCALARS(visitor, callback)       \
    do {                                                            \
        KEFIR_IR_TYPE_VISITOR_INIT_INTEGERS((visitor), (callback)); \
        KEFIR_IR_TYPE_VISITOR_INIT_FIXED_FP((visitor), (callback)); \
        (visitor)->visit[KEFIR_IR_TYPE_LONG_DOUBLE] = (callback);   \
    } while (0)
#define KEFIR_IR_TYPE_VISITOR_INIT_FIXED_COMPLEX(visitor, callback)   \
    do {                                                              \
        (visitor)->visit[KEFIR_IR_TYPE_COMPLEX_FLOAT32] = (callback); \
        (visitor)->visit[KEFIR_IR_TYPE_COMPLEX_FLOAT64] = (callback); \
    } while (0)
#define KEFIR_IR_TYPE_VISITOR_INIT_COMPLEX(visitor, callback)             \
    do {                                                                  \
        KEFIR_IR_TYPE_VISITOR_INIT_FIXED_COMPLEX(visitor, callback);      \
        (visitor)->visit[KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE] = (callback); \
    } while (0)

#define KEFIR_IR_BITFIELD_PARAM(_width, _base_size) \
    ((((kefir_uint64_t) (_width)) << 32) | ((kefir_uint32_t) (_base_size)))
#define KEFIR_IR_BITFIELD_PARAM_GET_WIDTH(_param) (((kefir_uint64_t) (_param)) >> 32)
#define KEFIR_IR_BITFIELD_PARAM_GET_BASE_SIZE(_param) (((kefir_uint32_t) (_param)))

#endif
