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
        assert(MASK(test1(i), 6) == MASK(i + 1, 6));
        assert(MASK(test2(i), 14) == MASK(i + 1, 14));
        assert(MASK(test3(i), 29) == MASK(i + 1, 29));
        assert(MASK(test4(i), 60) == MASK(i + 1, 60));

        struct S2 s2 = test5((struct S2) {{~0ull, i}});
        assert(s2.arr[0] == 0);
        assert(MASK(s2.arr[1], 56) == MASK(i + 1, 56));
    }
    return EXIT_SUCCESS;
}
