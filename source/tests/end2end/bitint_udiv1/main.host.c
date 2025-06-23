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
#include <limits.h>
#include "./definitions.h"

int main(void) {
#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))

    struct S5 s5 = div6((struct S5) {{0, 0, 0, 0, 1}}, (struct S5) {{2, 0, 0, 0, 0}});
    assert(s5.arr[0] == 0);
    assert(s5.arr[1] == 0);
    assert(s5.arr[2] == 0);
    assert(s5.arr[3] == (1ull << 63));
    assert(MASK(s5.arr[4], 54) == 0);

    for (unsigned long i = 0; i < 512; i += 4) {
        for (unsigned long j = 0; j < 512; j += 4) {
            if (MASK(j, 6) == 0) {
                continue;
            }

            if (i < (1 << 5) && j < (1 << 5)) {
                assert(MASK(div1(i, j), 6) == MASK(i / j, 6));
            }
            assert(MASK(div2(i, j), 14) == MASK(i / j, 14));
            assert(MASK(div3(i, j), 29) == MASK(i / j, 29));
            assert(MASK(div4(i, j), 58) == MASK(i / j, 58));

            struct S2 s2 = div5((struct S2) {{i, 0}}, (struct S2) {{j, 0}});
            assert(s2.arr[0] == (unsigned long) (i / j));
            assert(MASK(s2.arr[1], 56) == 0);

            struct S5 s5 = div6((struct S5) {{i, 0, 0, 0, 0}}, (struct S5) {{j, 0, 0, 0, 0}});
            assert(s5.arr[0] == (unsigned long) (i / j));
            assert(s5.arr[1] == 0);
            assert(s5.arr[2] == 0);
            assert(s5.arr[3] == 0);
            assert(MASK(s5.arr[4], 54) == MASK(0, 54));
        }
    }
    return EXIT_SUCCESS;
}
