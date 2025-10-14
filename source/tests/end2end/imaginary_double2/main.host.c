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
#include <complex.h>
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

extern _Decimal32 d32;
extern _Decimal64 d64;
extern _Decimal128 d128;

extern double f32;
extern double f64;
extern double f128;

_Decimal32 fi64_to_d32(double);
_Decimal64 fi64_to_d64(double);
_Decimal128 fi64_to_d128(double);
float d32_to_fi64(_Decimal32);
float d64_to_fi64(_Decimal64);
float d128_to_fi64(_Decimal128);

#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    assert(decimal32_eq(d32, 0.0));
    assert(decimal64_eq(d64, 0.0));
    assert(decimal128_eq(d128, 0.0));

    assert(fabs(f32) < 1e-6);
    assert(fabs(f64) < 1e-6);
    assert(fabs(f128) < 1e-6);

    assert(decimal32_eq(fi64_to_d32(3.14159), 0.0));
    assert(decimal64_eq(fi64_to_d64(3.14159), 0.0));
    assert(decimal128_eq(fi64_to_d128(3.14159), 0.0));

    assert(fabs(d32_to_fi64(3.14159)) < 1e-6);
    assert(fabs(d64_to_fi64(3.14159)) < 1e-6);
    assert(fabs(d128_to_fi64(3.14159)) < 1e-6);
#endif
    return EXIT_SUCCESS;
}
