/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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
    _Complex long double t1 = test1();
    assert(fabsl(creall(t1)) < EPSILON_LD);
    assert(fabsl(cimagl(t1) - 18.0L) < EPSILON_LD);

    for (double a = -10.0; a < 10.0; a += 0.1) {
        for (double b = -10.0; b < 10.0; b += 0.1) {
            _Complex float f32 = cmpf32((float) a, (float) b);
            _Complex double f64 = cmpf64((double) a, (double) b);
            _Complex long double ld = cmpld((long double) a, (long double) b);

            assert(fabs(crealf(f32) - (float) a) < EPSILON_F);
            assert(fabs(cimagf(f32) - (float) b) < EPSILON_F);
            assert(fabs(creal(f64) - (double) a) < EPSILON_D);
            assert(fabs(cimag(f64) - (double) b) < EPSILON_D);
            assert(fabsl(creall(ld) - (long double) a) < EPSILON_LD);
            assert(fabsl(cimagl(ld) - (long double) b) < EPSILON_LD);
        }
    }
    return EXIT_SUCCESS;
}
