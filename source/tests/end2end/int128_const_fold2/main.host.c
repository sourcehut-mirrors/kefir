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
    !defined(__NetBSD__) && !defined(__DragonFly__) &&                                                               \
    (__GNUC__ >= 14 || defined(__KEFIRCC_DECIMAL_BITINT_CONV_SUPPORT__)) && !defined(KEFIR_PLATFORM_DECIMAL_DPD)
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

struct i128 {
    unsigned long arr[2];
};

_Decimal32 i128_to_d32(void);
_Decimal64 i128_to_d64(void);
_Decimal128 i128_to_d128(void);
struct i128 int128_from_d32(void);
struct i128 int128_from_d64(void);
struct i128 int128_from_d128(void);

_Decimal32 u128_to_d32(void);
_Decimal64 u128_to_d64(void);
_Decimal128 u128_to_d128(void);
struct i128 uint128_from_d32(void);
struct i128 uint128_from_d64(void);
struct i128 uint128_from_d128(void);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    assert(decimal32_eq(i128_to_d32(), -482748318319ll));
    assert(decimal64_eq(i128_to_d64(), -482748318319ll));
    assert(decimal128_eq(i128_to_d128(), -482748318319ll));

    struct i128 res;
    res = int128_from_d32();
    assert(res.arr[0] == 317131ll);
    assert(res.arr[1] == 0);

    res = int128_from_d64();
    assert(res.arr[0] == (unsigned long) -31783131ll);
    assert(res.arr[1] == ~0ull);

    res = int128_from_d128();
    assert(res.arr[0] == 31783119931ll);
    assert(res.arr[1] == 0);

    assert(decimal32_eq(u128_to_d32(), 482748318319ll));
    assert(decimal64_eq(u128_to_d64(), 482748318319ll));
    assert(decimal128_eq(u128_to_d128(), 482748318319ll));

    res = uint128_from_d32();
    assert(res.arr[0] == 317131ll);
    assert(res.arr[1] == 0);

    res = uint128_from_d64();
    assert(res.arr[0] == (unsigned long) 31783131ll);
    assert(res.arr[1] == 0);

    res = uint128_from_d128();
    assert(res.arr[0] == 31783119931ll);
    assert(res.arr[1] == 0);
#endif
    return EXIT_SUCCESS;
}
