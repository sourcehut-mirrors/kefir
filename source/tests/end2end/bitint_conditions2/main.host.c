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
        assert(test1(i) == (MASK(i, 6) ? 1 : -1));
        assert(test2(i) == (MASK(i, 14) ? 1 : -1));
        assert(test3(i) == (MASK(i, 29) ? 1 : -1));
        assert(test4(i) == (MASK(i, 60) ? 1 : -1));
        assert(test5((struct S2) {{0, 0}}) == -1);
        assert(test5((struct S2) {{i, 0}}) == (i ? 1 : -1));
        assert(test5((struct S2) {{0, i}}) == (MASK(i, 56) ? 1 : -1));
        assert(test5((struct S2) {{i, i}}) == (i ? 1 : -1));
        assert(test6((struct S6) {{i, 0, 0, 0, 0, 0}}) == (i ? 1 : -1));
        assert(test6((struct S6) {{0, 0, 0, 0, 0, i}}) == (MASK(i, 40) ? 1 : -1));
        assert(test6((struct S6) {{0, 0, 0, 0, 0, ~i}}) == (MASK(~i, 40) ? 1 : -1));
    }
    return EXIT_SUCCESS;
}
