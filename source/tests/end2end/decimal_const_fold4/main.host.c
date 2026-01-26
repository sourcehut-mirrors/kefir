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

#if ((defined(__GNUC__) && !defined(__clang__) && !defined(__KEFIRCC__)) || defined(__KEFIRCC_DECIMAL_SUPPORT__)) && !defined(__NetBSD__) && !defined(__DragonFly__) && (__GNUC__ >= 14 || defined(__KEFIRCC_DECIMAL_BITINT_CONV_SUPPORT__)) && !defined(KEFIR_PLATFORM_DECIMAL_DPD)
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

struct S2 {
    unsigned long arr[2];
};

struct S2 dec32_to_signed(void);
struct S2 dec32_to_unsigned(void);
struct S2 dec64_to_signed(void);
struct S2 dec64_to_unsigned(void);
struct S2 dec128_to_signed(void);
struct S2 dec128_to_unsigned(void);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    struct S2 s2;

    s2 = dec32_to_signed();
    assert(s2.arr[0] == (unsigned long) -6705ll);
    assert(s2.arr[1] == (1ul << 56) - 1);

    s2 = dec32_to_unsigned();
    assert(s2.arr[0] == (unsigned long) 9013);
    assert(s2.arr[1] == 0);

    s2 = dec64_to_signed();
    assert(s2.arr[0] == (unsigned long) 1910);
    assert(s2.arr[1] == 0);

    s2 = dec64_to_unsigned();
    assert(s2.arr[0] == (unsigned long) 13819);
    assert(s2.arr[1] == 0);

    s2 = dec128_to_signed();
    assert(s2.arr[0] == (unsigned long) -993718819ll);
    assert(s2.arr[1] == (1ul << 56) - 1);

    s2 = dec128_to_unsigned();
    assert(s2.arr[0] == 38194);
    assert(s2.arr[1] == 0);
#endif
    return EXIT_SUCCESS;
}
