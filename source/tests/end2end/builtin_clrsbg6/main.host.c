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
    assert(test1(0) == 16);
    assert(test1(1) == 15);
    assert(test1(2) == 14);
    assert(test1(3) == 14);
    assert(test1(4) == 13);
    assert(test1(5) == 13);
    assert(test1(1 << 14) == 1);
    assert(test1(1 << 15) == 0);
    assert(test1(1 << 16) == 0);
    assert(test1(1 << 17) == 16);
    assert(test1((1 << 16) - 1) == 0);
    assert(test1((1 << 15) - 1) == 1);
    assert(test1(-1) == 16);

    assert(test2(0) == 31);
    assert(test2(1) == 30);
    assert(test2(2) == 29);
    assert(test2(3) == 29);
    assert(test2(4) == 28);
    assert(test2(5) == 28);
    assert(test2(1 << 14) == 16);
    assert(test2(1 << 15) == 15);
    assert(test2(1 << 16) == 14);
    assert(test2(1 << 17) == 13);
    assert(test2((1 << 16) - 1) == 15);
    assert(test2((1 << 15) - 1) == 16);
    assert(test2(1 << 29) == 1);
    assert(test2(1 << 30) == 0);
    assert(test2(1u << 31) == 0);
    assert(test2(-1) == 31);

    assert(test3(0) == 58);
    assert(test3(1) == 57);
    assert(test3(2) == 56);
    assert(test3(3) == 56);
    assert(test3(4) == 55);
    assert(test3(5) == 55);
    assert(test3(1 << 14) == 43);
    assert(test3(1 << 15) == 42);
    assert(test3(1 << 16) == 41);
    assert(test3(1 << 17) == 40);
    assert(test3((1 << 16) - 1) == 42);
    assert(test3((1 << 15) - 1) == 43);
    assert(test3(1 << 29) == 28);
    assert(test3(1 << 30) == 27);
    assert(test3(1u << 31) == 26);
    assert(test3(1ull << 56) == 1);
    assert(test3(1ull << 57) == 0);
    assert(test3(1ull << 58) == 0);
    assert(test3(-1) == 58);

    assert(test4(0) == 63);
    assert(test4(1) == 62);
    assert(test4(2) == 61);
    assert(test4(3) == 61);
    assert(test4(4) == 60);
    assert(test4(5) == 60);
    assert(test4(1 << 14) == 48);
    assert(test4(1 << 15) == 47);
    assert(test4(1 << 16) == 46);
    assert(test4(1 << 17) == 45);
    assert(test4((1 << 16) - 1) == 47);
    assert(test4((1 << 15) - 1) == 48);
    assert(test4(1 << 29) == 33);
    assert(test4(1 << 30) == 32);
    assert(test4(1u << 31) == 31);
    assert(test4(1ull << 56) == 6);
    assert(test4(1ull << 57) == 5);
    assert(test4(1ull << 58) == 4);
    assert(test4(1ull << 61) == 1);
    assert(test4(1ull << 62) == 0);
    assert(test4(1ull << 63) == 0);
    assert(test4(-1) == 63);

    assert(test5((struct S2) {{0, 0}}) == 119);
    assert(test5((struct S2) {{1, 0}}) == 118);
    assert(test5((struct S2) {{1 << 20, 0}}) == 98);
    assert(test5((struct S2) {{1ul << 41, 0}}) == 77);
    assert(test5((struct S2) {{1ul << 63, 0}}) == 55);
    assert(test5((struct S2) {{0, 1}}) == 54);
    assert(test5((struct S2) {{0, 1 << 30}}) == 24);
    assert(test5((struct S2) {{0, 1ul << 44}}) == 10);
    assert(test5((struct S2) {{0, 1ul << 53}}) == 1);
    assert(test5((struct S2) {{0, 1ul << 54}}) == 0);
    assert(test5((struct S2) {{~0ull, ~0ull}}) == 119);
    assert(test5((struct S2) {{~0ull, (~0ull) >> 9}}) == 0);
    assert(test5((struct S2) {{~0ull, (~0ull) >> 10}}) == 1);

    assert(test6((struct S5) {{0, 0, 0, 0, 0}}) == 314);
    assert(test6((struct S5) {{1, 0, 0, 0, 0}}) == 313);
    assert(test6((struct S5) {{1ull << 50, 0, 0, 0, 0}}) == 263);
    assert(test6((struct S5) {{1ull << 63, 0, 0, 0, 0}}) == 250);
    assert(test6((struct S5) {{0, 1, 0, 0, 0}}) == 249);
    assert(test6((struct S5) {{0, 0, 1 << 5, 0, 0}}) == 180);
    assert(test6((struct S5) {{0, 0, 1ull << 63, 0, 0}}) == 122);
    assert(test6((struct S5) {{0, 0, 0, 1, 0}}) == 121);
    assert(test6((struct S5) {{0, 0, 0, 1 << 30, 0}}) == 91);
    assert(test6((struct S5) {{0, 0, 0, 1ull << 63, 0}}) == 58);
    assert(test6((struct S5) {{0, 0, 0, 0, 1}}) == 57);
    assert(test6((struct S5) {{0, 0, 0, 0, 1ull << 56}}) == 1);
    assert(test6((struct S5) {{0, 0, 0, 0, 1ull << 57}}) == 0);
    assert(test6((struct S5) {{0, 0, 0, 0, 1ull << 58}}) == 0);
    assert(test6((struct S5) {{0, 0, 0, 0, 1ull << 59}}) == 314);
    assert(test6((struct S5) {{~0ull, ~0ull, ~0ull, ~0ull, ~0ull}}) == 314);
    assert(test6((struct S5) {{~0ull, ~0ull, ~0ull, ~0ull, (~0ull) >> 5}}) == 314);
    assert(test6((struct S5) {{~0ull, ~0ull, ~0ull, ~0ull, (~0ull) >> 6}}) == 0);
    assert(test6((struct S5) {{~0ull, ~0ull, ~0ull, ~0ull, (~0ull) >> 7}}) == 1);
    return EXIT_SUCCESS;
}
