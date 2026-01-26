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
    _Atomic struct i128 arg = (struct i128) {{1234, -5678}};
    struct i128 res = load_i128(&arg);
    assert(res.arr[0] == 1234);
    assert(res.arr[1] == (unsigned long) -5678);

    arg = (struct i128) {{9876, 14524}};
    res = load_u128(&arg);
    assert(res.arr[0] == 9876);
    assert(res.arr[1] == 14524);

    store_i128(&arg, (struct i128){{-1539281, 482918}});
    struct i128 val = arg;
    assert(val.arr[0] == (unsigned long) -1539281);
    assert(val.arr[1] == 482918);
    store_u128(&arg, (struct i128){{93813911, -381931}});
    val = arg;
    assert(val.arr[0] == 93813911);
    assert(val.arr[1] == (unsigned long) -381931);

    val = add_i128(&arg, (struct i128){{~0ull, 0}});
    assert(val.arr[0] == 93813910);
    assert(val.arr[1] == (unsigned long) -381930);
    val = arg;
    assert(val.arr[0] == 93813910);
    assert(val.arr[1] == (unsigned long) -381930);

    val = add_u128(&arg, (struct i128){{~0ull, 0}});
    assert(val.arr[0] == 93813909);
    assert(val.arr[1] == (unsigned long) -381929);
    val = arg;
    assert(val.arr[0] == 93813909);
    assert(val.arr[1] == (unsigned long) -381929);
    return EXIT_SUCCESS;
}
