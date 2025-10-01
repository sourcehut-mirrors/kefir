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

#if defined(__GNUC__) && !defined(__clang__) && !defined(__KEFIRCC__)
#pragma GCC diagnostic ignored "-Wpedantic"
#define ENABLE_DECIMAL_TEST
_Bool decimal32_eq(_Decimal32 a, _Decimal32 b) {
    _Decimal32 diff = (a - b) * 1000;
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

_Decimal32 sum32(int, ...);
_Decimal64 sum64(int, ...);
_Decimal128 sum128(int, ...);
_Decimal128 sum128_2(int, ...);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    assert(decimal32_eq(sum32(0), 0.0));
    assert(decimal32_eq(sum32(0, (_Decimal32) 3.14159), 0.0));
    assert(decimal32_eq(sum32(1, (_Decimal32) 3.14159), 3.14159));
    assert(decimal32_eq(sum32(1, (_Decimal32) 3.14159, (_Decimal32) 2.71), 3.14159));
    assert(decimal32_eq(sum32(2, (_Decimal32) 3.14159, (_Decimal32) 2.71), 3.14159 + 2.71));
    assert(decimal32_eq(sum32(3, (_Decimal32) 3.14159, (_Decimal32) 2.71, (_Decimal32) -1), 3.14159 + 2.71 - 1));
    assert(decimal32_eq(sum32(4, (_Decimal32) 3.14159, (_Decimal32) 2.71, (_Decimal32) -1, (_Decimal32) 0.0), 3.14159 + 2.71 - 1));
    assert(decimal32_eq(sum32(5, (_Decimal32) 3.14159, (_Decimal32) 2.71, (_Decimal32) -1, (_Decimal32) 0.0, (_Decimal32) 1000.1), 3.14159 + 2.71 - 1 + 1000.1));
    assert(decimal32_eq(sum32(6, (_Decimal32) 3.14159, (_Decimal32) 2.71, (_Decimal32) -1, (_Decimal32) 0.0, (_Decimal32) 1000.1, (_Decimal32) 0.001), 3.14159 + 2.71 - 1 + 1000.1 + 0.001));
    assert(decimal32_eq(sum32(7, (_Decimal32) 3.14159, (_Decimal32) 2.71, (_Decimal32) -1, (_Decimal32) 0.0, (_Decimal32) 1000.1, (_Decimal32) 0.001,
        (_Decimal32) -234.2), 3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2));
    assert(decimal32_eq(sum32(8, (_Decimal32) 3.14159, (_Decimal32) 2.71, (_Decimal32) -1, (_Decimal32) 0.0, (_Decimal32) 1000.1, (_Decimal32) 0.001,
        (_Decimal32) -234.2, (_Decimal32) 3.281), 3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2 + 3.281));
    assert(decimal32_eq(sum32(9, (_Decimal32) 3.14159, (_Decimal32) 2.71, (_Decimal32) -1, (_Decimal32) 0.0, (_Decimal32) 1000.1, (_Decimal32) 0.001,
        (_Decimal32) -234.2, (_Decimal32) 3.281, (_Decimal32) -9000), 3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2 + 3.281 - 9000));
    assert(decimal32_eq(sum32(10, (_Decimal32) 3.14159, (_Decimal32) 2.71, (_Decimal32) -1, (_Decimal32) 0.0, (_Decimal32) 1000.1, (_Decimal32) 0.001,
        (_Decimal32) -234.2, (_Decimal32) 3.281, (_Decimal32) -9000, (_Decimal32) 0.12), 3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2 + 3.281 - 9000 + 0.12));
    assert(decimal32_eq(sum32(11, (_Decimal32) 3.14159, (_Decimal32) 2.71, (_Decimal32) -1, (_Decimal32) 0.0, (_Decimal32) 1000.1, (_Decimal32) 0.001,
        (_Decimal32) -234.2, (_Decimal32) 3.281, (_Decimal32) -9000, (_Decimal32) 0.12, (_Decimal32) 1.1),
        3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2 + 3.281 - 9000 + 0.12 + 1.1));
    assert(decimal32_eq(sum32(12, (_Decimal32) 3.14159, (_Decimal32) 2.71, (_Decimal32) -1, (_Decimal32) 0.0, (_Decimal32) 1000.1, (_Decimal32) 0.001,
        (_Decimal32) -234.2, (_Decimal32) 3.281, (_Decimal32) -9000, (_Decimal32) 0.12, (_Decimal32) 0.0, (_Decimal32) 1.1),
        3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2 + 3.281 - 9000 + 0.12 + 1.1));

    assert(decimal64_eq(sum64(0), 0.0));
    assert(decimal64_eq(sum64(0, (_Decimal64) 3.14159), 0.0));
    assert(decimal64_eq(sum64(1, (_Decimal64) 3.14159), 3.14159));
    assert(decimal64_eq(sum64(1, (_Decimal64) 3.14159, (_Decimal64) 2.71), 3.14159));
    assert(decimal64_eq(sum64(2, (_Decimal64) 3.14159, (_Decimal64) 2.71), 3.14159 + 2.71));
    assert(decimal64_eq(sum64(3, (_Decimal64) 3.14159, (_Decimal64) 2.71, (_Decimal64) -1), 3.14159 + 2.71 - 1));
    assert(decimal64_eq(sum64(4, (_Decimal64) 3.14159, (_Decimal64) 2.71, (_Decimal64) -1, (_Decimal64) 0.0), 3.14159 + 2.71 - 1));
    assert(decimal64_eq(sum64(5, (_Decimal64) 3.14159, (_Decimal64) 2.71, (_Decimal64) -1, (_Decimal64) 0.0, (_Decimal64) 1000.1), 3.14159 + 2.71 - 1 + 1000.1));
    assert(decimal64_eq(sum64(6, (_Decimal64) 3.14159, (_Decimal64) 2.71, (_Decimal64) -1, (_Decimal64) 0.0, (_Decimal64) 1000.1, (_Decimal64) 0.001), 3.14159 + 2.71 - 1 + 1000.1 + 0.001));
    assert(decimal64_eq(sum64(7, (_Decimal64) 3.14159, (_Decimal64) 2.71, (_Decimal64) -1, (_Decimal64) 0.0, (_Decimal64) 1000.1, (_Decimal64) 0.001,
        (_Decimal64) -234.2), 3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2));
    assert(decimal64_eq(sum64(8, (_Decimal64) 3.14159, (_Decimal64) 2.71, (_Decimal64) -1, (_Decimal64) 0.0, (_Decimal64) 1000.1, (_Decimal64) 0.001,
        (_Decimal64) -234.2, (_Decimal64) 3.281), 3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2 + 3.281));
    assert(decimal64_eq(sum64(9, (_Decimal64) 3.14159, (_Decimal64) 2.71, (_Decimal64) -1, (_Decimal64) 0.0, (_Decimal64) 1000.1, (_Decimal64) 0.001,
        (_Decimal64) -234.2, (_Decimal64) 3.281, (_Decimal64) -9000), 3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2 + 3.281 - 9000));
    assert(decimal64_eq(sum64(10, (_Decimal64) 3.14159, (_Decimal64) 2.71, (_Decimal64) -1, (_Decimal64) 0.0, (_Decimal64) 1000.1, (_Decimal64) 0.001,
        (_Decimal64) -234.2, (_Decimal64) 3.281, (_Decimal64) -9000, (_Decimal64) 0.12), 3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2 + 3.281 - 9000 + 0.12));
    assert(decimal64_eq(sum64(11, (_Decimal64) 3.14159, (_Decimal64) 2.71, (_Decimal64) -1, (_Decimal64) 0.0, (_Decimal64) 1000.1, (_Decimal64) 0.001,
        (_Decimal64) -234.2, (_Decimal64) 3.281, (_Decimal64) -9000, (_Decimal64) 0.12, (_Decimal64) 1.1),
        3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2 + 3.281 - 9000 + 0.12 + 1.1));
    assert(decimal64_eq(sum64(12, (_Decimal64) 3.14159, (_Decimal64) 2.71, (_Decimal64) -1, (_Decimal64) 0.0, (_Decimal64) 1000.1, (_Decimal64) 0.001,
        (_Decimal64) -234.2, (_Decimal64) 3.281, (_Decimal64) -9000, (_Decimal64) 0.12, (_Decimal64) 0.0, (_Decimal64) 1.1),
        3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2 + 3.281 - 9000 + 0.12 + 1.1));

    assert(decimal128_eq(sum128(0), 0.0));
    assert(decimal128_eq(sum128(0, (_Decimal128) 3.14159), 0.0));
    assert(decimal128_eq(sum128(1, (_Decimal128) 3.14159), 3.14159));
    assert(decimal128_eq(sum128(1, (_Decimal128) 3.14159, (_Decimal128) 2.71), 3.14159));
    assert(decimal128_eq(sum128(2, (_Decimal128) 3.14159, (_Decimal128) 2.71), 3.14159 + 2.71));
    assert(decimal128_eq(sum128(3, (_Decimal128) 3.14159, (_Decimal128) 2.71, (_Decimal128) -1), 3.14159 + 2.71 - 1));
    assert(decimal128_eq(sum128(4, (_Decimal128) 3.14159, (_Decimal128) 2.71, (_Decimal128) -1, (_Decimal128) 0.0), 3.14159 + 2.71 - 1));
    assert(decimal128_eq(sum128(5, (_Decimal128) 3.14159, (_Decimal128) 2.71, (_Decimal128) -1, (_Decimal128) 0.0, (_Decimal128) 1000.1), 3.14159 + 2.71 - 1 + 1000.1));
    assert(decimal128_eq(sum128(6, (_Decimal128) 3.14159, (_Decimal128) 2.71, (_Decimal128) -1, (_Decimal128) 0.0, (_Decimal128) 1000.1, (_Decimal128) 0.001), 3.14159 + 2.71 - 1 + 1000.1 + 0.001));
    assert(decimal128_eq(sum128(7, (_Decimal128) 3.14159, (_Decimal128) 2.71, (_Decimal128) -1, (_Decimal128) 0.0, (_Decimal128) 1000.1, (_Decimal128) 0.001,
        (_Decimal128) -234.2), 3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2));
    assert(decimal128_eq(sum128(8, (_Decimal128) 3.14159, (_Decimal128) 2.71, (_Decimal128) -1, (_Decimal128) 0.0, (_Decimal128) 1000.1, (_Decimal128) 0.001,
        (_Decimal128) -234.2, (_Decimal128) 3.281), 3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2 + 3.281));
    assert(decimal128_eq(sum128(9, (_Decimal128) 3.14159, (_Decimal128) 2.71, (_Decimal128) -1, (_Decimal128) 0.0, (_Decimal128) 1000.1, (_Decimal128) 0.001,
        (_Decimal128) -234.2, (_Decimal128) 3.281, (_Decimal128) -9000), 3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2 + 3.281 - 9000));
    assert(decimal128_eq(sum128(10, (_Decimal128) 3.14159, (_Decimal128) 2.71, (_Decimal128) -1, (_Decimal128) 0.0, (_Decimal128) 1000.1, (_Decimal128) 0.001,
        (_Decimal128) -234.2, (_Decimal128) 3.281, (_Decimal128) -9000, (_Decimal128) 0.12), 3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2 + 3.281 - 9000 + 0.12));
    assert(decimal128_eq(sum128(11, (_Decimal128) 3.14159, (_Decimal128) 2.71, (_Decimal128) -1, (_Decimal128) 0.0, (_Decimal128) 1000.1, (_Decimal128) 0.001,
        (_Decimal128) -234.2, (_Decimal128) 3.281, (_Decimal128) -9000, (_Decimal128) 0.12, (_Decimal128) 1.1),
        3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2 + 3.281 - 9000 + 0.12 + 1.1));
    assert(decimal128_eq(sum128(12, (_Decimal128) 3.14159, (_Decimal128) 2.71, (_Decimal128) -1, (_Decimal128) 0.0, (_Decimal128) 1000.1, (_Decimal128) 0.001,
        (_Decimal128) -234.2, (_Decimal128) 3.281, (_Decimal128) -9000, (_Decimal128) 0.12, (_Decimal128) 0.0, (_Decimal128) 1.1),
        3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2 + 3.281 - 9000 + 0.12 + 1.1));

    assert(decimal128_eq(sum128_2(12, (_Decimal128) 3.14159, 0.0, (_Decimal128) 2.71, 0.0, (_Decimal128) -1, 0.0, (_Decimal128) 0.0, 0.0, (_Decimal128) 1000.1, 0.0, (_Decimal128) 0.001, 0.0,
        (_Decimal128) -234.2, 0.0, (_Decimal128) 3.281, 0.0, (_Decimal128) -9000, 0.0, (_Decimal128) 0.12, 0.0, (_Decimal128) 0.0, 0.0, (_Decimal128) 1.1, 0.0),
        3.14159 + 2.71 - 1 + 1000.1 + 0.001 - 234.2 + 3.281 - 9000 + 0.12 + 1.1));
#endif
    return EXIT_SUCCESS;
}
