/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#if ((defined(__GNUC__) && !defined(__clang__) && !defined(__KEFIRCC__)) || defined(__KEFIRCC_DECIMAL_SUPPORT__)) && \
    !defined(__NetBSD__) && !defined(__DragonFly__)
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

_Decimal32 int_to_dec32(void);
_Decimal32 uint_to_dec32(void);
_Decimal32 float_to_dec32(void);
_Decimal32 float_to_dec32(void);
_Decimal32 double_to_dec32(void);
_Decimal32 long_double_to_dec32(void);

_Decimal64 int_to_dec64(void);
_Decimal64 uint_to_dec64(void);
_Decimal64 float_to_dec64(void);
_Decimal64 float_to_dec64(void);
_Decimal64 double_to_dec64(void);
_Decimal64 long_double_to_dec64(void);

_Decimal128 int_to_dec128(void);
_Decimal128 uint_to_dec128(void);
_Decimal128 float_to_dec128(void);
_Decimal128 float_to_dec128(void);
_Decimal128 double_to_dec128(void);
_Decimal128 long_double_to_dec128(void);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    assert(decimal32_eq(int_to_dec32(), -58427));
    assert(decimal32_eq(uint_to_dec32(), 842819));
    assert(decimal32_eq(float_to_dec32(), 6.524));
    assert(decimal32_eq(double_to_dec32(), -0.482913e5));
    assert(decimal32_eq(long_double_to_dec32(), 5284.45e4));

    assert(decimal64_eq(int_to_dec64(), -58427));
    assert(decimal64_eq(uint_to_dec64(), 842819));
    assert(decimal64_eq(float_to_dec64(), 6.524));
    assert(decimal64_eq(double_to_dec64(), -0.482913e5));
    assert(decimal64_eq(long_double_to_dec64(), 5284.45e4));

    assert(decimal128_eq(int_to_dec128(), -58427));
    assert(decimal128_eq(uint_to_dec128(), 842819));
    assert(decimal128_eq(float_to_dec128(), 6.524));
    assert(decimal128_eq(double_to_dec128(), -0.482913e5));
    assert(decimal128_eq(long_double_to_dec128(), 5284.45e4));
#endif
    return EXIT_SUCCESS;
}
