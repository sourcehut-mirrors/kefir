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

#define EPSILON_LD 1e-8

long double sum(long double x, long double y) {
    return x + y;
}

long double mul(long double x, long double y) {
    return x * y;
}

int main(void) {
    for (long double x = -100.0L; x < 100.0L; x += 0.1L) {
        for (long double y = -5.0L; y < 5.0L; y += 0.1L) {
            assert(fabsl(my_hypot(x, y) - (x * x + y * y)) < EPSILON_LD);
        }
    }
    return EXIT_SUCCESS;
}
