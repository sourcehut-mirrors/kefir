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
#include <math.h>
#include <assert.h>
#include "./definitions.h"

#define EPSILON_F 1e-3
#define EPSILON_D 1e-3

static long test_factorial(long x) {
    if (x < 0) {
        return 0;
    }

    if (x < 2) {
        return x;
    }

    return test_factorial(x - 1) * x;
}

int main(void) {
    for (int i = -10; i <= 10; i++) {
        assert(factorial(i) == test_factorial(i));
        assert(factorial64(i) == test_factorial(i));
        assert(fabs(factorialf32(i) - (float) test_factorial(i)) < EPSILON_F);
        assert(fabs(factorialf64(i) - (double) test_factorial(i)) < EPSILON_D);
    }

    assert(fabs(quad_equation(1, 1, -12, 0) - (-4.0)) < EPSILON_D);
    assert(fabs(quad_equation(1, 1, -12, 1) - (3.0)) < EPSILON_D);
    assert(fabs(quad_equation(3, 25, -18, 0) - (-9.0)) < EPSILON_D);
    assert(fabs(quad_equation(3, 25, -18, 1) - (2.0 / 3.0)) < EPSILON_D);
    return EXIT_SUCCESS;
}
