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
#include <complex.h>
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

#endif

int main(void) {
    assert(f32_size == sizeof(float));
    assert(f32_alignment == _Alignof(float));
    assert(fabs(*f32_const_ptr - 9.831f) < 1e-6);

    int i = 0;
    assert(f32_compat[i++] == 1);
    assert(f32_compat[i++] == 1);
    assert(f32_compat[i++] == 1);
    assert(f32_compat[i++] == 2);
    assert(f32_compat[i++] == 2);
    assert(f32_compat[i++] == 3);
    assert(f32_compat[i++] == 3);
    assert(f32_compat[i++] == 1);
    assert(f32_compat[i++] == 1);
    assert(f32_compat[i++] == 3);
    assert(f32_compat[i++] == 3);
    assert(f32_compat[i++] == 1);
    assert(f32_compat[i++] == 1);
    assert(f32_compat[i++] == 3);
    assert(f32_compat[i++] == 3);
    assert(f32_compat[i++] == 3);
    assert(f32_compat[i++] == 3);
    assert(f32_compat[i++] == 3);
    assert(f32_compat[i++] == 3);
    assert(f32_compat[i++] == 3);
    assert(f32_compat[i++] == 3);

    i = 0;
    assert(fabs(f32_arr[i++] - 3.14159f) < 1e-6);
    assert(fabs(f32_arr[i++]) < 1e-6);
    assert(fabs(f32_arr[i++] - 2.71828f) < 1e-6);
    assert(fabs(f32_arr[i++] + 3.14159f) < 1e-6);
    assert(fabs(f32_arr[i++]) < 1e-6);
    assert(fabs(f32_arr[i++] - 2.71828) < 1e-6);
    assert(fabs(f32_arr[i++] + 1.14159) < 1e-6);
    assert(fabs(f32_arr[i++]) < 1e-6);
    assert(fabs(f32_arr[i++] - 2.71823) < 1e-6);
    assert(fabs(f32_arr[i++]) < 1e-6);
    assert(fabs(f32_arr[i++] - 9420.0) < 1e-6);
    assert(fabs(f32_arr[i++]) < 1e-6);
    assert(fabs(f32_arr[i++] - 9420.0) < 1e-6);
    assert(fabs(f32_arr[i++]) < 1e-6);
    assert(fabs(f32_arr[i++] - 9420.0) < 1e-6);
    assert(fabs(f32_arr[i++]) < 1e-6);
    assert(fabs(f32_arr[i++] - 9420.0) < 1e-6);
    assert(fabs(f32_arr[i++]) < 1e-6);
    assert(fabs(f32_arr[i++] - 9420.0) < 1e-6);
    assert(fabs(f32_arr[i++]) < 1e-6);
    assert(fabs(f32_arr[i++]) < 1e-6);
    assert(fabs(f32_arr[i++]) < 1e-6);
    assert(fabs(f32_arr[i++]) < 1e-6);
    assert(fabs(f32_arr[i++]) < 1e-6);
    assert(fabs(f32_arr[i++]) < 1e-6);

    assert(fabs(f32_arr2[0]) < 1e-6);
    assert(fabs(f32_arr2[1]) < 1e-6);
    assert(fabs(f32_arr2[2]) < 1e-6);

    assert(fabs(creal(f32_arr3[0])) < 1e-6);
    assert(fabs(creal(f32_arr3[1])) < 1e-6);
    assert(fabs(creal(f32_arr3[2])) < 1e-6);
    assert(fabs(cimag(f32_arr3[0]) - 4721.7f) < 1e-3);
    assert(fabs(cimag(f32_arr3[1]) + 4721.7) < 1e-3);
    assert(fabs(cimag(f32_arr3[2]) - 4761.7) < 1e-3);

    assert(fabs(f32_arr4[0]) < 1e-6);
    assert(fabs(f32_arr4[1]) < 1e-6);
    assert(fabs(f32_arr4[2]) < 1e-6);

    assert(fabs(creal(f32_arr5[0])) < 1e-6);
    assert(fabs(creal(f32_arr5[1])) < 1e-6);
    assert(fabs(creal(f32_arr5[2])) < 1e-6);
    assert(fabs(cimag(f32_arr5[0]) - 4721.7f) < 1e-3);
    assert(fabs(cimag(f32_arr5[1]) + 4721.7) < 1e-3);
    assert(fabs(cimag(f32_arr5[2]) - 4761.7) < 1e-3);

    assert(fabs(f64_arr2[0]) < 1e-6);
    assert(fabs(f64_arr2[1]) < 1e-6);
    assert(fabs(f64_arr2[2]) < 1e-6);

    assert(fabs(creal(f64_arr3[0])) < 1e-6);
    assert(fabs(creal(f64_arr3[1])) < 1e-6);
    assert(fabs(creal(f64_arr3[2])) < 1e-6);
    assert(fabs(cimag(f64_arr3[0]) - 4721.7f) < 1e-3);
    assert(fabs(cimag(f64_arr3[1]) + 4721.7) < 1e-3);
    assert(fabs(cimag(f64_arr3[2]) - 4761.7) < 1e-3);

    assert(fabs(f64_arr4[0]) < 1e-6);
    assert(fabs(f64_arr4[1]) < 1e-6);
    assert(fabs(f64_arr4[2]) < 1e-6);

    assert(fabs(creal(f64_arr5[0])) < 1e-6);
    assert(fabs(creal(f64_arr5[1])) < 1e-6);
    assert(fabs(creal(f64_arr5[2])) < 1e-6);
    assert(fabs(cimag(f64_arr5[0]) - 4721.7f) < 1e-3);
    assert(fabs(cimag(f64_arr5[1]) + 4721.7) < 1e-3);
    assert(fabs(cimag(f64_arr5[2]) - 4761.7) < 1e-3);

    assert(fabs(f64_arr6[0]) < 1e-6);
    assert(fabs(f64_arr6[1]) < 1e-6);
    assert(fabs(f64_arr6[2]) < 1e-6);

    assert(fabs(creal(f64_arr7[0])) < 1e-6);
    assert(fabs(creal(f64_arr7[1])) < 1e-6);
    assert(fabs(creal(f64_arr7[2])) < 1e-6);
    assert(fabs(cimag(f64_arr7[0]) - 4721.7f) < 1e-3);
    assert(fabs(cimag(f64_arr7[1]) + 4721.7) < 1e-3);
    assert(fabs(cimag(f64_arr7[2]) - 4761.7) < 1e-3);

    assert(fabsl(f80_arr2[0]) < 1e-6);
    assert(fabsl(f80_arr2[1]) < 1e-6);
    assert(fabsl(f80_arr2[2]) < 1e-6);

    assert(fabsl(creall(f80_arr3[0])) < 1e-6);
    assert(fabsl(creall(f80_arr3[1])) < 1e-6);
    assert(fabsl(creall(f80_arr3[2])) < 1e-6);
    assert(fabsl(cimagl(f80_arr3[0]) - 4721.7f) < 1e-3);
    assert(fabsl(cimagl(f80_arr3[1]) + 4721.7) < 1e-3);
    assert(fabsl(cimagl(f80_arr3[2]) - 4761.7) < 1e-3);

    assert(fabsl(f80_arr4[0]) < 1e-6);
    assert(fabsl(f80_arr4[1]) < 1e-6);
    assert(fabsl(f80_arr4[2]) < 1e-6);

    assert(fabsl(creall(f80_arr5[0])) < 1e-6);
    assert(fabsl(creall(f80_arr5[1])) < 1e-6);
    assert(fabsl(creall(f80_arr5[2])) < 1e-6);
    assert(fabsl(cimagl(f80_arr5[0]) - 4721.7f) < 1e-3);
    assert(fabsl(cimagl(f80_arr5[1]) + 4721.7) < 1e-3);
    assert(fabsl(cimagl(f80_arr5[2]) - 4761.7) < 1e-3);

    assert(fabsl(f80_arr6[0]) < 1e-6);
    assert(fabsl(f80_arr6[1]) < 1e-6);
    assert(fabsl(f80_arr6[2]) < 1e-6);

    assert(fabsl(creall(f80_arr7[0])) < 1e-6);
    assert(fabsl(creall(f80_arr7[1])) < 1e-6);
    assert(fabsl(creall(f80_arr7[2])) < 1e-6);
    assert(fabsl(cimagl(f80_arr7[0]) - 4721.7f) < 1e-3);
    assert(fabsl(cimagl(f80_arr7[1]) + 4721.7) < 1e-3);
    assert(fabsl(cimagl(f80_arr7[2]) - 4761.7) < 1e-3);

    i = 0;
    assert(fabs(f32_ops[i++] - 12.0f) < 1e-6);
    assert(fabs(f32_ops[i++] - 3.0f) < 1e-6);
    assert(fabs(f32_ops[i++] - 9.0f) < 1e-6);
    assert(fabs(f32_ops[i++] - 12.0f) < 1e-6);
    assert(fabs(f32_ops[i++] - 6.0f) < 1e-6);
    assert(fabs(f32_ops[i++] - 3.1f) < 1e-6);
    assert(fabs(f32_ops[i++] - 3.2f) < 1e-6);
    assert(fabs(f32_ops[i++] - 3.4f) < 1e-6);
    assert(fabs(f32_ops[i++] - 3.5f) < 1e-6);

    assert(fabs(f32_ops[i++] + 6.0f) < 1e-6);
    assert(fabs(f32_ops[i++] - 3.0f) < 1e-6);
    assert(fabs(f32_ops[i++] + 9.0f) < 1e-6);
    assert(fabs(f32_ops[i++] + 6.0f) < 1e-6);
    assert(fabs(f32_ops[i++] - 12.0f) < 1e-6);
    assert(fabs(f32_ops[i++] - 3.1f) < 1e-6);
    assert(fabs(f32_ops[i++] - 3.2f) < 1e-6);
    assert(fabs(f32_ops[i++] + 3.4f) < 1e-6);
    assert(fabs(f32_ops[i++] + 3.5f) < 1e-6);

    assert(fabs(f32_ops[i++]) < 1e-6);
    assert(fabs(f32_ops[i++] - 3.0 * 9.0) < 1e-6);
    assert(fabs(f32_ops[i++] - 3.0 * 9.0) < 1e-6);
    _Complex float exp = (6.0f + 9.0f * I) * (3.0f * I);
    assert(fabs(f32_ops[i++] - cimag(exp)) < 1e-6);
    exp = (6.0f + 9.0f * I) * (-3.0f * I);
    assert(fabs(f32_ops[i++] - cimag(exp)) < 1e-6);
    assert(fabs(f32_ops[i++] - 3.1 * -5000) < 1e-6);
    assert(fabs(f32_ops[i++] - 3.2 * 5000) < 1e-6);
    assert(fabs(f32_ops[i++] - 3.4 * -5000) < 1e-6);
    assert(fabs(f32_ops[i++] - 3.5 * 5000) < 1e-6);

    assert(fabs(f32_ops[i++]) < 1e-6);
    assert(fabs(f32_ops[i++] - 3.0 / 9.0) < 1e-6);
    exp = (3.0f) / (9.0f * I);
    assert(fabs(f32_ops[i++] - cimag(exp)) < 1e-6);
    exp = (3.0f * I) / (6.0f + 9.0f * I);
    assert(fabs(f32_ops[i++] - cimag(exp)) < 1e-6);
    exp = (6.0f + 9.0f * I) / (-3.0f * I);
    assert(fabs(f32_ops[i++] - cimag(exp)) < 1e-6);
    assert(fabs(f32_ops[i++] - 3.1 / -5000) < 1e-6);
    assert(fabs(f32_ops[i++] - 3.2 / 5000) < 1e-6);
    exp = (-5000) / (3.4f * I);
    assert(fabs(f32_ops[i++] - cimag(exp)) < 1e-3);
    exp = (5000u) / (3.5f * I);
    assert(fabs(f32_ops[i++] - cimag(exp)) < 1e-3);

    assert(fabs(f32_ops[i++] + 3.0) < 1e-6);
    assert(fabs(f32_ops[i++] + 9.0) < 1e-6);
    return EXIT_SUCCESS;
}
