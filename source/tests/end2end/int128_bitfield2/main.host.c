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
    assert(res.arr[0] == 0xbad0c0full);
    assert(res.arr[1] == 0);

    res = get_b();
    assert(res.arr[0] == -0x81831bcde83131ull);
    assert(res.arr[1] == 0x3f);

    set_a((struct i128){{~1ull, ~0ull}});

    res = get_a();
    assert(res.arr[0] == (1ull << 32) - 2);
    assert(res.arr[1] == 0);

    res = get_b();
    assert(res.arr[0] == -0x81831bcde83131ull);
    assert(res.arr[1] == 0x3f);

    set_b((struct i128){{0x12345, 0xa}});

    res = get_a();
    assert(res.arr[0] == (1ull << 32) - 2);
    assert(res.arr[1] == 0);

    res = get_b();
    assert(res.arr[0] == 0x12345);
    assert(res.arr[1] == 0xa);
    return EXIT_SUCCESS;
}
