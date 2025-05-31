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

#ifndef KEFIR_UTIL_BIGINT_H_
#define KEFIR_UTIL_BIGINT_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"

typedef struct kefir_bigint {
    kefir_uint8_t *digits;
    kefir_size_t bitwidth;
    kefir_size_t capacity;
} kefir_bigint_t;

kefir_result_t kefir_bigint_init(struct kefir_bigint *);
kefir_result_t kefir_bigint_free(struct kefir_mem *, struct kefir_bigint *);

kefir_result_t kefir_bigint_copy(struct kefir_bigint *, const struct kefir_bigint *);
kefir_result_t kefir_bigint_copy_resize(struct kefir_mem *, struct kefir_bigint *, const struct kefir_bigint *);
kefir_result_t kefir_bigint_move(struct kefir_bigint *, struct kefir_bigint *);

kefir_result_t kefir_bigint_resize_nocast(struct kefir_mem *, struct kefir_bigint *, kefir_size_t);
kefir_result_t kefir_bigint_resize_cast_signed(struct kefir_mem *, struct kefir_bigint *, kefir_size_t);
kefir_result_t kefir_bigint_resize_cast_unsigned(struct kefir_mem *, struct kefir_bigint *, kefir_size_t);

kefir_size_t kefir_bigint_min_native_signed_width(kefir_int64_t);
kefir_size_t kefir_bigint_min_native_unsigned_width(kefir_uint64_t);
kefir_size_t kefir_bigint_min_unsigned_width(const struct kefir_bigint *);

kefir_result_t kefir_bigint_set_signed_value(struct kefir_bigint *, kefir_int64_t);
kefir_result_t kefir_bigint_set_unsigned_value(struct kefir_bigint *, kefir_uint64_t);
kefir_result_t kefir_bigint_get_signed(const struct kefir_bigint *, kefir_int64_t *);
kefir_result_t kefir_bigint_get_unsigned(const struct kefir_bigint *, kefir_uint64_t *);
kefir_result_t kefir_bigint_get_bits(const struct kefir_bigint *, kefir_size_t, kefir_size_t, kefir_uint64_t *);

kefir_result_t kefir_bigint_cast_signed(struct kefir_bigint *, kefir_size_t, kefir_size_t);
kefir_result_t kefir_bigint_cast_unsigned(struct kefir_bigint *, kefir_size_t, kefir_size_t);

kefir_result_t kefir_bigint_negate(struct kefir_bigint *);
kefir_result_t kefir_bigint_add(struct kefir_bigint *, const struct kefir_bigint *);
kefir_result_t kefir_bigint_subtract(struct kefir_bigint *, const struct kefir_bigint *);

kefir_result_t kefir_bigint_invert(struct kefir_bigint *);
kefir_result_t kefir_bigint_and(struct kefir_bigint *, const struct kefir_bigint *);
kefir_result_t kefir_bigint_or(struct kefir_bigint *, const struct kefir_bigint *);
kefir_result_t kefir_bigint_xor(struct kefir_bigint *, const struct kefir_bigint *);

kefir_result_t kefir_bigint_left_shift(struct kefir_bigint *, kefir_size_t);
kefir_result_t kefir_bigint_right_shift(struct kefir_bigint *, kefir_size_t);
kefir_result_t kefir_bigint_arithmetic_right_shift(struct kefir_bigint *, kefir_size_t);

kefir_result_t kefir_bigint_unsigned_multiply(struct kefir_bigint *, const struct kefir_bigint *,
                                              const struct kefir_bigint *, struct kefir_bigint *);
kefir_result_t kefir_bigint_signed_multiply(struct kefir_bigint *, struct kefir_bigint *, const struct kefir_bigint *,
                                            struct kefir_bigint *);

kefir_result_t kefir_bigint_unsigned_divide(struct kefir_bigint *, struct kefir_bigint *, const struct kefir_bigint *);
kefir_result_t kefir_bigint_signed_divide(struct kefir_bigint *, struct kefir_bigint *, struct kefir_bigint *);

