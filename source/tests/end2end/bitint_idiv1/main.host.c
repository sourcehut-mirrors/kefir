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
    for (long i = -512; i < 512; i += 8) {
        for (long j = -512; j < 512; j += 8) {
            if (MASK(j, 6) == 0) {
                continue;
            }

            if (i >= -(1 << 5) - 1 && i < (1 << 5) && j >= -(1 << 5) - 1 && j < (1 << 5)) {
                assert(MASK(div1(i, j), 6) == MASK(i / j, 6));
            }
            assert(MASK(div2(i, j), 14) == MASK(i / j, 14));
            assert(MASK(div3(i, j), 29) == MASK(i / j, 29));
            assert(MASK(div4(i, j), 58) == MASK(i / j, 58));
        }
    }

    for (long i = -128; i < 128; i += 16) {
        for (long j = -128; j < 128; j += 16) {
            if (MASK(j, 6) == 0) {
                continue;
            }

            struct S2 s2 = div5((struct S2) {{i, (i < 0 ? -1ull : 0ull)}}, (struct S2) {{j, (j < 0 ? -1ull : 0ull)}});
            assert(s2.arr[0] == (unsigned long) (i / j));
            assert(MASK(s2.arr[1], 56) == MASK(((i < 0) == (j < 0) || (i / j) == 0) ? 0ull : -1ull, 56));

            const unsigned long s5_icmp = (i < 0 ? ~0ull : 0ull);
            const unsigned long s5_jcmp = (j < 0 ? ~0ull : 0ull);
            const unsigned long s5_high = ((i < 0) == (j < 0) || (i / j) == 0) ? 0ll : -1ll;
            struct S5 s5 = div6((struct S5) {{i, s5_icmp, s5_icmp, s5_icmp, s5_icmp}},
                                (struct S5) {{j, s5_jcmp, s5_jcmp, s5_jcmp, s5_jcmp}});
            assert(s5.arr[0] == (unsigned long) (i / j));
            assert(s5.arr[1] == s5_high);
            assert(s5.arr[2] == s5_high);
            assert(s5.arr[3] == s5_high);
            assert(MASK(s5.arr[4], 54) == MASK(s5_high, 54));
        }
    }
    return EXIT_SUCCESS;
}
