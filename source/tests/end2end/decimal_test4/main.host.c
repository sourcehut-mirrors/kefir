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
#define ENABLE_DECIMAL_TEST
#pragma GCC diagnostic ignored "-Wpedantic"
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

#define DECIMAL_TYPE _Decimal32
#define ALG_PREFIX ref_dec32
#include "./common.h"

#undef DECIMAL_TYPE
#undef ALG_PREFIX
#define DECIMAL_TYPE _Decimal64
#define ALG_PREFIX ref_dec64
#include "./common.h"

#undef DECIMAL_TYPE
#undef ALG_PREFIX
#define DECIMAL_TYPE _Decimal128
#define ALG_PREFIX ref_dec128
#include "./common.h"

_Decimal32 CONCAT(test_dec32, _run)(unsigned long, int);
_Decimal64 CONCAT(test_dec64, _run)(unsigned long, int);
_Decimal128 CONCAT(test_dec128, _run)(unsigned long, int);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    assert(decimal32_eq(CONCAT(test_dec32, _run)(16, 100), CONCAT(ref_dec32, _run)(16, 100)));
    assert(decimal64_eq(CONCAT(test_dec64, _run)(16, 100), CONCAT(ref_dec64, _run)(16, 100)));
    assert(decimal128_eq(CONCAT(test_dec128, _run)(16, 100), CONCAT(ref_dec128, _run)(16, 100)));
#endif
    return EXIT_SUCCESS;
}
