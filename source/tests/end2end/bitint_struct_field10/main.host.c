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
    struct S1 s1 = {0};
    struct S2 s2 = {0};
    for (int i = -4096; i < 4096; i++) {
        set1(&s1, i);
        set2(&s1, i + 1);
        set3(&s1, i + 2);
        set4(&s1, i + 3);
        set5(&s1, i + 4);
        set6(&s1, i + 5);
        set7(&s1, i + 6);
        set8(&s1, i + 7);
        set9(&s1, i + 8);

        assert(s1.a == MASK(i, 5));
        assert(s1.b == MASK(i + 1, 45));
        assert(s1.c == MASK(i + 2, 10));
        assert(s1.d == MASK(i + 3, 30));
        assert(s1.e == MASK(i + 4, 25));
        assert(s1.f == MASK(i + 5, 8));
        assert(s1.g == MASK(i + 6, 23));
        assert(s1.h == MASK(i + 7, 15));
        assert(s1.i == MASK(i + 8, 4));

        assert(MASK(get1(&s1), 5) == MASK(i, 5));
        assert(MASK(get2(&s1), 45) == MASK(i + 1, 45));
        assert(MASK(get3(&s1), 10) == MASK(i + 2, 10));
        assert(MASK(get4(&s1), 30) == MASK(i + 3, 30));
        assert(MASK(get5(&s1), 25) == MASK(i + 4, 25));
        assert(MASK(get6(&s1), 8) == MASK(i + 5, 8));
        assert(MASK(get7(&s1), 23) == MASK(i + 6, 23));
        assert(MASK(get8(&s1), 15) == MASK(i + 7, 15));
        assert(MASK(get9(&s1), 4) == MASK(i + 8, 4));

        set11(&s2, i + 1);
        set12(&s2, i + 2);
        set10(&s2, i);

        assert(s2.arr[0] == (unsigned long) i);
        assert(MASK(s2.arr[1], 16) == MASK(i >= 0 ? 0 : ~0ull, 16));

        assert(MASK(s2.arr[1] >> 16, 48) == MASK(i + 1, 48));
        assert(s2.arr[2] == (i + 1 >= 0 ? 0 : ~0ull));
        assert(MASK(s2.arr[3], 8) == MASK(i + 1 >= 0 ? 0 : ~0ull, 8));

        assert(MASK(s2.arr[3] >> 8, 56) == MASK(i + 2, 56));
        assert(MASK(s2.arr[4], 44) == MASK(i + 2 >= 0 ? 0 : ~0ull, 44));

        assert(get10(&s2) == i);
        assert(get11(&s2) == i + 1);
        assert(get12(&s2) == i + 2);
    }
    return EXIT_SUCCESS;
}
