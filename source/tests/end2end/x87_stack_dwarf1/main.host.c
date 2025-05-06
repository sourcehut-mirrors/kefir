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
#include <assert.h>
#include <math.h>
#include "./definitions.h"

int main(void) {
    for (long double a = -10.0L; a < 10.0L; a += 0.1L) {
        for (long double b = -2.0L; b < 2.0L; b += 0.1L) {
            for (long double c = -2.0L; c < 2.0L; c += 0.1L) {
                assert(fabsl(compute(a, b, c) - (b * b - 4 * a * c)) < 1e-8);
            }
        }
    }
    return EXIT_SUCCESS;
}
