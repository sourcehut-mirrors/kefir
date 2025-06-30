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

int main(void) {
#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))
    assert(MASK(get1(), 6) == 9);
    assert(MASK(get3(), 6) == 57);
    assert(MASK(get5(), 6) == 22);
    assert(MASK(get7(), 6) == 52);

    struct S2 s2 = get2();
    assert(s2.arr[0] == 0x123);
    assert(MASK(s2.arr[1], 56) == 0);

    s2 = get4();
    assert(s2.arr[0] == 0xabcbdbeb5362251ull);
    assert(MASK(s2.arr[1], 56) == 0);

    s2 = get6();
    assert(s2.arr[0] == 0xaaaeeefff333222ull);
    assert(MASK(s2.arr[1], 56) == 0);

    s2 = get8();
    assert(s2.arr[0] == 0x0261371428417418ull);
    assert(MASK(s2.arr[1], 56) == 0x123);
    return EXIT_SUCCESS;
}
