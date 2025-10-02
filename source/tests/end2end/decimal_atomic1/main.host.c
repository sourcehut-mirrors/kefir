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

#if defined(__GNUC__) && !defined(__clang__) && !defined(__KEFIRCC__) && !defined(__NetBSD__)
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

_Decimal32 load32(_Atomic _Decimal32 *);
_Decimal64 load64(_Atomic _Decimal64 *);
_Decimal128 load128(_Atomic _Decimal128 *);

void store32(_Atomic _Decimal32 *, _Decimal32);
void store64(_Atomic _Decimal64 *, _Decimal64);
void store128(_Atomic _Decimal128 *, _Decimal128);

_Decimal32 add32(_Atomic _Decimal32 *, _Decimal32);
_Decimal64 add64(_Atomic _Decimal64 *, _Decimal64);
_Decimal128 add128(_Atomic _Decimal128 *, _Decimal128);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    _Atomic _Decimal32 x32 = 3.1489;
    assert(decimal32_eq(load32(&x32), 3.1489));

    _Atomic _Decimal64 x64 = 582.43819;
    assert(decimal64_eq(load64(&x64), 582.43819));

    _Atomic _Decimal128 x128 = 831831.3718;
    assert(decimal128_eq(load128(&x128), 831831.3718));

    store32(&x32, -3189.2);
    assert(decimal32_eq(x32, -3189.2));

    store64(&x64, 9000198.134);
    assert(decimal64_eq(x64, 9000198.134));

    store128(&x128, -832919.31891);
    assert(decimal128_eq(x128, -832919.31891));

    x32 = 4.1326;
    assert(decimal32_eq(add32(&x32, -6.42), 4.1326 - 6.42));

    x64 = 4.1326;
    assert(decimal64_eq(add64(&x64, -6.42), 4.1326 - 6.42));

    x128 = 4.1326;
    assert(decimal128_eq(add128(&x128, -6.42), 4.1326 - 6.42));
#endif
    return EXIT_SUCCESS;
}
