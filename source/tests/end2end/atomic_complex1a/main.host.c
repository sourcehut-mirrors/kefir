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

#define EPSILON_F 1e-3
#define EPSILON_D 1e-6
#define EPSILON_LD 1e-8

_Atomic _Complex float f32_1;
_Atomic _Complex double f64_1;
_Atomic _Complex long double ld_1;

int main(void) {
    _Complex float f32_res, f32_expected;
    _Complex double f64_res, f64_expected;
    _Complex long double ld_res, ld_expected;

    for (double x = -100.0; x < 100.0; x += 1.0) {
        for (double y = -10.0; y < 10.0; y += 0.1) {
            f32_1 = x + y * I;
            f64_1 = x + y * I;
            ld_1 = x + y * I;
            _Atomic _Complex float f32_2 = y + x * I;
            _Atomic _Complex double f64_2 = y + x * I;
            _Atomic _Complex long double ld_2 = y + x * I;

            f32_res = add_f32(&f32_2);
            f32_expected = f32_1 + f32_2;
            assert(fabs(crealf(f32_res) - crealf(f32_expected)) < EPSILON_F);
            assert(fabs(cimagf(f32_res) - cimagf(f32_expected)) < EPSILON_F);

            f32_res = sub_f32(&f32_2);
            f32_expected = f32_1 - f32_2;
            assert(fabs(crealf(f32_res) - crealf(f32_expected)) < EPSILON_F);
            assert(fabs(cimagf(f32_res) - cimagf(f32_expected)) < EPSILON_F);

            f32_res = mul_f32(&f32_2);
            f32_expected = f32_1 * f32_2;
            assert(fabs(crealf(f32_res) - crealf(f32_expected)) < EPSILON_F);
            assert(fabs(cimagf(f32_res) - cimagf(f32_expected)) < EPSILON_F);

            f32_res = div_f32(&f32_2);
            f32_expected = f32_1 / f32_2;
            assert(fabs(crealf(f32_res) - crealf(f32_expected)) < EPSILON_F);
            assert(fabs(cimagf(f32_res) - cimagf(f32_expected)) < EPSILON_F);

            f32_res = neg_f32();
            f32_expected = -f32_1;
            assert(fabs(crealf(f32_res) - crealf(f32_expected)) < EPSILON_F);
            assert(fabs(cimagf(f32_res) - cimagf(f32_expected)) < EPSILON_F);

            f64_res = add_f64(&f64_2);
            f64_expected = f64_1 + f64_2;
            assert(fabs(creal(f64_res) - creal(f64_expected)) < EPSILON_D);
            assert(fabs(cimag(f64_res) - cimag(f64_expected)) < EPSILON_D);

            f64_res = sub_f64(&f64_2);
            f64_expected = f64_1 - f64_2;
            assert(fabs(creal(f64_res) - creal(f64_expected)) < EPSILON_D);
            assert(fabs(cimag(f64_res) - cimag(f64_expected)) < EPSILON_D);

            f64_res = mul_f64(&f64_2);
            f64_expected = f64_1 * f64_2;
            assert(fabs(creal(f64_res) - creal(f64_expected)) < EPSILON_D);
            assert(fabs(cimag(f64_res) - cimag(f64_expected)) < EPSILON_D);

            f64_res = div_f64(&f64_2);
            f64_expected = f64_1 / f64_2;
            assert(fabs(creal(f64_res) - creal(f64_expected)) < EPSILON_D);
            assert(fabs(cimag(f64_res) - cimag(f64_expected)) < EPSILON_D);

            f64_res = neg_f64();
            f64_expected = -f64_1;
            assert(fabs(creal(f64_res) - creal(f64_expected)) < EPSILON_D);
            assert(fabs(cimag(f64_res) - cimag(f64_expected)) < EPSILON_D);

            ld_res = add_ld(&ld_2);
            ld_expected = ld_1 + ld_2;
            assert(fabsl(creall(ld_res) - creall(ld_expected)) < EPSILON_LD);
            assert(fabsl(cimagl(ld_res) - cimagl(ld_expected)) < EPSILON_LD);

            ld_res = sub_ld(&ld_2);
            ld_expected = ld_1 - ld_2;
            assert(fabsl(creall(ld_res) - creall(ld_expected)) < EPSILON_LD);
            assert(fabsl(cimagl(ld_res) - cimagl(ld_expected)) < EPSILON_LD);

            ld_res = mul_ld(&ld_2);
            ld_expected = ld_1 * ld_2;
            assert(fabsl(creall(ld_res) - creall(ld_expected)) < EPSILON_LD);
            assert(fabsl(cimagl(ld_res) - cimagl(ld_expected)) < EPSILON_LD);

            ld_res = div_ld(&ld_2);
            ld_expected = ld_1 / ld_2;
            assert(fabsl(creall(ld_res) - creall(ld_expected)) < EPSILON_LD);
            assert(fabsl(cimagl(ld_res) - cimagl(ld_expected)) < EPSILON_LD);

            ld_res = neg_ld();
            ld_expected = -ld_1;
            assert(fabsl(creall(ld_res) - creall(ld_expected)) < EPSILON_LD);
            assert(fabsl(cimagl(ld_res) - cimagl(ld_expected)) < EPSILON_LD);
        }
    }
    return EXIT_SUCCESS;
}
