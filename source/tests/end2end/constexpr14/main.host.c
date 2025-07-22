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
#include <assert.h>
#include "./definitions.h"

int main(void) {
#define MASK(_x, _y) (((unsigned long) (_x)) & ((1ull << (_y)) - 1))
    assert(MASK(get1(), 6) == MASK(-12, 6));
    assert(MASK(get2(), 12) == MASK(-1024, 12));
    assert(MASK(get3(), 30) == MASK(-0xcafe, 30));
    assert(MASK(get4(), 60) == MASK(-0xcafe0badll, 60));

    struct S1 s1 = get5();
    assert(s1.arr[0] == (~0xaaaabbbbddddeeeeull) + 1);
    assert(MASK(s1.arr[1], 57) == MASK((~0x123409876655ull), 57));

    struct S2 s2 = get6();
    assert(s2.arr[0] == (~0xaaaabbbbddddeeeeull) + 1);
    assert(s2.arr[1] == ~0x123409876655ull);
    assert(s2.arr[2] == ~0ull);
    assert(MASK(s2.arr[3], 58) == MASK(~0ull, 58));
    return EXIT_SUCCESS;
}
