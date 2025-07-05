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

        assert(MASK(s1.a, 5) == MASK(i, 5));
        assert(MASK(s1.b, 12) == MASK(i + 1, 12));
        assert(MASK(s1.c, 20) == MASK(i + 2, 20));
        assert(MASK(s1.d, 27) == MASK(i + 3, 27));
        assert(MASK(s1.e, 35) == MASK(i + 4, 35));
        assert(MASK(s1.f, 46) == MASK(i + 5, 46));
        assert(MASK(s1.g, 53) == MASK(i + 6, 53));
        assert(MASK(s1.h, 59) == MASK(i + 7, 59));
        assert(s1.i[0] == (unsigned long) (i + 8));
        assert(MASK(s1.i[1], 36) == MASK(i + 8 >= 0 ? 0 : ~0ull, 36));

        assert(MASK(get1(&s1), 5) == MASK(i, 5));
        assert(MASK(get2(&s1), 12) == MASK(i + 1, 12));
        assert(MASK(get3(&s1), 20) == MASK(i + 2, 20));
        assert(MASK(get4(&s1), 27) == MASK(i + 3, 27));
        assert(MASK(get5(&s1), 35) == MASK(i + 4, 35));
        assert(MASK(get6(&s1), 46) == MASK(i + 5, 46));
        assert(MASK(get7(&s1), 53) == MASK(i + 6, 53));
        assert(MASK(get8(&s1), 59) == MASK(i + 7, 59));
        assert(get9(&s1) == i + 8);

        struct S1 s2 = {i, i + 1, i + 2, i + 3, i + 4, i + 5, i + 6, i + 7, {i + 8, (i + 8 >= 0 ? 0 : ~0ull)}};
        assert(s1.a == s2.a);
        assert(s1.b == s2.b);
        assert(s1.c == s2.c);
        assert(s1.d == s2.d);
        assert(s1.e == s2.e);
        assert(s1.f == s2.f);
        assert(s1.g == s2.g);
        assert(s1.h == s2.h);
        assert(s1.i[0] == s2.i[0]);
        assert(MASK(s1.i[1], 36) == MASK(s2.i[1], 36));
    }
    return EXIT_SUCCESS;
}
