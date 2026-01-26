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
    assert(f32x_size == sizeof(double));
    assert(f32x_alignment == _Alignof(double));
    assert(f64_size == sizeof(double));
    assert(f64_alignment == _Alignof(double));
    assert(f64x_size == sizeof(long double));
    assert(f64x_alignment == _Alignof(long double));
    assert(f80_size == sizeof(long double));
    assert(f80_alignment == _Alignof(long double));

    int i = 0;
    assert(compat[i++] == 1);
    assert(compat[i++] == 2);
    assert(compat[i++] == 3);
    assert(compat[i++] == 4);
    assert(compat[i++] == 5);
    assert(compat[i++] == 1);
    assert(compat[i++] == 2);
    assert(compat[i++] == 3);
    assert(compat[i++] == 4);
    assert(compat[i++] == 5);
    assert(compat[i++] == 2);
    assert(compat[i++] == 3);
    assert(compat[i++] == 4);
    assert(compat[i++] == 5);
    assert(compat[i++] == 3);
    assert(compat[i++] == 4);
    assert(compat[i++] == 5);
    assert(compat[i++] == 4);
    assert(compat[i++] == 5);
    assert(compat[i++] == 5);

    assert(fabs(f32_const - 3.14159) < 1e-6);
    assert(fabs(f32x_const + 3.14159) < 1e-6);
    assert(fabs(f64_const - 2.71828) < 1e-6);
    assert(fabsl(f64x_const + 2.71828) < 1e-6);
    assert(fabsl(f80_const - 90.5429) < 1e-6);

    _Complex float f32_exp, f32_res;
    f32_exp = (3.0 * I) + (-7.0 * I);
    f32_res = f32_op(0, 3.0, -7.0);
    assert(fabs(creal(f32_exp) - creal(f32_res)) < 1e-6);
    assert(fabs(cimag(f32_exp) - cimag(f32_res)) < 1e-6);

    f32_exp = (3.0 * I) - (-7.0 * I);
    f32_res = f32_op(1, 3.0, -7.0);
    assert(fabs(creal(f32_exp) - creal(f32_res)) < 1e-6);
    assert(fabs(cimag(f32_exp) - cimag(f32_res)) < 1e-6);

    f32_exp = (3.0 * I) * (-7.0 * I);
    f32_res = f32_op(2, 3.0, -7.0);
    assert(fabs(creal(f32_exp) - creal(f32_res)) < 1e-6);
    assert(fabs(cimag(f32_exp) - cimag(f32_res)) < 1e-6);

    f32_exp = (3.0 * I) / (-7.0 * I);
    f32_res = f32_op(3, 3.0, -7.0);
    assert(fabs(creal(f32_exp) - creal(f32_res)) < 1e-6);
    assert(fabs(cimag(f32_exp) - cimag(f32_res)) < 1e-6);

    f32_exp = -(3.0 * I);
    f32_res = f32_op(4, 3.0, -7.0);
    assert(fabs(creal(f32_exp) - creal(f32_res)) < 1e-6);
    assert(fabs(cimag(f32_exp) - cimag(f32_res)) < 1e-6);

    _Complex double f64_exp, f64_res;
    f64_exp = (3.0 * I) + (-7.0 * I);
    f64_res = f32x_op(0, 3.0, -7.0);
    assert(fabs(creal(f64_exp) - creal(f64_res)) < 1e-6);
    assert(fabs(cimag(f64_exp) - cimag(f64_res)) < 1e-6);

    f64_exp = (3.0 * I) - (-7.0 * I);
    f64_res = f32x_op(1, 3.0, -7.0);
    assert(fabs(creal(f64_exp) - creal(f64_res)) < 1e-6);
    assert(fabs(cimag(f64_exp) - cimag(f64_res)) < 1e-6);

    f64_exp = (3.0 * I) * (-7.0 * I);
    f64_res = f32x_op(2, 3.0, -7.0);
    assert(fabs(creal(f64_exp) - creal(f64_res)) < 1e-6);
    assert(fabs(cimag(f64_exp) - cimag(f64_res)) < 1e-6);

    f64_exp = (3.0 * I) / (-7.0 * I);
    f64_res = f32x_op(3, 3.0, -7.0);
    assert(fabs(creal(f64_exp) - creal(f64_res)) < 1e-6);
    assert(fabs(cimag(f64_exp) - cimag(f64_res)) < 1e-6);

    f64_exp = -(3.0 * I);
    f64_res = f32x_op(4, 3.0, -7.0);
    assert(fabs(creal(f64_exp) - creal(f64_res)) < 1e-6);
    assert(fabs(cimag(f64_exp) - cimag(f64_res)) < 1e-6);

    f64_exp = (3.0 * I) + (-7.0 * I);
    f64_res = f64_op(0, 3.0, -7.0);
    assert(fabs(creal(f64_exp) - creal(f64_res)) < 1e-6);
    assert(fabs(cimag(f64_exp) - cimag(f64_res)) < 1e-6);

    f64_exp = (3.0 * I) - (-7.0 * I);
    f64_res = f64_op(1, 3.0, -7.0);
    assert(fabs(creal(f64_exp) - creal(f64_res)) < 1e-6);
    assert(fabs(cimag(f64_exp) - cimag(f64_res)) < 1e-6);

    f64_exp = (3.0 * I) * (-7.0 * I);
    f64_res = f64_op(2, 3.0, -7.0);
    assert(fabs(creal(f64_exp) - creal(f64_res)) < 1e-6);
    assert(fabs(cimag(f64_exp) - cimag(f64_res)) < 1e-6);

    f64_exp = (3.0 * I) / (-7.0 * I);
    f64_res = f64_op(3, 3.0, -7.0);
    assert(fabs(creal(f64_exp) - creal(f64_res)) < 1e-6);
    assert(fabs(cimag(f64_exp) - cimag(f64_res)) < 1e-6);

    f64_exp = -(3.0 * I);
    f64_res = f64_op(4, 3.0, -7.0);
    assert(fabs(creal(f64_exp) - creal(f64_res)) < 1e-6);
    assert(fabs(cimag(f64_exp) - cimag(f64_res)) < 1e-6);

    _Complex long double f80_exp, f80_res;
    f80_exp = (3.0 * I) + (-7.0 * I);
    f80_res = f64x_op(0, 3.0, -7.0);
    assert(fabsl(creall(f80_exp) - creall(f80_res)) < 1e-6);
    assert(fabsl(cimagl(f80_exp) - cimagl(f80_res)) < 1e-6);

    f80_exp = (3.0 * I) - (-7.0 * I);
    f80_res = f64x_op(1, 3.0, -7.0);
    assert(fabsl(creall(f80_exp) - creall(f80_res)) < 1e-6);
    assert(fabsl(cimagl(f80_exp) - cimagl(f80_res)) < 1e-6);

    f80_exp = (3.0 * I) * (-7.0 * I);
    f80_res = f64x_op(2, 3.0, -7.0);
    assert(fabsl(creall(f80_exp) - creall(f80_res)) < 1e-6);
    assert(fabsl(cimagl(f80_exp) - cimagl(f80_res)) < 1e-6);

    f80_exp = (3.0 * I) / (-7.0 * I);
    f80_res = f64x_op(3, 3.0, -7.0);
    assert(fabsl(creall(f80_exp) - creall(f80_res)) < 1e-6);
    assert(fabsl(cimagl(f80_exp) - cimagl(f80_res)) < 1e-6);

    f80_exp = -(3.0 * I);
    f80_res = f64x_op(4, 3.0, -7.0);
    assert(fabsl(creall(f80_exp) - creall(f80_res)) < 1e-6);
    assert(fabsl(cimagl(f80_exp) - cimagl(f80_res)) < 1e-6);

    f80_exp = (3.0 * I) + (-7.0 * I);
    f80_res = f80_op(0, 3.0, -7.0);
    assert(fabsl(creall(f80_exp) - creall(f80_res)) < 1e-6);
    assert(fabsl(cimagl(f80_exp) - cimagl(f80_res)) < 1e-6);

    f80_exp = (3.0 * I) - (-7.0 * I);
    f80_res = f80_op(1, 3.0, -7.0);
    assert(fabsl(creall(f80_exp) - creall(f80_res)) < 1e-6);
    assert(fabsl(cimagl(f80_exp) - cimagl(f80_res)) < 1e-6);

    f80_exp = (3.0 * I) * (-7.0 * I);
    f80_res = f80_op(2, 3.0, -7.0);
    assert(fabsl(creall(f80_exp) - creall(f80_res)) < 1e-6);
    assert(fabsl(cimagl(f80_exp) - cimagl(f80_res)) < 1e-6);

    f80_exp = (3.0 * I) / (-7.0 * I);
    f80_res = f80_op(3, 3.0, -7.0);
    assert(fabsl(creall(f80_exp) - creall(f80_res)) < 1e-6);
    assert(fabsl(cimagl(f80_exp) - cimagl(f80_res)) < 1e-6);

    f80_exp = -(3.0 * I);
    f80_res = f80_op(4, 3.0, -7.0);
    assert(fabsl(creall(f80_exp) - creall(f80_res)) < 1e-6);
    assert(fabsl(cimagl(f80_exp) - cimagl(f80_res)) < 1e-6);
    return EXIT_SUCCESS;
}
