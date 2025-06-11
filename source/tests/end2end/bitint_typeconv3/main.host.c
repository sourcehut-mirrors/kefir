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
    assert(get1() == -3);
    assert(get1a() == 3);
    assert(get2() == 10);
    assert(get3() == -4096);
    assert(get3a() == 4096);
    assert(get4() == 2048);
    assert(get5() == -0xcafeb);
    assert(get5a() == 0xcafeb);
    assert(get6() == 0xff1ffu);
    assert(get7() == -64327324284627417ll);
    assert(get7a() == 64327324284627417ll);
    assert(get8() == 0xfecefeefecull);

    struct S2 s2 = get9();
    assert(s2.arr[0] == (unsigned long) -1028332837181762172ll);
    assert((s2.arr[1] & ((1ull << 47) - 1)) == (((unsigned long) -1) & ((1ull << 47) - 1)));

    s2 = get9a();
    assert(s2.arr[0] == (unsigned long) 1028332837181762172ll);
    assert((s2.arr[1] & ((1ull << 47) - 1)) == 0);

    s2 = get10();
    assert(s2.arr[0] == (unsigned long) 0x376324623ebdef36ull);
    assert((s2.arr[1] & ((1ull << 47) - 1)) == 0);

    struct S6 s6 = get11();
    assert(s6.arr[0] == (unsigned long) -1028332837181762172ll);
    assert(s6.arr[1] == (unsigned long) -1);
    assert(s6.arr[2] == (unsigned long) -1);
    assert(s6.arr[3] == (unsigned long) -1);
    assert(s6.arr[4] == (unsigned long) -1);
    assert((s6.arr[5] & ((1ull << 30) - 1)) == (((unsigned long) -1) & ((1ull << 30) - 1)));

    s6 = get11a();
    assert(s6.arr[0] == (unsigned long) 1028332837181762172ll);
    assert(s6.arr[1] == (unsigned long) 0);
    assert(s6.arr[2] == (unsigned long) 0);
    assert(s6.arr[3] == (unsigned long) 0);
    assert(s6.arr[4] == (unsigned long) 0);
    assert((s6.arr[5] & ((1ull << 30) - 1)) == 0);

    s6 = get12();
    assert(s6.arr[0] == (unsigned long) 0x376324623ebdef36ull);
    assert(s6.arr[1] == (unsigned long) 0);
    assert(s6.arr[2] == (unsigned long) 0);
    assert(s6.arr[3] == (unsigned long) 0);
    assert(s6.arr[4] == (unsigned long) 0);
    assert((s6.arr[5] & ((1ull << 30) - 1)) == 0);
    return EXIT_SUCCESS;
}
