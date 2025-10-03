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

#if ((defined(__GNUC__) && !defined(__clang__) && !defined(__KEFIRCC__)) || defined(__KEFIRCC_DECIMAL_SUPPORT__)) && !defined(__NetBSD__)
#pragma GCC diagnostic ignored "-Wpedantic"
#define ENABLE_DECIMAL_TEST
#include <float.h>

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

extern _Decimal32 d32_inf;
extern _Decimal64 d64_inf;
extern _Decimal128 d128_inf;

extern _Decimal32 d32_nan;
extern _Decimal64 d64_nan;
extern _Decimal128 d128_nan;

extern _Decimal32 d32_snan;
extern _Decimal64 d64_snan;
extern _Decimal128 d128_snan;

_Decimal32 get_d32_inf(void);
_Decimal64 get_d64_inf(void);
_Decimal128 get_d128_inf(void);

_Decimal32 get_d32_nan(void);
_Decimal64 get_d64_nan(void);
_Decimal128 get_d128_nan(void);

_Decimal32 get_d32_snan(void);
_Decimal64 get_d64_snan(void);
_Decimal128 get_d128_snan(void);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    assert(d32_inf == 1.0df / 0.0df);
    assert(d64_inf == 1.0dd / 0.0dd);
    assert(d128_inf == 1.0dl / 0.0dl);

    assert(d32_nan != d32_nan);
    assert(d64_nan != d64_nan);
    assert(d128_nan != d128_nan);

    assert(d32_snan != d32_snan);
    assert(d64_snan != d64_snan);
    assert(d128_snan != d128_snan);

    assert(get_d32_inf() == 1.0df / 0.0df);
    assert(get_d64_inf() == 1.0dd / 0.0dd);
    assert(get_d128_inf() == 1.0dl / 0.0dl);

    _Decimal32 res32 = get_d32_nan();
    assert(res32 != res32);

    _Decimal64 res64 = get_d64_nan();
    assert(res64 != res64);

    _Decimal128 res128 = get_d128_nan();
    assert(res128 != res128);

    res32 = get_d32_snan();
    assert(res32 != res32);

    res64 = get_d64_snan();
    assert(res64 != res64);

    res128 = get_d128_snan();
    assert(res128 != res128);
#endif
    return EXIT_SUCCESS;
}
