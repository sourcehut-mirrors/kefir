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

_Decimal32 fn1(_Decimal32 x, _Decimal32 y) {
    return x + y;
}

_Decimal32 fn2(_Decimal32 x, _Decimal32 y) {
    return x - y;
}

_Decimal64 fn3(_Decimal64 x, _Decimal64 y) {
    return x + y;
}

_Decimal64 fn4(_Decimal64 x, _Decimal64 y) {
    return x - y;
}

_Decimal128 fn5(_Decimal128 x, _Decimal128 y) {
    return x + y;
}

_Decimal128 fn6(_Decimal128 x, _Decimal128 y) {
    return x - y;
}

_Decimal32 test1(_Decimal32 *);
_Decimal32 test2(_Decimal32 (*)(_Decimal32, _Decimal32), _Decimal32 *);
_Decimal64 test3(_Decimal64 *);
_Decimal64 test4(_Decimal64 (*)(_Decimal64, _Decimal64), _Decimal64 *);
_Decimal128 test5(_Decimal128 *);
_Decimal128 test6(_Decimal128 (*)(_Decimal128, _Decimal128), _Decimal128 *);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    assert(decimal32_eq(test1((_Decimal32[]){3.14159, 2.71828}), 3.14159 + 2.71828));
    assert(decimal32_eq(test2(fn2, (_Decimal32[]){3.14159, 2.71828}), 3.14159 - 2.71828));
    assert(decimal64_eq(test3((_Decimal64[]){3.14159, 2.71828}), 3.14159 + 2.71828));
    assert(decimal64_eq(test4(fn4, (_Decimal64[]){3.14159, 2.71828}), 3.14159 - 2.71828));
    assert(decimal128_eq(test5((_Decimal128[]){3.14159, 2.71828}), 3.14159 + 2.71828));
    assert(decimal128_eq(test6(fn6, (_Decimal128[]){3.14159, 2.71828}), 3.14159 - 2.71828));
#endif
    return EXIT_SUCCESS;
}
