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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "./definitions.h"

#if ((defined(__GNUC__) && !defined(__clang__) && !defined(__KEFIRCC__)) || defined(__KEFIRCC_DECIMAL_SUPPORT__)) && !defined(__NetBSD__)
#pragma GCC diagnostic ignored "-Wpedantic"
#define ENABLE_DECIMAL_TEST
_Bool decimal32_eq(_Decimal32 a, _Decimal32 b) {
    _Decimal32 diff = (a - b) * 10000;
    return diff < 1 && diff > -1;
}

_Bool decimal64_eq(_Decimal64 a, _Decimal64 b) {
    _Decimal64 diff = (a - b) * 100000;
    return diff < 1 && diff > -1;
}

_Bool decimal128_eq(_Decimal128 a, _Decimal128 b) {
    _Decimal128 diff = (a - b) * 1000000;
    return diff < 1 && diff > -1;
}

_Decimal32 d64_to_d32(_Decimal64);
_Decimal32 d128_to_d32(_Decimal128);
_Decimal64 d32_to_d64(_Decimal32);
_Decimal64 d128_to_d64(_Decimal128);
_Decimal128 d32_to_d128(_Decimal32);
_Decimal128 d64_to_d128(_Decimal64);

float d32_to_f32(_Decimal32);
double d32_to_f64(_Decimal32);
long double d32_to_f80(_Decimal32);
float d64_to_f32(_Decimal64);
double d64_to_f64(_Decimal64);
long double d64_to_f80(_Decimal64);
float d128_to_f32(_Decimal128);
double d128_to_f64(_Decimal128);
long double d128_to_f80(_Decimal128);

_Decimal32 f32_to_d32(float);
_Decimal32 f64_to_d32(double);
_Decimal32 f80_to_d32(long double);
_Decimal64 f32_to_d64(float);
_Decimal64 f64_to_d64(double);
_Decimal64 f80_to_d64(long double);
_Decimal128 f32_to_d128(float);
_Decimal128 f64_to_d128(double);
_Decimal128 f80_to_d128(long double);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    assert(decimal32_eq(d64_to_d32(3.14159), 3.14159));
    assert(decimal32_eq(d128_to_d32(3.14159), 3.14159));
    assert(decimal64_eq(d32_to_d64(3.14159), 3.14159));
    assert(decimal64_eq(d128_to_d64(3.14159), 3.14159));
    assert(decimal128_eq(d32_to_d128(3.14159), 3.14159));
    assert(decimal128_eq(d64_to_d128(3.14159), 3.14159));

    assert(fabs(d32_to_f32(3.14159) - 3.14159f) < 1e-5);
    assert(fabs(d32_to_f64(3.14159) - 3.14159) < 1e-7);
    assert(fabsl(d32_to_f80(3.14159) - 3.14159L) < 1e-8L);
    assert(fabs(d64_to_f32(3.14159) - 3.14159f) < 1e-5);
    assert(fabs(d64_to_f64(3.14159) - 3.14159) < 1e-7);
    assert(fabsl(d64_to_f80(3.14159) - 3.14159L) < 1e-8L);
    assert(fabs(d128_to_f32(3.14159) - 3.14159f) < 1e-5);
    assert(fabs(d128_to_f64(3.14159) - 3.14159) < 1e-7);
    assert(fabsl(d128_to_f80(3.14159) - 3.14159L) < 1e-8L);

    assert(decimal32_eq(f32_to_d32(3.14159), 3.14159));
    assert(decimal32_eq(f64_to_d32(3.14159), 3.14159));
    assert(decimal32_eq(f80_to_d32(3.14159), 3.14159));
    assert(decimal64_eq(f32_to_d64(3.14159), 3.14159));
    assert(decimal64_eq(f64_to_d64(3.14159), 3.14159));
    assert(decimal64_eq(f80_to_d64(3.14159), 3.14159));
    assert(decimal128_eq(f32_to_d128(3.14159), 3.14159));
    assert(decimal128_eq(f64_to_d128(3.14159), 3.14159));
    assert(decimal128_eq(f80_to_d128(3.14159), 3.14159));
#endif
    return EXIT_SUCCESS;
}
