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

#if ((defined(__GNUC__) && !defined(__clang__) && !defined(__KEFIRCC__)) || defined(__KEFIRCC_DECIMAL_SUPPORT__)) && !defined(__NetBSD__) && !defined(__DragonFly__)
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

_Decimal32 add32(void);
_Decimal64 add64(void);
_Decimal128 add128(void);
_Decimal32 sub32(void);
_Decimal64 sub64(void);
_Decimal128 sub128(void);
_Decimal32 mul32(void);
_Decimal64 mul64(void);
_Decimal128 mul128(void);
_Decimal32 div32(void);
_Decimal64 div64(void);
_Decimal128 div128(void);
_Decimal32 neg32(void);
_Decimal64 neg64(void);
_Decimal128 neg128(void);
int eq32_1(void);
int eq32_2(void);
int eq64_1(void);
int eq64_2(void);
int eq128_1(void);
int eq128_2(void);
int gt32_1(void);
int gt32_2(void);
int gt64_1(void);
int gt64_2(void);
int gt128_1(void);
int gt128_2(void);
int lt32_1(void);
int lt32_2(void);
int lt64_1(void);
int lt64_2(void);
int lt128_1(void);
int lt128_2(void);
int isnan32_1(void);
int isnan32_2(void);
int isnan64_1(void);
int isnan64_2(void);
int isnan128_1(void);
int isnan128_2(void);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    assert(decimal32_eq(add32(), 6.428 + 1891.1));
    assert(decimal64_eq(add64(), 94291.42 + 9103.14));
    assert(decimal128_eq(add128(), 842419.931 + 39103.193));
    assert(decimal32_eq(sub32(), 6.428 - 1891.1));
    assert(decimal64_eq(sub64(), 94291.42 - 9103.14));
    assert(decimal128_eq(sub128(), 842419.931 - 39103.193));
    assert(decimal32_eq(mul32(), 6.428 * 1891.1));
    assert(decimal64_eq(mul64(), 94291.42 * 9103.14));
    assert(decimal128_eq(mul128(), 842419.931 * 39103.193));
    assert(decimal32_eq(div32(), 6.428 / 1891.1));
    assert(decimal64_eq(div64(), 94291.42 / 9103.14));
    assert(decimal128_eq(div128(), 842419.931 / 39103.193));
    assert(decimal32_eq(neg32(), -6.428));
    assert(decimal64_eq(neg64(), -94291.42));
    assert(decimal128_eq(neg128(), -842419.931));
    assert(!eq32_1());
    assert(eq32_2());
    assert(!eq64_1());
    assert(eq64_2());
    assert(!eq128_1());
    assert(eq128_2());
    assert(!gt32_1());
    assert(gt32_2());
    assert(gt64_1());
    assert(!gt64_2());
    assert(gt128_1());
    assert(!gt128_2());
    assert(lt32_1());
    assert(!lt32_2());
    assert(!lt64_1());
    assert(lt64_2());
    assert(!lt128_1());
    assert(lt128_2());
    assert(!isnan32_1());
    assert(isnan32_2());
    assert(!isnan64_1());
    assert(isnan64_2());
    assert(!isnan128_1());
    assert(isnan128_2());
#endif
    return EXIT_SUCCESS;
}
