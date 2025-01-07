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
    _Atomic _Complex float f32;
    _Atomic _Complex double f64;
    // _Atomic _Complex long double ld;

    _Complex float f32_res, f32_expected;
    _Complex double f64_res, f64_expected;
    // _Complex long double ld_res, ld_expected;

    for (double x = -10.0; x < 10.0; x += 0.1) {
        for (double y = -10.0; y < 10.0; y += 0.1) {
#define TEST(_id, _op)                                                \
    f32 = x + y * I;                                                  \
    f32_res = _id##_f32(&f32, y + x / 2 * I);                         \
    f32_expected = (x + y * I) _op(y + x / 2 * I);                    \
    assert(fabs(crealf(f32_res) - crealf(f32_expected)) < EPSILON_F); \
    assert(fabs(cimagf(f32_res) - cimagf(f32_expected)) < EPSILON_F); \
    assert(fabs(crealf(f32) - crealf(f32_expected)) < EPSILON_F);     \
    assert(fabs(cimagf(f32) - cimagf(f32_expected)) < EPSILON_F);     \
                                                                      \
    f64 = x + y * I;                                                  \
    f64_res = _id##_f64(&f64, y + x / 2 * I);                         \
    f64_expected = (x + y * I) _op(y + x / 2 * I);                    \
    assert(fabs(creal(f64_res) - creal(f64_expected)) < EPSILON_D);   \
    assert(fabs(cimag(f64_res) - cimag(f64_expected)) < EPSILON_D);   \
    assert(fabs(creal(f64) - creal(f64_expected)) < EPSILON_D);       \
    assert(fabs(cimag(f64) - cimag(f64_expected)) < EPSILON_D);       \
                                                                      \
    /* ld = x + y * I;                                                \
    ld_res = _id##_ld(&ld, y + x / 2 * I);                            \
    ld_expected = (x + y * I) _op (y + x / 2 * I);                    \
    assert(fabsl(creall(ld_res) - creall(ld_expected)) < EPSILON_LD); \
    assert(fabsl(cimagl(ld_res) - cimagl(ld_expected)) < EPSILON_LD); \
    assert(fabsl(creall(ld) - creall(ld_expected)) < EPSILON_LD);     \
    assert(fabsl(cimagl(ld) - cimagl(ld_expected)) < EPSILON_LD) */

            TEST(multiply, *);
            TEST(divide, /);
            TEST(add, +);
            TEST(subtract, -);
        }
    }
    return EXIT_SUCCESS;
}
