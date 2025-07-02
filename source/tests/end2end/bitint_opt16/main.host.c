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

int main(void) {
#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))
    struct S2 s2 = lshift1();
    assert(s2.arr[0] == 39093746760ull);
    assert(MASK(s2.arr[1], 56) == 0);

    s2 = lshift2();
    assert(s2.arr[0] == 78187493520ull);
    assert(MASK(s2.arr[1], 56) == 0);

    s2 = rshift1();
    assert(s2.arr[0] == 610839793ull);
    assert(MASK(s2.arr[1], 56) == 0);

    s2 = rshift2();
    assert(s2.arr[0] == 305419896ull);
    assert(MASK(s2.arr[1], 56) == 0);

    s2 = rshift3();
    assert(s2.arr[0] == (unsigned long) -610839794ll);
    assert(MASK(s2.arr[1], 56) == MASK(~0ull, 56));

    s2 = rshift4();
    assert(s2.arr[0] == (unsigned long) -305419897ull);
    assert(MASK(s2.arr[1], 56) == MASK(4503599627370495ull, 56));
    return EXIT_SUCCESS;
}
