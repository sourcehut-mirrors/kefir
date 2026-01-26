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

struct i128 value = {{0, 0}};

int main(void) {
    struct i128 zero = get();
    assert(zero.arr[0] == 0);
    assert(zero.arr[1] == 0);

    set((struct i128){{0xcafebabeull, 0x0badc0ffeull}});
    assert(value.arr[0] == 0xcafebabeull);
    assert(value.arr[1] == 0x0badc0ffeull);

    value.arr[0] = ~value.arr[0];
    value.arr[1] = ~value.arr[1];

    struct i128 val = get();
    assert(val.arr[0] == ~0xcafebabeull);
    assert(val.arr[1] == ~0x0badc0ffeull);

    val = test((struct i128){{0x183918dd, 0x92829189fec}});
    assert(val.arr[0] == 0x183918dd);
    assert(val.arr[1] == 0x92829189fec);
    return EXIT_SUCCESS;
}
