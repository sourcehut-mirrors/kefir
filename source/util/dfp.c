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

#else
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

#pragma GCC diagnostic pop
#endif
