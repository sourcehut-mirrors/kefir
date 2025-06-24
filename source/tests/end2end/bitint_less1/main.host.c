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
#include <limits.h>
#include "./definitions.h"

static long mask(unsigned long x, int width) {
    const unsigned long msb = (x >> (width - 1)) & 1;
    if (msb) {
        return (x << (sizeof(unsigned long) * CHAR_BIT - width) >> (sizeof(unsigned long) * CHAR_BIT - width)) |
               ~((1ull << width) - 1);
    } else {
        return (x << (sizeof(unsigned long) * CHAR_BIT - width) >> (sizeof(unsigned long) * CHAR_BIT - width));
    }
}

#define MASK(_x, _y) mask(_x, _y)

int main(void) {
    for (long i = -512; i < 512; i++) {
        for (long j = -512; j < 512; j++) {
            assert(less1(i, j) == (MASK(i, 6) < MASK(j, 6)));
            assert(less2(i, j) == (MASK(i, 14) < MASK(j, 14)));
            assert(less3(i, j) == (MASK(i, 29) < MASK(j, 29)));
            assert(less4(i, j) == (MASK(i, 58) < MASK(j, 58)));

            int s2 = less5((struct S2) {{i, i < 0 ? ~0ull : 0ull}}, (struct S2) {{j, j < 0 ? ~0ull : 0ull}});
            assert(s2 == (i < j));

            s2 = less5((struct S2) {{0, j + 1}}, (struct S2) {{0, i - 1}});
            assert(s2 == (j + 1 < i - 1));

            int s5 = less6((struct S5) {{i, 0, i, 0, i}}, (struct S5) {{j, 0, j, 0, j}});
            assert(s5 == (i < j));
        }
    }
    return EXIT_SUCCESS;
}
