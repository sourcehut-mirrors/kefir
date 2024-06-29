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

int main(void) {
#ifdef __x86_64__
    for (double x = 0.1; x < 10.0; x += 0.1) {
        for (double y = 0.1; y < 10.0; y += 0.1) {
            assert(fabs(subf((float) x, (float) y) - (x - y)) <= 1.0e-3);
            assert(fabs(sub(x, y) - (x - y)) <= 1.0e-6);
            assert(fabsl(subl(x, y) - (x - y)) <= 1.0e-8L);

            assert(fabs(mydivf((float) x, (float) y) - (x / y)) <= 1.0e-3);
            assert(fabs(mydiv(x, y) - (x / y)) <= 1.0e-6);
            assert(fabsl(mydivl(x, y) - (x / y)) <= 1.0e-8L);

            assert(fabs(mydivrf((float) x, (float) y) - (y / x)) <= 1.0e-3);
            assert(fabs(mydivr(x, y) - (y / x)) <= 1.0e-6);
            assert(fabsl(mydivrl(x, y) - (y / x)) <= 1.0e-8L);
        }
    }
#endif
    return EXIT_SUCCESS;
}
