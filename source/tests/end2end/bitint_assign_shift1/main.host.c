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
        struct S2 s2 = test1((struct S2) {{i, 0}});
        assert(s2.arr[0] == (i << 8));
        assert(MASK(s2.arr[1], 36) == 0);

        s2 = test2((struct S2) {{i, -1ll}});
        assert(s2.arr[0] == ((i >> 8) | (0xffull << 56)));
        assert(MASK(s2.arr[1], 36) == MASK(~0ull, 36));

        s2 = test3((struct S2) {{i, -1ll}});
        assert(s2.arr[0] == ((i >> 8) | (0xffull << 56)));
        assert(MASK(s2.arr[1], 36) == 0xffffffffull);
    }
    return EXIT_SUCCESS;
}
