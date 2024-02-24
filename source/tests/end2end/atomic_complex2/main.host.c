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

int main(void) {
    _Atomic _Complex float f32;
    _Atomic _Complex double f64;
    _Atomic _Complex long double ld;

    _Atomic _Complex float f32_arr[4];
    _Atomic _Complex double f64_arr[4];
    _Atomic _Complex long double ld_arr[4];

    for (double x = -100.0; x < 100.0; x += 1.0) {
        for (double y = -10.0; y < 10.0; y += 0.1) {
            f32 = x + y * I;
            f64 = x + y * I;
            ld = x + y * I;

            _Complex float f32_res = f32_load(&f32);
            assert(fabs(crealf(f32_res) - x) < EPSILON_F);
            assert(fabs(cimagf(f32_res) - y) < EPSILON_F);

            _Complex double f64_res = f64_load(&f64);
            assert(fabs(creal(f64_res) - x) < EPSILON_D);
            assert(fabs(cimag(f64_res) - y) < EPSILON_D);

            _Complex long double ld_res = ld_load(&ld);
            assert(fabsl(creall(ld_res) - x) < EPSILON_LD);
            assert(fabsl(cimagl(ld_res) - y) < EPSILON_LD);

            f32_store(&f32, y + x * I);
            assert(fabs(crealf(f32) - y) < EPSILON_F);
            assert(fabs(cimagf(f32) - x) < EPSILON_F);

            f64_store(&f64, y + x * I);
            assert(fabs(creal(f64) - y) < EPSILON_D);
            assert(fabs(cimag(f64) - x) < EPSILON_D);

            ld_store(&ld, y + x * I);
            assert(fabsl(creall(ld) - y) < EPSILON_LD);
            assert(fabsl(cimagl(ld) - x) < EPSILON_LD);

            f32_arr[0] = x + y * I;
            f32_arr[1] = x + 2 * y * I;
            f32_arr[2] = x + 3 * y * I;
            f32_arr[3] = x + 4 * y * I;

            f64_arr[0] = y + x * I;
            f64_arr[1] = y + 2 * x * I;
            f64_arr[2] = y + 3 * x * I;
            f64_arr[3] = y + 4 * x * I;

            ld_arr[0] = -x - y * I;
            ld_arr[1] = -x - 2 * y * I;
            ld_arr[2] = -x - 3 * y * I;
            ld_arr[3] = -x - 4 * y * I;

            for (int i = 0; i < 4; i++) {
                f32_res = f32_load_index(f32_arr, i);
                assert(fabs(crealf(f32_res) - x) < EPSILON_F);
                assert(fabs(cimagf(f32_res) - ((i + 1) * y)) < EPSILON_F);

                f32_store_index(f32_arr, i, x / 2 + y / 2 * I);
                assert(fabs(crealf(f32_arr[i]) - x / 2) < EPSILON_F);
                assert(fabs(cimagf(f32_arr[i]) - y / 2) < EPSILON_F);

                f64_res = f64_load_index(f64_arr, i);
                assert(fabs(creal(f64_res) - y) < EPSILON_D);
                assert(fabs(cimag(f64_res) - ((i + 1) * x)) < EPSILON_D);

                f64_store_index(f64_arr, i, x / 2 + y / 2 * I);
                assert(fabs(creal(f64_arr[i]) - x / 2) < EPSILON_D);
                assert(fabs(cimag(f64_arr[i]) - y / 2) < EPSILON_D);

                ld_res = ld_load_index(ld_arr, i);
                assert(fabsl(creall(ld_res) + x) < EPSILON_LD);
                assert(fabsl(cimagl(ld_res) + ((i + 1) * y)) < EPSILON_LD);

                ld_store_index(ld_arr, i, x / 2 + y / 2 * I);
                assert(fabsl(creall(ld_arr[i]) - x / 2) < EPSILON_LD);
                assert(fabsl(cimagl(ld_arr[i]) - y / 2) < EPSILON_LD);
            }

            struct Str1 str1 = {.a = x + y * I, .b = y + x * I, .c = -x - y * I};
            f32_res = str1_a(&str1);
            assert(fabs(crealf(f32_res) - x) < EPSILON_F);
            assert(fabs(cimagf(f32_res) - y) < EPSILON_F);

            str1_set_a(&str1, x - y * I);
            assert(fabs(crealf(str1.a) - x) < EPSILON_F);
            assert(fabs(cimagf(str1.a) + y) < EPSILON_F);

            f64_res = str1_b(&str1);
            assert(fabs(creal(f64_res) - y) < EPSILON_D);
            assert(fabs(cimag(f64_res) - x) < EPSILON_D);

            str1_set_b(&str1, y - x * I);
            assert(fabs(creal(str1.b) - y) < EPSILON_D);
            assert(fabs(cimag(str1.b) + x) < EPSILON_D);

            ld_res = str1_c(&str1);
            assert(fabsl(creall(ld_res) + x) < EPSILON_LD);
            assert(fabsl(cimagl(ld_res) + y) < EPSILON_LD);

            str1_set_c(&str1, -x + y * I);
            assert(fabsl(creall(str1.c) + x) < EPSILON_LD);
            assert(fabsl(cimagl(str1.c) - y) < EPSILON_LD);
        }
    }
    return EXIT_SUCCESS;
}
