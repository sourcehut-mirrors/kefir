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

unsigned char a = 34;
unsigned short b = 1024;
unsigned int c = 0x1ffff;
unsigned long d = 0xbadc0ffe;
unsigned long e[2] = {0x4e4e45dec6e5dull, 0xffedcdefed7ull};
unsigned long f[6] = {0x1234edef4eull,  0x26352645246ull,     0x746264726324dadcull,
                      0x63624254625ull, (unsigned long) -1ll, 0x1231ull};

int main(void) {
    assert(get1() == 34);
    assert(get2() == 1024);
    assert(get3() == 0x1ffff);
    assert(get4() == 0xbadc0ffe);
    assert(get5().arr[0] == 0x4e4e45dec6e5dull);
    assert(get5().arr[1] == 0xffedcdefed7ull);
    assert(get6().arr[0] == 0x1234edef4eull);
    assert(get6().arr[1] == 0x26352645246ull);
    assert(get6().arr[2] == 0x746264726324dadcull);
    assert(get6().arr[3] == 0x63624254625ull);
    assert(get6().arr[4] == (unsigned long) -1ll);
    assert(get6().arr[5] == 0x1231ull);
    return EXIT_SUCCESS;
}
