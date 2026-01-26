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

float dec32_to_float(void);
double dec32_to_double(void);
long double dec32_to_long_double(void);
long dec32_to_long(void);
unsigned long dec32_to_ulong(void);
_Decimal64 dec32_to_dec64(void);
_Decimal128 dec32_to_dec128(void);

float dec64_to_float(void);
double dec64_to_double(void);
long double dec64_to_long_double(void);
long dec64_to_long(void);
unsigned long dec64_to_ulong(void);
_Decimal32 dec64_to_dec32(void);
_Decimal128 dec64_to_dec128(void);

float dec128_to_float(void);
double dec128_to_double(void);
long double dec128_to_long_double(void);
long dec128_to_long(void);
unsigned long dec128_to_ulong(void);
_Decimal32 dec128_to_dec32(void);
_Decimal64 dec128_to_dec64(void);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    assert(fabs(dec32_to_float() - 3.14159) < 1e-5);
    assert(fabs(dec32_to_double() - 2.17828) < 1e-7);
    assert(fabsl(dec32_to_long_double() + 8.53142) < 1e-9);
    assert(dec32_to_long() == -58400);
    assert(dec32_to_ulong() == 1234);
    assert(decimal64_eq(dec32_to_dec64(), 5832.52));
    assert(decimal128_eq(dec32_to_dec128(), -10192.1));

    assert(fabs(dec64_to_float() - 3.14159) < 1e-5);
    assert(fabs(dec64_to_double() - 2.17828) < 1e-7);
    assert(fabsl(dec64_to_long_double() + 8.53142) < 1e-9);
    assert(dec64_to_long() == -58400);
    assert(dec64_to_ulong() == 1234);
    assert(decimal32_eq(dec64_to_dec32(), 5832.52));
    assert(decimal128_eq(dec64_to_dec128(), -10192.1));

    assert(fabs(dec128_to_float() - 3.14159) < 1e-5);
    assert(fabs(dec128_to_double() - 2.17828) < 1e-7);
    assert(fabsl(dec128_to_long_double() + 8.53142) < 1e-9);
    assert(dec128_to_long() == -58400);
    assert(dec128_to_ulong() == 1234);
    assert(decimal32_eq(dec128_to_dec32(), 5832.52));
    assert(decimal64_eq(dec128_to_dec64(), -10192.1));
#endif
    return EXIT_SUCCESS;
}
