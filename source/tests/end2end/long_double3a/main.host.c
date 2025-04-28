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
#include "./definitions.h"

long double sum10(const long double a, long double b, long double c, long double d, volatile long double e,
                  long double f, long double g, long double h, long const volatile double i, long double j) {
    return a + b + c + d + e + f + g + h + i + j;
}

int main(void) {
    for (long double x = 0.0l; x < 1000.0l; x += 0.1l) {
        for (long double step = -1.0l; step <= 1.0l; step += 0.1l) {
            long double res = sum_progression(x, step);
            long double expected = x * 10.0l + step * 45.0l;
            assert(fabsl(res - expected) < 1e-6);
        }
    }
    return EXIT_SUCCESS;
}
