/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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
#include <limits.h>
#include "./definitions.h"

int main(void) {
    assert(test1());
    assert(test10());

    for (long i = -4096; i < 4096; i++) {
        assert(test2((struct S1) {{i, i * 2, i - 1, i + 100, -i, 0, i / 2, i + 1000}}) ==
               (i + i * 2 + i - 1 + i + 100 - i + i / 2 + i + 1000));
        assert(test3((struct S1) {0}, (struct S1) {{i, i * 2, i - 1, i + 100, -i, 0, i / 2, i + 1000}}) ==
               (i + i * 2 + i - 1 + i + 100 - i + i / 2 + i + 1000));
        assert(test4((struct S1) {{-1, -1, -1, -1, -1, -1, -1, -1}}, (struct S1) {0},
                     (struct S1) {{i, i * 2, i - 1, i + 100, -i, 0, i / 2, i + 1000}}) ==
               -(i + i * 2 + i - 1 + i + 100 - i + i / 2 + i + 1000));
        assert(test5(0, (struct S1) {{1, 1, 1, 1, 1, 1, 1, 1}}) == 0);
        assert(test5(1, (struct S1) {{1, 1, 1, 1, 1, 1, 1, 1}},
                     (struct S1) {{i, i * 2, i - 1, i + 100, -i, 0, i / 2, i + 1000}}) ==
               (i + i * 2 + i - 1 + i + 100 - i + i / 2 + i + 1000));
        assert(test5(2, (struct S1) {{1, 1, 1, 1, 1, 1, 1, 1}},
                     (struct S1) {{i, i * 2, i - 1, i + 100, -i, 0, i / 2, i + 1000}},
                     (struct S1) {{1, 2, 3, 4, 5, 6, 7, 8}}) ==
               (i + i * 2 + i - 1 + i + 100 - i + i / 2 + i + 1000 + 36));

        assert(test6(0) != NULL);
        assert(test7(0) != NULL);
        assert(test8(0) != NULL);
        assert(test8(1) != NULL);
        assert(test9(0) != NULL);
        assert(test9(1) != NULL);
    }
    return EXIT_SUCCESS;
}