kefir_result_t kefir_bigint_unsigned_compare(const struct kefir_bigint *, const struct kefir_bigint *, kefir_int_t *);
kefir_result_t kefir_bigint_signed_compare(const struct kefir_bigint *, const struct kefir_bigint *, kefir_int_t *);

kefir_result_t kefir_bigint_unsigned_parse10_into(struct kefir_mem *, struct kefir_bigint *, const char *,
                                                  kefir_size_t);
kefir_result_t kefir_bigint_unsigned_parse10(struct kefir_mem *, struct kefir_bigint *, const char *, kefir_size_t);

kefir_result_t kefir_bigint_unsigned_parse16_into(struct kefir_mem *, struct kefir_bigint *, const char *,
                                                  kefir_size_t);
kefir_result_t kefir_bigint_unsigned_parse16(struct kefir_mem *, struct kefir_bigint *, const char *, kefir_size_t);

kefir_result_t kefir_bigint_unsigned_parse8_into(struct kefir_mem *, struct kefir_bigint *, const char *, kefir_size_t);
kefir_result_t kefir_bigint_unsigned_parse8(struct kefir_mem *, struct kefir_bigint *, const char *, kefir_size_t);

kefir_result_t kefir_bigint_unsigned_parse2_into(struct kefir_mem *, struct kefir_bigint *, const char *, kefir_size_t);
kefir_result_t kefir_bigint_unsigned_parse2(struct kefir_mem *, struct kefir_bigint *, const char *, kefir_size_t);

kefir_result_t kefir_bigint_signed_parse(struct kefir_mem *, struct kefir_bigint *, const char *, kefir_size_t,
                                         kefir_uint_t);

kefir_result_t kefir_bigint_unsigned_format10_into(struct kefir_mem *, struct kefir_bigint *, char *, kefir_size_t);
kefir_result_t kefir_bigint_unsigned_format10(struct kefir_mem *, struct kefir_bigint *, char **, kefir_size_t *);

kefir_result_t kefir_bigint_unsigned_format16_into(const struct kefir_bigint *, char *, kefir_size_t);
kefir_result_t kefir_bigint_unsigned_format16(struct kefir_mem *, struct kefir_bigint *, char **, kefir_size_t *);

kefir_result_t kefir_bigint_unsigned_format8_into(const struct kefir_bigint *, char *, kefir_size_t);
kefir_result_t kefir_bigint_unsigned_format8(struct kefir_mem *, struct kefir_bigint *, char **, kefir_size_t *);

kefir_result_t kefir_bigint_unsigned_format2_into(const struct kefir_bigint *, char *, kefir_size_t);
kefir_result_t kefir_bigint_unsigned_format2(struct kefir_mem *, struct kefir_bigint *, char **, kefir_size_t *);

kefir_result_t kefir_bigint_signed_to_float(struct kefir_bigint *, struct kefir_bigint *, kefir_float32_t *);
kefir_result_t kefir_bigint_signed_to_double(struct kefir_bigint *, struct kefir_bigint *, kefir_float64_t *);
kefir_result_t kefir_bigint_signed_to_long_double(struct kefir_bigint *, struct kefir_bigint *, kefir_long_double_t *);

kefir_result_t kefir_bigint_unsigned_to_float(struct kefir_bigint *, struct kefir_bigint *, kefir_float32_t *);
kefir_result_t kefir_bigint_unsigned_to_double(struct kefir_bigint *, struct kefir_bigint *, kefir_float64_t *);
kefir_result_t kefir_bigint_unsigned_to_long_double(struct kefir_bigint *, struct kefir_bigint *,
                                                    kefir_long_double_t *);

kefir_result_t kefir_bigint_signed_from_float(struct kefir_bigint *, kefir_float32_t);
kefir_result_t kefir_bigint_signed_from_double(struct kefir_bigint *, kefir_float64_t);
kefir_result_t kefir_bigint_signed_from_long_double(struct kefir_bigint *, kefir_long_double_t);

kefir_result_t kefir_bigint_unsigned_from_float(struct kefir_bigint *, kefir_float32_t);
kefir_result_t kefir_bigint_unsigned_from_double(struct kefir_bigint *, kefir_float64_t);

#endif
