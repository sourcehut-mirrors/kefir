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
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include "./definitions.h"

int main(void) {
#ifdef __x86_64__
    for (long a = -100; a < 100; a++) {
        for (long d = -100; d < 100; d++) {
            assert(test1(a, 1343, -281, d) == ((a + 1343 - 281) ^ d));

            assert(sum1((struct S1){.a = (int) a, .b = (int) d}) == (int) a + (int) d);

            assert(sum2((struct S2){.arr = {a, 2 * a, a ^ d, d, 2 * d, d * a}}) ==
                   (a + 2 * a + (a ^ d) + d + 2 * d + d * a));
        }
    }
#endif
    return EXIT_SUCCESS;
}
