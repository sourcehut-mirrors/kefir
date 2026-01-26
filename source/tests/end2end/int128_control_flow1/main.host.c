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

static int ref_fact(int x) {
    if (x < 1) {
        return 1;
    } else {
        return x * ref_fact(x - 1);
    }
}

int main(void) {
    assert(test1((struct i128) {{0, 0}}, 100, 200) == 200);
    assert(test1((struct i128) {{1, 0}}, 100, 200) == 100);
    assert(test1((struct i128) {{-1, 0}}, 100, 200) == 100);
    assert(test1((struct i128) {{0, 1}}, 100, 200) == 100);
    assert(test1((struct i128) {{0, -1}}, 100, 200) == 100);
    assert(test1((struct i128) {{-1, -1}}, 100, 200) == 100);

    assert(test2((struct i128) {{0, 0}}, 100, 200) == 200);
    assert(test2((struct i128) {{1, 0}}, 100, 200) == 100);
    assert(test2((struct i128) {{-1, 0}}, 100, 200) == 100);
    assert(test2((struct i128) {{0, 1}}, 100, 200) == 100);
    assert(test2((struct i128) {{0, -1}}, 100, 200) == 100);
    assert(test2((struct i128) {{-1, -1}}, 100, 200) == 100);

    for (int i = 0; i < 10; i++) {
        struct i128 res = fact1((struct i128) {{i, 0}});
        assert(res.arr[0] == (unsigned long) ref_fact(i));
        assert(res.arr[1] == 0);

        res = fact2((struct i128) {{i, 0}});
        assert(res.arr[0] == (unsigned long) ref_fact(i));
        assert(res.arr[1] == 0);

        res = fact3((struct i128) {{i, 0}});
        assert(res.arr[0] == (unsigned long) ref_fact(i));
        assert(res.arr[1] == 0);

        res = fact4((struct i128) {{i, 0}});
        assert(res.arr[0] == (unsigned long) ref_fact(i));
        assert(res.arr[1] == 0);

        res = fact5((struct i128) {{i, 0}});
        assert(res.arr[0] == (unsigned long) ref_fact(i));
        assert(res.arr[1] == 0);

        res = fact6((struct i128) {{i, 0}});
        assert(res.arr[0] == (unsigned long) ref_fact(i));
        assert(res.arr[1] == 0);
    }

    for (int i = -1500; i < 1500; i++) {
        struct i128 arg = {{i, i >= 0 ? 0 : -1}};
        struct i128 res = select1(arg);
        struct i128 res2 = select2(arg);
        if (i == 0) {
            assert(res.arr[0] == (unsigned long) -100);
            assert(res.arr[1] == (unsigned long) -1);
            assert(res2.arr[0] == (unsigned long) -100);
            assert(res2.arr[1] == (unsigned long) -1);
        } else if (i == -1) {
            assert(res.arr[0] == (unsigned long) -200);
            assert(res.arr[1] == (unsigned long) -1);
            assert(res2.arr[0] == (unsigned long) -200);
            assert(res2.arr[1] == (unsigned long) -1);
        } else if (i == 1) {
            assert(res.arr[0] == 200);
            assert(res.arr[1] == 0);
            assert(res2.arr[0] == 200);
            assert(res2.arr[1] == 0);
        } else if (i >= 100 && i <= 1000) {
            assert(res.arr[0] == 0);
            assert(res.arr[1] == 0);
            assert(res2.arr[0] == 0);
            assert(res2.arr[1] == 0);
        } else if (i >= -1000 && i <= -100) {
            assert(res.arr[0] == 10);
            assert(res.arr[1] == 0);
            assert(res2.arr[0] == 10);
            assert(res2.arr[1] == 0);
        } else {
            assert(res.arr[0] == (unsigned long) -1);
            assert(res.arr[1] == (unsigned long) -1);
            assert(res2.arr[0] == (unsigned long) -1);
            assert(res2.arr[1] == (unsigned long) -1);
        }
    }
    return EXIT_SUCCESS;
}
