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

#define COND(x) ((_Bool) (x))

int main(void) {
    assert(fabs(crealf(test1_f32) - crealf(FORMULA1)) < EPSILON_F);
    assert(fabs(cimagf(test1_f32) - cimagf(FORMULA1)) < EPSILON_F);
    assert(fabs(creal(test1_f64) - creal(FORMULA1)) < EPSILON_D);
    assert(fabs(cimag(test1_f64) - cimag(FORMULA1)) < EPSILON_D);
    assert(fabsl(creall(test1_ld) - creall(FORMULA1)) < EPSILON_LD);
    assert(fabsl(cimagl(test1_ld) - cimagl(FORMULA1)) < EPSILON_LD);

    assert(fabs(crealf(test2_f32) - crealf(FORMULA2)) < EPSILON_F);
    assert(fabs(cimagf(test2_f32) - cimagf(FORMULA2)) < EPSILON_F);
    assert(fabs(creal(test2_f64) - creal(FORMULA2)) < EPSILON_D);
    assert(fabs(cimag(test2_f64) - cimag(FORMULA2)) < EPSILON_D);
    assert(fabsl(creall(test2_ld) - creall(FORMULA2)) < EPSILON_LD);
    assert(fabsl(cimagl(test2_ld) - cimagl(FORMULA2)) < EPSILON_LD);

    assert(fabs(crealf(test3_f32) - crealf(FORMULA3)) < EPSILON_F);
    assert(fabs(cimagf(test3_f32) - cimagf(FORMULA3)) < EPSILON_F);
    assert(fabs(creal(test3_f64) - creal(FORMULA3)) < EPSILON_D);
    assert(fabs(cimag(test3_f64) - cimag(FORMULA3)) < EPSILON_D);
    assert(fabsl(creall(test3_ld) - creall(FORMULA3)) < EPSILON_LD);
    assert(fabsl(cimagl(test3_ld) - cimagl(FORMULA3)) < EPSILON_LD);

    assert(fabs(crealf(test4_f32) - crealf(FORMULA4)) < EPSILON_F);
    assert(fabs(cimagf(test4_f32) - cimagf(FORMULA4)) < EPSILON_F);
    assert(fabs(creal(test4_f64) - creal(FORMULA4)) < EPSILON_D);
    assert(fabs(cimag(test4_f64) - cimag(FORMULA4)) < EPSILON_D);
    assert(fabsl(creall(test4_ld) - creall(FORMULA4)) < EPSILON_LD);
    assert(fabsl(cimagl(test4_ld) - cimagl(FORMULA4)) < EPSILON_LD);
    return EXIT_SUCCESS;
}
