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
    struct S8 s8 = test1(0, (struct S8) {{1234, 0xbad0c, ~0ull, 0x449294, 0, ~0xfefec, 5324, 1}});
    assert(s8.arr[0] == 1234);
    assert(s8.arr[1] == 0xbad0c);
    assert(s8.arr[2] == ~0ull);
    assert(s8.arr[3] == 0x449294);
    assert(s8.arr[4] == 0);
    assert(s8.arr[5] == (unsigned long) ~0xfefec);
    assert(s8.arr[6] == 5324);
    assert(s8.arr[7] == 1);

    s8 = test1(1, (struct S8) {{1234, 0xbad0c, ~0ull, 0x449294, 0, ~0xfefec, 5324, 1}},
               (struct S8) {{1, 2, 3, 4, 5, 6, 7, 8}});
    assert(s8.arr[0] == 1);
    assert(s8.arr[1] == 2);
    assert(s8.arr[2] == 3);
    assert(s8.arr[3] == 4);
    assert(s8.arr[4] == 5);
    assert(s8.arr[5] == 6);
    assert(s8.arr[6] == 7);
    assert(s8.arr[7] == 8);
    return EXIT_SUCCESS;
}
