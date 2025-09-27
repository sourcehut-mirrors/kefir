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

#include "kefir/util/dfp.h"
#include "kefir/core/platform.h"
#include "kefir/core/error.h"
#include "kefir/core/error_format.h"
#include "kefir/core/util.h"

#ifndef KEFIR_PLATFORM_HAS_DECIMAL_FP
#include <stdlib.h>
#include <stdio.h>

kefir_bool_t kefir_dfp_is_supported(void) {
    return false;
}

#define FAIL_NOT_SUPPORTED \
    do { \
        KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Decimal floatig-point is not supported on this platform"); \
        kefir_format_error_tabular(stderr, kefir_current_error()); \
        exit(EXIT_FAILURE); \
    } while (0)

kefir_dfp_decimal32_t kefir_dfp_decimal32_from_int64(kefir_int64_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_from_uint64(kefir_uint64_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_from_int64(kefir_int64_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_from_uint64(kefir_uint64_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_from_int64(kefir_int64_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_from_uint64(kefir_uint64_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_from_long_double(kefir_long_double_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_from_long_double(kefir_long_double_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_from_long_double(kefir_long_double_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_long_double_t kefir_dfp_decimal32_to_long_double(kefir_dfp_decimal32_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_long_double_t kefir_dfp_decimal64_to_long_double(kefir_dfp_decimal64_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_long_double_t kefir_dfp_decimal128_to_long_double(kefir_dfp_decimal128_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_from_decimal64(kefir_dfp_decimal64_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_from_decimal128(kefir_dfp_decimal128_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_from_decimal32(kefir_dfp_decimal32_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_from_decimal128(kefir_dfp_decimal128_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_from_decimal32(kefir_dfp_decimal32_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_from_decimal64(kefir_dfp_decimal64_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal32_to_bool(kefir_dfp_decimal32_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;    
}

kefir_bool_t kefir_dfp_decimal64_to_bool(kefir_dfp_decimal64_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;    
}

kefir_bool_t kefir_dfp_decimal128_to_bool(kefir_dfp_decimal128_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;    
}

kefir_int64_t kefir_dfp_decimal32_to_int64(kefir_dfp_decimal32_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;    
}

kefir_int64_t kefir_dfp_decimal64_to_int64(kefir_dfp_decimal64_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;    
}

kefir_int64_t kefir_dfp_decimal128_to_int64(kefir_dfp_decimal128_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;    
}

kefir_uint64_t kefir_dfp_decimal32_to_uint64(kefir_dfp_decimal32_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;    
}

kefir_uint64_t kefir_dfp_decimal64_to_uint64(kefir_dfp_decimal64_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;    
}

kefir_uint64_t kefir_dfp_decimal128_to_uint64(kefir_dfp_decimal128_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;    
}

kefir_float32_t kefir_dfp_decimal32_to_float(kefir_dfp_decimal32_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;    
}

kefir_float32_t kefir_dfp_decimal64_to_float(kefir_dfp_decimal64_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;    
}

kefir_float32_t kefir_dfp_decimal128_to_float(kefir_dfp_decimal128_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;    
}

kefir_float64_t kefir_dfp_decimal32_to_double(kefir_dfp_decimal32_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;    
}

kefir_float64_t kefir_dfp_decimal64_to_double(kefir_dfp_decimal64_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;    
}

kefir_float64_t kefir_dfp_decimal128_to_double(kefir_dfp_decimal128_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;    
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_add(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_sub(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_mul(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_div(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_neg(kefir_dfp_decimal32_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_add(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_sub(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_mul(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_div(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_neg(kefir_dfp_decimal64_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_add(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_sub(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_mul(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_div(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_neg(kefir_dfp_decimal128_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal32_equals(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal64_equals(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal128_equals(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal32_not_equals(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal64_not_equals(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal128_not_equals(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal32_greater(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal64_greater(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal128_greater(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal32_greater_or_equal(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal64_greater_or_equal(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal128_greater_or_equal(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal32_less(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal64_less(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal128_less(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal32_less_or_equal(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal64_less_or_equal(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal128_less_or_equal(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    UNUSED(x);
    UNUSED(y);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_scan(const char *input) {
    UNUSED(input);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_scan(const char *input) {
    UNUSED(input);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_scan(const char *input) {
    UNUSED(input);
    FAIL_NOT_SUPPORTED;
}

void kefir_dfp_decimal32_format(char *str, kefir_size_t len, kefir_dfp_decimal32_t value) {
    UNUSED(str);
    UNUSED(len);
    UNUSED(value);
    FAIL_NOT_SUPPORTED;
}

void kefir_dfp_decimal64_format(char *str, kefir_size_t len, kefir_dfp_decimal64_t value) {
    UNUSED(str);
    UNUSED(len);
    UNUSED(value);
    FAIL_NOT_SUPPORTED;
}

void kefir_dfp_decimal128_format(char *str, kefir_size_t len, kefir_dfp_decimal128_t value) {
    UNUSED(str);
    UNUSED(len);
    UNUSED(value);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_from_signed_bitint(const struct kefir_bigint *value) {
    UNUSED(value);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_from_unsigned_bitint(const struct kefir_bigint *value) {
    UNUSED(value);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_from_signed_bitint(const struct kefir_bigint *value) {
    UNUSED(value);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_from_unsigned_bitint(const struct kefir_bigint *value) {
    UNUSED(value);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_from_signed_bitint(const struct kefir_bigint *value) {
    UNUSED(value);
    FAIL_NOT_SUPPORTED;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_from_unsigned_bitint(const struct kefir_bigint *value) {
    UNUSED(value);
    FAIL_NOT_SUPPORTED;
}

void kefir_dfp_decimal32_to_signed_bitint(const struct kefir_bigint *value, kefir_dfp_decimal32_t decimal) {
    UNUSED(value);
    UNUSED(decimal);
    FAIL_NOT_SUPPORTED;
}

void kefir_dfp_decimal32_to_unsigned_bitint(const struct kefir_bigint *value, kefir_dfp_decimal32_t decimal) {
    UNUSED(value);
    UNUSED(decimal);
    FAIL_NOT_SUPPORTED;
}

void kefir_dfp_decimal64_to_signed_bitint(const struct kefir_bigint *value, kefir_dfp_decimal64_t decimal) {
    UNUSED(value);
    UNUSED(decimal);
    FAIL_NOT_SUPPORTED;
}

void kefir_dfp_decimal64_to_unsigned_bitint(const struct kefir_bigint *value, kefir_dfp_decimal64_t decimal) {
    UNUSED(value);
    UNUSED(decimal);
    FAIL_NOT_SUPPORTED;
}

void kefir_dfp_decimal128_to_signed_bitint(const struct kefir_bigint *value, kefir_dfp_decimal128_t decimal) {
    UNUSED(value);
    UNUSED(decimal);
    FAIL_NOT_SUPPORTED;
}

void kefir_dfp_decimal128_to_unsigned_bitint(const struct kefir_bigint *value, kefir_dfp_decimal128_t decimal) {
    UNUSED(value);
    UNUSED(decimal);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal32_isnan(kefir_dfp_decimal32_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal64_isnan(kefir_dfp_decimal64_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal128_isnan(kefir_dfp_decimal128_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal32_isinf(kefir_dfp_decimal32_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal64_isinf(kefir_dfp_decimal64_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

kefir_bool_t kefir_dfp_decimal128_isinf(kefir_dfp_decimal128_t x) {
    UNUSED(x);
    FAIL_NOT_SUPPORTED;
}

#else
#include <stdio.h>
#include <math.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

union decimal32_view {
    kefir_dfp_decimal32_t shim;
    _Decimal32 decimal;    
};

union decimal64_view {
    kefir_dfp_decimal64_t shim;
    _Decimal64 decimal;    
};

union decimal128_view {
    kefir_dfp_decimal128_t shim;
    _Decimal128 decimal;    
};

kefir_bool_t kefir_dfp_is_supported(void) {
    return true;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_from_int64(kefir_int64_t x) {
    union decimal32_view view = {
        .decimal = (_Decimal32) x
    };
    return view.shim;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_from_uint64(kefir_uint64_t x) {
    union decimal32_view view = {
        .decimal = (_Decimal32) x
    };
    return view.shim;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_from_int64(kefir_int64_t x) {
    union decimal64_view view = {
        .decimal = (_Decimal64) x
    };
    return view.shim;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_from_uint64(kefir_uint64_t x) {
    union decimal64_view view = {
        .decimal = (_Decimal64) x
    };
    return view.shim;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_from_int64(kefir_int64_t x) {
    union decimal128_view view = {
        .decimal = (_Decimal128) x
    };
    return view.shim;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_from_uint64(kefir_uint64_t x) {
    union decimal128_view view = {
        .decimal = (_Decimal128) x
    };
    return view.shim;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_from_long_double(kefir_long_double_t x) {
    union decimal32_view view = {
        .decimal = (_Decimal32) x
    };
    return view.shim;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_from_long_double(kefir_long_double_t x) {
    union decimal64_view view = {
        .decimal = (_Decimal64) x
    };
    return view.shim;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_from_long_double(kefir_long_double_t x) {
    union decimal128_view view = {
        .decimal = (_Decimal128) x
    };
    return view.shim;
}

kefir_long_double_t kefir_dfp_decimal32_to_long_double(kefir_dfp_decimal32_t x) {
    union decimal32_view view = {
        .shim = x
    };
    return (kefir_long_double_t) view.decimal;
}

kefir_long_double_t kefir_dfp_decimal64_to_long_double(kefir_dfp_decimal64_t x) {
    union decimal64_view view = {
        .shim = x
    };
    return (kefir_long_double_t) view.decimal;
}

kefir_long_double_t kefir_dfp_decimal128_to_long_double(kefir_dfp_decimal128_t x) {
    union decimal128_view view = {
        .shim = x
    };
    return (kefir_long_double_t) view.decimal;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_from_decimal64(kefir_dfp_decimal64_t x) {
    union decimal64_view view = {
        .shim = x
    };
    union decimal32_view out_view = {
        .decimal = (_Decimal32) view.decimal
    };
    return out_view.shim;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_from_decimal128(kefir_dfp_decimal128_t x) {
    union decimal128_view view = {
        .shim = x
    };
    union decimal32_view out_view = {
        .decimal = (_Decimal32) view.decimal
    };
    return out_view.shim;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_from_decimal32(kefir_dfp_decimal32_t x) {
    union decimal32_view view = {
        .shim = x
    };
    union decimal64_view out_view = {
        .decimal = (_Decimal64) view.decimal
    };
    return out_view.shim;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_from_decimal128(kefir_dfp_decimal128_t x) {
    union decimal128_view view = {
        .shim = x
    };
    union decimal64_view out_view = {
        .decimal = (_Decimal64) view.decimal
    };
    return out_view.shim;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_from_decimal32(kefir_dfp_decimal32_t x) {
    union decimal32_view view = {
        .shim = x
    };
    union decimal128_view out_view = {
        .decimal = (_Decimal128) view.decimal
    };
    return out_view.shim;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_from_decimal64(kefir_dfp_decimal64_t x) {
    union decimal64_view view = {
        .shim = x
    };
    union decimal128_view out_view = {
        .decimal = (_Decimal128) view.decimal
    };
    return out_view.shim;
}

kefir_bool_t kefir_dfp_decimal32_to_bool(kefir_dfp_decimal32_t x) {
    union decimal32_view view = {
        .shim = x
    };
    return (kefir_bool_t) view.decimal;
}

kefir_bool_t kefir_dfp_decimal64_to_bool(kefir_dfp_decimal64_t x) {
    union decimal64_view view = {
        .shim = x
    };
    return (kefir_bool_t) view.decimal;
}

kefir_bool_t kefir_dfp_decimal128_to_bool(kefir_dfp_decimal128_t x) {
    union decimal128_view view = {
        .shim = x
    };
    return (kefir_bool_t) view.decimal; 
}

kefir_int64_t kefir_dfp_decimal32_to_int64(kefir_dfp_decimal32_t x) {
    union decimal32_view view = {
        .shim = x
    };
    return (kefir_int64_t) view.decimal;
}

kefir_int64_t kefir_dfp_decimal64_to_int64(kefir_dfp_decimal64_t x) {
    union decimal64_view view = {
        .shim = x
    };
    return (kefir_int64_t) view.decimal;
}

kefir_int64_t kefir_dfp_decimal128_to_int64(kefir_dfp_decimal128_t x) {
    union decimal128_view view = {
        .shim = x
    };
    return (kefir_int64_t) view.decimal;
}

kefir_uint64_t kefir_dfp_decimal32_to_uint64(kefir_dfp_decimal32_t x) {
    union decimal32_view view = {
        .shim = x
    };
    return (kefir_uint64_t) view.decimal;    
}

kefir_uint64_t kefir_dfp_decimal64_to_uint64(kefir_dfp_decimal64_t x) {
    union decimal64_view view = {
        .shim = x
    };
    return (kefir_uint64_t) view.decimal;   
}

kefir_uint64_t kefir_dfp_decimal128_to_uint64(kefir_dfp_decimal128_t x) {
    union decimal128_view view = {
        .shim = x
    };
    return (kefir_uint64_t) view.decimal;   
}

kefir_float32_t kefir_dfp_decimal32_to_float(kefir_dfp_decimal32_t x) {
    union decimal32_view view = {
        .shim = x
    };
    return (kefir_float32_t) view.decimal;
}

kefir_float32_t kefir_dfp_decimal64_to_float(kefir_dfp_decimal64_t x) {
    union decimal64_view view = {
        .shim = x
    };
    return (kefir_float32_t) view.decimal;
}

kefir_float32_t kefir_dfp_decimal128_to_float(kefir_dfp_decimal128_t x) {
    union decimal128_view view = {
        .shim = x
    };
    return (kefir_float32_t) view.decimal;
}

kefir_float64_t kefir_dfp_decimal32_to_double(kefir_dfp_decimal32_t x) {
    union decimal32_view view = {
        .shim = x
    };
    return (kefir_float64_t) view.decimal;
}

kefir_float64_t kefir_dfp_decimal64_to_double(kefir_dfp_decimal64_t x) {
    union decimal64_view view = {
        .shim = x
    };
    return (kefir_float64_t) view.decimal;
}

kefir_float64_t kefir_dfp_decimal128_to_double(kefir_dfp_decimal128_t x) {
    union decimal128_view view = {
        .shim = x
    };
    return (kefir_float64_t) view.decimal;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_add(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    union decimal32_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    union decimal32_view res_view = {
        .decimal = lhs_view.decimal + rhs_view.decimal
    };
    return res_view.shim;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_sub(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    union decimal32_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    union decimal32_view res_view = {
        .decimal = lhs_view.decimal - rhs_view.decimal
    };
    return res_view.shim;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_mul(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    union decimal32_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    union decimal32_view res_view = {
        .decimal = lhs_view.decimal * rhs_view.decimal
    };
    return res_view.shim;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_div(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    union decimal32_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    union decimal32_view res_view = {
        .decimal = lhs_view.decimal / rhs_view.decimal
    };
    return res_view.shim;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_neg(kefir_dfp_decimal32_t x) {
    union decimal32_view lhs_view = {
        .shim = x,
    };
    union decimal32_view res_view = {
        .decimal = -lhs_view.decimal
    };
    return res_view.shim;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_add(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    union decimal64_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    union decimal64_view res_view = {
        .decimal = lhs_view.decimal + rhs_view.decimal
    };
    return res_view.shim;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_sub(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    union decimal64_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    union decimal64_view res_view = {
        .decimal = lhs_view.decimal - rhs_view.decimal
    };
    return res_view.shim;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_mul(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    union decimal64_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    union decimal64_view res_view = {
        .decimal = lhs_view.decimal * rhs_view.decimal
    };
    return res_view.shim;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_div(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    union decimal64_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    union decimal64_view res_view = {
        .decimal = lhs_view.decimal / rhs_view.decimal
    };
    return res_view.shim;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_neg(kefir_dfp_decimal64_t x) {
    union decimal64_view lhs_view = {
        .shim = x,
    };
    union decimal64_view res_view = {
        .decimal = -lhs_view.decimal
    };
    return res_view.shim;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_add(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    union decimal128_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    union decimal128_view res_view = {
        .decimal = lhs_view.decimal + rhs_view.decimal
    };
    return res_view.shim;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_sub(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    union decimal128_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    union decimal128_view res_view = {
        .decimal = lhs_view.decimal - rhs_view.decimal
    };
    return res_view.shim;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_mul(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    union decimal128_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    union decimal128_view res_view = {
        .decimal = lhs_view.decimal * rhs_view.decimal
    };
    return res_view.shim;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_div(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    union decimal128_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    union decimal128_view res_view = {
        .decimal = lhs_view.decimal / rhs_view.decimal
    };
    return res_view.shim;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_neg(kefir_dfp_decimal128_t x) {
    union decimal128_view lhs_view = {
        .shim = x,
    };
    union decimal128_view res_view = {
        .decimal = -lhs_view.decimal
    };
    return res_view.shim;
}

kefir_bool_t kefir_dfp_decimal32_equals(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    union decimal32_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    return lhs_view.decimal == rhs_view.decimal;
}

kefir_bool_t kefir_dfp_decimal64_equals(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    union decimal64_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    return lhs_view.decimal == rhs_view.decimal;
}

kefir_bool_t kefir_dfp_decimal128_equals(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    union decimal128_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    return lhs_view.decimal == rhs_view.decimal;
}

kefir_bool_t kefir_dfp_decimal32_not_equals(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    union decimal32_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    return lhs_view.decimal != rhs_view.decimal;
}

kefir_bool_t kefir_dfp_decimal64_not_equals(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    union decimal64_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    return lhs_view.decimal != rhs_view.decimal;
}

kefir_bool_t kefir_dfp_decimal128_not_equals(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    union decimal128_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    return lhs_view.decimal != rhs_view.decimal;
}

kefir_bool_t kefir_dfp_decimal32_greater(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    union decimal32_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    return lhs_view.decimal > rhs_view.decimal;
}

kefir_bool_t kefir_dfp_decimal64_greater(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    union decimal64_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    return lhs_view.decimal > rhs_view.decimal;
}

kefir_bool_t kefir_dfp_decimal128_greater(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    union decimal128_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    return lhs_view.decimal > rhs_view.decimal;
}

kefir_bool_t kefir_dfp_decimal32_greater_or_equal(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    union decimal32_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    return lhs_view.decimal >= rhs_view.decimal;
}

kefir_bool_t kefir_dfp_decimal64_greater_or_equal(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    union decimal64_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    return lhs_view.decimal >= rhs_view.decimal;
}

kefir_bool_t kefir_dfp_decimal128_greater_or_equal(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    union decimal128_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    return lhs_view.decimal >= rhs_view.decimal;
}

kefir_bool_t kefir_dfp_decimal32_less(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    union decimal32_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    return lhs_view.decimal < rhs_view.decimal;
}

kefir_bool_t kefir_dfp_decimal64_less(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    union decimal64_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    return lhs_view.decimal < rhs_view.decimal;
}

kefir_bool_t kefir_dfp_decimal128_less(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    union decimal128_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    return lhs_view.decimal < rhs_view.decimal;
}

kefir_bool_t kefir_dfp_decimal32_less_or_equal(kefir_dfp_decimal32_t x, kefir_dfp_decimal32_t y) {
    union decimal32_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    return lhs_view.decimal <= rhs_view.decimal;
}

kefir_bool_t kefir_dfp_decimal64_less_or_equal(kefir_dfp_decimal64_t x, kefir_dfp_decimal64_t y) {
    union decimal64_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    return lhs_view.decimal <= rhs_view.decimal;
}

kefir_bool_t kefir_dfp_decimal128_less_or_equal(kefir_dfp_decimal128_t x, kefir_dfp_decimal128_t y) {
    union decimal128_view lhs_view = {
        .shim = x,
    }, rhs_view = {
        .shim = y
    };
    return lhs_view.decimal <= rhs_view.decimal;
}

#define SCAN_DECIMAL_IMPL(_decimal_type, _num_of_sigificant_digits, _value_ptr) \
    do { \
        *(_value_ptr) = 0; \
        kefir_int64_t exponent = 0, digits = 0; \
        kefir_bool_t negative = false; \
 \
        if (*input == '-') { \
            negative = true; \
            input++; \
        } \
 \
        for (; *input != '\0' && *input != '.' && *input != 'e' && *input != 'E'; input++) { \
            if (*input >= '0' && *input <= '9') { \
                if (digits < (_num_of_sigificant_digits) || (_num_of_sigificant_digits) < 0) { \
                    *(_value_ptr) = *(_value_ptr) * 10 + (_decimal_type)(*input - '0'); \
                } else if (digits == (_num_of_sigificant_digits)) { \
                    if ((*input - '0') >= 5) { \
                        (*(_value_ptr))++; \
                    } \
                } \
                digits++; \
            } \
        } \
 \
        if (*input == '.') { \
            input++; \
            for (; *input != '\0' && *input != 'e' && *input != 'E'; input++) { \
                if (*input >= '0' && *input <= '9') { \
                    if (digits < (_num_of_sigificant_digits) || (_num_of_sigificant_digits) < 0) { \
                        *(_value_ptr) = *(_value_ptr) * 10 + (_decimal_type)(*input - '0'); \
                        exponent--; \
                    } else if (digits == (_num_of_sigificant_digits)) { \
                        if ((*input - '0') >= 5) { \
                            (*(_value_ptr))++; \
                        } \
                    } \
                    digits++; \
                } \
            } \
        } \
 \
        if (*input == 'e' || *input == 'E') { \
            input++; \
            exponent += strtoll(input, NULL, 10); \
        } \
 \
        for (; exponent > 0; exponent--) { \
            *(_value_ptr) *= 10; \
        } \
        for (; exponent < 0; exponent++) { \
            *(_value_ptr) /= 10; \
        } \
        if (negative) { \
            *(_value_ptr) *= -1; \
        } \
    } while (0)

kefir_dfp_decimal32_t kefir_dfp_decimal32_scan(const char *input) {
    union decimal32_view view = {
        .decimal = 0
    };
    SCAN_DECIMAL_IMPL(_Decimal32, -1, &view.decimal);
    return view.shim;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_scan(const char *input) {
    union decimal64_view view = {
        .decimal = 0
    };
    SCAN_DECIMAL_IMPL(_Decimal64, -1, &view.decimal);
    return view.shim;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_scan(const char *input) {
    union decimal128_view view = {
        .decimal = 0
    };
    SCAN_DECIMAL_IMPL(_Decimal128, 34, &view.decimal);
    return view.shim;
}

#undef SCAN_DECIMAL_IMPL

#define PRINT_DECIMAL_IMPL(_str, _len, _value_ptr, _digits) \
    do { \
        if ((*(_value_ptr)) != (*(_value_ptr))) {\
            snprintf(str, len, "NaN"); \
            break; \
        } \
        if ((*(_value_ptr)) == 1.0df / 0.0df || (*(_value_ptr)) == -1.0df / 0.0df) {\
            snprintf(str, len, "%sInfinity", *(_value_ptr) < 0 ? "-" : ""); \
            break; \
        } \
 \
        kefir_size_t written = 0; \
 \
        if (*(_value_ptr) < 0) { \
            written += snprintf(str + written, len - written, "-"); \
            *(_value_ptr) = -*(_value_ptr); \
        } \
 \
        kefir_int64_t exp = 0; \
        for (; *(_value_ptr) >= 10; exp++) { \
            *(_value_ptr) /= 10; \
        } \
        for (; *(_value_ptr) > 0 && *(_value_ptr) < 1; exp--) { \
            *(_value_ptr) *= 10; \
        } \
 \
        kefir_int64_t d = (kefir_int64_t) *(_value_ptr); \
        written += snprintf(str + written, len - written, "%" KEFIR_INT64_FMT, d); \
        *(_value_ptr) = (*(_value_ptr) - d) * 10; \
 \
        if (*(_value_ptr) > 0) { \
            written += snprintf(str + written, len - written, "."); \
            for (int i = 1; i < (_digits) && *(_value_ptr) > 0; i++) { \
                d = (kefir_int64_t) *(_value_ptr); \
                written += snprintf(str + written, len - written, "%" KEFIR_INT64_FMT, d); \
                *(_value_ptr) = (*(_value_ptr) - d) * 10; \
            } \
        } \
 \
        if (exp != 0) { \
            written += snprintf(str + written, len - written, "e%" KEFIR_INT64_FMT, exp); \
        } \
    } while (0)

void kefir_dfp_decimal32_format(char *str, kefir_size_t len, kefir_dfp_decimal32_t value) {
    union decimal32_view view = {
        .shim = value
    };
    PRINT_DECIMAL_IMPL(str, len, &view.decimal, 7);
}

void kefir_dfp_decimal64_format(char *str, kefir_size_t len, kefir_dfp_decimal64_t value) {
    union decimal64_view view = {
        .shim = value
    };
    PRINT_DECIMAL_IMPL(str, len, &view.decimal, 16);
}

void kefir_dfp_decimal128_format(char *str, kefir_size_t len, kefir_dfp_decimal128_t value) {
    union decimal128_view view = {
        .shim = value
    };
    PRINT_DECIMAL_IMPL(str, len, &view.decimal, 34);
}

#undef PRINT_DECIMAL_IMPL

extern _Decimal32 __bid_floatbitintsd(const kefir_uint64_t *, kefir_int64_t);
extern _Decimal64 __bid_floatbitintdd(const kefir_uint64_t *, kefir_int64_t);
extern _Decimal128 __bid_floatbitinttd(const kefir_uint64_t *, kefir_int64_t);

kefir_dfp_decimal32_t kefir_dfp_decimal32_from_signed_bitint(const struct kefir_bigint *value) {
    union decimal32_view view = {
        .decimal = __bid_floatbitintsd((const kefir_uint64_t *) value->digits, -(kefir_int64_t) value->bitwidth)
    };
    return view.shim;
}

kefir_dfp_decimal32_t kefir_dfp_decimal32_from_unsigned_bitint(const struct kefir_bigint *value) {
    union decimal32_view view = {
        .decimal = __bid_floatbitintsd((const kefir_uint64_t *) value->digits, (kefir_int64_t) value->bitwidth)
    };
    return view.shim;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_from_signed_bitint(const struct kefir_bigint *value) {
    union decimal64_view view = {
        .decimal = __bid_floatbitintdd((const kefir_uint64_t *) value->digits, -(kefir_int64_t) value->bitwidth)
    };
    return view.shim;
}

kefir_dfp_decimal64_t kefir_dfp_decimal64_from_unsigned_bitint(const struct kefir_bigint *value) {
    union decimal64_view view = {
        .decimal = __bid_floatbitintdd((const kefir_uint64_t *) value->digits, (kefir_int64_t) value->bitwidth)
    };
    return view.shim;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_from_signed_bitint(const struct kefir_bigint *value) {
    union decimal128_view view = {
        .decimal = __bid_floatbitinttd((const kefir_uint64_t *) value->digits, -(kefir_int64_t) value->bitwidth)
    };
    return view.shim;
}

kefir_dfp_decimal128_t kefir_dfp_decimal128_from_unsigned_bitint(const struct kefir_bigint *value) {
    union decimal128_view view = {
        .decimal = __bid_floatbitinttd((const kefir_uint64_t *) value->digits, (kefir_int64_t) value->bitwidth)
    };
    return view.shim;
}

void __bid_fixsdbitint(kefir_uint64_t *r, kefir_int64_t, _Decimal32);
void __bid_fixddbitint(kefir_uint64_t *r, kefir_int64_t, _Decimal64);
void __bid_fixtdbitint(kefir_uint64_t *r, kefir_int64_t, _Decimal128);

void kefir_dfp_decimal32_to_signed_bitint(const struct kefir_bigint *value, kefir_dfp_decimal32_t decimal) {
    union decimal32_view view = {
        .shim = decimal
    };
    __bid_fixsdbitint((kefir_uint64_t *) value->digits, -(kefir_int64_t) value->bitwidth, view.decimal);
}

void kefir_dfp_decimal32_to_unsigned_bitint(const struct kefir_bigint *value, kefir_dfp_decimal32_t decimal) {
    union decimal32_view view = {
        .shim = decimal
    };
    __bid_fixsdbitint((kefir_uint64_t *) value->digits, (kefir_int64_t) value->bitwidth, view.decimal);
}

void kefir_dfp_decimal64_to_signed_bitint(const struct kefir_bigint *value, kefir_dfp_decimal64_t decimal) {
    union decimal64_view view = {
        .shim = decimal
    };
    __bid_fixddbitint((kefir_uint64_t *) value->digits, -(kefir_int64_t) value->bitwidth, view.decimal);
}

void kefir_dfp_decimal64_to_unsigned_bitint(const struct kefir_bigint *value, kefir_dfp_decimal64_t decimal) {
    union decimal64_view view = {
        .shim = decimal
    };
    __bid_fixddbitint((kefir_uint64_t *) value->digits, (kefir_int64_t) value->bitwidth, view.decimal);
}

void kefir_dfp_decimal128_to_signed_bitint(const struct kefir_bigint *value, kefir_dfp_decimal128_t decimal) {
    union decimal128_view view = {
        .shim = decimal
    };
    __bid_fixtdbitint((kefir_uint64_t *) value->digits, -(kefir_int64_t) value->bitwidth, view.decimal);
}

void kefir_dfp_decimal128_to_unsigned_bitint(const struct kefir_bigint *value, kefir_dfp_decimal128_t decimal) {
    union decimal128_view view = {
        .shim = decimal
    };
    __bid_fixtdbitint((kefir_uint64_t *) value->digits, (kefir_int64_t) value->bitwidth, view.decimal);
}

kefir_bool_t kefir_dfp_decimal32_isnan(kefir_dfp_decimal32_t x) {
    union decimal32_view view = {
        .shim = x
    };
    return view.decimal != view.decimal;
}

kefir_bool_t kefir_dfp_decimal64_isnan(kefir_dfp_decimal64_t x) {
    union decimal64_view view = {
        .shim = x
    };
    return view.decimal != view.decimal;
}

kefir_bool_t kefir_dfp_decimal128_isnan(kefir_dfp_decimal128_t x) {
    union decimal128_view view = {
        .shim = x
    };
    return view.decimal != view.decimal;
}

kefir_bool_t kefir_dfp_decimal32_isinf(kefir_dfp_decimal32_t x) {
    union decimal32_view view = {
        .shim = x
    };
    _Decimal32 positive_inf = 1.0df / 0.0df;
    return view.decimal == positive_inf || view.decimal == -positive_inf;
}

kefir_bool_t kefir_dfp_decimal64_isinf(kefir_dfp_decimal64_t x) {
    union decimal64_view view = {
        .shim = x
    };
    _Decimal64 positive_inf = 1.0dd / 0.0dd;
    return view.decimal == positive_inf || view.decimal == -positive_inf;
}

kefir_bool_t kefir_dfp_decimal128_isinf(kefir_dfp_decimal128_t x) {
    union decimal128_view view = {
        .shim = x
    };
    _Decimal128 positive_inf = 1.0dl / 0.0dl;
    return view.decimal == positive_inf || view.decimal == -positive_inf;
}

#pragma GCC diagnostic pop
#endif
