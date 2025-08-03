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
#include <math.h>
#include "./definitions.h"

int main(void) {
    assert(test1(0, -1) == -1);
    assert(test1(1, -1) == 0);
    assert(test1(2, -1) == 1);
    assert(test1(3, -1) == 0);
    assert(test1(4, -1) == 2);
    assert(test1(5, -1) == 0);
    assert(test1(1 << 15, -1) == 15);
    assert(test1(1 << 16, -1) == 16);
    assert(test1(1 << 17, -1) == -1);

    assert(test2(0, -1) == -1);
    assert(test2(1, -1) == 0);
    assert(test2(2, -1) == 1);
    assert(test2(3, -1) == 0);
    assert(test2(4, -1) == 2);
    assert(test2(5, -1) == 0);
    assert(test2(1 << 15, -1) == 15);
    assert(test2(1 << 16, -1) == 16);
    assert(test2(1 << 17, -1) == 17);
    assert(test2(1 << 30, -1) == 30);
    assert(test2(1ull << 31, -1) == 31);

    assert(test3(0, -1) == -1);
    assert(test3(1, -1) == 0);
    assert(test3(2, -1) == 1);
    assert(test3(3, -1) == 0);
    assert(test3(4, -1) == 2);
    assert(test3(5, -1) == 0);
    assert(test3(1 << 15, -1) == 15);
    assert(test3(1 << 16, -1) == 16);
    assert(test3(1 << 17, -1) == 17);
    assert(test3(1 << 30, -1) == 30);
    assert(test3(1ull << 31, -1) == 31);
    assert(test3(1ull << 57, -1) == 57);
    assert(test3(1ull << 58, -1) == 58);
    assert(test3(1ull << 59, -1) == -1);

    assert(test4(0, -1) == -1);
    assert(test4(1, -1) == 0);
    assert(test4(2, -1) == 1);
    assert(test4(3, -1) == 0);
    assert(test4(4, -1) == 2);
    assert(test4(5, -1) == 0);
    assert(test4(1 << 15, -1) == 15);
    assert(test4(1 << 16, -1) == 16);
    assert(test4(1 << 17, -1) == 17);
    assert(test4(1 << 30, -1) == 30);
    assert(test4(1ull << 31, -1) == 31);
    assert(test4(1ull << 57, -1) == 57);
    assert(test4(1ull << 58, -1) == 58);
    assert(test4(1ull << 62, -1) == 62);
    assert(test4(1ull << 63, -1) == 63);

    assert(test5((struct S2) {{0, 0}}, -1) == -1);
    assert(test5((struct S2) {{1, 0}}, -1) == 0);
    assert(test5((struct S2) {{2, 0}}, -1) == 1);
    assert(test5((struct S2) {{3, 0}}, -1) == 0);
    assert(test5((struct S2) {{4, 0}}, -1) == 2);
    assert(test5((struct S2) {{5, 0}}, -1) == 0);
    assert(test5((struct S2) {{1 << 30, 0}}, -1) == 30);
    assert(test5((struct S2) {{1ull << 63, 0}}, -1) == 63);
    assert(test5((struct S2) {{0, 1}}, -1) == 64);
    assert(test5((struct S2) {{0, 1 << 10}}, -1) == 74);
    assert(test5((struct S2) {{0, 1ull << 54}}, -1) == 118);
    assert(test5((struct S2) {{0, 1ull << 55}}, -1) == 119);
    assert(test5((struct S2) {{0, 1ull << 56}}, -1) == -1);

    assert(test6((struct S5) {{0, 0, 0, 0, 0}}, -1) == -1);
    assert(test6((struct S5) {{1, 0, 0, 0, 0}}, -1) == 0);
    assert(test6((struct S5) {{1ull << 40, 0, 0, 0, 0}}, -1) == 40);
    assert(test6((struct S5) {{0, 1, 0, 0, 0}}, -1) == 64);
    assert(test6((struct S5) {{0, 1 << 30, 0, 0, 0}}, -1) == 94);
    assert(test6((struct S5) {{0, 0, 1, 0, 0}}, -1) == 128);
    assert(test6((struct S5) {{0, 0, 1 << 20, 0, 0}}, -1) == 148);
    assert(test6((struct S5) {{0, 0, 0, 1ull << 50, 0}}, -1) == 242);
    assert(test6((struct S5) {{0, 0, 0, 1ull << 63, 0}}, -1) == 255);
    assert(test6((struct S5) {{0, 0, 0, 0, 1}}, -1) == 256);
    assert(test6((struct S5) {{0, 0, 0, 0, 1ull << 57}}, -1) == 313);
    assert(test6((struct S5) {{0, 0, 0, 0, 1ull << 58}}, -1) == 314);
    assert(test6((struct S5) {{0, 0, 0, 0, 1ull << 59}}, -1) == -1);
    return EXIT_SUCCESS;
}
