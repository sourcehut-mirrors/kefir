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

static _Bool flag1 = 0;
static _Bool flag2 = 0;
static _Bool flag3 = 0;
static _Bool flag4 = 0;
static _Bool flag5 = 0;
static _Bool flag6 = 0;

#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))
void fn1(unsigned char x) {
    assert(MASK(x, 6) == 23);
    flag1 = 1;
}

void fn2(unsigned short x) {
    assert(MASK(x, 14) == MASK((unsigned long) -4096, 14));
    flag2 = 1;
}

void fn3(unsigned int x) {
    assert(MASK(x, 29) == MASK((unsigned long) 0xcafe22, 29));
    flag3 = 1;
}

void fn4(unsigned long x) {
    assert(MASK(x, 62) == MASK((unsigned long) 0xbad0c0ffe, 62));
    flag4 = 1;
}

void fn5(struct S2 x) {
    assert(x.arr[0] == 0xaaaa00002222ccccull);
    assert(MASK(x.arr[1], 58) == 0xeeee);
    flag5 = 1;
}

void fn6(struct S6 x) {
    assert(x.arr[0] == 0xddddaaaaeeeebbbbull);
    assert(x.arr[1] == 0x222233330000ccccull);
    assert(x.arr[2] == 0x00002222cccc4444ull);
    assert(x.arr[3] == 0xeeeeaaaaull);
    flag6 = 1;
}

int main(void) {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();

    assert(flag1);
    assert(flag2);
    assert(flag3);
    assert(flag4);
    assert(flag5);
    assert(flag6);
    return EXIT_SUCCESS;
}
