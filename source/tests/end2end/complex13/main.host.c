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

int main(void) {
    for (double a = -10.0; a < 10.0; a += 0.1) {
        for (double b = -10.0; b < 10.0; b += 0.1) {
            _Complex float f32 = (float) a + (float) b * I;

            _Complex float f32res = sum32(f32, f32 + 1, f32 - I, f32 + 2, f32 - 2 * I, f32 + 3, f32 - 3 * I, f32 + 4,
                                          f32 - 4 * I, f32 + 5, f32 - 5 * I, f32 / 2);
            _Complex float expected = f32 + f32 + 1 + f32 - I + f32 + 2 + f32 - 2 * I + f32 + 3 + f32 - 3 * I + f32 +
                                      4 + f32 - 4 * I + f32 + 5 + f32 - 5 * I + f32 / 2;
            assert(fabs(crealf(f32res) - crealf(expected)) < EPSILON_F);
            assert(fabs(cimagf(f32res) - cimagf(expected)) < EPSILON_F);
        }
    }
    return EXIT_SUCCESS;
}
