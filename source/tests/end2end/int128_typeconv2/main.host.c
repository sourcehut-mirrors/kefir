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

_Decimal32 i128_to_d32(struct i128);
_Decimal64 i128_to_d64(struct i128);
_Decimal128 i128_to_d128(struct i128);

_Decimal32 u128_to_d32(struct i128);
_Decimal64 u128_to_d64(struct i128);
_Decimal128 u128_to_d128(struct i128);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    assert(decimal32_eq(i128_to_d32((struct i128){{0, 0}}), 0.0));
    assert(decimal32_eq(i128_to_d32((struct i128){{1, 0}}), 1.0));
    assert(decimal32_eq(i128_to_d32((struct i128){{1024, 0}}), 1024.0));
    assert(decimal32_eq(i128_to_d32((struct i128){{~0ull, 0}}), (_Decimal32) ~0ull));
    assert(decimal32_eq(i128_to_d32((struct i128){{0, 1}}), 1 + (_Decimal32) ~0ull));
    assert(decimal32_eq(i128_to_d32((struct i128){{-1, -1}}), -1));

    assert(decimal32_eq(u128_to_d32((struct i128){{0, 0}}), 0.0));
    assert(decimal32_eq(u128_to_d32((struct i128){{1, 0}}), 1.0));
    assert(decimal32_eq(u128_to_d32((struct i128){{1024, 0}}), 1024.0));
    assert(decimal32_eq(u128_to_d32((struct i128){{~0ull, 0}}), (_Decimal32) ~0ull));
    assert(decimal32_eq(u128_to_d32((struct i128){{0, 1}}), 1 + (_Decimal32) ~0ull));
    assert(decimal32_eq(u128_to_d32((struct i128){{-1, -1}}), 3.402824E+38df));

    assert(decimal64_eq(i128_to_d64((struct i128){{0, 0}}), 0.0));
    assert(decimal64_eq(i128_to_d64((struct i128){{1, 0}}), 1.0));
    assert(decimal64_eq(i128_to_d64((struct i128){{1024, 0}}), 1024.0));
    assert(decimal64_eq(i128_to_d64((struct i128){{~0ull, 0}}), (_Decimal64) ~0ull));
    assert(decimal64_eq(i128_to_d64((struct i128){{0, 1}}), 1 + (_Decimal64) ~0ull));
    assert(decimal64_eq(i128_to_d64((struct i128){{-1, -1}}), -1));

    assert(decimal64_eq(u128_to_d64((struct i128){{0, 0}}), 0.0));
    assert(decimal64_eq(u128_to_d64((struct i128){{1, 0}}), 1.0));
    assert(decimal64_eq(u128_to_d64((struct i128){{1024, 0}}), 1024.0));
    assert(decimal64_eq(u128_to_d64((struct i128){{~0ull, 0}}), (_Decimal64) ~0ull));
    assert(decimal64_eq(u128_to_d64((struct i128){{0, 1}}), 1 + (_Decimal64) ~0ull));
    assert(decimal64_eq(u128_to_d64((struct i128){{-1, -1}}), 3.402823669209385E+38dd));

    assert(decimal128_eq(i128_to_d128((struct i128){{0, 0}}), 0.0));
    assert(decimal128_eq(i128_to_d128((struct i128){{1, 0}}), 1.0));
    assert(decimal128_eq(i128_to_d128((struct i128){{1024, 0}}), 1024.0));
    assert(decimal128_eq(i128_to_d128((struct i128){{~0ull, 0}}), (_Decimal128) ~0ull));
    assert(decimal128_eq(i128_to_d128((struct i128){{0, 1}}), 1 + (_Decimal128) ~0ull));
    assert(decimal128_eq(i128_to_d128((struct i128){{-1, -1}}), -1));

    assert(decimal128_eq(u128_to_d128((struct i128){{0, 0}}), 0.0));
    assert(decimal128_eq(u128_to_d128((struct i128){{1, 0}}), 1.0));
    assert(decimal128_eq(u128_to_d128((struct i128){{1024, 0}}), 1024.0));
    assert(decimal128_eq(u128_to_d128((struct i128){{~0ull, 0}}), (_Decimal128) ~0ull));
    assert(decimal128_eq(u128_to_d128((struct i128){{0, 1}}), 1 + (_Decimal128) ~0ull));
    assert(decimal128_eq(u128_to_d128((struct i128){{-1, -1}}), 3.402823669209384634633746074317682E+38dl));
#endif
    return EXIT_SUCCESS;
}
