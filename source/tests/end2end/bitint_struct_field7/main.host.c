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
#include <float.h>
#include "./definitions.h"

int main(void) {
#define MASK(_x, _y) (((unsigned long) (_x)) & ((1ull << (_y)) - 1))
    for (int i = -1024; i < 1024; i++) {
        struct S1 s1 = {.a = i, .b = i + 1, .c = i + 2, .e = i + 4};
        assert(MASK(get2(s1), 5) == MASK(s1.a, 5));
        assert(MASK(get3(s1), 45) == MASK(s1.b, 45));
        assert(MASK(get4(s1), 10) == MASK(s1.c, 10));
        assert(MASK(get6(s1), 25) == MASK(s1.e, 25));

        struct S2 s2 = {.arr[0] = ((unsigned long) i) << 10,
                        .arr[1] = i >= 0 ? 0 : ~0ull,
                        .arr[2] = MASK(i >= 0 ? 0 : ~0ull, 2) | (((unsigned long) (i + 1)) << 2)};
        assert(get5(s2) == i);
        assert(get7(s2) == i + 1);
    }
    return EXIT_SUCCESS;
}
