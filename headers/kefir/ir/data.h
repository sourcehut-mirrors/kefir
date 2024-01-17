/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#ifndef KEFIR_IR_DATA_H_
#define KEFIR_IR_DATA_H_

#include <stdbool.h>
#include "kefir/ir/type.h"
#include "kefir/core/block_tree.h"
#include "kefir/core/mem.h"

typedef enum kefir_ir_data_storage {
    KEFIR_IR_DATA_GLOBAL_STORAGE,
    KEFIR_IR_DATA_THREAD_LOCAL_STORAGE
} kefir_ir_data_storage_t;

typedef struct kefir_ir_data {
    kefir_ir_data_storage_t storage;
    kefir_id_t type_id;
    const struct kefir_ir_type *type;
    kefir_size_t total_length;
    struct kefir_block_tree value_tree;
    kefir_bool_t finalized;
    kefir_bool_t defined;
} kefir_ir_data_t;

typedef enum kefir_ir_string_literal_type {
    KEFIR_IR_STRING_LITERAL_MULTIBYTE,
    KEFIR_IR_STRING_LITERAL_UNICODE16,
    KEFIR_IR_STRING_LITERAL_UNICODE32
} kefir_ir_string_literal_type_t;

typedef enum kefir_ir_data_value_type {
    KEFIR_IR_DATA_VALUE_UNDEFINED,
    KEFIR_IR_DATA_VALUE_INTEGER,
    KEFIR_IR_DATA_VALUE_FLOAT32,
    KEFIR_IR_DATA_VALUE_FLOAT64,
    KEFIR_IR_DATA_VALUE_LONG_DOUBLE,
    KEFIR_IR_DATA_VALUE_COMPLEX_FLOAT32,
    KEFIR_IR_DATA_VALUE_COMPLEX_FLOAT64,
    KEFIR_IR_DATA_VALUE_COMPLEX_LONG_DOUBLE,
    KEFIR_IR_DATA_VALUE_STRING,
    KEFIR_IR_DATA_VALUE_POINTER,
    KEFIR_IR_DATA_VALUE_STRING_POINTER,
    KEFIR_IR_DATA_VALUE_RAW,
    KEFIR_IR_DATA_VALUE_AGGREGATE
} kefir_ir_data_value_type_t;

typedef struct kefir_ir_data_value {
    kefir_ir_data_value_type_t type;
    kefir_bool_t defined;
    union {
        kefir_int64_t integer;
        kefir_float32_t float32;
        kefir_float64_t float64;
        kefir_long_double_t long_double;
        struct {
            kefir_float32_t real;
            kefir_float32_t imaginary;
        } complex_float32;
        struct {
            kefir_float64_t real;
            kefir_float64_t imaginary;
        } complex_float64;
        struct {
            kefir_long_double_t real;
            kefir_long_double_t imaginary;
        } complex_long_double;
        struct {
            const char *reference;
            kefir_int64_t offset;
        } pointer;
        struct {
            const void *data;
            kefir_size_t length;
        } raw;
        struct {
            kefir_id_t id;
            kefir_int64_t offset;
        } string_ptr;
    } value;
} kefir_ir_data_value_t;

kefir_result_t kefir_ir_data_alloc(struct kefir_mem *, kefir_ir_data_storage_t, const struct kefir_ir_type *,
                                   kefir_id_t, struct kefir_ir_data *);

kefir_result_t kefir_ir_data_free(struct kefir_mem *, struct kefir_ir_data *);

kefir_result_t kefir_ir_data_set_integer(struct kefir_mem *, struct kefir_ir_data *, kefir_size_t, kefir_int64_t);

kefir_result_t kefir_ir_data_set_bitfield(struct kefir_mem *, struct kefir_ir_data *, kefir_size_t, kefir_uint64_t,
                                          kefir_size_t, kefir_size_t);

kefir_result_t kefir_ir_data_set_float32(struct kefir_mem *, struct kefir_ir_data *, kefir_size_t, kefir_float32_t);

kefir_result_t kefir_ir_data_set_float64(struct kefir_mem *, struct kefir_ir_data *, kefir_size_t, kefir_float64_t);

kefir_result_t kefir_ir_data_set_long_double(struct kefir_mem *, struct kefir_ir_data *, kefir_size_t,
                                             kefir_long_double_t);

kefir_result_t kefir_ir_data_set_complex_float32(struct kefir_mem *, struct kefir_ir_data *, kefir_size_t,
                                                 kefir_float32_t, kefir_float32_t);

kefir_result_t kefir_ir_data_set_complex_float64(struct kefir_mem *, struct kefir_ir_data *, kefir_size_t,
                                                 kefir_float64_t, kefir_float64_t);

kefir_result_t kefir_ir_data_set_complex_long_double(struct kefir_mem *, struct kefir_ir_data *, kefir_size_t,
                                                     kefir_long_double_t, kefir_long_double_t);

kefir_result_t kefir_ir_data_set_string(struct kefir_mem *, struct kefir_ir_data *, kefir_size_t,
                                        kefir_ir_string_literal_type_t, const void *, kefir_size_t);

kefir_result_t kefir_ir_data_set_pointer(struct kefir_mem *, struct kefir_ir_data *, kefir_size_t, const char *,
                                         kefir_size_t);

kefir_result_t kefir_ir_data_set_string_pointer(struct kefir_mem *, struct kefir_ir_data *, kefir_size_t, kefir_id_t,
                                                kefir_int64_t);

kefir_result_t kefir_ir_data_set_raw(struct kefir_mem *, struct kefir_ir_data *, kefir_size_t, const void *,
                                     kefir_size_t);

kefir_result_t kefir_ir_data_finalize(struct kefir_ir_data *);

kefir_result_t kefir_ir_data_value_at(const struct kefir_ir_data *, kefir_size_t, const struct kefir_ir_data_value **);

typedef struct kefir_ir_data_map_iterator {
    struct kefir_block_tree_iterator value_iter;
    kefir_size_t has_mapped_values;
    kefir_size_t next_mapped_slot;
} kefir_ir_data_map_iterator_t;

kefir_result_t kefir_ir_data_map_iter(const struct kefir_ir_data *, struct kefir_ir_data_map_iterator *);
kefir_result_t kefir_ir_data_map_next(struct kefir_ir_data_map_iterator *);
kefir_result_t kefir_ir_data_map_skip_to(const struct kefir_ir_data *, struct kefir_ir_data_map_iterator *,
                                         kefir_size_t);

#endif
