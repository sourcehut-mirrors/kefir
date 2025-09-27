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

#ifndef KEFIR_UTIL_DFP_H_
#define KEFIR_UTIL_DFP_H_

#include "kefir/core/basic-types.h"
#include "kefir/core/mem.h"

typedef struct kefir_dfp_decimal32 {
    kefir_uint32_t uvalue;
} kefir_dfp_decimal32_t;

typedef struct kefir_dfp_decimal64 {
    kefir_uint64_t uvalue;
} kefir_dfp_decimal64_t;

typedef struct kefir_dfp_decimal128 {
    kefir_uint64_t uvalue[2];
} kefir_dfp_decimal128_t;

kefir_bool_t kefir_dfp_is_supported(void);
kefir_dfp_decimal32_t kefir_dfp_decimal32_from_int64(kefir_int64_t);
kefir_dfp_decimal32_t kefir_dfp_decimal32_from_uint64(kefir_uint64_t);
kefir_dfp_decimal64_t kefir_dfp_decimal64_from_int64(kefir_int64_t);
kefir_dfp_decimal64_t kefir_dfp_decimal64_from_uint64(kefir_uint64_t);
kefir_dfp_decimal128_t kefir_dfp_decimal128_from_int64(kefir_int64_t);
kefir_dfp_decimal128_t kefir_dfp_decimal128_from_uint64(kefir_uint64_t);
kefir_dfp_decimal32_t kefir_dfp_decimal32_from_long_double(kefir_long_double_t);
kefir_dfp_decimal64_t kefir_dfp_decimal64_from_long_double(kefir_long_double_t);
kefir_dfp_decimal128_t kefir_dfp_decimal128_from_long_double(kefir_long_double_t);
kefir_long_double_t kefir_dfp_decimal32_to_long_double(kefir_dfp_decimal32_t);
kefir_long_double_t kefir_dfp_decimal64_to_long_double(kefir_dfp_decimal64_t);
kefir_long_double_t kefir_dfp_decimal128_to_long_double(kefir_dfp_decimal128_t);
kefir_dfp_decimal32_t kefir_dfp_decimal32_from_decimal64(kefir_dfp_decimal64_t);
kefir_dfp_decimal32_t kefir_dfp_decimal32_from_decimal128(kefir_dfp_decimal128_t);
kefir_dfp_decimal64_t kefir_dfp_decimal64_from_decimal32(kefir_dfp_decimal32_t);
kefir_dfp_decimal64_t kefir_dfp_decimal64_from_decimal128(kefir_dfp_decimal128_t);
kefir_dfp_decimal128_t kefir_dfp_decimal128_from_decimal32(kefir_dfp_decimal32_t);
kefir_dfp_decimal128_t kefir_dfp_decimal128_from_decimal64(kefir_dfp_decimal64_t);
kefir_bool_t kefir_dfp_decimal32_to_bool(kefir_dfp_decimal32_t);
kefir_bool_t kefir_dfp_decimal64_to_bool(kefir_dfp_decimal64_t);
kefir_bool_t kefir_dfp_decimal128_to_bool(kefir_dfp_decimal128_t);
kefir_int64_t kefir_dfp_decimal32_to_int64(kefir_dfp_decimal32_t);
kefir_int64_t kefir_dfp_decimal64_to_int64(kefir_dfp_decimal64_t);
kefir_int64_t kefir_dfp_decimal128_to_int64(kefir_dfp_decimal128_t);
kefir_uint64_t kefir_dfp_decimal32_to_uint64(kefir_dfp_decimal32_t);
kefir_uint64_t kefir_dfp_decimal64_to_uint64(kefir_dfp_decimal64_t);
kefir_uint64_t kefir_dfp_decimal128_to_uint64(kefir_dfp_decimal128_t);
kefir_float32_t kefir_dfp_decimal32_to_float(kefir_dfp_decimal32_t);
kefir_float32_t kefir_dfp_decimal64_to_float(kefir_dfp_decimal64_t);
kefir_float32_t kefir_dfp_decimal128_to_float(kefir_dfp_decimal128_t);
kefir_float64_t kefir_dfp_decimal32_to_double(kefir_dfp_decimal32_t);
kefir_float64_t kefir_dfp_decimal64_to_double(kefir_dfp_decimal64_t);
kefir_float64_t kefir_dfp_decimal128_to_double(kefir_dfp_decimal128_t);
kefir_dfp_decimal32_t kefir_dfp_decimal32_add(kefir_dfp_decimal32_t, kefir_dfp_decimal32_t);
kefir_dfp_decimal32_t kefir_dfp_decimal32_sub(kefir_dfp_decimal32_t, kefir_dfp_decimal32_t);
kefir_dfp_decimal32_t kefir_dfp_decimal32_mul(kefir_dfp_decimal32_t, kefir_dfp_decimal32_t);
kefir_dfp_decimal32_t kefir_dfp_decimal32_div(kefir_dfp_decimal32_t, kefir_dfp_decimal32_t);
kefir_dfp_decimal32_t kefir_dfp_decimal32_neg(kefir_dfp_decimal32_t);
kefir_dfp_decimal64_t kefir_dfp_decimal64_add(kefir_dfp_decimal64_t, kefir_dfp_decimal64_t);
kefir_dfp_decimal64_t kefir_dfp_decimal64_sub(kefir_dfp_decimal64_t, kefir_dfp_decimal64_t);
kefir_dfp_decimal64_t kefir_dfp_decimal64_mul(kefir_dfp_decimal64_t, kefir_dfp_decimal64_t);
kefir_dfp_decimal64_t kefir_dfp_decimal64_div(kefir_dfp_decimal64_t, kefir_dfp_decimal64_t);
kefir_dfp_decimal64_t kefir_dfp_decimal64_neg(kefir_dfp_decimal64_t);
kefir_dfp_decimal128_t kefir_dfp_decimal128_add(kefir_dfp_decimal128_t, kefir_dfp_decimal128_t);
kefir_dfp_decimal128_t kefir_dfp_decimal128_sub(kefir_dfp_decimal128_t, kefir_dfp_decimal128_t);
kefir_dfp_decimal128_t kefir_dfp_decimal128_mul(kefir_dfp_decimal128_t, kefir_dfp_decimal128_t);
kefir_dfp_decimal128_t kefir_dfp_decimal128_div(kefir_dfp_decimal128_t, kefir_dfp_decimal128_t);
kefir_dfp_decimal128_t kefir_dfp_decimal128_neg(kefir_dfp_decimal128_t);
kefir_dfp_decimal32_t kefir_dfp_decimal32_scan(const char *);
kefir_dfp_decimal64_t kefir_dfp_decimal64_scan(const char *);
kefir_dfp_decimal128_t kefir_dfp_decimal128_scan(const char *);
void kefir_dfp_decimal32_format(char *, kefir_size_t, kefir_dfp_decimal32_t);
void kefir_dfp_decimal64_format(char *, kefir_size_t, kefir_dfp_decimal64_t);
void kefir_dfp_decimal128_format(char *, kefir_size_t, kefir_dfp_decimal128_t);

#endif
