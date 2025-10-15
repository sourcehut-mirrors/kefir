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

int main(void) {
    assert(f64_size == sizeof(double));
    assert(f64_alignment == _Alignof(double));
    assert(fabs(*f64_const_ptr - 9.831) < 1e-6);

    int i = 0;
    assert(f64_compat[i++] == 1);
    assert(f64_compat[i++] == 1);
    assert(f64_compat[i++] == 1);
    assert(f64_compat[i++] == 2);
    assert(f64_compat[i++] == 2);
    assert(f64_compat[i++] == 3);
    assert(f64_compat[i++] == 3);
    assert(f64_compat[i++] == 1);
    assert(f64_compat[i++] == 1);
    assert(f64_compat[i++] == 3);
    assert(f64_compat[i++] == 3);
    assert(f64_compat[i++] == 1);
    assert(f64_compat[i++] == 1);
    assert(f64_compat[i++] == 3);
    assert(f64_compat[i++] == 3);
    assert(f64_compat[i++] == 3);
    assert(f64_compat[i++] == 3);
    assert(f64_compat[i++] == 3);
    assert(f64_compat[i++] == 3);
    assert(f64_compat[i++] == 3);
    assert(f64_compat[i++] == 3);

    assert(f64_iarr[0] == 0);
    assert(f64_iarr[1] == 0);
    assert(f64_iarr[2] == 1);
    assert(f64_iarr[3] == 0);
    assert(f64_iarr[4] == 0);

    i = 0;
    assert(fabs(f64_arr[i++] - 3.14159f) < 1e-6);
    assert(fabs(f64_arr[i++]) < 1e-6);
    assert(fabs(f64_arr[i++] - 2.71828f) < 1e-6);
    assert(fabs(f64_arr[i++] + 3.14159f) < 1e-6);
    assert(fabs(f64_arr[i++]) < 1e-6);
    assert(fabs(f64_arr[i++] - 2.71828) < 1e-6);
    assert(fabs(f64_arr[i++] + 1.14159) < 1e-6);
    assert(fabs(f64_arr[i++]) < 1e-6);
    assert(fabs(f64_arr[i++] - 2.71823) < 1e-6);
    assert(fabs(f64_arr[i++]) < 1e-6);
    assert(fabs(f64_arr[i++] - 9420.0) < 1e-6);
    assert(fabs(f64_arr[i++]) < 1e-6);
    assert(fabs(f64_arr[i++] - 9420.0) < 1e-6);
    assert(fabs(f64_arr[i++]) < 1e-6);
    assert(fabs(f64_arr[i++] - 9420.0) < 1e-6);
    assert(fabs(f64_arr[i++]) < 1e-6);
    assert(fabs(f64_arr[i++] - 9420.0) < 1e-6);
    assert(fabs(f64_arr[i++]) < 1e-6);
    assert(fabs(f64_arr[i++] - 9420.0) < 1e-6);
    assert(fabs(f64_arr[i++]) < 1e-6);
    assert(fabs(f64_arr[i++]) < 1e-6);
    assert(fabs(f64_arr[i++]) < 1e-6);
    assert(fabs(f64_arr[i++]) < 1e-6);
    assert(fabs(f64_arr[i++]) < 1e-6);
    assert(fabs(f64_arr[i++]) < 1e-6);

    i = 0;
    assert(fabs(f64_ops[i++] - 12.0f) < 1e-6);
    assert(fabs(f64_ops[i++] - 3.0f) < 1e-6);
    assert(fabs(f64_ops[i++] - 9.0f) < 1e-6);
    assert(fabs(f64_ops[i++] - 12.0f) < 1e-6);
    assert(fabs(f64_ops[i++] - 6.0f) < 1e-6);
    assert(fabs(f64_ops[i++] - 3.1f) < 1e-6);
    assert(fabs(f64_ops[i++] - 3.2f) < 1e-6);
    assert(fabs(f64_ops[i++] - 3.4f) < 1e-6);
    assert(fabs(f64_ops[i++] - 3.5f) < 1e-6);

    assert(fabs(f64_ops[i++] + 6.0f) < 1e-6);
    assert(fabs(f64_ops[i++] - 3.0f) < 1e-6);
    assert(fabs(f64_ops[i++] + 9.0f) < 1e-6);
    assert(fabs(f64_ops[i++] + 6.0f) < 1e-6);
    assert(fabs(f64_ops[i++] - 12.0f) < 1e-6);
    assert(fabs(f64_ops[i++] - 3.1f) < 1e-6);
    assert(fabs(f64_ops[i++] - 3.2f) < 1e-6);
    assert(fabs(f64_ops[i++] + 3.4f) < 1e-6);
    assert(fabs(f64_ops[i++] + 3.5f) < 1e-6);

    assert(fabs(f64_ops[i++]) < 1e-6);
    assert(fabs(f64_ops[i++] - 3.0 * 9.0) < 1e-6);
    assert(fabs(f64_ops[i++] - 3.0 * 9.0) < 1e-6);
    _Complex double exp = (6.0f + 9.0f * I) * (3.0f * I);
    assert(fabs(f64_ops[i++] - cimag(exp)) < 1e-6);
    exp = (6.0f + 9.0f * I) * (-3.0f * I);
    assert(fabs(f64_ops[i++] - cimag(exp)) < 1e-6);
    assert(fabs(f64_ops[i++] - 3.1 * -5000) < 1e-3);
    assert(fabs(f64_ops[i++] - 3.2 * 5000) < 1e-3);
    assert(fabs(f64_ops[i++] - 3.4 * -5000) < 1e-3);
    assert(fabs(f64_ops[i++] - 3.5 * 5000) < 1e-3);

    assert(fabs(f64_ops[i++]) < 1e-6);
    assert(fabs(f64_ops[i++] - 3.0 / 9.0) < 1e-6);
    exp = (3.0f) / (9.0f * I);
    assert(fabs(f64_ops[i++] - cimag(exp)) < 1e-6);
    exp = (3.0f * I) / (6.0f + 9.0f * I);
    assert(fabs(f64_ops[i++] - cimag(exp)) < 1e-6);
    exp = (6.0f + 9.0f * I) / (-3.0f * I);
    assert(fabs(f64_ops[i++] - cimag(exp)) < 1e-6);
    assert(fabs(f64_ops[i++] - 3.1 / -5000) < 1e-6);
    assert(fabs(f64_ops[i++] - 3.2 / 5000) < 1e-6);
    exp = (-5000) / (3.4f * I);
    assert(fabs(f64_ops[i++] - cimag(exp)) < 1e-3);
    exp = (5000u) / (3.5f * I);
    assert(fabs(f64_ops[i++] - cimag(exp)) < 1e-3);

    assert(fabs(f64_ops[i++] + 3.0) < 1e-6);
    assert(fabs(f64_ops[i++] + 9.0) < 1e-6);

    i = 0;
    assert(fabs(f64_ops2[i++]) < 1e-6);
    assert(fabs(f64_ops2[i++] - 9.0) < 1e-6);
    assert(fabs(f64_ops2[i++] - 3.0) < 1e-6);
    assert(fabs(f64_ops2[i++] - 6.0) < 1e-6);
    assert(fabs(f64_ops2[i++] - 6.0) < 1e-6);
    assert(fabs(f64_ops2[i++] + 5000) < 1e-6);
    assert(fabs(f64_ops2[i++] - 5000) < 1e-6);
    assert(fabs(f64_ops2[i++] + 5000) < 1e-6);
    assert(fabs(f64_ops2[i++] - 5000) < 1e-6);

    assert(fabs(f64_ops2[i++]) < 1e-6);
    assert(fabs(f64_ops2[i++] + 9.0) < 1e-6);
    assert(fabs(f64_ops2[i++] - 3.0) < 1e-6);
    assert(fabs(f64_ops2[i++] + 6.0) < 1e-6);
    assert(fabs(f64_ops2[i++] - 6.0) < 1e-6);
    assert(fabs(f64_ops2[i++] - 5000) < 1e-6);
    assert(fabs(f64_ops2[i++] + 5000) < 1e-6);
    assert(fabs(f64_ops2[i++] + 5000) < 1e-6);
    assert(fabs(f64_ops2[i++] - 5000) < 1e-6);

    assert(fabs(f64_ops2[i++] + 27.0) < 1e-6);
    assert(fabs(f64_ops2[i++]) < 1e-6);
    assert(fabs(f64_ops2[i++]) < 1e-6);
    exp = (6.0f + 9.0f * I) * (3.0f * I);
    assert(fabs(f64_ops2[i++] - creal(exp)) < 1e-6);
    exp = (6.0f + 9.0f * I) * (-3.0f * I);
    assert(fabs(f64_ops2[i++] - creal(exp)) < 1e-6);
    assert(fabs(f64_ops2[i++]) < 1e-6);
    assert(fabs(f64_ops2[i++]) < 1e-6);
    assert(fabs(f64_ops2[i++]) < 1e-6);
    assert(fabs(f64_ops2[i++]) < 1e-6);

    assert(fabs(f64_ops2[i++] - 1.0 / 3) < 1e-6);
    assert(fabs(f64_ops2[i++]) < 1e-6);
    exp = (3.0f) / (9.0f * I);
    assert(fabs(f64_ops2[i++] - creal(exp)) < 1e-6);
    exp = (3.0f * I) / (6.0f + 9.0f * I);
    assert(fabs(f64_ops2[i++] - creal(exp)) < 1e-6);
    exp = (6.0f + 9.0f * I) / (-3.0f * I);
    assert(fabs(f64_ops2[i++] - creal(exp)) < 1e-6);
    assert(fabs(f64_ops2[i++]) < 1e-6);
    assert(fabs(f64_ops2[i++]) < 1e-6);
    exp = (-5000) / (3.4f * I);
    assert(fabs(f64_ops2[i++] - creal(exp)) < 1e-3);
    exp = (5000u) / (3.5f * I);
    assert(fabs(f64_ops2[i++] - creal(exp)) < 1e-3);

    assert(fabs(f64_ops2[i++]) < 1e-6);
    assert(fabs(f64_ops2[i++]) < 1e-6);

    assert(fabs(fi64_to_f32(4.14159)) < 1e-6);
    assert(fabs(fi64_to_f64(5.14159)) < 1e-6);
    assert(fabsl(fi64_to_f80(6.14159)) < 1e-6);

    assert(fabs(fi64_to_fi32(3.14159) - 3.14159) < 1e-6);
    assert(fabs(fi64_to_fi64(-3.14159) + 3.14159) < 1e-6);
    assert(fabsl(fi64_to_fi80(-3.64159) + 3.64159) < 1e-6);

    assert(fabs(f64_to_fi32(3.14159)) < 1e-6);
    assert(fabs(f64_to_fi64(3.14859)) < 1e-6);
    assert(fabsl(f64_to_fi80(3.54159)) < 1e-6);

    _Complex float res = fi64_to_cf32(7.632);
    assert(fabs(creal(res)) < 1e-6);
    assert(fabs(cimag(res) - 7.632) < 1e-6);
    _Complex double res64 = fi64_to_cf64(8.632);
    assert(fabs(creal(res64)) < 1e-6);
    assert(fabs(cimag(res64) - 8.632) < 1e-6);
    _Complex long double res80 = fi64_to_cf80(9.632);
    assert(fabsl(creall(res80)) < 1e-6);
    assert(fabsl(cimagl(res80) - 9.632) < 1e-6);

    assert(fi64_to_i64(-901.42) == 0);
    assert(fi64_to_u64(901.42) == 0);
    assert(fabs(i64_to_fi64(-94242)) < 1e-6);
    assert(fabs(u64_to_fi64(94722)) < 1e-6);

    assert(fabs(fi64_neg(2.71828) + 2.71828) < 1e-6);
    assert(fabs(fi64_neg(-2.71828) - 2.71828) < 1e-6);
    assert(fabs(fi64_add(-2.71828, 10.9918) - (-2.71828 + 10.9918)) < 1e-6);
    assert(fabs(fi64_add(2.71828, -10.9918) - (2.71828 - 10.9918)) < 1e-6);
    assert(fabs(fi64_sub(-2.71828, 10.9918) - (-2.71828 - 10.9918)) < 1e-6);
    assert(fabs(fi64_sub(2.71828, -10.9918) - (2.71828 + 10.9918)) < 1e-6);
    assert(fabs(fi64_mul(-2.71828, 10.9918) + (-2.71828 * 10.9918)) < 1e-4);
    assert(fabs(fi64_mul(-2.71828, -10.9918) + (-2.71828 * -10.9918)) < 1e-4);
    assert(fabs(fi64_mul(2.71828, -10.9918) + (2.71828 * -10.9918)) < 1e-4);
    assert(fabs(fi64_mul(2.71828, 10.9918) + (2.71828 * 10.9918)) < 1e-4);
    assert(fabs(fi64_div(-2.71828, 10.9918) - (-2.71828 / 10.9918)) < 1e-4);
    assert(fabs(fi64_div(-2.71828, -10.9918) - (-2.71828 / -10.9918)) < 1e-4);
    assert(fabs(fi64_div(2.71828, -10.9918) - (2.71828 / -10.9918)) < 1e-4);
    assert(fabs(fi64_div(2.71828, 10.9918) - (2.71828 / 10.9918)) < 1e-4);

    res = fi64_f64_add(3.14159, -2.7182);
    assert(fabs(creal(res) + 2.7182) < 1e-6);
    assert(fabs(cimag(res) - 3.14159) < 1e-6);
    res = fi64_f64_add(3.14159, 2.7182);
    assert(fabs(creal(res) - 2.7182) < 1e-6);
    assert(fabs(cimag(res) - 3.14159) < 1e-6);
    res = fi64_f64_add(-3.14159, -2.7182);
    assert(fabs(creal(res) + 2.7182) < 1e-6);
    assert(fabs(cimag(res) + 3.14159) < 1e-6);
    res = fi64_f64_add(-3.14159, 2.7182);
    assert(fabs(creal(res) - 2.7182) < 1e-6);
    assert(fabs(cimag(res) + 3.14159) < 1e-6);
    res = fi64_f64_sub(-3.14159, -2.7182);
    assert(fabs(creal(res) - 2.7182) < 1e-6);
    assert(fabs(cimag(res) + 3.14159) < 1e-6);
    res = fi64_f64_sub(-3.14159, 2.7182);
    assert(fabs(creal(res) + 2.7182) < 1e-6);
    assert(fabs(cimag(res) + 3.14159) < 1e-6);
    res = fi64_f64_sub(3.14159, 2.7182);
    assert(fabs(creal(res) + 2.7182) < 1e-6);
    assert(fabs(cimag(res) - 3.14159) < 1e-6);
    assert(fabs(fi64_f64_mul(-3.14159, -2.7182) - (-3.14159 * -2.7182)) < 1e-6);
    assert(fabs(fi64_f64_mul(3.14159, -2.7182) - (3.14159 * -2.7182)) < 1e-6);
    assert(fabs(fi64_f64_mul(3.14159, 2.7182) - (3.14159 * 2.7182)) < 1e-6);
    assert(fabs(fi64_f64_mul(-3.14159, 2.7182) - (-3.14159 * 2.7182)) < 1e-6);
    assert(fabs(fi64_f64_div(3.14159, 2.7182) - (3.14159 / 2.7182)) < 1e-6);
    assert(fabs(fi64_f64_div(3.14159, -2.7182) - (3.14159 / -2.7182)) < 1e-6);
    assert(fabs(fi64_f64_div(-3.14159, -2.7182) - (-3.14159 / -2.7182)) < 1e-6);
    assert(fabs(fi64_f64_div(-3.14159, 2.7182) - (-3.14159 / 2.7182)) < 1e-6);

    res = f64_fi64_add(3.14159, -2.7182);
    assert(fabs(creal(res) - 3.14159) < 1e-6);
    assert(fabs(cimag(res) + 2.7182) < 1e-6);
    res = f64_fi64_add(3.14159, 2.7182);
    assert(fabs(creal(res) - 3.14159) < 1e-6);
    assert(fabs(cimag(res) - 2.7182) < 1e-6);
    res = f64_fi64_add(-3.14159, 2.7182);
    assert(fabs(creal(res) + 3.14159) < 1e-6);
    assert(fabs(cimag(res) - 2.7182) < 1e-6);
    res = f64_fi64_add(-3.14159, -2.7182);
    assert(fabs(creal(res) + 3.14159) < 1e-6);
    assert(fabs(cimag(res) + 2.7182) < 1e-6);
    res = f64_fi64_sub(3.14159, -2.7182);
    assert(fabs(creal(res) - 3.14159) < 1e-6);
    assert(fabs(cimag(res) - 2.7182) < 1e-6);
    res = f64_fi64_sub(3.14159, 2.7182);
    assert(fabs(creal(res) - 3.14159) < 1e-6);
    assert(fabs(cimag(res) + 2.7182) < 1e-6);
    res = f64_fi64_sub(-3.14159, 2.7182);
    assert(fabs(creal(res) + 3.14159) < 1e-6);
    assert(fabs(cimag(res) + 2.7182) < 1e-6);
    res = f64_fi64_sub(-3.14159, -2.7182);
    assert(fabs(creal(res) + 3.14159) < 1e-6);
    assert(fabs(cimag(res) - 2.7182) < 1e-6);
    assert(fabs(f64_fi64_mul(-3.14159, -2.7182) - (-3.14159 * -2.7182)) < 1e-6);
    assert(fabs(f64_fi64_mul(-3.14159, 2.7182) - (-3.14159 * 2.7182)) < 1e-6);
    assert(fabs(f64_fi64_mul(3.14159, 2.7182) - (3.14159 * 2.7182)) < 1e-6);
    assert(fabs(f64_fi64_mul(3.14159, -2.7182) - (3.14159 * -2.7182)) < 1e-6);
    assert(fabs(f64_fi64_div(3.14159, 2.7182) - (-3.14159 / 2.7182)) < 1e-6);
    assert(fabs(f64_fi64_div(3.14159, -2.7182) - (-3.14159 / -2.7182)) < 1e-6);
    assert(fabs(f64_fi64_div(-3.14159, -2.7182) - (3.14159 / -2.7182)) < 1e-6);
    assert(fabs(f64_fi64_div(-3.14159, 2.7182) - (3.14159 / 2.7182)) < 1e-6);

    exp = (3.14159 + 2.8847 * I) + (1.4 * I);
    res = cf64_fi64_add(3.14159 + 2.8847 * I, 1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (3.14159 - 2.8847 * I) + (1.4 * I);
    res = cf64_fi64_add(3.14159 - 2.8847 * I, 1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (3.14159 + 2.8847 * I) + (-1.4 * I);
    res = cf64_fi64_add(3.14159 + 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-3.14159 - 2.8847 * I) + (-1.4 * I);
    res = cf64_fi64_add(-3.14159 - 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);

    exp = (3.14159 + 2.8847 * I) - (1.4 * I);
    res = cf64_fi64_sub(3.14159 + 2.8847 * I, 1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (3.14159 - 2.8847 * I) - (1.4 * I);
    res = cf64_fi64_sub(3.14159 - 2.8847 * I, 1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (3.14159 + 2.8847 * I) - (-1.4 * I);
    res = cf64_fi64_sub(3.14159 + 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-3.14159 - 2.8847 * I) - (-1.4 * I);
    res = cf64_fi64_sub(-3.14159 - 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);

    exp = (3.14159 + 2.8847 * I) * (1.4 * I);
    res = cf64_fi64_mul(3.14159 + 2.8847 * I, 1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (3.14159 + 2.8847 * I) * (-1.4 * I);
    res = cf64_fi64_mul(3.14159 + 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (3.14159 - 2.8847 * I) * (1.4 * I);
    res = cf64_fi64_mul(3.14159 - 2.8847 * I, 1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-3.14159 + 2.8847 * I) * (-1.4 * I);
    res = cf64_fi64_mul(-3.14159 + 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-3.14159 - 2.8847 * I) * (-1.4 * I);
    res = cf64_fi64_mul(-3.14159 - 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);

    exp = (3.14159 + 2.8847 * I) / (1.4 * I);
    res = cf64_fi64_div(3.14159 + 2.8847 * I, 1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (3.14159 + 2.8847 * I) / (-1.4 * I);
    res = cf64_fi64_div(3.14159 + 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (3.14159 - 2.8847 * I) / (1.4 * I);
    res = cf64_fi64_div(3.14159 - 2.8847 * I, 1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-3.14159 + 2.8847 * I) / (-1.4 * I);
    res = cf64_fi64_div(-3.14159 + 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-3.14159 - 2.8847 * I) / (-1.4 * I);
    res = cf64_fi64_div(-3.14159 - 2.8847 * I, -1.4);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);

    exp = (1.4 * I) + (3.14159 + 2.8847 * I);
    res = fi64_cf64_add(1.4, 3.14159 + 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (1.4 * I) + (3.14159 - 2.8847 * I);
    res = fi64_cf64_add(1.4, 3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-1.4 * I) + (3.14159 - 2.8847 * I);
    res = fi64_cf64_add(-1.4, 3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-1.4 * I) + (-3.14159 - 2.8847 * I);
    res = fi64_cf64_add(-1.4, -3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);

    exp = (1.4 * I) - (3.14159 + 2.8847 * I);
    res = fi64_cf64_sub(1.4, 3.14159 + 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (1.4 * I) - (3.14159 - 2.8847 * I);
    res = fi64_cf64_sub(1.4, 3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-1.4 * I) - (3.14159 - 2.8847 * I);
    res = fi64_cf64_sub(-1.4, 3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-1.4 * I) - (-3.14159 - 2.8847 * I);
    res = fi64_cf64_sub(-1.4, -3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);

    exp = (1.4 * I) * (3.14159 + 2.8847 * I);
    res = fi64_cf64_mul(1.4, 3.14159 + 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (1.4 * I) * (3.14159 - 2.8847 * I);
    res = fi64_cf64_mul(1.4, 3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-1.4 * I) * (3.14159 - 2.8847 * I);
    res = fi64_cf64_mul(-1.4, 3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-1.4 * I) * (-3.14159 - 2.8847 * I);
    res = fi64_cf64_mul(-1.4, -3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);

    exp = (1.4 * I) / (3.14159 + 2.8847 * I);
    res = fi64_cf64_div(1.4, 3.14159 + 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (1.4 * I) / (3.14159 - 2.8847 * I);
    res = fi64_cf64_div(1.4, 3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-1.4 * I) / (3.14159 - 2.8847 * I);
    res = fi64_cf64_div(-1.4, 3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);
    exp = (-1.4 * I) / (-3.14159 - 2.8847 * I);
    res = fi64_cf64_div(-1.4, -3.14159 - 2.8847 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-6);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-6);

    assert(fi64_to_bool(3.14159));
    assert(!fi64_to_bool(0.0));
    assert(fi64_to_bool(nan("")));
    assert(fi64_to_bool(INFINITY));

    assert(fabs(cf32_to_fi64(3.0f)) < 1e-6);
    assert(fabs(cf32_to_fi64(3.0f * I) - 3.0f) < 1e-6);
    assert(fabs(cf32_to_fi64(-3.0f * I) + 3.0f) < 1e-6);
    assert(fabs(cf32_to_fi64(3.0f + 1.0 * I) - 1.0f) < 1e-6);
    assert(fabs(cf32_to_fi64(3.0f - 10.0 * I) + 10.0f) < 1e-6);

    assert(fabs(cf64_to_fi64(3.0f)) < 1e-6);
    assert(fabs(cf64_to_fi64(3.0f * I) - 3.0f) < 1e-6);
    assert(fabs(cf64_to_fi64(-3.0f * I) + 3.0f) < 1e-6);
    assert(fabs(cf64_to_fi64(3.0f + 1.0 * I) - 1.0f) < 1e-6);
    assert(fabs(cf64_to_fi64(3.0f - 10.0 * I) + 10.0f) < 1e-6);

    assert(fabs(cf80_to_fi64(3.0f)) < 1e-6);
    assert(fabs(cf80_to_fi64(3.0f * I) - 3.0f) < 1e-6);
    assert(fabs(cf80_to_fi64(-3.0f * I) + 3.0f) < 1e-6);
    assert(fabs(cf80_to_fi64(3.0f + 1.0 * I) - 1.0f) < 1e-6);
    assert(fabs(cf80_to_fi64(3.0f - 10.0 * I) + 10.0f) < 1e-6);
    return EXIT_SUCCESS;
}
