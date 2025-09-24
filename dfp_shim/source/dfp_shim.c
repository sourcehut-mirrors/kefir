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

#include "kefir/dfp_shim.h"

#if defined(__STDC_IEC_60559_DFP__) || (defined(__GNUC__) && !defined (__clang__))
#define KEFIR_HAS_DECIMAL_FP_SHIM
#endif

#ifndef KEFIR_HAS_DECIMAL_FP_SHIM
#error "Unable compile Kefir decimal floating-point shim"
#endif

union decimal32_view {
    kefir_dfp_shim_decimal32_t shim;
    _Decimal32 decimal;    
};

union decimal64_view {
    kefir_dfp_shim_decimal64_t shim;
    _Decimal64 decimal;    
};

union decimal128_view {
    kefir_dfp_shim_decimal128_t shim;
    _Decimal128 decimal;    
};

kefir_dfp_shim_decimal32_t kefir_dfp_shim_decimal32_from_int64(kefir_int64_t x) {
    union decimal32_view view = {
        .decimal = (_Decimal32) x
    };
    return view.shim;
}

kefir_dfp_shim_decimal32_t kefir_dfp_shim_decimal32_from_uint64(kefir_uint64_t x) {
    union decimal32_view view = {
        .decimal = (_Decimal32) x
    };
    return view.shim;
}

kefir_dfp_shim_decimal64_t kefir_dfp_shim_decimal64_from_int64(kefir_int64_t x) {
    union decimal64_view view = {
        .decimal = (_Decimal64) x
    };
    return view.shim;
}

kefir_dfp_shim_decimal64_t kefir_dfp_shim_decimal64_from_uint64(kefir_uint64_t x) {
    union decimal64_view view = {
        .decimal = (_Decimal64) x
    };
    return view.shim;
}

kefir_dfp_shim_decimal128_t kefir_dfp_shim_decimal128_from_int64(kefir_int64_t x) {
    union decimal128_view view = {
        .decimal = (_Decimal128) x
    };
    return view.shim;
}

kefir_dfp_shim_decimal128_t kefir_dfp_shim_decimal128_from_uint64(kefir_uint64_t x) {
    union decimal128_view view = {
        .decimal = (_Decimal128) x
    };
    return view.shim;
}
