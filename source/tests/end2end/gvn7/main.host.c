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
#define UMASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))

int main(void) {
    for (int i = -100; i < 100; i++) {
        for (int j = -100; j < 100; j++) {
#define TEST(_name, _op)                                                                             \
    do {                                                                                             \
        assert(MASK(_name(1, i, j), 19) == MASK(i _op j, 19));                                       \
        assert(MASK(_name(0, i, j), 19) == MASK(i _op j, 19));                                       \
        assert(UMASK(u##_name(1, i, j), 19) == UMASK(((unsigned int) i) _op((unsigned int) j), 19)); \
        assert(UMASK(u##_name(0, i, j), 19) == UMASK(((unsigned int) i) _op((unsigned int) j), 19)); \
    } while (0)

            TEST(add, +);
            TEST(sub, -);
            TEST(mul, *);
            TEST(and, &);
            TEST(or, |);
            TEST(xor, ^);
            if (i >= 0 && j >= 0 && j < 8) {
                TEST(shl, <<);
            }
            if (j >= 0 && j < 8) {
                TEST(shr, >>);
            }

            assert(MASK(test(1, i, j), 19) == MASK(i + j, 19));
            assert(MASK(test(0, i, j), 7) == MASK(i + j, 7));
            assert(MASK(test2(1, i), 19) == MASK(i, 19));
            assert(MASK(test2(0, i), 4) == MASK(i, 4));
        }
    }
    return EXIT_SUCCESS;
}
