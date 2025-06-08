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
    assert(get1() == 30);
    assert(get2() == 1024);
    assert(get3() == 0xfffffe);
    assert(get4() == 0xcafebabe123ull);

    const struct B1 b1 = get5();
    assert(b1.arr[0] == 0x4baad32109876543ull);
    assert(b1.arr[1] == 0xfeca123ull);

    const struct B2 b2 = get6();
    assert(b2.arr[0] == 0x54321abcdefedbcaull);
    assert(b2.arr[1] == 0x8712345678909876ull);
    assert(b2.arr[2] == 0xabcdef09ull);

    const struct B3 b3 = get7();
    assert(b3.arr[0] == 0x9999888877776666ull);
    assert(b3.arr[1] == 0xcccdddeeefff0000ull);
    assert(b3.arr[2] == 0x6677889900aaabbbull);
    assert(b3.arr[3] == 0xddeeff1122334455ull);
    assert(b3.arr[4] == 0xaabbccull);
    ;
    return EXIT_SUCCESS;
}
