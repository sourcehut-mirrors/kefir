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
#pragma GCC diagnostic ignored "-Wpedantic"
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

#define ENABLE_DECIMAL_TEST
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

_Decimal32 CONCAT(test_dec32, _linear_regression)(const CONCAT(ref_dec32, _Point_t) *, unsigned long, _Decimal32 *,
                                                  _Decimal32 *);
_Decimal64 CONCAT(test_dec64, _linear_regression)(const CONCAT(ref_dec64, _Point_t) *, unsigned long, _Decimal64 *,
                                                  _Decimal64 *);
_Decimal128 CONCAT(test_dec128, _linear_regression)(const CONCAT(ref_dec128, _Point_t) *, unsigned long, _Decimal128 *,
                                                    _Decimal128 *);

void test32(void) {
    CONCAT(ref_dec32, _Point_t)
    points[5] = {{1.0df, 2.0df}, {2.0df, 4.1df}, {3.0df, 6.0df}, {4.0df, 8.1df}, {5.0df, 10.0df}};
    _Decimal32 ref_m, ref_b, test_m, test_b;

    _Decimal32 ref_error = CONCAT(ref_dec32, _linear_regression)(points, 5, &ref_m, &ref_b);
    _Decimal32 test_error = CONCAT(test_dec32, _linear_regression)(points, 5, &test_m, &test_b);

    assert(decimal32_eq(ref_m, test_m));
    assert(decimal32_eq(ref_b, test_b));
    assert(decimal32_eq(ref_error, test_error));
}

void test64(void) {
    CONCAT(ref_dec64, _Point_t)
    points[5] = {{1.0df, 2.0df}, {2.0df, 4.1df}, {3.0df, 6.0df}, {4.0df, 8.1df}, {5.0df, 10.0df}};
    _Decimal64 ref_m, ref_b, test_m, test_b;

    _Decimal64 ref_error = CONCAT(ref_dec64, _linear_regression)(points, 5, &ref_m, &ref_b);
    _Decimal64 test_error = CONCAT(test_dec64, _linear_regression)(points, 5, &test_m, &test_b);

    assert(decimal64_eq(ref_m, test_m));
    assert(decimal64_eq(ref_b, test_b));
    assert(decimal64_eq(ref_error, test_error));
}

void test128(void) {
    CONCAT(ref_dec128, _Point_t)
    points[5] = {{1.0df, 2.0df}, {2.0df, 4.1df}, {3.0df, 6.0df}, {4.0df, 8.1df}, {5.0df, 10.0df}};
    _Decimal128 ref_m, ref_b, test_m, test_b;

    _Decimal128 ref_error = CONCAT(ref_dec128, _linear_regression)(points, 5, &ref_m, &ref_b);
    _Decimal128 test_error = CONCAT(test_dec128, _linear_regression)(points, 5, &test_m, &test_b);

    assert(decimal128_eq(ref_m, test_m));
    assert(decimal128_eq(ref_b, test_b));
    assert(decimal128_eq(ref_error, test_error));
}
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    test32();
    test64();
    test128();
#endif
    return EXIT_SUCCESS;
}
