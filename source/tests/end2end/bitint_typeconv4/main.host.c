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
    assert(get0() == -1777182);
    assert(get1() == -1777182);
    assert(get2() == 0x1234543);
    assert(get3() == 116391380);
    assert(get4() == -5965737456325395ll);
    assert(get5() == 2882400237ll);
    assert(get6() == 2305692889512463564ull);

    struct S4 s4 = get7();
    assert(s4.arr[0] == (unsigned long) -152627362517262ll);
    assert(s4.arr[1] == (unsigned long) -1ll);
    assert(s4.arr[2] == (unsigned long) -1ll);
    assert((s4.arr[3] & ((1ull << 8) - 1)) == (1ull << 8) - 1);

    s4 = get8();
    assert(s4.arr[0] == (unsigned long) 152627362517262ll);
    assert(s4.arr[1] == (unsigned long) 0);
    assert(s4.arr[2] == (unsigned long) 0);
    assert((s4.arr[3] & ((1ull << 8) - 1)) == 0);

    s4 = get9();
    assert(s4.arr[0] == (unsigned long) 0xc0ffe4e3e231ull);
    assert(s4.arr[1] == (unsigned long) 0);
    assert(s4.arr[2] == (unsigned long) 0);
    assert((s4.arr[3] & ((1ull << 8) - 1)) == 0);

    struct S7 s7 = get10();
    assert(s7.arr[0] == (unsigned long) -8372472467146471642ll);
    assert(s7.arr[1] == (unsigned long) -1);
    assert(s7.arr[2] == (unsigned long) -1);
    assert(s7.arr[3] == (unsigned long) -1);
    assert(s7.arr[4] == (unsigned long) -1);
    assert(s7.arr[5] == (unsigned long) -1);
    assert((s7.arr[6] & ((1ull << 46) - 1)) == ((1ull << 46) - 1));

    s7 = get11();
    assert(s7.arr[0] == (unsigned long) 0x5728738346471642ull);
    assert(s7.arr[1] == (unsigned long) 0x7138724827287538ull);
    assert(s7.arr[2] == (unsigned long) 0x837247246);
    assert(s7.arr[3] == (unsigned long) 0);
    assert(s7.arr[4] == (unsigned long) 0);
    assert(s7.arr[5] == (unsigned long) 0);
    assert((s7.arr[6] & ((1ull << 46) - 1)) == 0);

    s7 = get12();
    assert(s7.arr[0] == (unsigned long) 0x637261ebdbcebde7ull);
    assert(s7.arr[1] == (unsigned long) 0xcbbdbcbac6462774ull);
    assert(s7.arr[2] == (unsigned long) 0xbdbcbbebdbdbcbebull);
    assert(s7.arr[3] == (unsigned long) 0xbabcbeull);
    assert(s7.arr[4] == (unsigned long) 0);
    assert(s7.arr[5] == (unsigned long) 0);
    assert((s7.arr[6] & ((1ull << 46) - 1)) == 0);
    return EXIT_SUCCESS;
}
