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
    struct i128 res = i128_neg((struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_neg((struct i128) {{1, 0}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_neg((struct i128) {{2, 0}});
    assert(res.arr[0] == (unsigned long) -2);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_neg((struct i128) {{0, 1}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_neg((struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) 0);

    res = i128_neg((struct i128) {{-10, -1}});
    assert(res.arr[0] == (unsigned long) 10);
    assert(res.arr[1] == (unsigned long) 0);

    res = i128_neg((struct i128) {{0, -1}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 1);

    res = i128_neg((struct i128) {{-1, 0}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_neg((struct i128) {{1, -1}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) 0);

    res = u128_neg((struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_neg((struct i128) {{1, 0}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_neg((struct i128) {{2, 0}});
    assert(res.arr[0] == (unsigned long) -2);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_neg((struct i128) {{0, 1}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_neg((struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) 0);

    res = u128_neg((struct i128) {{-10, -1}});
    assert(res.arr[0] == (unsigned long) 10);
    assert(res.arr[1] == (unsigned long) 0);

    res = u128_neg((struct i128) {{0, -1}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 1);

    res = u128_neg((struct i128) {{-1, 0}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_neg((struct i128) {{1, -1}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) 0);

    res = i128_not((struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_not((struct i128) {{1, 0}});
    assert(res.arr[0] == (unsigned long) -2);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_not((struct i128) {{1024, 0}});
    assert(res.arr[0] == (unsigned long) -1025);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_not((struct i128) {{-2, 0}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_not((struct i128) {{-1, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_not((struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);

    res = i128_not((struct i128) {{0, -1}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) 0);

    res = i128_not((struct i128) {{1, -1}});
    assert(res.arr[0] == (unsigned long) -2);
    assert(res.arr[1] == (unsigned long) 0);

    res = i128_not((struct i128) {{1, -10}});
    assert(res.arr[0] == (unsigned long) -2);
    assert(res.arr[1] == (unsigned long) 9);

    res = u128_not((struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_not((struct i128) {{1, 0}});
    assert(res.arr[0] == (unsigned long) -2);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_not((struct i128) {{1024, 0}});
    assert(res.arr[0] == (unsigned long) -1025);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_not((struct i128) {{-2, 0}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_not((struct i128) {{-1, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_not((struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);

    res = u128_not((struct i128) {{0, -1}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) 0);

    res = u128_not((struct i128) {{1, -1}});
    assert(res.arr[0] == (unsigned long) -2);
    assert(res.arr[1] == (unsigned long) 0);

    res = u128_not((struct i128) {{1, -10}});
    assert(res.arr[0] == (unsigned long) -2);
    assert(res.arr[1] == (unsigned long) 9);

    assert(i128_bnot((struct i128) {{0, 0}}));
    assert(!i128_bnot((struct i128) {{1, 0}}));
    assert(!i128_bnot((struct i128) {{1025, 0}}));
    assert(!i128_bnot((struct i128) {{-1, 0}}));
    assert(!i128_bnot((struct i128) {{0, 1}}));
    assert(!i128_bnot((struct i128) {{0, 0xffff}}));
    assert(!i128_bnot((struct i128) {{0, -1}}));
    assert(!i128_bnot((struct i128) {{1, -1}}));
    assert(!i128_bnot((struct i128) {{-1, -1}}));

    assert(u128_bnot((struct i128) {{0, 0}}));
    assert(!u128_bnot((struct i128) {{1, 0}}));
    assert(!u128_bnot((struct i128) {{1025, 0}}));
    assert(!u128_bnot((struct i128) {{-1, 0}}));
    assert(!u128_bnot((struct i128) {{0, 1}}));
    assert(!u128_bnot((struct i128) {{0, 0xffff}}));
    assert(!u128_bnot((struct i128) {{0, -1}}));
    assert(!u128_bnot((struct i128) {{1, -1}}));
    assert(!u128_bnot((struct i128) {{-1, -1}}));

    struct i128 arg = {{0, 0}};
    res = i128_preinc(&arg);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);
    assert(arg.arr[0] == 1);
    assert(arg.arr[1] == 0);

    arg = (struct i128) {{~0ull, 0}};
    res = i128_preinc(&arg);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 1);
    assert(arg.arr[0] == 0);
    assert(arg.arr[1] == 1);

    arg = (struct i128) {{~0ull - 1, ~0ull}};
    res = i128_preinc(&arg);
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == ~0ull);
    assert(arg.arr[0] == ~0ull);
    assert(arg.arr[1] == ~0ull);

    arg = (struct i128) {{~0ull, ~0ull}};
    res = i128_preinc(&arg);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);
    assert(arg.arr[0] == 0);
    assert(arg.arr[1] == 0);

    arg = (struct i128) {{0, 0}};
    res = i128_predec(&arg);
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == ~0ull);
    assert(arg.arr[0] == ~0ull);
    assert(arg.arr[1] == ~0ull);

    arg = (struct i128) {{~0ull, 0}};
    res = i128_predec(&arg);
    assert(res.arr[0] == ~0ull - 1);
    assert(res.arr[1] == 0);
    assert(arg.arr[0] == ~0ull - 1);
    assert(arg.arr[1] == 0);

    arg = (struct i128) {{~0ull - 1, ~0ull}};
    res = i128_predec(&arg);
    assert(res.arr[0] == ~0ull - 2);
    assert(res.arr[1] == ~0ull);
    assert(arg.arr[0] == ~0ull - 2);
    assert(arg.arr[1] == ~0ull);

    arg = (struct i128) {{~0ull, ~0ull}};
    res = i128_predec(&arg);
    assert(res.arr[0] == ~0ull - 1);
    assert(res.arr[1] == ~0ull);
    assert(arg.arr[0] == ~0ull - 1);
    assert(arg.arr[1] == ~0ull);

    arg = (struct i128) {{0, 0}};
    res = i128_postinc(&arg);
    assert(arg.arr[0] == 1);
    assert(arg.arr[1] == 0);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    arg = (struct i128) {{~0ull, 0}};
    res = i128_postinc(&arg);
    assert(arg.arr[0] == 0);
    assert(arg.arr[1] == 1);
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == 0);

    arg = (struct i128) {{~0ull - 1, ~0ull}};
    res = i128_postinc(&arg);
    assert(arg.arr[0] == ~0ull);
    assert(arg.arr[1] == ~0ull);
    assert(res.arr[0] == ~0ull - 1);
    assert(res.arr[1] == ~0ull);

    arg = (struct i128) {{~0ull, ~0ull}};
    res = i128_postinc(&arg);
    assert(arg.arr[0] == 0);
    assert(arg.arr[1] == 0);
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == ~0ull);

    arg = (struct i128) {{0, 0}};
    res = i128_postdec(&arg);
    assert(arg.arr[0] == ~0ull);
    assert(arg.arr[1] == ~0ull);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    arg = (struct i128) {{~0ull, 0}};
    res = i128_postdec(&arg);
    assert(arg.arr[0] == ~0ull - 1);
    assert(arg.arr[1] == 0);
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == 0);

    arg = (struct i128) {{~0ull - 1, ~0ull}};
    res = i128_postdec(&arg);
    assert(arg.arr[0] == ~0ull - 2);
    assert(arg.arr[1] == ~0ull);
    assert(res.arr[0] == ~0ull - 1);
    assert(res.arr[1] == ~0ull);

    arg = (struct i128) {{~0ull, ~0ull}};
    res = i128_postdec(&arg);
    assert(arg.arr[0] == ~0ull - 1);
    assert(arg.arr[1] == ~0ull);
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == ~0ull);
    return EXIT_SUCCESS;
}
