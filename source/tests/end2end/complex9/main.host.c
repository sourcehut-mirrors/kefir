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

int main(void) {
    for (double real = -10.0; real < 10.0; real += 0.1) {
        for (double imaginary = -10.0; imaginary < 10.0; imaginary += 0.1) {
            _Complex float f32 = real + imaginary * I;
            _Complex double f64 = real + imaginary * I;
            _Complex long double ld = real + imaginary * I;

            _Complex float f32_res = f32_preinc(&f32);
            assert(fabs(crealf(f32_res) - (real + 1)) < EPSILON_F);
            assert(fabs(cimagf(f32_res) - imaginary) < EPSILON_F);
            assert(fabs(crealf(f32) - (real + 1)) < EPSILON_F);
            assert(fabs(cimagf(f32) - imaginary) < EPSILON_F);

            _Complex double f64_res = f64_preinc(&f64);
            assert(fabs(creal(f64_res) - (real + 1)) < EPSILON_D);
            assert(fabs(cimag(f64_res) - imaginary) < EPSILON_D);
            assert(fabs(creal(f64) - (real + 1)) < EPSILON_D);
            assert(fabs(cimag(f64) - imaginary) < EPSILON_D);

            _Complex long double ld_res = ld_preinc(&ld);
            assert(fabsl(creall(ld_res) - (real + 1)) < EPSILON_LD);
            assert(fabsl(cimagl(ld_res) - imaginary) < EPSILON_LD);
            assert(fabsl(creall(ld) - (real + 1)) < EPSILON_LD);
            assert(fabsl(cimagl(ld) - imaginary) < EPSILON_LD);

            f32 = real + imaginary * I;
            f64 = real + imaginary * I;
            ld = real + imaginary * I;

            f32_res = f32_postinc(&f32);
            assert(fabs(crealf(f32_res) - real) < EPSILON_F);
            assert(fabs(cimagf(f32_res) - imaginary) < EPSILON_F);
            assert(fabs(crealf(f32) - (real + 1)) < EPSILON_F);
            assert(fabs(cimagf(f32) - imaginary) < EPSILON_F);

            f64_res = f64_postinc(&f64);
            assert(fabs(creal(f64_res) - real) < EPSILON_D);
            assert(fabs(cimag(f64_res) - imaginary) < EPSILON_D);
            assert(fabs(creal(f64) - (real + 1)) < EPSILON_D);
            assert(fabs(cimag(f64) - imaginary) < EPSILON_D);

            ld_res = ld_postinc(&ld);
            assert(fabsl(creall(ld_res) - real) < EPSILON_LD);
            assert(fabsl(cimagl(ld_res) - imaginary) < EPSILON_LD);
            assert(fabsl(creall(ld) - (real + 1)) < EPSILON_LD);
            assert(fabsl(cimagl(ld) - imaginary) < EPSILON_LD);

            f32 = real + imaginary * I;
            f64 = real + imaginary * I;
            ld = real + imaginary * I;

            f32_res = f32_predec(&f32);
            assert(fabs(crealf(f32_res) - (real - 1)) < EPSILON_F);
            assert(fabs(cimagf(f32_res) - imaginary) < EPSILON_F);
            assert(fabs(crealf(f32) - (real - 1)) < EPSILON_F);
            assert(fabs(cimagf(f32) - imaginary) < EPSILON_F);

            f64_res = f64_predec(&f64);
            assert(fabs(creal(f64_res) - (real - 1)) < EPSILON_D);
            assert(fabs(cimag(f64_res) - imaginary) < EPSILON_D);
            assert(fabs(creal(f64) - (real - 1)) < EPSILON_D);
            assert(fabs(cimag(f64) - imaginary) < EPSILON_D);

            ld_res = ld_predec(&ld);
            assert(fabsl(creall(ld_res) - (real - 1)) < EPSILON_LD);
            assert(fabsl(cimagl(ld_res) - imaginary) < EPSILON_LD);
            assert(fabsl(creall(ld) - (real - 1)) < EPSILON_LD);
            assert(fabsl(cimagl(ld) - imaginary) < EPSILON_LD);

            f32 = real + imaginary * I;
            f64 = real + imaginary * I;
            ld = real + imaginary * I;

            f32_res = f32_postdec(&f32);
            assert(fabs(crealf(f32_res) - real) < EPSILON_F);
            assert(fabs(cimagf(f32_res) - imaginary) < EPSILON_F);
            assert(fabs(crealf(f32) - (real - 1)) < EPSILON_F);
            assert(fabs(cimagf(f32) - imaginary) < EPSILON_F);

            f64_res = f64_postdec(&f64);
            assert(fabs(creal(f64_res) - real) < EPSILON_D);
            assert(fabs(cimag(f64_res) - imaginary) < EPSILON_D);
            assert(fabs(creal(f64) - (real - 1)) < EPSILON_D);
            assert(fabs(cimag(f64) - imaginary) < EPSILON_D);

            ld_res = ld_postdec(&ld);
            assert(fabsl(creall(ld_res) - real) < EPSILON_LD);
            assert(fabsl(cimagl(ld_res) - imaginary) < EPSILON_LD);
            assert(fabsl(creall(ld) - (real - 1)) < EPSILON_LD);
            assert(fabsl(cimagl(ld) - imaginary) < EPSILON_LD);
        }
    }
    return EXIT_SUCCESS;
}
