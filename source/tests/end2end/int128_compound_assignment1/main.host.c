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
    struct i128 arg, res;

    arg = (struct i128) {{0, 0}};
    res = i128_add(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);
    assert(arg.arr[0] == 0);
    assert(arg.arr[1] == 0);

    arg = (struct i128) {{0, 0}};
    res = i128_add(&arg, (struct i128) {{0xcafe, 0xc0ffee}});
    assert(res.arr[0] == 0xcafe);
    assert(res.arr[1] == 0xc0ffee);
    assert(arg.arr[0] == 0xcafe);
    assert(arg.arr[1] == 0xc0ffee);

    arg = (struct i128) {{-1, -1}};
    res = i128_add(&arg, (struct i128) {{0xcafe, 0xc0ffee}});
    assert(res.arr[0] == 0xcafe - 1);
    assert(res.arr[1] == 0xc0ffee);
    assert(arg.arr[0] == 0xcafe - 1);
    assert(arg.arr[1] == 0xc0ffee);

    arg = (struct i128) {{0, -1}};
    res = i128_add(&arg, (struct i128) {{0xcafe, 0xc0ffee}});
    assert(res.arr[0] == 0xcafe);
    assert(res.arr[1] == 0xc0ffee - 1);
    assert(arg.arr[0] == 0xcafe);
    assert(arg.arr[1] == 0xc0ffee - 1);

    arg = (struct i128) {{~0ull, 0}};
    res = i128_add(&arg, (struct i128) {{0xcafe, 0xc0ffee}});
    assert(res.arr[0] == 0xcafe - 1);
    assert(res.arr[1] == 0xc0ffee + 1);
    assert(arg.arr[0] == 0xcafe - 1);
    assert(arg.arr[1] == 0xc0ffee + 1);

    arg = (struct i128) {{0, 0}};
    res = u128_add(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);
    assert(arg.arr[0] == 0);
    assert(arg.arr[1] == 0);

    arg = (struct i128) {{0, 0}};
    res = u128_add(&arg, (struct i128) {{0xcafe, 0xc0ffee}});
    assert(res.arr[0] == 0xcafe);
    assert(res.arr[1] == 0xc0ffee);
    assert(arg.arr[0] == 0xcafe);
    assert(arg.arr[1] == 0xc0ffee);

    arg = (struct i128) {{-1, -1}};
    res = u128_add(&arg, (struct i128) {{0xcafe, 0xc0ffee}});
    assert(res.arr[0] == 0xcafe - 1);
    assert(res.arr[1] == 0xc0ffee);
    assert(arg.arr[0] == 0xcafe - 1);
    assert(arg.arr[1] == 0xc0ffee);

    arg = (struct i128) {{0, -1}};
    res = u128_add(&arg, (struct i128) {{0xcafe, 0xc0ffee}});
    assert(res.arr[0] == 0xcafe);
    assert(res.arr[1] == 0xc0ffee - 1);
    assert(arg.arr[0] == 0xcafe);
    assert(arg.arr[1] == 0xc0ffee - 1);

    arg = (struct i128) {{~0ull, 0}};
    res = u128_add(&arg, (struct i128) {{0xcafe, 0xc0ffee}});
    assert(res.arr[0] == 0xcafe - 1);
    assert(res.arr[1] == 0xc0ffee + 1);
    assert(arg.arr[0] == 0xcafe - 1);
    assert(arg.arr[1] == 0xc0ffee + 1);

    arg = (struct i128) {{0, 0}};
    res = i128_sub(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);
    assert(arg.arr[0] == 0);
    assert(arg.arr[1] == 0);

    arg = (struct i128) {{0xbad0, 0xc0de}};
    res = i128_sub(&arg, (struct i128) {{0xf3, 0x98}});
    assert(res.arr[0] == 0xbad0 - 0xf3);
    assert(res.arr[1] == 0xc0de - 0x98);
    assert(arg.arr[0] == 0xbad0 - 0xf3);
    assert(arg.arr[1] == 0xc0de - 0x98);

    arg = (struct i128) {{0xbad0, 0xc0de}};
    res = i128_sub(&arg, (struct i128) {{-0xf3, -0x98}});
    assert(res.arr[0] == 0xbad0 + 0xf3);
    assert(res.arr[1] == 0xc0de + 0x97);
    assert(arg.arr[0] == 0xbad0 + 0xf3);
    assert(arg.arr[1] == 0xc0de + 0x97);

    arg = (struct i128) {{0xbad0, 0xc0de}};
    res = i128_sub(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == 0xbad0 + 1);
    assert(res.arr[1] == 0xc0de + 0);
    assert(arg.arr[0] == 0xbad0 + 1);
    assert(arg.arr[1] == 0xc0de + 0);

    arg = (struct i128) {{0xbad0, 0xc0de}};
    res = i128_sub(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == 0xbad0 + 1);
    assert(res.arr[1] == 0xc0de + 0);
    assert(arg.arr[0] == 0xbad0 + 1);
    assert(arg.arr[1] == 0xc0de + 0);

    arg = (struct i128) {{0xbad0, 0xc0de}};
    res = i128_sub(&arg, (struct i128) {{0, -1}});
    assert(res.arr[0] == 0xbad0 + 0);
    assert(res.arr[1] == 0xc0de + 1);
    assert(arg.arr[0] == 0xbad0 + 0);
    assert(arg.arr[1] == 0xc0de + 1);

    arg = (struct i128) {{0, 0}};
    res = u128_sub(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);
    assert(arg.arr[0] == 0);
    assert(arg.arr[1] == 0);

    arg = (struct i128) {{0xbad0, 0xc0de}};
    res = u128_sub(&arg, (struct i128) {{0xf3, 0x98}});
    assert(res.arr[0] == 0xbad0 - 0xf3);
    assert(res.arr[1] == 0xc0de - 0x98);
    assert(arg.arr[0] == 0xbad0 - 0xf3);
    assert(arg.arr[1] == 0xc0de - 0x98);

    arg = (struct i128) {{0xbad0, 0xc0de}};
    res = u128_sub(&arg, (struct i128) {{-0xf3, -0x98}});
    assert(res.arr[0] == 0xbad0 + 0xf3);
    assert(res.arr[1] == 0xc0de + 0x97);
    assert(arg.arr[0] == 0xbad0 + 0xf3);
    assert(arg.arr[1] == 0xc0de + 0x97);

    arg = (struct i128) {{0xbad0, 0xc0de}};
    res = u128_sub(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == 0xbad0 + 1);
    assert(res.arr[1] == 0xc0de + 0);
    assert(arg.arr[0] == 0xbad0 + 1);
    assert(arg.arr[1] == 0xc0de + 0);

    arg = (struct i128) {{0xbad0, 0xc0de}};
    res = u128_sub(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == 0xbad0 + 1);
    assert(res.arr[1] == 0xc0de + 0);
    assert(arg.arr[0] == 0xbad0 + 1);
    assert(arg.arr[1] == 0xc0de + 0);

    arg = (struct i128) {{0xbad0, 0xc0de}};
    res = u128_sub(&arg, (struct i128) {{0, -1}});
    assert(res.arr[0] == 0xbad0 + 0);
    assert(res.arr[1] == 0xc0de + 1);
    assert(arg.arr[0] == 0xbad0 + 0);
    assert(arg.arr[1] == 0xc0de + 1);

    arg = (struct i128) {{0, 0}};
    res = i128_mul(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);
    assert(arg.arr[0] == 0);
    assert(arg.arr[1] == 0);

    arg = (struct i128) {{1234, -1}};
    res = i128_mul(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);
    assert(arg.arr[0] == 0);
    assert(arg.arr[1] == 0);

    arg = (struct i128) {{0, 0}};
    res = i128_mul(&arg, (struct i128) {{49199, 4929}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);
    assert(arg.arr[0] == 0);
    assert(arg.arr[1] == 0);

    arg = (struct i128) {{2, 0}};
    res = i128_mul(&arg, (struct i128) {{0xcafe, 0xcece}});
    assert(res.arr[0] == 0xcafe * 2);
    assert(res.arr[1] == 0xcece * 2);
    assert(arg.arr[0] == 0xcafe * 2);
    assert(arg.arr[1] == 0xcece * 2);

    arg = (struct i128) {{0x5643, 0x909ac}};
    res = i128_mul(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) -0x5643);
    assert(res.arr[1] == (unsigned long) -0x909ad);
    assert(arg.arr[0] == (unsigned long) -0x5643);
    assert(arg.arr[1] == (unsigned long) -0x909ad);

    arg = (struct i128) {{-1, -1}};
    res = i128_mul(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 1);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{-1, 0}};
    res = i128_mul(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) -1);
    assert(arg.arr[0] == (unsigned long) 1);
    assert(arg.arr[1] == (unsigned long) -1);

    arg = (struct i128) {{1, -1}};
    res = i128_mul(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) -1);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0, 0}};
    res = u128_mul(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);
    assert(arg.arr[0] == 0);
    assert(arg.arr[1] == 0);

    arg = (struct i128) {{0x55cedf, 0x12dac4}};
    res = u128_mul(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);
    assert(arg.arr[0] == 0);
    assert(arg.arr[1] == 0);

    arg = (struct i128) {{0, 0}};
    res = u128_mul(&arg, (struct i128) {{0xffcdd4, 0x901dce}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);
    assert(arg.arr[0] == 0);
    assert(arg.arr[1] == 0);

    arg = (struct i128) {{0x754, 0x2234}};
    res = u128_mul(&arg, (struct i128) {{4, 0}});
    assert(res.arr[0] == 0x754 * 4);
    assert(res.arr[1] == 0x2234 * 4);
    assert(arg.arr[0] == 0x754 * 4);
    assert(arg.arr[1] == 0x2234 * 4);

    arg = (struct i128) {{90, 0}};
    res = u128_mul(&arg, (struct i128) {{49291, 6882}});
    assert(res.arr[0] == 49291 * 90);
    assert(res.arr[1] == 6882 * 90);
    assert(arg.arr[0] == 49291 * 90);
    assert(arg.arr[1] == 6882 * 90);

    arg = (struct i128) {{-1, -1}};
    res = u128_mul(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);
    assert(arg.arr[0] == 1);
    assert(arg.arr[1] == 0);

    arg = (struct i128) {{0x5643, 0x909ac}};
    res = u128_mul(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) -0x5643);
    assert(res.arr[1] == (unsigned long) -0x909ad);
    assert(arg.arr[0] == (unsigned long) -0x5643);
    assert(arg.arr[1] == (unsigned long) -0x909ad);

    arg = (struct i128) {{-1, -1}};
    res = u128_mul(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 1);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{-1, 0}};
    res = u128_mul(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) -1);
    assert(arg.arr[0] == (unsigned long) 1);
    assert(arg.arr[1] == (unsigned long) -1);

    arg = (struct i128) {{1, -1}};
    res = u128_mul(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) -1);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0, 0}};
    res = i128_div(&arg, (struct i128) {{1, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0, 0}};
    res = i128_div(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{1, 0}};
    res = i128_div(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);
    assert(arg.arr[0] == (unsigned long) -1);
    assert(arg.arr[1] == (unsigned long) -1);

    arg = (struct i128) {{0xcafe, 0xc0de0}};
    res = i128_div(&arg, (struct i128) {{2, 0}});
    assert(res.arr[0] == (unsigned long) 0xcafe / 2);
    assert(res.arr[1] == (unsigned long) 0xc0de0 / 2);
    assert(arg.arr[0] == (unsigned long) 0xcafe / 2);
    assert(arg.arr[1] == (unsigned long) 0xc0de0 / 2);

    arg = (struct i128) {{0xcafe, 0xc0de0}};
    res = i128_div(&arg, (struct i128) {{-2, -1}});
    assert(res.arr[0] == (unsigned long) (0xcafe / -2));
    assert(res.arr[1] == (unsigned long) (0xc0de0 / -2) - 1);
    assert(arg.arr[0] == (unsigned long) (0xcafe / -2));
    assert(arg.arr[1] == (unsigned long) (0xc0de0 / -2) - 1);

    arg = (struct i128) {{-2, -1}};
    res = i128_div(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 2);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 2);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{-1, -1}};
    res = i128_div(&arg, (struct i128) {{-2, -1}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0, 0}};
    res = u128_div(&arg, (struct i128) {{1, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0, 0}};
    res = u128_div(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{1, 0}};
    res = u128_div(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{-2, -1}};
    res = u128_div(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{-1, -1}};
    res = u128_div(&arg, (struct i128) {{-2, -1}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 1);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0, 0}};
    res = i128_mod(&arg, (struct i128) {{1, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0, 0}};
    res = i128_mod(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{100, 1000}};
    res = i128_mod(&arg, (struct i128) {{1, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{100, 1000}};
    res = i128_mod(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{25, 0}};
    res = i128_mod(&arg, (struct i128) {{8, 0}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 1);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{25, 0}};
    res = i128_mod(&arg, (struct i128) {{-8, -1}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 1);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{-25, -1}};
    res = i128_mod(&arg, (struct i128) {{-8, -1}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);
    assert(arg.arr[0] == (unsigned long) -1);
    assert(arg.arr[1] == (unsigned long) -1);

    arg = (struct i128) {{-25, -1}};
    res = i128_mod(&arg, (struct i128) {{8, 0}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);
    assert(arg.arr[0] == (unsigned long) -1);
    assert(arg.arr[1] == (unsigned long) -1);

    arg = (struct i128) {{0, 0}};
    res = u128_mod(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{100, 1000}};
    res = u128_mod(&arg, (struct i128) {{1, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{100, 1000}};
    res = u128_mod(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 100);
    assert(res.arr[1] == (unsigned long) 1000);
    assert(arg.arr[0] == (unsigned long) 100);
    assert(arg.arr[1] == (unsigned long) 1000);

    arg = (struct i128) {{25, 0}};
    res = u128_mod(&arg, (struct i128) {{8, 0}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 1);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{25, 0}};
    res = u128_mod(&arg, (struct i128) {{-8, -1}});
    assert(res.arr[0] == (unsigned long) 25);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 25);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{-25, -1}};
    res = u128_mod(&arg, (struct i128) {{-8, -1}});
    assert(res.arr[0] == (unsigned long) -25);
    assert(res.arr[1] == (unsigned long) -1);
    assert(arg.arr[0] == (unsigned long) -25);
    assert(arg.arr[1] == (unsigned long) -1);

    arg = (struct i128) {{-25, -1}};
    res = u128_mod(&arg, (struct i128) {{8, 0}});
    assert(res.arr[0] == (unsigned long) 7);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 7);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0, 0}};
    res = i128_and(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0xcafe56, 0xc0f54f04}};
    res = i128_and(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0, 0}};
    res = i128_and(&arg, (struct i128) {{0xcafe56, 0xc0f54f04}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0xcafe56, 0xc0f54f04}};
    res = i128_and(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 0xcafe56);
    assert(res.arr[1] == (unsigned long) 0xc0f54f04);
    assert(arg.arr[0] == (unsigned long) 0xcafe56);
    assert(arg.arr[1] == (unsigned long) 0xc0f54f04);

    arg = (struct i128) {{-1, -1}};
    res = i128_and(&arg, (struct i128) {{0xcafe56, 0xc0f54f04}});
    assert(res.arr[0] == (unsigned long) 0xcafe56);
    assert(res.arr[1] == (unsigned long) 0xc0f54f04);
    assert(arg.arr[0] == (unsigned long) 0xcafe56);
    assert(arg.arr[1] == (unsigned long) 0xc0f54f04);

    arg = (struct i128) {{0xcafe56, 0xc0f54f04}};
    res = i128_and(&arg, (struct i128) {{0x54f43f5, 0x673542}});
    assert(res.arr[0] == (unsigned long) (0xcafe56 & 0x54f43f5));
    assert(res.arr[1] == (unsigned long) (0xc0f54f04 & 0x673542));
    assert(arg.arr[0] == (unsigned long) (0xcafe56 & 0x54f43f5));
    assert(arg.arr[1] == (unsigned long) (0xc0f54f04 & 0x673542));

    arg = (struct i128) {{0x54f43f5, 0x673542}};
    res = i128_and(&arg, (struct i128) {{0xcafe56, 0xc0f54f04}});
    assert(res.arr[0] == (unsigned long) (0xcafe56 & 0x54f43f5));
    assert(res.arr[1] == (unsigned long) (0xc0f54f04 & 0x673542));
    assert(arg.arr[0] == (unsigned long) (0xcafe56 & 0x54f43f5));
    assert(arg.arr[1] == (unsigned long) (0xc0f54f04 & 0x673542));

    arg = (struct i128) {{0, 0}};
    res = u128_and(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0xcafe56, 0xc0f54f04}};
    res = u128_and(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0, 0}};
    res = u128_and(&arg, (struct i128) {{0xcafe56, 0xc0f54f04}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0xcafe56, 0xc0f54f04}};
    res = u128_and(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) 0xcafe56);
    assert(res.arr[1] == (unsigned long) 0xc0f54f04);
    assert(arg.arr[0] == (unsigned long) 0xcafe56);
    assert(arg.arr[1] == (unsigned long) 0xc0f54f04);

    arg = (struct i128) {{-1, -1}};
    res = u128_and(&arg, (struct i128) {{0xcafe56, 0xc0f54f04}});
    assert(res.arr[0] == (unsigned long) 0xcafe56);
    assert(res.arr[1] == (unsigned long) 0xc0f54f04);
    assert(arg.arr[0] == (unsigned long) 0xcafe56);
    assert(arg.arr[1] == (unsigned long) 0xc0f54f04);

    arg = (struct i128) {{0xcafe56, 0xc0f54f04}};
    res = u128_and(&arg, (struct i128) {{0x54f43f5, 0x673542}});
    assert(res.arr[0] == (unsigned long) (0xcafe56 & 0x54f43f5));
    assert(res.arr[1] == (unsigned long) (0xc0f54f04 & 0x673542));
    assert(arg.arr[0] == (unsigned long) (0xcafe56 & 0x54f43f5));
    assert(arg.arr[1] == (unsigned long) (0xc0f54f04 & 0x673542));

    arg = (struct i128) {{0x54f43f5, 0x673542}};
    res = u128_and(&arg, (struct i128) {{0xcafe56, 0xc0f54f04}});
    assert(res.arr[0] == (unsigned long) (0xcafe56 & 0x54f43f5));
    assert(res.arr[1] == (unsigned long) (0xc0f54f04 & 0x673542));
    assert(arg.arr[0] == (unsigned long) (0xcafe56 & 0x54f43f5));
    assert(arg.arr[1] == (unsigned long) (0xc0f54f04 & 0x673542));

    arg = (struct i128) {{0, 0}};
    res = i128_or(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0xcafe56, 0xc0f54f04}};
    res = i128_or(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 0xcafe56);
    assert(res.arr[1] == (unsigned long) 0xc0f54f04);
    assert(arg.arr[0] == (unsigned long) 0xcafe56);
    assert(arg.arr[1] == (unsigned long) 0xc0f54f04);

    arg = (struct i128) {{0, 0}};
    res = i128_or(&arg, (struct i128) {{0xcafe56, 0xc0f54f04}});
    assert(res.arr[0] == (unsigned long) 0xcafe56);
    assert(res.arr[1] == (unsigned long) 0xc0f54f04);
    assert(arg.arr[0] == (unsigned long) 0xcafe56);
    assert(arg.arr[1] == (unsigned long) 0xc0f54f04);

    arg = (struct i128) {{0xcafe56, 0xc0f54f04}};
    res = i128_or(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);
    assert(arg.arr[0] == (unsigned long) -1);
    assert(arg.arr[1] == (unsigned long) -1);

    arg = (struct i128) {{-1, -1}};
    res = i128_or(&arg, (struct i128) {{0xcafe56, 0xc0f54f04}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);
    assert(arg.arr[0] == (unsigned long) -1);
    assert(arg.arr[1] == (unsigned long) -1);

    arg = (struct i128) {{0xcafe56, 0xc0f54f04}};
    res = i128_or(&arg, (struct i128) {{0x54f43f5, 0x673542}});
    assert(res.arr[0] == (unsigned long) (0xcafe56 | 0x54f43f5));
    assert(res.arr[1] == (unsigned long) (0xc0f54f04 | 0x673542));
    assert(arg.arr[0] == (unsigned long) (0xcafe56 | 0x54f43f5));
    assert(arg.arr[1] == (unsigned long) (0xc0f54f04 | 0x673542));

    arg = (struct i128) {{0x54f43f5, 0x673542}};
    res = i128_or(&arg, (struct i128) {{0xcafe56, 0xc0f54f04}});
    assert(res.arr[0] == (unsigned long) (0xcafe56 | 0x54f43f5));
    assert(res.arr[1] == (unsigned long) (0xc0f54f04 | 0x673542));
    assert(arg.arr[0] == (unsigned long) (0xcafe56 | 0x54f43f5));
    assert(arg.arr[1] == (unsigned long) (0xc0f54f04 | 0x673542));

    arg = (struct i128) {{0, 0}};
    res = u128_or(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0xcafe56, 0xc0f54f04}};
    res = u128_or(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 0xcafe56);
    assert(res.arr[1] == (unsigned long) 0xc0f54f04);
    assert(arg.arr[0] == (unsigned long) 0xcafe56);
    assert(arg.arr[1] == (unsigned long) 0xc0f54f04);

    arg = (struct i128) {{0, 0}};
    res = u128_or(&arg, (struct i128) {{0xcafe56, 0xc0f54f04}});
    assert(res.arr[0] == (unsigned long) 0xcafe56);
    assert(res.arr[1] == (unsigned long) 0xc0f54f04);
    assert(arg.arr[0] == (unsigned long) 0xcafe56);
    assert(arg.arr[1] == (unsigned long) 0xc0f54f04);

    arg = (struct i128) {{0xcafe56, 0xc0f54f04}};
    res = u128_or(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);
    assert(arg.arr[0] == (unsigned long) -1);
    assert(arg.arr[1] == (unsigned long) -1);

    arg = (struct i128) {{-1, -1}};
    res = u128_or(&arg, (struct i128) {{0xcafe56, 0xc0f54f04}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);
    assert(arg.arr[0] == (unsigned long) -1);
    assert(arg.arr[1] == (unsigned long) -1);

    arg = (struct i128) {{0xcafe56, 0xc0f54f04}};
    res = u128_or(&arg, (struct i128) {{0x54f43f5, 0x673542}});
    assert(res.arr[0] == (unsigned long) (0xcafe56 | 0x54f43f5));
    assert(res.arr[1] == (unsigned long) (0xc0f54f04 | 0x673542));
    assert(arg.arr[0] == (unsigned long) (0xcafe56 | 0x54f43f5));
    assert(arg.arr[1] == (unsigned long) (0xc0f54f04 | 0x673542));

    arg = (struct i128) {{0x54f43f5, 0x673542}};
    res = u128_or(&arg, (struct i128) {{0xcafe56, 0xc0f54f04}});
    assert(res.arr[0] == (unsigned long) (0xcafe56 | 0x54f43f5));
    assert(res.arr[1] == (unsigned long) (0xc0f54f04 | 0x673542));
    assert(arg.arr[0] == (unsigned long) (0xcafe56 | 0x54f43f5));
    assert(arg.arr[1] == (unsigned long) (0xc0f54f04 | 0x673542));

    arg = (struct i128) {{0, 0}};
    res = i128_xor(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0xcafe56, 0xc0f54f04}};
    res = i128_xor(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 0xcafe56);
    assert(res.arr[1] == (unsigned long) 0xc0f54f04);
    assert(arg.arr[0] == (unsigned long) 0xcafe56);
    assert(arg.arr[1] == (unsigned long) 0xc0f54f04);

    arg = (struct i128) {{0, 0}};
    res = i128_xor(&arg, (struct i128) {{0xcafe56, 0xc0f54f04}});
    assert(res.arr[0] == (unsigned long) 0xcafe56);
    assert(res.arr[1] == (unsigned long) 0xc0f54f04);
    assert(arg.arr[0] == (unsigned long) 0xcafe56);
    assert(arg.arr[1] == (unsigned long) 0xc0f54f04);

    arg = (struct i128) {{0xcafe56, 0xc0f54f04}};
    res = i128_xor(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == ~(unsigned long) 0xcafe56);
    assert(res.arr[1] == ~(unsigned long) 0xc0f54f04);
    assert(arg.arr[0] == ~(unsigned long) 0xcafe56);
    assert(arg.arr[1] == ~(unsigned long) 0xc0f54f04);

    arg = (struct i128) {{-1, -1}};
    res = i128_xor(&arg, (struct i128) {{0xcafe56, 0xc0f54f04}});
    assert(res.arr[0] == ~(unsigned long) 0xcafe56);
    assert(res.arr[1] == ~(unsigned long) 0xc0f54f04);
    assert(arg.arr[0] == ~(unsigned long) 0xcafe56);
    assert(arg.arr[1] == ~(unsigned long) 0xc0f54f04);

    arg = (struct i128) {{0xcafe56, 0xc0f54f04}};
    res = i128_xor(&arg, (struct i128) {{0x54f43f5, 0x673542}});
    assert(res.arr[0] == (unsigned long) (0xcafe56 ^ 0x54f43f5));
    assert(res.arr[1] == (unsigned long) (0xc0f54f04 ^ 0x673542));
    assert(arg.arr[0] == (unsigned long) (0xcafe56 ^ 0x54f43f5));
    assert(arg.arr[1] == (unsigned long) (0xc0f54f04 ^ 0x673542));

    arg = (struct i128) {{0x54f43f5, 0x673542}};
    res = i128_xor(&arg, (struct i128) {{0xcafe56, 0xc0f54f04}});
    assert(res.arr[0] == (unsigned long) (0xcafe56 ^ 0x54f43f5));
    assert(res.arr[1] == (unsigned long) (0xc0f54f04 ^ 0x673542));
    assert(arg.arr[0] == (unsigned long) (0xcafe56 ^ 0x54f43f5));
    assert(arg.arr[1] == (unsigned long) (0xc0f54f04 ^ 0x673542));

    arg = (struct i128) {{0, 0}};
    res = u128_xor(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0xcafe56, 0xc0f54f04}};
    res = u128_xor(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 0xcafe56);
    assert(res.arr[1] == (unsigned long) 0xc0f54f04);
    assert(arg.arr[0] == (unsigned long) 0xcafe56);
    assert(arg.arr[1] == (unsigned long) 0xc0f54f04);

    arg = (struct i128) {{0, 0}};
    res = u128_xor(&arg, (struct i128) {{0xcafe56, 0xc0f54f04}});
    assert(res.arr[0] == (unsigned long) 0xcafe56);
    assert(res.arr[1] == (unsigned long) 0xc0f54f04);
    assert(arg.arr[0] == (unsigned long) 0xcafe56);
    assert(arg.arr[1] == (unsigned long) 0xc0f54f04);

    arg = (struct i128) {{0xcafe56, 0xc0f54f04}};
    res = u128_xor(&arg, (struct i128) {{-1, -1}});
    assert(res.arr[0] == ~(unsigned long) 0xcafe56);
    assert(res.arr[1] == ~(unsigned long) 0xc0f54f04);
    assert(arg.arr[0] == ~(unsigned long) 0xcafe56);
    assert(arg.arr[1] == ~(unsigned long) 0xc0f54f04);

    arg = (struct i128) {{-1, -1}};
    res = u128_xor(&arg, (struct i128) {{0xcafe56, 0xc0f54f04}});
    assert(res.arr[0] == ~(unsigned long) 0xcafe56);
    assert(res.arr[1] == ~(unsigned long) 0xc0f54f04);
    assert(arg.arr[0] == ~(unsigned long) 0xcafe56);
    assert(arg.arr[1] == ~(unsigned long) 0xc0f54f04);

    arg = (struct i128) {{0xcafe56, 0xc0f54f04}};
    res = u128_xor(&arg, (struct i128) {{0x54f43f5, 0x673542}});
    assert(res.arr[0] == (unsigned long) (0xcafe56 ^ 0x54f43f5));
    assert(res.arr[1] == (unsigned long) (0xc0f54f04 ^ 0x673542));
    assert(arg.arr[0] == (unsigned long) (0xcafe56 ^ 0x54f43f5));
    assert(arg.arr[1] == (unsigned long) (0xc0f54f04 ^ 0x673542));

    arg = (struct i128) {{0x54f43f5, 0x673542}};
    res = u128_xor(&arg, (struct i128) {{0xcafe56, 0xc0f54f04}});
    assert(res.arr[0] == (unsigned long) (0xcafe56 ^ 0x54f43f5));
    assert(res.arr[1] == (unsigned long) (0xc0f54f04 ^ 0x673542));
    assert(arg.arr[0] == (unsigned long) (0xcafe56 ^ 0x54f43f5));
    assert(arg.arr[1] == (unsigned long) (0xc0f54f04 ^ 0x673542));

    arg = (struct i128) {{0, 0}};
    res = i128_shl(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0, 0}};
    res = i128_shl(&arg, (struct i128) {{20, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{1, 2}};
    res = i128_shl(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) 2);
    assert(arg.arr[0] == (unsigned long) 1);
    assert(arg.arr[1] == (unsigned long) 2);

    arg = (struct i128) {{1, 2}};
    res = i128_shl(&arg, (struct i128) {{4, 0}});
    assert(res.arr[0] == (unsigned long) 1 << 4);
    assert(res.arr[1] == (unsigned long) 2 << 4);
    assert(arg.arr[0] == (unsigned long) 1 << 4);
    assert(arg.arr[1] == (unsigned long) 2 << 4);

    arg = (struct i128) {{-1, -2}};
    res = i128_shl(&arg, (struct i128) {{4, 0}});
    assert(res.arr[0] == ((unsigned long) -1) << 4);
    assert(res.arr[1] == ((((unsigned long) -2) << 4) | 0xf));
    assert(arg.arr[0] == ((unsigned long) -1) << 4);
    assert(arg.arr[1] == ((((unsigned long) -2) << 4) | 0xf));

    arg = (struct i128) {{-1, 0}};
    res = i128_shl(&arg, (struct i128) {{64, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) -1);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) -1);

    arg = (struct i128) {{-1, 0}};
    res = i128_shl(&arg, (struct i128) {{63, 0}});
    assert(res.arr[0] == (unsigned long) 1ull << 63);
    assert(res.arr[1] == (unsigned long) (1ull << 63) - 1);
    assert(arg.arr[0] == (unsigned long) 1ull << 63);
    assert(arg.arr[1] == (unsigned long) (1ull << 63) - 1);

    arg = (struct i128) {{0, 0}};
    res = u128_shl(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0, 0}};
    res = u128_shl(&arg, (struct i128) {{20, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{1, 2}};
    res = u128_shl(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) 2);
    assert(arg.arr[0] == (unsigned long) 1);
    assert(arg.arr[1] == (unsigned long) 2);

    arg = (struct i128) {{1, 2}};
    res = u128_shl(&arg, (struct i128) {{4, 0}});
    assert(res.arr[0] == (unsigned long) 1 << 4);
    assert(res.arr[1] == (unsigned long) 2 << 4);
    assert(arg.arr[0] == (unsigned long) 1 << 4);
    assert(arg.arr[1] == (unsigned long) 2 << 4);

    arg = (struct i128) {{-1, -2}};
    res = u128_shl(&arg, (struct i128) {{4, 0}});
    assert(res.arr[0] == ((unsigned long) -1) << 4);
    assert(res.arr[1] == ((((unsigned long) -2) << 4) | 0xf));
    assert(arg.arr[0] == ((unsigned long) -1) << 4);
    assert(arg.arr[1] == ((((unsigned long) -2) << 4) | 0xf));

    arg = (struct i128) {{-1, 0}};
    res = u128_shl(&arg, (struct i128) {{64, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) -1);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) -1);

    arg = (struct i128) {{-1, 0}};
    res = u128_shl(&arg, (struct i128) {{63, 0}});
    assert(res.arr[0] == (unsigned long) 1ull << 63);
    assert(res.arr[1] == (unsigned long) (1ull << 63) - 1);
    assert(arg.arr[0] == (unsigned long) 1ull << 63);
    assert(arg.arr[1] == (unsigned long) (1ull << 63) - 1);

    arg = (struct i128) {{0, 0}};
    res = i128_shr(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0xafe, 0x0ffe0}};
    res = i128_shr(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 0xafe);
    assert(res.arr[1] == (unsigned long) 0x0ffe0);
    assert(arg.arr[0] == (unsigned long) 0xafe);
    assert(arg.arr[1] == (unsigned long) 0x0ffe0);

    arg = (struct i128) {{0xafe, 0x0ffe0}};
    res = i128_shr(&arg, (struct i128) {{4, 0}});
    assert(res.arr[0] == (unsigned long) 0xaf);
    assert(res.arr[1] == (unsigned long) 0x0ffe);
    assert(arg.arr[0] == (unsigned long) 0xaf);
    assert(arg.arr[1] == (unsigned long) 0x0ffe);

    arg = (struct i128) {{0xafe, 0x0ffe0}};
    res = i128_shr(&arg, (struct i128) {{8, 0}});
    assert(res.arr[0] == (unsigned long) (0xa | (0xeull << 60)));
    assert(res.arr[1] == (unsigned long) 0x0ff);
    assert(arg.arr[0] == (unsigned long) (0xa | (0xeull << 60)));
    assert(arg.arr[1] == (unsigned long) 0x0ff);

    arg = (struct i128) {{0xafe, -0x0ffe0}};
    res = i128_shr(&arg, (struct i128) {{8, 0}});
    assert(res.arr[0] == (unsigned long) (0xa | (0x2ull << 60)));
    assert(res.arr[1] == (unsigned long) -0x100);
    assert(arg.arr[0] == (unsigned long) (0xa | (0x2ull << 60)));
    assert(arg.arr[1] == (unsigned long) -0x100);

    arg = (struct i128) {{0, 0}};
    res = u128_shr(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 0);
    assert(res.arr[1] == (unsigned long) 0);
    assert(arg.arr[0] == (unsigned long) 0);
    assert(arg.arr[1] == (unsigned long) 0);

    arg = (struct i128) {{0xafe, 0x0ffe0}};
    res = u128_shr(&arg, (struct i128) {{0, 0}});
    assert(res.arr[0] == (unsigned long) 0xafe);
    assert(res.arr[1] == (unsigned long) 0x0ffe0);
    assert(arg.arr[0] == (unsigned long) 0xafe);
    assert(arg.arr[1] == (unsigned long) 0x0ffe0);

    arg = (struct i128) {{0xafe, 0x0ffe0}};
    res = u128_shr(&arg, (struct i128) {{4, 0}});
    assert(res.arr[0] == (unsigned long) 0xaf);
    assert(res.arr[1] == (unsigned long) 0x0ffe);
    assert(arg.arr[0] == (unsigned long) 0xaf);
    assert(arg.arr[1] == (unsigned long) 0x0ffe);

    arg = (struct i128) {{0xafe, 0x0ffe0}};
    res = u128_shr(&arg, (struct i128) {{8, 0}});
    assert(res.arr[0] == (unsigned long) (0xa | (0xeull << 60)));
    assert(res.arr[1] == (unsigned long) 0x0ff);
    assert(arg.arr[0] == (unsigned long) (0xa | (0xeull << 60)));
    assert(arg.arr[1] == (unsigned long) 0x0ff);

    arg = (struct i128) {{0xafe, -0x0ffe0}};
    res = u128_shr(&arg, (struct i128) {{8, 0}});
    assert(res.arr[0] == (unsigned long) (0xa | (0x2ull << 60)));
    assert(res.arr[1] == ((unsigned long) -0x0ffe0) >> 8);
    assert(arg.arr[0] == (unsigned long) (0xa | (0x2ull << 60)));
    assert(arg.arr[1] == ((unsigned long) -0x0ffe0) >> 8);

    return EXIT_SUCCESS;
}
