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

unsigned char a = 14;
unsigned short b = 1024;
unsigned int c = 0x1ffff;
unsigned long d = 0xbadc0ffe;
unsigned long e[2] = {0x4e4e45dec6e5dull, 0xffedcdefed7ull};
unsigned long f[6] = {0x1234edef4eull,  0x26352645246ull,     0x746264726324dadcull,
                      0x63624254625ull, (unsigned long) -1ll, 0x1231ull};

int main(void) {
#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))
    assert(MASK(add1(20), 6) == 34);
    assert(MASK(a, 6) == 34);
    assert(MASK(add2(1024), 14) == 2048);
    assert(MASK(b, 14) == 2048);
    assert(MASK(add3(0x20000), 27) == 0x3ffff);
    assert(MASK(c, 27) == 0x3ffff);
    assert(MASK(add4(0x100000000ull), 50) == 0x1badc0ffeull);
    assert(MASK(d, 50) == 0x1badc0ffeull);

    struct S2 s2 = add5((struct S2) {{1, 2}});
    assert(s2.arr[0] == 0x4e4e45dec6e5eull);
    assert(MASK(s2.arr[1], 47) == 0xffedcdefed9ull);
    assert(e[0] == 0x4e4e45dec6e5eull);
    assert(MASK(e[1], 47) == 0xffedcdefed9ull);

    struct S6 s6 = add6((struct S6) {{1, 2, 3, 4, 5, 6}});
    assert(s6.arr[0] == 0x1234edef4full);
    assert(s6.arr[1] == 0x26352645248ull);
    assert(s6.arr[2] == 0x746264726324dadfull);
    assert(s6.arr[3] == 0x63624254629ull);
    assert(s6.arr[4] == 4);
    assert(MASK(s6.arr[5], 46) == 0x1238ull);
    assert(f[0] == 0x1234edef4full);
    assert(f[1] == 0x26352645248ull);
    assert(f[2] == 0x746264726324dadfull);
    assert(f[3] == 0x63624254629ull);
    assert(f[4] == 4);
    assert(MASK(f[5], 46) == 0x1238ull);
    return EXIT_SUCCESS;
}
