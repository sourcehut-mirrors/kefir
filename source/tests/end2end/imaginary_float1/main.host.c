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
#include <complex.h>
#include "./definitions.h"

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

    assert(f32_iarr[0] == 0);
    assert(f32_iarr[1] == 0);
    assert(f32_iarr[2] == 1);
    assert(f32_iarr[3] == 0);
    assert(f32_iarr[4] == 0);

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

    i = 0;
    assert(fabs(f32_ops2[i++]) < 1e-6);
    assert(fabs(f32_ops2[i++] - 9.0) < 1e-6);
    assert(fabs(f32_ops2[i++] - 3.0) < 1e-6);
    assert(fabs(f32_ops2[i++] - 6.0) < 1e-6);
    assert(fabs(f32_ops2[i++] - 6.0) < 1e-6);
    assert(fabs(f32_ops2[i++] + 5000) < 1e-6);
    assert(fabs(f32_ops2[i++] - 5000) < 1e-6);
    assert(fabs(f32_ops2[i++] + 5000) < 1e-6);
    assert(fabs(f32_ops2[i++] - 5000) < 1e-6);

    assert(fabs(f32_ops2[i++]) < 1e-6);
    assert(fabs(f32_ops2[i++] + 9.0) < 1e-6);
    assert(fabs(f32_ops2[i++] - 3.0) < 1e-6);
    assert(fabs(f32_ops2[i++] + 6.0) < 1e-6);
    assert(fabs(f32_ops2[i++] - 6.0) < 1e-6);
    assert(fabs(f32_ops2[i++] - 5000) < 1e-6);
    assert(fabs(f32_ops2[i++] + 5000) < 1e-6);
    assert(fabs(f32_ops2[i++] + 5000) < 1e-6);
    assert(fabs(f32_ops2[i++] - 5000) < 1e-6);

    assert(fabs(f32_ops2[i++] + 27.0) < 1e-6);
    assert(fabs(f32_ops2[i++]) < 1e-6);
    assert(fabs(f32_ops2[i++]) < 1e-6);
    exp = (6.0f + 9.0f * I) * (3.0f * I);
    assert(fabs(f32_ops2[i++] - creal(exp)) < 1e-6);
    exp = (6.0f + 9.0f * I) * (-3.0f * I);
    assert(fabs(f32_ops2[i++] - creal(exp)) < 1e-6);
    assert(fabs(f32_ops2[i++]) < 1e-6);
    assert(fabs(f32_ops2[i++]) < 1e-6);
    assert(fabs(f32_ops2[i++]) < 1e-6);
    assert(fabs(f32_ops2[i++]) < 1e-6);

    assert(fabs(f32_ops2[i++] - 1.0 / 3) < 1e-6);
    assert(fabs(f32_ops2[i++]) < 1e-6);
    exp = (3.0f) / (9.0f * I);
    assert(fabs(f32_ops2[i++] - creal(exp)) < 1e-6);
    exp = (3.0f * I) / (6.0f + 9.0f * I);
    assert(fabs(f32_ops2[i++] - creal(exp)) < 1e-6);
    exp = (6.0f + 9.0f * I) / (-3.0f * I);
    assert(fabs(f32_ops2[i++] - creal(exp)) < 1e-6);
    assert(fabs(f32_ops2[i++]) < 1e-6);
    assert(fabs(f32_ops2[i++]) < 1e-6);
    exp = (-5000) / (3.4f * I);
    assert(fabs(f32_ops2[i++] - creal(exp)) < 1e-3);
    exp = (5000u) / (3.5f * I);
    assert(fabs(f32_ops2[i++] - creal(exp)) < 1e-3);

    assert(fabs(f32_ops2[i++]) < 1e-6);
    assert(fabs(f32_ops2[i++]) < 1e-6);

    assert(fabs(fi32_to_f32(4.14159)) < 1e-6);
    assert(fabs(fi32_to_f64(5.14159)) < 1e-6);
    assert(fabsl(fi32_to_f80(6.14159)) < 1e-6);

    assert(fabs(fi32_to_fi32(3.14159) - 3.14159) < 1e-6);
    assert(fabs(fi32_to_fi64(-3.14159) + 3.14159) < 1e-6);
    assert(fabsl(fi32_to_fi80(-3.64159) + 3.64159) < 1e-6);

    assert(fabs(f32_to_fi32(3.14159)) < 1e-6);
    assert(fabs(f32_to_fi64(3.14859)) < 1e-6);
    assert(fabsl(f32_to_fi80(3.54159)) < 1e-6);

    _Complex float res = fi32_to_cf32(7.632);
    assert(fabs(creal(res)) < 1e-6);
    assert(fabs(cimag(res) - 7.632) < 1e-6);
    _Complex double res64 = fi32_to_cf64(8.632);
    assert(fabs(creal(res64)) < 1e-6);
    assert(fabs(cimag(res64) - 8.632) < 1e-6);
    _Complex long double res80 = fi32_to_cf80(9.632);
    assert(fabsl(creall(res80)) < 1e-6);
    assert(fabsl(cimagl(res80) - 9.632) < 1e-6);

    assert(fi32_to_i64(-901.42) == 0);
    assert(fi32_to_u64(901.42) == 0);
    assert(fabs(i64_to_fi32(-94242)) < 1e-6);
    assert(fabs(u64_to_fi32(94722)) < 1e-6);

    assert(fabs(fi32_neg(2.71828) + 2.71828) < 1e-6);
    assert(fabs(fi32_neg(-2.71828) - 2.71828) < 1e-6);
    assert(fabs(fi32_add(-2.71828, 10.9918) - (-2.71828 + 10.9918)) < 1e-6);
    assert(fabs(fi32_add(2.71828, -10.9918) - (2.71828 - 10.9918)) < 1e-6);
    assert(fabs(fi32_sub(-2.71828, 10.9918) - (-2.71828 - 10.9918)) < 1e-6);
    assert(fabs(fi32_sub(2.71828, -10.9918) - (2.71828 + 10.9918)) < 1e-6);
    assert(fabs(fi32_mul(-2.71828, 10.9918) + (-2.71828 * 10.9918)) < 1e-4);
    assert(fabs(fi32_mul(-2.71828, -10.9918) + (-2.71828 * -10.9918)) < 1e-4);
    assert(fabs(fi32_mul(2.71828, -10.9918) + (2.71828 * -10.9918)) < 1e-4);
    assert(fabs(fi32_mul(2.71828, 10.9918) + (2.71828 * 10.9918)) < 1e-4);
    assert(fabs(fi32_div(-2.71828, 10.9918) - (-2.71828 / 10.9918)) < 1e-4);
    assert(fabs(fi32_div(-2.71828, -10.9918) - (-2.71828 / -10.9918)) < 1e-4);
    assert(fabs(fi32_div(2.71828, -10.9918) - (2.71828 / -10.9918)) < 1e-4);
    assert(fabs(fi32_div(2.71828, 10.9918) - (2.71828 / 10.9918)) < 1e-4);

    res = fi32_f32_add(3.14159, -2.7182);
    assert(fabs(creal(res) + 2.7182) < 1e-6);
    assert(fabs(cimag(res) - 3.14159) < 1e-6);
    res = fi32_f32_add(3.14159, 2.7182);
    assert(fabs(creal(res) - 2.7182) < 1e-6);
    assert(fabs(cimag(res) - 3.14159) < 1e-6);
    res = fi32_f32_add(-3.14159, -2.7182);
    assert(fabs(creal(res) + 2.7182) < 1e-6);
    assert(fabs(cimag(res) + 3.14159) < 1e-6);
    res = fi32_f32_add(-3.14159, 2.7182);
    assert(fabs(creal(res) - 2.7182) < 1e-6);
    assert(fabs(cimag(res) + 3.14159) < 1e-6);
    res = fi32_f32_sub(-3.14159, -2.7182);
    assert(fabs(creal(res) - 2.7182) < 1e-6);
    assert(fabs(cimag(res) + 3.14159) < 1e-6);
    res = fi32_f32_sub(-3.14159, 2.7182);
    assert(fabs(creal(res) + 2.7182) < 1e-6);
    assert(fabs(cimag(res) + 3.14159) < 1e-6);
    res = fi32_f32_sub(3.14159, 2.7182);
    assert(fabs(creal(res) + 2.7182) < 1e-6);
    assert(fabs(cimag(res) - 3.14159) < 1e-6);
    assert(fabs(fi32_f32_mul(-3.14159, -2.7182) - (-3.14159 * -2.7182)) < 1e-6);
    assert(fabs(fi32_f32_mul(3.14159, -2.7182) - (3.14159 * -2.7182)) < 1e-6);
    assert(fabs(fi32_f32_mul(3.14159, 2.7182) - (3.14159 * 2.7182)) < 1e-6);
    assert(fabs(fi32_f32_mul(-3.14159, 2.7182) - (-3.14159 * 2.7182)) < 1e-6);
    assert(fabs(fi32_f32_div(3.14159, 2.7182) - (3.14159 / 2.7182)) < 1e-6);
    assert(fabs(fi32_f32_div(3.14159, -2.7182) - (3.14159 / -2.7182)) < 1e-6);
    assert(fabs(fi32_f32_div(-3.14159, -2.7182) - (-3.14159 / -2.7182)) < 1e-6);
    assert(fabs(fi32_f32_div(-3.14159, 2.7182) - (-3.14159 / 2.7182)) < 1e-6);

    res = f32_fi32_add(3.14159, -2.7182);
    assert(fabs(creal(res) - 3.14159) < 1e-6);
    assert(fabs(cimag(res) + 2.7182) < 1e-6);
    res = f32_fi32_add(3.14159, 2.7182);
    assert(fabs(creal(res) - 3.14159) < 1e-6);
    assert(fabs(cimag(res) - 2.7182) < 1e-6);
    res = f32_fi32_add(-3.14159, 2.7182);
    assert(fabs(creal(res) + 3.14159) < 1e-6);
    assert(fabs(cimag(res) - 2.7182) < 1e-6);
    res = f32_fi32_add(-3.14159, -2.7182);
    assert(fabs(creal(res) + 3.14159) < 1e-6);
    assert(fabs(cimag(res) + 2.7182) < 1e-6);
    res = f32_fi32_sub(3.14159, -2.7182);
    assert(fabs(creal(res) - 3.14159) < 1e-6);
    assert(fabs(cimag(res) - 2.7182) < 1e-6);
    res = f32_fi32_sub(3.14159, 2.7182);
    assert(fabs(creal(res) - 3.14159) < 1e-6);
    assert(fabs(cimag(res) + 2.7182) < 1e-6);
    res = f32_fi32_sub(-3.14159, 2.7182);
    assert(fabs(creal(res) + 3.14159) < 1e-6);
    assert(fabs(cimag(res) + 2.7182) < 1e-6);
    res = f32_fi32_sub(-3.14159, -2.7182);
    assert(fabs(creal(res) + 3.14159) < 1e-6);
    assert(fabs(cimag(res) - 2.7182) < 1e-6);
    assert(fabs(f32_fi32_mul(-3.14159, -2.7182) - (-3.14159 * -2.7182)) < 1e-6);
    assert(fabs(f32_fi32_mul(-3.14159, 2.7182) - (-3.14159 * 2.7182)) < 1e-6);
    assert(fabs(f32_fi32_mul(3.14159, 2.7182) - (3.14159 * 2.7182)) < 1e-6);
    assert(fabs(f32_fi32_mul(3.14159, -2.7182) - (3.14159 * -2.7182)) < 1e-6);
    assert(fabs(f32_fi32_div(3.14159, 2.7182) - (-3.14159 / 2.7182)) < 1e-6);
    assert(fabs(f32_fi32_div(3.14159, -2.7182) - (-3.14159 / -2.7182)) < 1e-6);
    assert(fabs(f32_fi32_div(-3.14159, -2.7182) - (3.14159 / -2.7182)) < 1e-6);
    assert(fabs(f32_fi32_div(-3.14159, 2.7182) - (3.14159 / 2.7182)) < 1e-6);

    exp = (3.14159 + 2.8847 * I) + (1.4 * I);
    res = cf32_fi32_add(3.14159 + 2.8847 * I, 1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (3.14159 - 2.8847 * I) + (1.4 * I);
    res = cf32_fi32_add(3.14159 - 2.8847 * I, 1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (3.14159 + 2.8847 * I) + (-1.4 * I);
    res = cf32_fi32_add(3.14159 + 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-3.14159 - 2.8847 * I) + (-1.4 * I);
    res = cf32_fi32_add(-3.14159 - 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);

    exp = (3.14159 + 2.8847 * I) - (1.4 * I);
    res = cf32_fi32_sub(3.14159 + 2.8847 * I, 1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (3.14159 - 2.8847 * I) - (1.4 * I);
    res = cf32_fi32_sub(3.14159 - 2.8847 * I, 1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (3.14159 + 2.8847 * I) - (-1.4 * I);
    res = cf32_fi32_sub(3.14159 + 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-3.14159 - 2.8847 * I) - (-1.4 * I);
    res = cf32_fi32_sub(-3.14159 - 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);

    exp = (3.14159 + 2.8847 * I) * (1.4 * I);
    res = cf32_fi32_mul(3.14159 + 2.8847 * I, 1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (3.14159 + 2.8847 * I) * (-1.4 * I);
    res = cf32_fi32_mul(3.14159 + 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (3.14159 - 2.8847 * I) * (1.4 * I);
    res = cf32_fi32_mul(3.14159 - 2.8847 * I, 1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-3.14159 + 2.8847 * I) * (-1.4 * I);
    res = cf32_fi32_mul(-3.14159 + 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-3.14159 - 2.8847 * I) * (-1.4 * I);
    res = cf32_fi32_mul(-3.14159 - 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);

    exp = (3.14159 + 2.8847 * I) / (1.4 * I);
    res = cf32_fi32_div(3.14159 + 2.8847 * I, 1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (3.14159 + 2.8847 * I) / (-1.4 * I);
    res = cf32_fi32_div(3.14159 + 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (3.14159 - 2.8847 * I) / (1.4 * I);
    res = cf32_fi32_div(3.14159 - 2.8847 * I, 1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-3.14159 + 2.8847 * I) / (-1.4 * I);
    res = cf32_fi32_div(-3.14159 + 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-3.14159 - 2.8847 * I) / (-1.4 * I);
    res = cf32_fi32_div(-3.14159 - 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);

    exp = (1.4 * I) + (3.14159 + 2.8847 * I);
    res = fi32_cf32_add(1.4, 3.14159 + 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (1.4 * I) + (3.14159 - 2.8847 * I);
    res = fi32_cf32_add(1.4, 3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-1.4 * I) + (3.14159 - 2.8847 * I);
    res = fi32_cf32_add(-1.4, 3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-1.4 * I) + (-3.14159 - 2.8847 * I);
    res = fi32_cf32_add(-1.4, -3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);

    exp = (1.4 * I) - (3.14159 + 2.8847 * I);
    res = fi32_cf32_sub(1.4, 3.14159 + 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (1.4 * I) - (3.14159 - 2.8847 * I);
    res = fi32_cf32_sub(1.4, 3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-1.4 * I) - (3.14159 - 2.8847 * I);
    res = fi32_cf32_sub(-1.4, 3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-1.4 * I) - (-3.14159 - 2.8847 * I);
    res = fi32_cf32_sub(-1.4, -3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);

    exp = (1.4 * I) * (3.14159 + 2.8847 * I);
    res = fi32_cf32_mul(1.4, 3.14159 + 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (1.4 * I) * (3.14159 - 2.8847 * I);
    res = fi32_cf32_mul(1.4, 3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-1.4 * I) * (3.14159 - 2.8847 * I);
    res = fi32_cf32_mul(-1.4, 3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-1.4 * I) * (-3.14159 - 2.8847 * I);
    res = fi32_cf32_mul(-1.4, -3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);

    exp = (1.4 * I) / (3.14159 + 2.8847 * I);
    res = fi32_cf32_div(1.4, 3.14159 + 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (1.4 * I) / (3.14159 - 2.8847 * I);
    res = fi32_cf32_div(1.4, 3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-1.4 * I) / (3.14159 - 2.8847 * I);
    res = fi32_cf32_div(-1.4, 3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-1.4 * I) / (-3.14159 - 2.8847 * I);
    res = fi32_cf32_div(-1.4, -3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);

    assert(fi32_to_bool(3.14159f));
    assert(!fi32_to_bool(0.0));
    assert(fi32_to_bool(nan("")));
    assert(fi32_to_bool(INFINITY));

    assert(fabs(cf32_to_fi32(3.0f)) < 1e-6);
    assert(fabs(cf32_to_fi32(3.0f * I) - 3.0f) < 1e-6);
    assert(fabs(cf32_to_fi32(-3.0f * I) + 3.0f) < 1e-6);
    assert(fabs(cf32_to_fi32(3.0f + 1.0 * I) - 1.0f) < 1e-6);
    assert(fabs(cf32_to_fi32(3.0f - 10.0 * I) + 10.0f) < 1e-6);

    assert(fabs(cf64_to_fi32(3.0f)) < 1e-6);
    assert(fabs(cf64_to_fi32(3.0f * I) - 3.0f) < 1e-6);
    assert(fabs(cf64_to_fi32(-3.0f * I) + 3.0f) < 1e-6);
    assert(fabs(cf64_to_fi32(3.0f + 1.0 * I) - 1.0f) < 1e-6);
    assert(fabs(cf64_to_fi32(3.0f - 10.0 * I) + 10.0f) < 1e-6);

    assert(fabs(cf80_to_fi32(3.0f)) < 1e-6);
    assert(fabs(cf80_to_fi32(3.0f * I) - 3.0f) < 1e-6);
    assert(fabs(cf80_to_fi32(-3.0f * I) + 3.0f) < 1e-6);
    assert(fabs(cf80_to_fi32(3.0f + 1.0 * I) - 1.0f) < 1e-6);
    assert(fabs(cf80_to_fi32(3.0f - 10.0 * I) + 10.0f) < 1e-6);
    return EXIT_SUCCESS;
}
