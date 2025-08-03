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
    assert(test1(1) == 16);
    assert(test1(2) == 15);
    assert(test1(3) == 15);
    assert(test1(5) == 14);
    assert(test1(1 << 15) == 1);
    assert(test1(1 << 16) == 0);

    assert(test2(1) == 31);
    assert(test2(2) == 30);
    assert(test2(3) == 30);
    assert(test2(5) == 29);
    assert(test2(1 << 15) == 16);
    assert(test2(1 << 16) == 15);
    assert(test2(1 << 17) == 14);
    assert(test2(1 << 30) == 1);
    assert(test2(1ull << 31) == 0);

    assert(test3(1) == 58);
    assert(test3(2) == 57);
    assert(test3(3) == 57);
    assert(test3(5) == 56);
    assert(test3(1 << 15) == 43);
    assert(test3(1 << 16) == 42);
    assert(test3(1 << 17) == 41);
    assert(test3(1 << 30) == 28);
    assert(test3(1ull << 31) == 27);
    assert(test3(1ull << 57) == 1);
    assert(test3(1ull << 58) == 0);

    assert(test4(1) == 63);
    assert(test4(2) == 62);
    assert(test4(3) == 62);
    assert(test4(5) == 61);
    assert(test4(1 << 15) == 48);
    assert(test4(1 << 16) == 47);
    assert(test4(1 << 17) == 46);
    assert(test4(1 << 30) == 33);
    assert(test4(1ull << 31) == 32);
    assert(test4(1ull << 57) == 6);
    assert(test4(1ull << 58) == 5);
    assert(test4(1ull << 62) == 1);
    assert(test4(1ull << 63) == 0);

    assert(test5((struct S2) {{1, 0}}) == 119);
    assert(test5((struct S2) {{2, 0}}) == 118);
    assert(test5((struct S2) {{3, 0}}) == 118);
    assert(test5((struct S2) {{5, 0}}) == 117);
    assert(test5((struct S2) {{1 << 30, 0}}) == 89);
    assert(test5((struct S2) {{1ull << 63, 0}}) == 56);
    assert(test5((struct S2) {{0, 1}}) == 55);
    assert(test5((struct S2) {{0, 1 << 10}}) == 45);
    assert(test5((struct S2) {{0, 1ull << 54}}) == 1);
    assert(test5((struct S2) {{0, 1ull << 55}}) == 0);

    assert(test6((struct S5) {{1, 0, 0, 0, 0}}) == 314);
    assert(test6((struct S5) {{1ull << 40, 0, 0, 0, 0}}) == 274);
    assert(test6((struct S5) {{0, 1, 0, 0, 0}}) == 250);
    assert(test6((struct S5) {{0, 1 << 30, 0, 0, 0}}) == 220);
    assert(test6((struct S5) {{0, 0, 1, 0, 0}}) == 186);
    assert(test6((struct S5) {{0, 0, 1 << 20, 0, 0}}) == 166);
    assert(test6((struct S5) {{0, 0, 0, 1ull << 50, 0}}) == 72);
    assert(test6((struct S5) {{0, 0, 0, 1ull << 63, 0}}) == 59);
    assert(test6((struct S5) {{0, 0, 0, 0, 1}}) == 58);
    assert(test6((struct S5) {{0, 0, 0, 0, 1ull << 57}}) == 1);
    assert(test6((struct S5) {{0, 0, 0, 0, 1ull << 58}}) == 0);
    return EXIT_SUCCESS;
}
