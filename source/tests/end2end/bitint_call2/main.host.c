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
#include "./definitions.h"

#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))

unsigned char fn1(void) {
    return 23;
}

unsigned short fn2(void) {
    return 0x1000;
}

unsigned int fn3(void) {
    return 0xcafe0;
}

unsigned long fn4(void) {
    return 0x1ee52c0ffeull;
}

struct S2 fn5(void) {
    return (struct S2) {{0xc0ffe, 0xbadd0}};
}

struct S6 fn6(void) {
    return (struct S6) {{0x12345, 0x9876, 0xabcd, 0xdef123, ~0ull, 1}};
}

int main(void) {
    assert(MASK(test1(), 6) == 23);
    assert(MASK(test2(), 14) == 0x1000);
    assert(MASK(test3(), 29) == 0xcafe0);
    assert(MASK(test4(), 60) == 0x1ee52c0ffeull);

    struct S2 s2 = test5();
    assert(s2.arr[0] == 0xc0ffe);
    assert(MASK(s2.arr[1], 58) == 0xbadd0);

    struct S6 s6 = test6();
    assert(s6.arr[0] == 0x12345);
    assert(s6.arr[1] == 0x9876);
    assert(s6.arr[2] == 0xabcd);
    assert(s6.arr[3] == 0xdef123);
    assert(s6.arr[4] == ~0ull);
    assert(s6.arr[5] == 1);
    return EXIT_SUCCESS;
}
