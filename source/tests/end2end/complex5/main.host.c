/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#define EPSILON_F 1e-3
#define EPSILON_D 1e-6
#define EPSILON_LD 1e-8

_Complex float sum32(_Complex float a, _Complex float b, _Complex float c, _Complex float d, _Complex float e,
                     _Complex float f, _Complex float g, _Complex float h, _Complex float i, _Complex float j,
                     _Complex float k, _Complex float l) {
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

_Complex double sum64(_Complex double a, _Complex double b, _Complex double c, _Complex double d, _Complex double e,
                      _Complex double f, _Complex double g, _Complex double h, _Complex double i, _Complex double j,
                      _Complex double k, _Complex double l) {
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

_Complex long double sumld(_Complex long double a, _Complex long double b, _Complex long double c,
                           _Complex long double d, _Complex long double e, _Complex long double f,
                           _Complex long double g, _Complex long double h, _Complex long double i,
                           _Complex long double j, _Complex long double k, _Complex long double l) {
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

#define FORMULA(_x)                                                                                          \
    (((_x) + 0) + ((_x) + 1) + ((_x) + 2) + ((_x) + 3) + ((_x) + 4) + ((_x) + 5) + ((_x) + 6) + ((_x) + 7) + \
     ((_x) + 8) + ((_x) + 9) + ((_x) + 10) + ((_x) + 11))

int main(void) {
    for (double a = -10.0; a < 10.0; a += 0.1) {
        for (double b = -10.0; b < 10.0; b += 0.1) {
            _Complex float f32x = (float) a + I * (float) b;
            _Complex float f32y = (float) b + I * (float) a;
            _Complex double f64x = (double) a + I * (double) b;
            _Complex double f64y = (double) b + I * (double) a;
            _Complex long double ldx = (long double) a + I * (long double) b;
            _Complex long double ldy = (long double) b + I * (long double) a;

            _Complex float res32 = test32(f32x);
            _Complex float expected_res32 = FORMULA(f32x);
            assert(fabs(crealf(res32) - crealf(expected_res32)) < EPSILON_F);
            assert(fabs(cimagf(res32) - cimagf(expected_res32)) < EPSILON_F);

            _Complex double res64 = test64(f64x);
            _Complex double expected_res64 = FORMULA(f64x);
            assert(fabs(creal(res64) - creal(expected_res64)) < EPSILON_D);
            assert(fabs(cimag(res64) - cimag(expected_res64)) < EPSILON_D);

            _Complex long double resld = testld(ldx);
            _Complex long double expected_resld = FORMULA(ldx);
            assert(fabsl(creall(resld) - creall(expected_resld)) < EPSILON_LD);
            assert(fabsl(cimagl(resld) - cimagl(expected_resld)) < EPSILON_LD);

            union Union1 union1 =
                test_struct((struct Struct1){.a = f32x, .b = ldx, .c = f64x, .x = ldy, .y = f32y, .z = f64y});
            _Complex long double expected1 = f32x + ldx + f64x + ldy + f32y + f64y;
            assert(fabsl(creall(union1.c) - creall(expected1)) < EPSILON_LD);
            assert(fabsl(cimagl(union1.c) - cimagl(expected1)) < EPSILON_LD);

            _Complex float vf32_1 = vsum32(0);
            _Complex float vf32_2 = vsum32(1, f32x);
            _Complex float vf32_3 = vsum32(1, f32x, f32y);
            _Complex float vf32_4 = vsum32(2, f32x, f32y);
            _Complex float vf32_5 = vsum32(3, f32x, f32y, (_Complex float) 1.0f);
            _Complex float vf32_6 = vsum32(5, f32x, f32y, (_Complex float) 1.0f, f32x, f32y);
            assert(fabs(crealf(vf32_1)) < EPSILON_F);
            assert(fabs(cimagf(vf32_1)) < EPSILON_F);
            assert(fabs(crealf(vf32_2) - crealf(f32x)) < EPSILON_F);
            assert(fabs(cimagf(vf32_2) - cimagf(f32x)) < EPSILON_F);
            assert(fabs(crealf(vf32_3) - crealf(f32x)) < EPSILON_F);
            assert(fabs(cimagf(vf32_3) - cimagf(f32x)) < EPSILON_F);
            assert(fabs(crealf(vf32_4) - crealf(f32x + f32y)) < EPSILON_F);
            assert(fabs(cimagf(vf32_4) - cimagf(f32x + f32y)) < EPSILON_F);
            assert(fabs(crealf(vf32_5) - crealf(f32x + f32y + 1.0f)) < EPSILON_F);
            assert(fabs(cimagf(vf32_5) - cimagf(f32x + f32y + 1.0f)) < EPSILON_F);
            assert(fabs(crealf(vf32_6) - crealf(f32x + f32y + 1.0f + f32x + f32y)) < EPSILON_F);
            assert(fabs(cimagf(vf32_6) - cimagf(f32x + f32y + 1.0f + f32x + f32y)) < EPSILON_F);

            _Complex double vf64_1 = vsum64(0);
            _Complex double vf64_2 = vsum64(1, f64x);
            _Complex double vf64_3 = vsum64(1, f64x, f64y);
            _Complex double vf64_4 = vsum64(2, f64x, f64y);
            _Complex double vf64_5 = vsum64(3, f64x, f64y, (_Complex double) 1.0);
            _Complex double vf64_6 = vsum64(5, f64x, f64y, (_Complex double) 1.0, f64x, f64y);
            assert(fabs(creal(vf64_1)) < EPSILON_D);
            assert(fabs(cimag(vf64_1)) < EPSILON_D);
            assert(fabs(creal(vf64_2) - creal(f64x)) < EPSILON_D);
            assert(fabs(cimag(vf64_2) - cimag(f64x)) < EPSILON_D);
            assert(fabs(creal(vf64_3) - creal(f64x)) < EPSILON_D);
            assert(fabs(cimag(vf64_3) - cimag(f64x)) < EPSILON_D);
            assert(fabs(creal(vf64_4) - creal(f64x + f64y)) < EPSILON_D);
            assert(fabs(cimag(vf64_4) - cimag(f64x + f64y)) < EPSILON_D);
            assert(fabs(creal(vf64_5) - creal(f64x + f64y + 1.0)) < EPSILON_D);
            assert(fabs(cimag(vf64_5) - cimag(f64x + f64y + 1.0)) < EPSILON_D);
            assert(fabs(creal(vf64_6) - creal(f64x + f64y + 1.0 + f64x + f64y)) < EPSILON_D);
            assert(fabs(cimag(vf64_6) - cimag(f64x + f64y + 1.0 + f64x + f64y)) < EPSILON_D);

            _Complex long double vld_1 = vsumld(0);
            _Complex long double vld_2 = vsumld(1, ldx);
            _Complex long double vld_3 = vsumld(1, ldx, ldy);
            _Complex long double vld_4 = vsumld(2, ldx, ldy);
            _Complex long double vld_5 = vsumld(3, ldx, ldy, (_Complex long double) 1.0);
            _Complex long double vld_6 = vsumld(5, ldx, ldy, (_Complex long double) 1.0, ldx, ldy);
            assert(fabsl(creall(vld_1)) < EPSILON_LD);
            assert(fabsl(cimagl(vld_1)) < EPSILON_LD);
            assert(fabsl(creall(vld_2) - creall(ldx)) < EPSILON_LD);
            assert(fabsl(cimagl(vld_2) - cimagl(ldx)) < EPSILON_LD);
            assert(fabsl(creall(vld_3) - creall(ldx)) < EPSILON_LD);
            assert(fabsl(cimagl(vld_3) - cimagl(ldx)) < EPSILON_LD);
            assert(fabsl(creall(vld_4) - creall(ldx + ldy)) < EPSILON_LD);
            assert(fabsl(cimagl(vld_4) - cimagl(ldx + ldy)) < EPSILON_LD);
            assert(fabsl(creall(vld_5) - creall(ldx + ldy + 1.0)) < EPSILON_LD);
            assert(fabsl(cimagl(vld_5) - cimagl(ldx + ldy + 1.0)) < EPSILON_LD);
            assert(fabsl(creall(vld_6) - creall(ldx + ldy + 1.0 + ldx + ldy)) < EPSILON_LD);
            assert(fabsl(cimagl(vld_6) - cimagl(ldx + ldy + 1.0 + ldx + ldy)) < EPSILON_LD);
        }
    }
    return EXIT_SUCCESS;
}
