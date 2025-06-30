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
    assert(MASK(get1(), 26) == 26);
    assert(MASK(get2(), 14) == 1024);
    assert(MASK(get3(), 29) == 0xffffe8);
    assert(MASK(get4(), 60) == 0x56437262ull);

    struct S2 s2 = get5();
    assert(s2.arr[0] == 0x5322362515edefull);
    assert(MASK(s2.arr[1], 56) == 0);
    return EXIT_SUCCESS;
}
