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
#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))
    for (unsigned long i = 0; i < 512; i++) {
        for (unsigned long j = 0; j < 512; j++) {
            assert(MASK(sub1(i, j), 6) == MASK(MASK(i, 6) - MASK(j, 6), 6));
            assert(MASK(sub2(i, j), 14) == MASK(MASK(i, 14) - MASK(j, 14), 14));
            assert(MASK(sub3(i, j), 29) == MASK(MASK(i, 29) - MASK(j, 29), 29));
            assert(MASK(sub4(i, j), 58) == MASK(MASK(i, 58) - MASK(j, 58), 58));

            struct S2 s2 = sub5((struct S2) {{i, 0}}, (struct S2) {{j, 0}});
            assert(s2.arr[0] == i - j);
            assert(MASK(s2.arr[1], 56) == MASK(i >= j ? 0 : ~0ull, 56));

            s2 = sub5((struct S2) {{i, 0}}, (struct S2) {{0, j}});
            assert(s2.arr[0] == i);
            assert(MASK(s2.arr[1], 56) == MASK(-j, 56));

            s2 = sub5((struct S2) {{0, i}}, (struct S2) {{0, j}});
            assert(s2.arr[0] == 0);
            assert(MASK(s2.arr[1], 56) == MASK(i - j, 56));

            struct S5 s5 = sub6((struct S5) {{i, 0, i, 0, i}}, (struct S5) {{j, 0, j, 0, j}});
            assert(s5.arr[0] == i - j);
            assert(s5.arr[1] == (i >= j ? 0 : ~0ull));
            assert(s5.arr[2] == i - j - (i >= j ? 0 : 1));
            assert(s5.arr[3] == (i >= j ? 0 : ~0ull));
            assert(MASK(s5.arr[4], 54) == MASK(i - j - (i >= j ? 0 : 1), 54));
        }
    }

    struct S5 s5 = sub6((struct S5) {{0ull, 0ull, 0ull, 0ull, 0}}, (struct S5) {{1, 0, 0, 0, 0}});
    assert(s5.arr[0] == ~0ull);
    assert(s5.arr[1] == ~0ull);
    assert(s5.arr[2] == ~0ull);
    assert(s5.arr[3] == ~0ull);
    assert(MASK(s5.arr[4], 54) == MASK(~0ull, 54));
    return EXIT_SUCCESS;
}
