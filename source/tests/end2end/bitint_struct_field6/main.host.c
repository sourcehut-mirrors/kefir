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
        struct S1 s1 = get1(i);
        assert(MASK(s1.a, 6) == MASK(i, 6));
        assert(MASK(s1.b, 50) == MASK(i + 1, 50));
        assert(MASK(s1.c, 14) == MASK(i + 2, 14));
        assert(s1.d[0] == (unsigned long) (i + 3));
        assert(s1.d[1] == (i + 3 >= 0 ? 0 : ~0ull));
        assert(MASK(s1.d[2], 22) == MASK((i + 3 >= 0 ? 0 : ~0ull), 22));
        assert(MASK(s1.e, 29) == MASK(i + 4, 29));
        assert(s1.f[0] == (unsigned long) (i + 5));
        assert(MASK(s1.f[1], 56) == MASK((i + 5 >= 0 ? 0 : ~0ull), 56));

        assert(MASK(get2(s1), 6) == MASK(i, 6));
        assert(MASK(get3(s1), 50) == MASK(i + 1, 50));
        assert(MASK(get4(s1), 14) == MASK(i + 2, 14));
        assert(get5(s1) == (i + 3));
        assert(MASK(get6(s1), 29) == MASK(i + 4, 29));
        assert(get7(s1) == (i + 5));
    }
    return EXIT_SUCCESS;
}
