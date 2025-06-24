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
            assert(equals1(i, j) == (MASK(i, 6) == MASK(j, 6)));
            assert(equals2(i, j) == (MASK(i, 14) == MASK(j, 14)));
            assert(equals3(i, j) == (MASK(i, 29) == MASK(j, 29)));
            assert(equals4(i, j) == (MASK(i, 58) == MASK(j, 58)));

            int s2 = equals5((struct S2) {{i, 0}}, (struct S2) {{j, 0}});
            assert(s2 == (i == j));

            s2 = equals5((struct S2) {{0, j + 1}}, (struct S2) {{0, i - 1}});
            assert(s2 == (i - 1 == j + 1));

            int s5 = equals6((struct S5) {{i, 0, i, 0, i}}, (struct S5) {{j, 0, j, 0, j}});
            assert(s5 == (i == j));
        }
    }
    return EXIT_SUCCESS;
}
