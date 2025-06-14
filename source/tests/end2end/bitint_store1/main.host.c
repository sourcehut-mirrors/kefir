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

unsigned char a;
unsigned short b;
unsigned int c;
unsigned long d;
unsigned long e[2];
unsigned long f[6];

int main(void) {
    set1();
    set2();
    set3();
    set4();
    set5();
    set6();
    assert(a == 14);
    assert(b == 1024);
    assert(c == 0x1f3dec);
    assert(d == 0xb0adc0ffe);
    assert(e[0] == 0x646eddacdeb32ebdul);
    assert(e[1] == 0x6473);
    assert(f[0] == 0x6765434567765456ul);
    assert(f[1] == 0x456789edcbdecd45ul);
    assert(f[2] == 0x2345678987654323ul);
    assert(f[3] == 0x1);
    assert(f[4] == 0);
    assert((f[5] & ((1ull << 46) - 1)) == 0);
    return EXIT_SUCCESS;
}
