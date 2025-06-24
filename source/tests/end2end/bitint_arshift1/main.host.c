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

    struct S2 s2 = sar5((struct S2) {{0, 1ull << 55}}, (struct S2) {{56, 0}});
    assert(s2.arr[0] == (unsigned long) 1ll << 63);
    assert(MASK(s2.arr[1], 56) == MASK(-1ll, 56));
    for (long i = 0; i < 2048; i++) {
        for (unsigned long j = 0; j < sizeof(unsigned long) * CHAR_BIT; j++) {
            if (i < 32 && j < 6) {
                assert(MASK(sar1(i, j), 6) == MASK(MASK(i, 6) >> MASK(j, 6), 6));
            }
            if (j < 14) {
                assert(MASK(sar2(i, j), 14) == MASK(i >> MASK(j, 14), 14));
            }
            if (j < 29) {
                assert(MASK(sar3(i, j), 29) == MASK(i >> MASK(j, 29), 29));
            }
            if (j < 58) {
                assert(MASK(sar4(i, j), 58) == MASK(i >> MASK(j, 58), 58));
            }

            struct S2 s2 = sar5((struct S2) {{i, 0}}, (struct S2) {{j, 0}});
            assert(s2.arr[0] == (unsigned long) (i >> j));
            assert(MASK(s2.arr[1], 56) == 0);

            s2 = sar5((struct S2) {{0, i}}, (struct S2) {{j, 0}});
            if (j > 0) {
                assert(s2.arr[0] == ((unsigned long) i) << (sizeof(unsigned long) * CHAR_BIT - j));
            } else {
                assert(s2.arr[0] == 0);
            }
            assert(MASK(s2.arr[1], 56) == (unsigned long) (i >> j));

            s2 = sar5((struct S2) {{0, i}}, (struct S2) {{sizeof(unsigned long) * CHAR_BIT, 0}});
            assert(s2.arr[0] == (unsigned long) i);
            assert(MASK(s2.arr[1], 56) == 0);

            s2 = sar5((struct S2) {{0, i}}, (struct S2) {{sizeof(unsigned long) * CHAR_BIT + 3, 0}});
            assert(s2.arr[0] == (unsigned long) (i >> 3));
            assert(MASK(s2.arr[1], 56) == 0);

            struct S5 s5 = sar6((struct S5) {{i, 0, i, 0, i}}, (struct S5) {{j, 0, 0, 0, 0}});
            assert(s5.arr[0] == (unsigned long) i >> j);
            if (j > 0) {
                assert(s5.arr[1] == (unsigned long) i << (sizeof(unsigned long) * CHAR_BIT - j));
            } else {
                assert(s5.arr[1] == 0);
            }
            assert(s5.arr[2] == (unsigned long) i >> j);
            if (j > 0) {
                assert(s5.arr[3] == (unsigned long) i << (sizeof(unsigned long) * CHAR_BIT - j));
            } else {
                assert(s5.arr[3] == 0);
            }
            assert(MASK(s5.arr[4], 54) == MASK(i >> j, 54));

            s5 = sar6((struct S5) {{i, 0, i, 0, i}}, (struct S5) {{sizeof(unsigned long) * CHAR_BIT, 0, 0, 0, 0}});
            assert(s5.arr[0] == 0);
            assert(s5.arr[1] == (unsigned long) i);
            assert(s5.arr[2] == 0);
            assert(s5.arr[3] == (unsigned long) i);
            assert(MASK(s5.arr[4], 54) == 0);

            s5 = sar6((struct S5) {{i, 0, i, 0, i}}, (struct S5) {{sizeof(unsigned long) * CHAR_BIT + 3, 0, 0, 0, 0}});
            assert(s5.arr[0] == (unsigned long) i << (sizeof(unsigned long) * CHAR_BIT - 3));
            assert(s5.arr[1] == (unsigned long) i >> 3);
            assert(s5.arr[2] == (unsigned long) i << (sizeof(unsigned long) * CHAR_BIT - 3));
            assert(s5.arr[3] == (unsigned long) i >> 3);
            assert(MASK(s5.arr[4], 54) == 0);
        }
    }
    return EXIT_SUCCESS;
}
