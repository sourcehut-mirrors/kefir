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
    assert(test1(0) == 0);
    assert(test1(1) == 1);
    assert(test1(2) == 2);
    assert(test1(3) == 1);
    assert(test1(1 << 7) == 8);
    assert(test1(1 << 16) == 17);
    assert(test1(1 << 17) == 0);
    assert(test1((1 << 16 | 4)) == 3);

    assert(test2(0) == 0);
    assert(test2(1) == 1);
    assert(test2(2) == 2);
    assert(test2(3) == 1);
    assert(test2(1 << 7) == 8);
    assert(test2(1 << 16) == 17);
    assert(test2(1 << 17) == 18);
    assert(test2((1 << 16 | 4)) == 3);
    assert(test2(1u << 31) == 32);
    assert(test2((unsigned int) (1ull << 32)) == 0);

    assert(test3(0) == 0);
    assert(test3(1) == 1);
    assert(test3(2) == 2);
    assert(test3(3) == 1);
    assert(test3(1 << 7) == 8);
    assert(test3(1 << 16) == 17);
    assert(test3(1 << 17) == 18);
    assert(test3((1 << 16 | 4)) == 3);
    assert(test3(1u << 31) == 32);
    assert(test3(1ull << 32) == 33);
    assert(test3(1ull << 58) == 59);
    assert(test3((1ull << 58) | (1 << 7)) == 8);
    assert(test3(1ull << 59) == 0);

    assert(test4(0) == 0);
    assert(test4(1) == 1);
    assert(test4(2) == 2);
    assert(test4(3) == 1);
    assert(test4(1 << 7) == 8);
    assert(test4(1 << 16) == 17);
    assert(test4(1 << 17) == 18);
    assert(test4((1 << 16 | 4)) == 3);
    assert(test4(1u << 31) == 32);
    assert(test4(1ull << 32) == 33);
    assert(test4(1ull << 58) == 59);
    assert(test4((1ull << 58) | (1 << 7)) == 8);
    assert(test4(1ull << 59) == 60);
    assert(test4(1ull << 63) == 64);
    assert(test4(-1ll) == 1);

    assert(test5((struct S2) {{0, 0}}) == 0);
    assert(test5((struct S2) {{1, 0}}) == 1);
    assert(test5((struct S2) {{2, 0}}) == 2);
    assert(test5((struct S2) {{4, 0}}) == 3);
    assert(test5((struct S2) {{0, 1}}) == 65);
    assert(test5((struct S2) {{0, 1ull << 48}}) == 113);
    assert(test5((struct S2) {{0, 1ull << 55}}) == 120);
    assert(test5((struct S2) {{0, 1ull << 56}}) == 0);
    assert(test5((struct S2) {{4, 1ull << 55}}) == 3);

    assert(test6((struct S5) {{0, 0, 0, 0, 0}}) == 0);
    assert(test6((struct S5) {{1, 0, 0, 0, 0}}) == 1);
    assert(test6((struct S5) {{2, 0, 0, 0, 0}}) == 2);
    assert(test6((struct S5) {{8, 0, 0, 0, 0}}) == 4);
    assert(test6((struct S5) {{0, 1, 0, 0, 0}}) == 65);
    assert(test6((struct S5) {{0, 1ull << 31, 0, 0, 0}}) == 96);
    assert(test6((struct S5) {{0, 0, 1 << 5, 0, 0}}) == 134);
    assert(test6((struct S5) {{0, 0, 0, 1, 0}}) == 193);
    assert(test6((struct S5) {{0, 0, 0, 1 << 16, 0}}) == 209);
    assert(test6((struct S5) {{0, 0, 0, 0, 1}}) == 257);
    assert(test6((struct S5) {{0, 0, 0, 0, 1ull << 58}}) == 315);
    assert(test6((struct S5) {{0, 0, 0, 0, 1ull << 59}}) == 0);
    assert(test6((struct S5) {{0, 2, 0, 0, 1ull << 58}}) == 66);
    assert(test6((struct S5) {{4, 2, 0, 0, 1ull << 58}}) == 3);
    return EXIT_SUCCESS;
}
