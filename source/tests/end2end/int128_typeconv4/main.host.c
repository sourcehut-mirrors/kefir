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

#if ((defined(__GNUC__) && !defined(__clang__) && !defined(__KEFIRCC__)) || defined(__KEFIRCC_DECIMAL_SUPPORT__)) && !defined(__NetBSD__) && !defined(__DragonFly__) && !defined(KEFIR_PLATFORM_DECIMAL_DPD)
#pragma GCC diagnostic ignored "-Wpedantic"
#define ENABLE_DECIMAL_TEST
struct i128 {
    unsigned long arr[2];
};

struct i128 i128_from_d32(_Decimal32);
struct i128 i128_from_d64(_Decimal64);
struct i128 i128_from_d128(_Decimal128);
struct i128 u128_from_d32(_Decimal32);
struct i128 u128_from_d64(_Decimal64);
struct i128 u128_from_d128(_Decimal128);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    struct i128 res;

    res = i128_from_d32(0.0);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_from_d32(1.0);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i128_from_d32(-1.0);
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_d32(3.14159);
    assert(res.arr[0] == 3);
    assert(res.arr[1] == 0);

    res = i128_from_d32(-1024.556e2);
    assert(res.arr[0] == (unsigned long) -102455);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_d32(~0ull);
    assert(res.arr[0] == 0xfffffc4b83fd4000ull);
    assert(res.arr[1] == 0);

    res = i128_from_d32(-(_Decimal32) ~0ull);
    assert(res.arr[0] == (unsigned long) -0xfffffc4b83fd4000ull);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_d32(2 * (_Decimal32) ~0ull);
    assert(res.arr[0] == (unsigned long) 0xfffff89707fa8000ull);
    assert(res.arr[1] == (unsigned long) 1);

    res = i128_from_d64(0.0);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_from_d64(1.0);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i128_from_d64(-1.0);
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_d64(3.14159);
    assert(res.arr[0] == 3);
    assert(res.arr[1] == 0);

    res = i128_from_d64(-1024.556e2);
    assert(res.arr[0] == (unsigned long) -102455);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_d64(~0ull);
    assert(res.arr[0] == 0xfffffffffffff9b0ull);
    assert(res.arr[1] == 0);

    res = i128_from_d64(-(_Decimal64) ~0ull);
    assert(res.arr[0] == (unsigned long) -0xfffffffffffff9b0ull);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_d64(2 * (_Decimal64) ~0ull);
    assert(res.arr[0] == (unsigned long) 0xfffffffffffff360ull);
    assert(res.arr[1] == (unsigned long) 1);

    res = i128_from_d128(0.0);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_from_d128(1.0);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i128_from_d128(-1.0);
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_d128(3.14159);
    assert(res.arr[0] == 3);
    assert(res.arr[1] == 0);

    res = i128_from_d128(-1024.556e2);
    assert(res.arr[0] == (unsigned long) -102455);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_d128(~0ull);
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == 0);

    res = i128_from_d128(-(_Decimal128) ~0ull);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_d128(2 * (_Decimal128) ~0ull);
    assert(res.arr[0] == ~0ull - 1);
    assert(res.arr[1] == (unsigned long) 1);

    res = u128_from_d32(0.0);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_from_d32(1.0);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u128_from_d32(3.14159);
    assert(res.arr[0] == 3);
    assert(res.arr[1] == 0);

    res = u128_from_d32(1024.556e2);
    assert(res.arr[0] == (unsigned long) 102455);
    assert(res.arr[1] == (unsigned long) 0);

    res = u128_from_d32(~0ull);
    assert(res.arr[0] == 0xfffffc4b83fd4000ull);
    assert(res.arr[1] == 0);

    res = u128_from_d32(2 * (_Decimal32) ~0ull);
    assert(res.arr[0] == (unsigned long) 0xfffff89707fa8000ull);
    assert(res.arr[1] == (unsigned long) 1);

    res = u128_from_d64(0.0);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_from_d64(1.0);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u128_from_d64(3.14159);
    assert(res.arr[0] == 3);
    assert(res.arr[1] == 0);

    res = u128_from_d64(1024.556e2);
    assert(res.arr[0] == (unsigned long) 102455);
    assert(res.arr[1] == (unsigned long) 0);

    res = u128_from_d64(~0ull);
    assert(res.arr[0] == 0xfffffffffffff9b0ull);
    assert(res.arr[1] == 0);

    res = u128_from_d64(2 * (_Decimal64) ~0ull);
    assert(res.arr[0] == (unsigned long) 0xfffffffffffff360ull);
    assert(res.arr[1] == (unsigned long) 1);

    res = u128_from_d128(0.0);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_from_d128(1.0);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u128_from_d128(3.14159);
    assert(res.arr[0] == 3);
    assert(res.arr[1] == 0);

    res = u128_from_d128(1024.556e2);
    assert(res.arr[0] == (unsigned long) 102455);
    assert(res.arr[1] == (unsigned long) 0);

    res = u128_from_d128(~0ull);
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == 0);

    res = u128_from_d128(2 * (_Decimal128) ~0ull);
    assert(res.arr[0] == ~0ull - 1);
    assert(res.arr[1] == (unsigned long) 1);
#endif
    return EXIT_SUCCESS;
}
