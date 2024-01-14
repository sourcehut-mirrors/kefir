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
#include "./definitions.h"

#define EPSILON_D 1e-6

#ifdef __x86_64__
extern long factorial(long);
extern long test(void);
extern double custom_hypot(double, double);
#endif

long fact2(long x) {
    if (x <= 1) {
        return 1;
    }
    return x * fact2(x - 1);
}

int main(void) {
#ifdef __x86_64__
    assert(test() != 0);

    for (long x = 0; x <= 15; x++) {
        assert(factorial(x) == fact2(x));
    }

    for (double x = -100.0; x < 100.0; x += 0.1) {
        for (double y = -10.0; y < 10.0; y += 1.5) {
            assert(fabs(custom_hypot(x, y) - (x * x + y * y)) < EPSILON_D);
        }
    }
#endif
    return EXIT_SUCCESS;
}
