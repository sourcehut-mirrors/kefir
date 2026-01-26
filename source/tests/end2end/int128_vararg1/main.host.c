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

    res = sum(0);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = sum(0, (struct i128){{1, 2}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = sum(1, (struct i128){{1, 2}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 2);

    res = sum(1, (struct i128){{1, 2}}, (struct i128){{3, 4}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 2);

    res = sum(2, (struct i128){{1, 2}}, (struct i128){{3, 4}},
        (struct i128){{-1, -1}});
    assert(res.arr[0] == 4);
    assert(res.arr[1] == 6);

    res = sum(3, (struct i128){{1, 2}}, (struct i128){{3, 4}},
        (struct i128){{-1, -1}});
    assert(res.arr[0] == 3);
    assert(res.arr[1] == 6);

    res = sum(4, (struct i128){{1, 2}}, (struct i128){{3, 4}},
        (struct i128){{-1, -1}}, (struct i128){{~0ull, 0}});
    assert(res.arr[0] == 2);
    assert(res.arr[1] == 7);

    res = sum(5, (struct i128){{1, 2}}, (struct i128){{3, 4}},
        (struct i128){{-1, -1}}, (struct i128){{~0ull, 0}},
        (struct i128){{1000, 2000}});
    assert(res.arr[0] == 1002);
    assert(res.arr[1] == 2007);

    res = sum(6, (struct i128){{1, 2}}, (struct i128){{3, 4}},
        (struct i128){{-1, -1}}, (struct i128){{~0ull, 0}},
        (struct i128){{1000, 2000}}, (struct i128){{0, -1}});
    assert(res.arr[0] == 1002);
    assert(res.arr[1] == 2006);

    res = sum(7, (struct i128){{1, 2}}, (struct i128){{3, 4}},
        (struct i128){{-1, -1}}, (struct i128){{~0ull, 0}},
        (struct i128){{1000, 2000}}, (struct i128){{0, -1}},
        (struct i128){{-1002, -1}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 2006);

    res = sum(7, (struct i128){{1, 2}}, (struct i128){{3, 4}},
        (struct i128){{-1, -1}}, (struct i128){{~0ull, 0}},
        (struct i128){{1000, 2000}}, (struct i128){{0, -1}},
        (struct i128){{-1003, -1}});
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == 2005);

    res = sum(8, (struct i128){{1, 2}}, (struct i128){{3, 4}},
        (struct i128){{-1, -1}}, (struct i128){{~0ull, 0}},
        (struct i128){{1000, 2000}}, (struct i128){{0, -1}},
        (struct i128){{-1003, -1}}, (struct i128){{~0ull, 0}});
    assert(res.arr[0] == ~0ull - 1);
    assert(res.arr[1] == 2006);

    res = sum2(0);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = sum2(0, (struct i128){{1, 2}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = sum2(1, (struct i128){{1, 2}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 2);

    res = sum2(1, (struct i128){{1, 2}}, (struct i128){{3, 4}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 2);

    res = sum2(2, (struct i128){{1, 2}}, (struct i128){{3, 4}},
        (struct i128){{-1, -1}});
    assert(res.arr[0] == 4);
    assert(res.arr[1] == 6);

    res = sum2(3, (struct i128){{1, 2}}, (struct i128){{3, 4}},
        (struct i128){{-1, -1}});
    assert(res.arr[0] == 3);
    assert(res.arr[1] == 6);

    res = sum2(4, (struct i128){{1, 2}}, (struct i128){{3, 4}},
        (struct i128){{-1, -1}}, (struct i128){{~0ull, 0}});
    assert(res.arr[0] == 2);
    assert(res.arr[1] == 7);

    res = sum2(5, (struct i128){{1, 2}}, (struct i128){{3, 4}},
        (struct i128){{-1, -1}}, (struct i128){{~0ull, 0}},
        (struct i128){{1000, 2000}});
    assert(res.arr[0] == 1002);
    assert(res.arr[1] == 2007);

    res = sum2(6, (struct i128){{1, 2}}, (struct i128){{3, 4}},
        (struct i128){{-1, -1}}, (struct i128){{~0ull, 0}},
        (struct i128){{1000, 2000}}, (struct i128){{0, -1}});
    assert(res.arr[0] == 1002);
    assert(res.arr[1] == 2006);

    res = sum2(7, (struct i128){{1, 2}}, (struct i128){{3, 4}},
        (struct i128){{-1, -1}}, (struct i128){{~0ull, 0}},
        (struct i128){{1000, 2000}}, (struct i128){{0, -1}},
        (struct i128){{-1002, -1}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 2006);

    res = sum2(7, (struct i128){{1, 2}}, (struct i128){{3, 4}},
        (struct i128){{-1, -1}}, (struct i128){{~0ull, 0}},
        (struct i128){{1000, 2000}}, (struct i128){{0, -1}},
        (struct i128){{-1003, -1}});
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == 2005);

    res = sum2(8, (struct i128){{1, 2}}, (struct i128){{3, 4}},
        (struct i128){{-1, -1}}, (struct i128){{~0ull, 0}},
        (struct i128){{1000, 2000}}, (struct i128){{0, -1}},
        (struct i128){{-1003, -1}}, (struct i128){{~0ull, 0}});
    assert(res.arr[0] == ~0ull - 1);
    assert(res.arr[1] == 2006);
    return EXIT_SUCCESS;
}
