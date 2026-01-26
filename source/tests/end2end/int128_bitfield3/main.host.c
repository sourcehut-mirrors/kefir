/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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
#include <math.h>
#include "./definitions.h"

int main(void) {
    struct i128 res;

    res = get_a();
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = get_b();
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = get_c();
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = get_d();
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    set_c((struct i128) {{-18371882, -861}});

    res = get_a();
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = get_b();
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = get_c();
    assert(res.arr[0] == (unsigned long) -18371882);
    assert(res.arr[1] == (unsigned long) -861);

    res = get_d();
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    set_d((struct i128) {{-0x7261, 0}});

    res = get_a();
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = get_b();
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = get_c();
    assert(res.arr[0] == (unsigned long) -18371882);
    assert(res.arr[1] == (unsigned long) -861);

    res = get_d();
    assert(res.arr[0] == (unsigned long) -0x7261);
    assert(res.arr[1] == (unsigned long) -1);

    set_b((struct i128) {{0xbebabdbeb4bdull, 0x81724}});

    res = get_a();
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = get_b();
    assert(res.arr[0] == 0xbebabdbeb4bdull);
    assert(res.arr[1] == 0);

    res = get_c();
    assert(res.arr[0] == (unsigned long) -18371882);
    assert(res.arr[1] == (unsigned long) -861);

    res = get_d();
    assert(res.arr[0] == (unsigned long) -0x7261);
    assert(res.arr[1] == (unsigned long) -1);

    set_a((struct i128) {{~0ull, ~3ull}});

    res = get_a();
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == ~3ull);

    res = get_b();
    assert(res.arr[0] == 0xbebabdbeb4bdull);
    assert(res.arr[1] == 0);

    res = get_c();
    assert(res.arr[0] == (unsigned long) -18371882);
    assert(res.arr[1] == (unsigned long) -861);

    res = get_d();
    assert(res.arr[0] == (unsigned long) -0x7261);
    assert(res.arr[1] == (unsigned long) -1);

    set_c((struct i128) {{7921039, 998}});

    res = get_a();
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == ~3ull);

    res = get_b();
    assert(res.arr[0] == 0xbebabdbeb4bdull);
    assert(res.arr[1] == 0);

    res = get_c();
    assert(res.arr[0] == (unsigned long) 7921039);
    assert(res.arr[1] == (unsigned long) 998);

    res = get_d();
    assert(res.arr[0] == (unsigned long) -0x7261);
    assert(res.arr[1] == (unsigned long) -1);

    set_a((struct i128) {{9189, 9981}});

    res = get_a();
    assert(res.arr[0] == 9189);
    assert(res.arr[1] == 9981);

    res = get_b();
    assert(res.arr[0] == 0xbebabdbeb4bdull);
    assert(res.arr[1] == 0);

    res = get_c();
    assert(res.arr[0] == (unsigned long) 7921039);
    assert(res.arr[1] == (unsigned long) 998);

    res = get_d();
    assert(res.arr[0] == (unsigned long) -0x7261);
    assert(res.arr[1] == (unsigned long) -1);
    return EXIT_SUCCESS;
}
