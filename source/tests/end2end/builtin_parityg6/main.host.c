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
    assert(test1(2) == 1);
    assert(test1(3) == 0);
    assert(test1(4) == 1);
    assert(test1(5) == 0);
    assert(test1(7) == 1);
    assert(test1(1 << 16) == 1);
    assert(test1((1 << 16) | 1023) == 1);
    assert(test1((unsigned int) ~0ull) == 1);
    assert(test1((1u << 16) - 1) == 0);
    assert(test1(1u << 17) == 0);

    assert(test2(0) == 0);
    assert(test2(1) == 1);
    assert(test2(2) == 1);
    assert(test2(3) == 0);
    assert(test2(4) == 1);
    assert(test2(5) == 0);
    assert(test2(7) == 1);
    assert(test2(1 << 16) == 1);
    assert(test2((1 << 16) | 1023) == 1);
    assert(test2((unsigned int) ~0ull) == 0);
    assert(test2((1u << 16) - 1) == 0);
    assert(test2(1u << 17) == 1);
    assert(test2((1u << 31) | 1023) == 1);
    assert(test2((1u << 31) - 1) == 1);
    assert(test2(~0u) == 0);
    assert(test2((~0u) >> 3) == 1);

    assert(test3(0) == 0);
    assert(test3(1) == 1);
    assert(test3(2) == 1);
    assert(test3(3) == 0);
    assert(test3(4) == 1);
    assert(test3(5) == 0);
    assert(test3(7) == 1);
    assert(test3(1 << 16) == 1);
    assert(test3((1 << 16) | 1023) == 1);
    assert(test3((unsigned int) ~0ull) == 0);
    assert(test3((1u << 16) - 1) == 0);
    assert(test3(1u << 17) == 1);
    assert(test3((1u << 31) | 1023) == 1);
    assert(test3((1u << 31) - 1) == 1);
    assert(test3(~0u) == 0);
    assert(test3((~0u) >> 3) == 1);
    assert(test3(1ull << 58) == 1);
    assert(test3((1ull << 58) - 1) == 0);
    assert(test3(1ull << 59) == 0);
    assert(test3(~0ull) == 1);
    assert(test3((~0ull) >> 15) == 1);

    assert(test4(0) == 0);
    assert(test4(1) == 1);
    assert(test4(2) == 1);
    assert(test4(3) == 0);
    assert(test4(4) == 1);
    assert(test4(5) == 0);
    assert(test4(7) == 1);
    assert(test4(1 << 16) == 1);
    assert(test4((1 << 16) | 1023) == 1);
    assert(test4((unsigned int) ~0ull) == 0);
    assert(test4((1u << 16) - 1) == 0);
    assert(test4(1u << 17) == 1);
    assert(test4((1u << 31) | 1023) == 1);
    assert(test4((1u << 31) - 1) == 1);
    assert(test4(~0u) == 0);
    assert(test4((~0u) >> 3) == 1);
    assert(test4(1ull << 58) == 1);
    assert(test4((1ull << 58) - 1) == 0);
    assert(test4(1ull << 59) == 1);
    assert(test4(~0ull) == 0);
    assert(test4((~0ull) >> 15) == 1);
    assert(test4(1ull << 63) == 1);
    assert(test4((1ull << 63) - 1) == 1);

    assert(test5((struct S2) {{0, 0}}) == 0);
    assert(test5((struct S2) {{1, 0}}) == 1);
    assert(test5((struct S2) {{1023, 0}}) == 0);
    assert(test5((struct S2) {{1023, 1}}) == 1);
    assert(test5((struct S2) {{~0ull, 1}}) == 1);
    assert(test5((struct S2) {{0, ~0ull}}) == 0);
    assert(test5((struct S2) {{~0ull, ~0ull}}) == 0);

    assert(test6((struct S5) {{0, 0, 0, 0, 0}}) == 0);
    assert(test6((struct S5) {{1, 0, 0, 0, 0}}) == 1);
    assert(test6((struct S5) {{1, 1 << 2, 1 << 4, 1 << 8, 1 << 16}}) == 1);
    assert(test6((struct S5) {{~0ull, 1 << 2, 1 << 4, 1 << 8, ~0ull}}) == 0);
    assert(test6((struct S5) {{~0ull, ~0ull, ~0ull, ~0ull, ~0ull}}) == 1);
    return EXIT_SUCCESS;
}
