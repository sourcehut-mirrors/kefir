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
    for (unsigned long i = 0; i < 4096; i++) {
        assert(MASK(neg1(i), 6) == MASK(-i, 6));
        assert(MASK(neg2(i), 14) == MASK(-i, 14));
        assert(MASK(neg3(i), 29) == MASK(-i, 29));
        assert(MASK(neg4(i), 58) == MASK(-i, 58));

        struct S2 s2 = neg5((struct S2) {{i, 0}});
        assert(s2.arr[0] == -i);
        assert(MASK(s2.arr[1], 55) == MASK(i > 0 ? -1ll : 0, 55));

        s2 = neg5((struct S2) {{-i, i > 0 ? -1 : 0}});
        assert(s2.arr[0] == i);
        assert(MASK(s2.arr[1], 55) == 0);

        struct S5 s5 = neg6((struct S5) {{i, 0, 0, 0, 0}});
        assert(s5.arr[0] == -i);
        assert(s5.arr[1] == (unsigned long) (i > 0 ? -1ll : 0));
        assert(s5.arr[2] == (unsigned long) (i > 0 ? -1ll : 0));
        assert(s5.arr[3] == (unsigned long) (i > 0 ? -1ll : 0));
        assert(MASK(s5.arr[4], 55) == MASK(i > 0 ? -1ll : 0, 55));

        unsigned long s5_cmp = (unsigned long) (i > 0 ? -1ull : 0);
        s5 = neg6((struct S5) {{-i, s5_cmp, s5_cmp, s5_cmp, s5_cmp}});
        assert(s5.arr[0] == i);
        assert(s5.arr[1] == 0);
        assert(s5.arr[2] == 0);
        assert(s5.arr[3] == 0);
        assert(MASK(s5.arr[4], 55) == 0);
    }
    return EXIT_SUCCESS;
}
