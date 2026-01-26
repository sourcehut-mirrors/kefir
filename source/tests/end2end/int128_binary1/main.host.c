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

    // __int128 i128_add(__int128, __int128);
    // unsigned __int128 u128_add(unsigned __int128, unsigned __int128);
    // __int128 i128_sub(__int128, __int128);
    // unsigned __int128 u128_sub(unsigned __int128, unsigned __int128);
    // __int128 i128_and(__int128, __int128);
    // unsigned __int128 u128_and(unsigned __int128, unsigned __int128);
    // __int128 i128_or(__int128, __int128);
    // unsigned __int128 u128_or(unsigned __int128, unsigned __int128);
    // __int128 i128_xor(__int128, __int128);
    // unsigned __int128 u128_xor(unsigned __int128, unsigned __int128);
    struct i128 res = i128_add((struct i128) {{0, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_add((struct i128) {{1, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i128_add((struct i128) {{0, 0}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i128_add((struct i128) {{~0ull, 0}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 1);

    res = i128_add((struct i128) {{~0ull, 0}}, (struct i128) {{1, (1ull << 63) - 1}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 1ull << 63);

    res = i128_add((struct i128) {{~0ull, 0}}, (struct i128) {{0, ~0ull}});
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == ~0ull);

    res = i128_add((struct i128) {{1, 0}}, (struct i128) {{~0ull, ~0ull}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_add((struct i128) {{0, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_add((struct i128) {{1, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u128_add((struct i128) {{0, 0}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u128_add((struct i128) {{~0ull, 0}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 1);

    res = u128_add((struct i128) {{~0ull, 0}}, (struct i128) {{1, (1ull << 63) - 1}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 1ull << 63);

    res = u128_add((struct i128) {{~0ull, 0}}, (struct i128) {{0, ~0ull}});
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == ~0ull);

    res = u128_add((struct i128) {{1, 0}}, (struct i128) {{~0ull, ~0ull}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_sub((struct i128) {{0, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_sub((struct i128) {{1, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i128_sub((struct i128) {{0, 1}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == 0);

    res = i128_sub((struct i128) {{~0ull, 1}}, (struct i128) {{~0ull, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 1);

    res = i128_sub((struct i128) {{~0ull, ~0ull}}, (struct i128) {{~0ull, 1}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == (~0ull) - 1);

    res = i128_sub((struct i128) {{0ull, ~0ull}}, (struct i128) {{~0ull, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == (~0ull) - 1);

    res = i128_sub((struct i128) {{0, 0}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == ~0ull);

    res = i128_sub((struct i128) {{0, 0}}, (struct i128) {{0, 1}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == ~0ull);

    res = u128_sub((struct i128) {{0, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_sub((struct i128) {{1, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u128_sub((struct i128) {{0, 1}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == 0);

    res = u128_sub((struct i128) {{~0ull, 1}}, (struct i128) {{~0ull, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 1);

    res = u128_sub((struct i128) {{~0ull, ~0ull}}, (struct i128) {{~0ull, 1}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == (~0ull) - 1);

    res = u128_sub((struct i128) {{0ull, ~0ull}}, (struct i128) {{~0ull, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == (~0ull) - 1);

    res = u128_sub((struct i128) {{0, 0}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == ~0ull);

    res = u128_sub((struct i128) {{0, 0}}, (struct i128) {{0, 1}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == ~0ull);

    res = i128_and((struct i128) {{0, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_and((struct i128) {{1, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_and((struct i128) {{1, 0}}, (struct i128) {{0, 1}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_and((struct i128) {{0xcafebab0, 0xbadc0ffe}}, (struct i128) {{0xc0debad, 0xc0c01111}});
    assert(res.arr[0] == (0xcafebab0 & 0xc0debad));
    assert(res.arr[1] == (0xbadc0ffe & 0xc0c01111));

    res = i128_and((struct i128) {{0xcafebab0, ~0ull}}, (struct i128) {{~0ull, 0xc0c01111}});
    assert(res.arr[0] == 0xcafebab0);
    assert(res.arr[1] == 0xc0c01111);

    res = u128_and((struct i128) {{0, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_and((struct i128) {{1, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_and((struct i128) {{1, 0}}, (struct i128) {{0, 1}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_and((struct i128) {{0xcafebab0, 0xbadc0ffe}}, (struct i128) {{0xc0debad, 0xc0c01111}});
    assert(res.arr[0] == (0xcafebab0 & 0xc0debad));
    assert(res.arr[1] == (0xbadc0ffe & 0xc0c01111));

    res = u128_and((struct i128) {{0xcafebab0, ~0ull}}, (struct i128) {{~0ull, 0xc0c01111}});
    assert(res.arr[0] == 0xcafebab0);
    assert(res.arr[1] == 0xc0c01111);

    res = i128_or((struct i128) {{0, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_or((struct i128) {{1, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i128_or((struct i128) {{1, 0}}, (struct i128) {{0, 1}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 1);

    res = i128_or((struct i128) {{0xcafebab0, 0xbadc0ffe}}, (struct i128) {{0xc0debad, 0xc0c01111}});
    assert(res.arr[0] == (0xcafebab0 | 0xc0debad));
    assert(res.arr[1] == (0xbadc0ffe | 0xc0c01111));

    res = i128_or((struct i128) {{0xcafebab0, ~0ull}}, (struct i128) {{~0ull, 0xc0c01111}});
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == ~0ull);

    res = i128_or((struct i128) {{0xcafebab0, 0}}, (struct i128) {{0, 0xc0c01111}});
    assert(res.arr[0] == 0xcafebab0);
    assert(res.arr[1] == 0xc0c01111);

    res = u128_or((struct i128) {{0, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_or((struct i128) {{1, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u128_or((struct i128) {{1, 0}}, (struct i128) {{0, 1}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 1);

    res = u128_or((struct i128) {{0xcafebab0, 0xbadc0ffe}}, (struct i128) {{0xc0debad, 0xc0c01111}});
    assert(res.arr[0] == (0xcafebab0 | 0xc0debad));
    assert(res.arr[1] == (0xbadc0ffe | 0xc0c01111));

    res = u128_or((struct i128) {{0xcafebab0, ~0ull}}, (struct i128) {{~0ull, 0xc0c01111}});
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == ~0ull);

    res = u128_or((struct i128) {{0xcafebab0, 0}}, (struct i128) {{0, 0xc0c01111}});
    assert(res.arr[0] == 0xcafebab0);
    assert(res.arr[1] == 0xc0c01111);

    res = i128_xor((struct i128) {{0, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_xor((struct i128) {{1, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i128_xor((struct i128) {{1, 0}}, (struct i128) {{0, 1}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 1);

    res = i128_xor((struct i128) {{0xcafebab0, 0xbadc0ffe}}, (struct i128) {{0xc0debad, 0xc0c01111}});
    assert(res.arr[0] == (0xcafebab0 ^ 0xc0debad));
    assert(res.arr[1] == (0xbadc0ffe ^ 0xc0c01111));

    res = i128_xor((struct i128) {{0xcafebab0, ~0ull}}, (struct i128) {{~0ull, 0xc0c01111}});
    assert(res.arr[0] == ~0xcafebab0ull);
    assert(res.arr[1] == ~0xc0c01111ull);

    res = i128_xor((struct i128) {{0xcafebab0, 0}}, (struct i128) {{0, 0xc0c01111}});
    assert(res.arr[0] == 0xcafebab0);
    assert(res.arr[1] == 0xc0c01111);

    res = u128_xor((struct i128) {{0, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_xor((struct i128) {{1, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u128_xor((struct i128) {{1, 0}}, (struct i128) {{0, 1}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 1);

    res = u128_xor((struct i128) {{0xcafebab0, 0xbadc0ffe}}, (struct i128) {{0xc0debad, 0xc0c01111}});
    assert(res.arr[0] == (0xcafebab0 ^ 0xc0debad));
    assert(res.arr[1] == (0xbadc0ffe ^ 0xc0c01111));

    res = u128_xor((struct i128) {{0xcafebab0, ~0ull}}, (struct i128) {{~0ull, 0xc0c01111}});
    assert(res.arr[0] == ~0xcafebab0ull);
    assert(res.arr[1] == ~0xc0c01111ull);

    res = u128_xor((struct i128) {{0xcafebab0, 0}}, (struct i128) {{0, 0xc0c01111}});
    assert(res.arr[0] == 0xcafebab0);
    assert(res.arr[1] == 0xc0c01111);

    res = i128_mul((struct i128) {{0, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_mul((struct i128) {{1, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_mul((struct i128) {{0, 0}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_mul((struct i128) {{1, 0}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i128_mul((struct i128) {{1, 100}}, (struct i128) {{2, 0}});
    assert(res.arr[0] == 2);
    assert(res.arr[1] == 200);

    res = i128_mul((struct i128) {{3, 0}}, (struct i128) {{5, 200}});
    assert(res.arr[0] == 15);
    assert(res.arr[1] == 600);

    res = i128_mul((struct i128) {{3, 1}}, (struct i128) {{5, 200}});
    assert(res.arr[0] == 15);
    assert(res.arr[1] == 605);

    res = i128_mul((struct i128) {{-1, -1}}, (struct i128) {{1, 2}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -3);

    res = i128_mul((struct i128) {{-1, -1}}, (struct i128) {{-1, -1}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i128_mul((struct i128) {{-1, -1}}, (struct i128) {{-200, -1}});
    assert(res.arr[0] == 200);
    assert(res.arr[1] == 0);

    res = u128_mul((struct i128) {{0, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_mul((struct i128) {{1, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_mul((struct i128) {{0, 0}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_mul((struct i128) {{1, 0}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u128_mul((struct i128) {{1, 100}}, (struct i128) {{2, 0}});
    assert(res.arr[0] == 2);
    assert(res.arr[1] == 200);

    res = u128_mul((struct i128) {{3, 0}}, (struct i128) {{5, 200}});
    assert(res.arr[0] == 15);
    assert(res.arr[1] == 600);

    res = u128_mul((struct i128) {{3, 1}}, (struct i128) {{5, 200}});
    assert(res.arr[0] == 15);
    assert(res.arr[1] == 605);

    res = u128_mul((struct i128) {{-1, -1}}, (struct i128) {{1, 2}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -3);

    res = u128_mul((struct i128) {{-1, -1}}, (struct i128) {{-1, -1}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u128_mul((struct i128) {{-1, -1}}, (struct i128) {{-200, -1}});
    assert(res.arr[0] == 200);
    assert(res.arr[1] == 0);

    res = i128_div((struct i128) {{0, 0}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_div((struct i128) {{1, 0}}, (struct i128) {{-1, -1}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_div((struct i128) {{1000, 0}}, (struct i128) {{10, 0}});
    assert(res.arr[0] == 100);
    assert(res.arr[1] == 0);

    res = i128_div((struct i128) {{1000, 0}}, (struct i128) {{-10, -1}});
    assert(res.arr[0] == (unsigned long) -100);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_div((struct i128) {{(~0ull) << 1, 1}}, (struct i128) {{2, 0}});
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == 0);

    res = i128_div((struct i128) {{~0ull, 0}}, (struct i128) {{~0ull, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i128_div((struct i128) {{-1, -1}}, (struct i128) {{4, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_div((struct i128) {{1, -1}}, (struct i128) {{-1, -1}});
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == 0);

    res = i128_div((struct i128) {{~0ull, 0}}, (struct i128) {{-1, -1}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_div((struct i128) {{-1, (1ull << 63) - 1}}, (struct i128) {{-1, -1}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 1ull << 63);

    res = u128_div((struct i128) {{0, 0}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_div((struct i128) {{1, 0}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u128_div((struct i128) {{1, 0}}, (struct i128) {{-1, -1}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_div((struct i128) {{-1, -1}}, (struct i128) {{-1, -1}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u128_div((struct i128) {{-1, -1}}, (struct i128) {{4, 0}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (1ull << 62) - 1);

    res = u128_div((struct i128) {{-1, (1ull << 63) - 1}}, (struct i128) {{-1, -1}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_mod((struct i128) {{0, 0}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_mod((struct i128) {{10, 0}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_mod((struct i128) {{10, 0}}, (struct i128) {{4, 0}});
    assert(res.arr[0] == 2);
    assert(res.arr[1] == 0);

    res = i128_mod((struct i128) {{10, 0}}, (struct i128) {{-4, -1}});
    assert(res.arr[0] == 2);
    assert(res.arr[1] == 0);

    res = i128_mod((struct i128) {{10, 0}}, (struct i128) {{-4, -1}});
    assert(res.arr[0] == 2);
    assert(res.arr[1] == 0);

    res = i128_mod((struct i128) {{-10, -1}}, (struct i128) {{-4, -1}});
    assert(res.arr[0] == (unsigned long) -2);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_mod((struct i128) {{-10, -1}}, (struct i128) {{4, 0}});
    assert(res.arr[0] == (unsigned long) -2);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_mod((struct i128) {{3, 2}}, (struct i128) {{~0ull, 0}});
    assert(res.arr[0] == 5);
    assert(res.arr[1] == 0);

    res = u128_mod((struct i128) {{0, 0}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_mod((struct i128) {{10, 0}}, (struct i128) {{1, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_mod((struct i128) {{1, 0}}, (struct i128) {{10, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u128_mod((struct i128) {{0, 0}}, (struct i128) {{-1, -1}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_mod((struct i128) {{-10, -1}}, (struct i128) {{-4, -1}});
    assert(res.arr[0] == ~0ull - 9);
    assert(res.arr[1] == ~0ull);

    res = i128_shl((struct i128) {{0, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_shl((struct i128) {{0, 0}}, (struct i128) {{30, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_shl((struct i128) {{0xc0ffee, 0x0bad}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0xc0ffee);
    assert(res.arr[1] == 0x0bad);

    res = i128_shl((struct i128) {{0xc0ffee, 0x0bad}}, (struct i128) {{6, 0}});
    assert(res.arr[0] == 0xc0ffee << 6);
    assert(res.arr[1] == 0x0bad << 6);

    res = i128_shl((struct i128) {{0xc0ffee, 0x0bad}}, (struct i128) {{63, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == ((1ull << 63) | (0xc0ffee >> 1)));

    res = i128_shl((struct i128) {{0xc0ffee, 0x0bad}}, (struct i128) {{64, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0xc0ffee);

    res = i128_shl((struct i128) {{0xc0ffee, 0x0bad}}, (struct i128) {{72, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == (0xc0ffeeull << 8));

    res = i128_shl((struct i128) {{0xc0ffee, 0x0bad}}, (struct i128) {{126, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 1ull << 63);

    res = i128_shl((struct i128) {{0xc0ffee, 0x0bad}}, (struct i128) {{127, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_shl((struct i128) {{0xc0ffef, 0x0bad}}, (struct i128) {{127, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 1ull << 63);

    res = u128_shl((struct i128) {{0, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_shl((struct i128) {{0, 0}}, (struct i128) {{30, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_shl((struct i128) {{0xc0ffee, 0x0bad}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0xc0ffee);
    assert(res.arr[1] == 0x0bad);

    res = u128_shl((struct i128) {{0xc0ffee, 0x0bad}}, (struct i128) {{6, 0}});
    assert(res.arr[0] == 0xc0ffee << 6);
    assert(res.arr[1] == 0x0bad << 6);

    res = u128_shl((struct i128) {{0xc0ffee, 0x0bad}}, (struct i128) {{63, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == ((1ull << 63) | (0xc0ffee >> 1)));

    res = u128_shl((struct i128) {{0xc0ffee, 0x0bad}}, (struct i128) {{64, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0xc0ffee);

    res = u128_shl((struct i128) {{0xc0ffee, 0x0bad}}, (struct i128) {{72, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == (0xc0ffeeull << 8));

    res = u128_shl((struct i128) {{0xc0ffee, 0x0bad}}, (struct i128) {{126, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 1ull << 63);

    res = u128_shl((struct i128) {{0xc0ffee, 0x0bad}}, (struct i128) {{127, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_shl((struct i128) {{0xc0ffef, 0x0bad}}, (struct i128) {{127, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 1ull << 63);

    res = i128_shr((struct i128) {{0, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_shr((struct i128) {{0, 0}}, (struct i128) {{10, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_shr((struct i128) {{0xc0ffe, 0xbad0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0x0c0ffe);
    assert(res.arr[1] == 0xbad0);

    res = i128_shr((struct i128) {{0xc0ffe, 0xbad0}}, (struct i128) {{4, 0}});
    assert(res.arr[0] == 0x0c0ff);
    assert(res.arr[1] == 0xbad);

    res = i128_shr((struct i128) {{0xc0ffe, -0xbad0}}, (struct i128) {{4, 0}});
    assert(res.arr[0] == 0x0c0ff);
    assert(res.arr[1] == (unsigned long) -0xbad);

    res = i128_shr((struct i128) {{0xc0ffe, 0xbad0}}, (struct i128) {{60, 0}});
    assert(res.arr[0] == 0xbad00);
    assert(res.arr[1] == 0);

    res = i128_shr((struct i128) {{0xc0ffe, -0xbad0}}, (struct i128) {{60, 0}});
    assert(res.arr[0] == (unsigned long) -0xbad00);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_shr((struct i128) {{0xc0ffe, -0xbad0}}, (struct i128) {{64, 0}});
    assert(res.arr[0] == (unsigned long) -0xbad0);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_shr((struct i128) {{0xc0ffe, 0xbad0}}, (struct i128) {{64, 0}});
    assert(res.arr[0] == 0xbad0);
    assert(res.arr[1] == 0);

    res = i128_shr((struct i128) {{0xc0ffe, 0xbad0}}, (struct i128) {{72, 0}});
    assert(res.arr[0] == 0xba);
    assert(res.arr[1] == 0);

    res = i128_shr((struct i128) {{0xc0ffe, 0xbad0}}, (struct i128) {{127, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_shr((struct i128) {{-0xc0ffe, 0xbad0}}, (struct i128) {{127, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_shr((struct i128) {{0xc0ffe, -0xbad0}}, (struct i128) {{127, 0}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_shr((struct i128) {{0, 0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_shr((struct i128) {{0, 0}}, (struct i128) {{40, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_shr((struct i128) {{0xc0ffee, 0x0badd0}}, (struct i128) {{0, 0}});
    assert(res.arr[0] == 0xc0ffee);
    assert(res.arr[1] == 0x0badd0);

    res = u128_shr((struct i128) {{0xc0ffee, 0x0badd0}}, (struct i128) {{4, 0}});
    assert(res.arr[0] == 0xc0ffe);
    assert(res.arr[1] == 0x0badd);

    res = u128_shr((struct i128) {{0xc0ffee, 0x0badd0}}, (struct i128) {{8, 0}});
    assert(res.arr[0] == (0xc0ff | (0xdull << 60)));
    assert(res.arr[1] == 0x0bad);

    res = u128_shr((struct i128) {{0xc0ffee, -0x0badd0}}, (struct i128) {{8, 0}});
    assert(res.arr[0] == (0xc0ff | (0x3ull << 60)));
    assert(res.arr[1] == ((unsigned long) -0x0badd0) >> 8);

    res = u128_shr((struct i128) {{0xc0ffee, 0x0badd0}}, (struct i128) {{60, 0}});
    assert(res.arr[0] == 0x0badd00);
    assert(res.arr[1] == 0);

    res = u128_shr((struct i128) {{0xc0ffee, -0x0badd0}}, (struct i128) {{60, 0}});
    assert(res.arr[0] == (unsigned long) -0x0badd00);
    assert(res.arr[1] == 0xf);

    res = u128_shr((struct i128) {{0xc0ffee, 0x0badd0}}, (struct i128) {{64, 0}});
    assert(res.arr[0] == 0x0badd0);
    assert(res.arr[1] == 0);

    res = u128_shr((struct i128) {{0xc0ffee, -0x0badd0}}, (struct i128) {{64, 0}});
    assert(res.arr[0] == (unsigned long) -0x0badd0);
    assert(res.arr[1] == 0);

    res = u128_shr((struct i128) {{0xc0ffee, 0x0badd0}}, (struct i128) {{72, 0}});
    assert(res.arr[0] == 0x0bad);
    assert(res.arr[1] == 0);

    res = u128_shr((struct i128) {{0xc0ffee, -0x0badd0}}, (struct i128) {{72, 0}});
    assert(res.arr[0] == ((unsigned long) -0x0badd0) >> 8);
    assert(res.arr[1] == 0);

    res = u128_shr((struct i128) {{0xc0ffee, 0x0badd0}}, (struct i128) {{127, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_shr((struct i128) {{0xc0ffee, -0x0badd0}}, (struct i128) {{127, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    assert(i128_eq((struct i128) {{0, 0}}, (struct i128) {{0, 0}}));
    assert(i128_eq((struct i128) {{0xc00e8d8f, 0x88248ffe}}, (struct i128) {{0xc00e8d8f, 0x88248ffe}}));
    assert(!i128_eq((struct i128) {{1, 0}}, (struct i128) {{0, 0}}));
    assert(!i128_eq((struct i128) {{0, 0}}, (struct i128) {{1, 0}}));
    assert(i128_eq((struct i128) {{-1, 0}}, (struct i128) {{-1, 0}}));
    assert(i128_eq((struct i128) {{-1, -1}}, (struct i128) {{-1, -1}}));
    assert(!i128_eq((struct i128) {{-1, -1}}, (struct i128) {{-2, -1}}));
    assert(i128_eq((struct i128) {{0, -1}}, (struct i128) {{0, -1}}));
    assert(!i128_eq((struct i128) {{1, -1}}, (struct i128) {{0, -1}}));

    assert(u128_eq((struct i128) {{0, 0}}, (struct i128) {{0, 0}}));
    assert(u128_eq((struct i128) {{0xc00e8d8f, 0x88248ffe}}, (struct i128) {{0xc00e8d8f, 0x88248ffe}}));
    assert(!u128_eq((struct i128) {{1, 0}}, (struct i128) {{0, 0}}));
    assert(!u128_eq((struct i128) {{0, 0}}, (struct i128) {{1, 0}}));
    assert(u128_eq((struct i128) {{-1, 0}}, (struct i128) {{-1, 0}}));
    assert(u128_eq((struct i128) {{-1, -1}}, (struct i128) {{-1, -1}}));
    assert(!u128_eq((struct i128) {{-1, -1}}, (struct i128) {{-2, -1}}));
    assert(u128_eq((struct i128) {{0, -1}}, (struct i128) {{0, -1}}));
    assert(!u128_eq((struct i128) {{1, -1}}, (struct i128) {{0, -1}}));

    assert(!i128_neq((struct i128) {{0, 0}}, (struct i128) {{0, 0}}));
    assert(!i128_neq((struct i128) {{0xc00e8d8f, 0x88248ffe}}, (struct i128) {{0xc00e8d8f, 0x88248ffe}}));
    assert(i128_neq((struct i128) {{1, 0}}, (struct i128) {{0, 0}}));
    assert(i128_neq((struct i128) {{0, 0}}, (struct i128) {{1, 0}}));
    assert(!i128_neq((struct i128) {{-1, 0}}, (struct i128) {{-1, 0}}));
    assert(!i128_neq((struct i128) {{-1, -1}}, (struct i128) {{-1, -1}}));
    assert(i128_neq((struct i128) {{-1, -1}}, (struct i128) {{-2, -1}}));
    assert(!i128_neq((struct i128) {{0, -1}}, (struct i128) {{0, -1}}));
    assert(i128_neq((struct i128) {{1, -1}}, (struct i128) {{0, -1}}));

    assert(!u128_neq((struct i128) {{0, 0}}, (struct i128) {{0, 0}}));
    assert(!u128_neq((struct i128) {{0xc00e8d8f, 0x88248ffe}}, (struct i128) {{0xc00e8d8f, 0x88248ffe}}));
    assert(u128_neq((struct i128) {{1, 0}}, (struct i128) {{0, 0}}));
    assert(u128_neq((struct i128) {{0, 0}}, (struct i128) {{1, 0}}));
    assert(!u128_neq((struct i128) {{-1, 0}}, (struct i128) {{-1, 0}}));
    assert(!u128_neq((struct i128) {{-1, -1}}, (struct i128) {{-1, -1}}));
    assert(u128_neq((struct i128) {{-1, -1}}, (struct i128) {{-2, -1}}));
    assert(!u128_neq((struct i128) {{0, -1}}, (struct i128) {{0, -1}}));
    assert(u128_neq((struct i128) {{1, -1}}, (struct i128) {{0, -1}}));

    assert(!i128_greater((struct i128) {{0, 0}}, (struct i128) {{0, 0}}));
    assert(i128_greater((struct i128) {{1, 0}}, (struct i128) {{0, 0}}));
    assert(i128_greater((struct i128) {{-1, 0}}, (struct i128) {{0, 0}}));
    assert(!i128_greater((struct i128) {{-1, -1}}, (struct i128) {{0, 0}}));
    assert(i128_greater((struct i128) {{-1, -1}}, (struct i128) {{-2, -1}}));
    assert(i128_greater((struct i128) {{0, 1}}, (struct i128) {{~0ull, 0}}));
    assert(!i128_greater((struct i128) {{0, 1}}, (struct i128) {{~0ull, 1}}));
    assert(!i128_greater((struct i128) {{~0ull, 1}}, (struct i128) {{~0ull, 1}}));
    assert(i128_greater((struct i128) {{~0ull, 2}}, (struct i128) {{~0ull, 1}}));
    assert(!i128_greater((struct i128) {{0, -1}}, (struct i128) {{0, -1}}));
    assert(i128_greater((struct i128) {{1, -1}}, (struct i128) {{0, -1}}));

    assert(!i128_less((struct i128) {{0, 0}}, (struct i128) {{0, 0}}));
    assert(!i128_less((struct i128) {{1, 0}}, (struct i128) {{0, 0}}));
    assert(i128_less((struct i128) {{0, 0}}, (struct i128) {{1, 0}}));
    assert(!i128_less((struct i128) {{-1, 0}}, (struct i128) {{0, 0}}));
    assert(i128_less((struct i128) {{0, 0}}, (struct i128) {{-1, 0}}));
    assert(i128_less((struct i128) {{-1, -1}}, (struct i128) {{0, 0}}));
    assert(!i128_less((struct i128) {{0, 0}}, (struct i128) {{-1, -1}}));
    assert(!i128_less((struct i128) {{-1, -1}}, (struct i128) {{-2, -1}}));
    assert(i128_less((struct i128) {{-2, -1}}, (struct i128) {{-1, -1}}));
    assert(!i128_less((struct i128) {{0, 1}}, (struct i128) {{~0ull, 0}}));
    assert(i128_less((struct i128) {{~0ull, 0}}, (struct i128) {{0, 1}}));
    assert(i128_less((struct i128) {{0, 1}}, (struct i128) {{~0ull, 1}}));
    assert(!i128_less((struct i128) {{~0ull, 1}}, (struct i128) {{~0ull, 1}}));
    assert(!i128_less((struct i128) {{~0ull, 2}}, (struct i128) {{~0ull, 1}}));
    assert(i128_less((struct i128) {{~0ull, 1}}, (struct i128) {{~0ull, 2}}));
    assert(!i128_less((struct i128) {{0, -1}}, (struct i128) {{0, -1}}));
    assert(!i128_less((struct i128) {{1, -1}}, (struct i128) {{0, -1}}));
    assert(i128_less((struct i128) {{0, -1}}, (struct i128) {{1, -1}}));

    assert(!i128_above((struct i128) {{0, 0}}, (struct i128) {{0, 0}}));
    assert(i128_above((struct i128) {{1, 0}}, (struct i128) {{0, 0}}));
    assert(i128_above((struct i128) {{-1, 0}}, (struct i128) {{0, 0}}));
    assert(i128_above((struct i128) {{-1, -1}}, (struct i128) {{0, 0}}));
    assert(!i128_above((struct i128) {{0, 0}}, (struct i128) {{1, 0}}));
    assert(!i128_above((struct i128) {{0, 0}}, (struct i128) {{-1, 0}}));
    assert(!i128_above((struct i128) {{0, 0}}, (struct i128) {{-1, -1}}));
    assert(i128_above((struct i128) {{1, 1}}, (struct i128) {{~0ull, 0}}));
    assert(i128_above((struct i128) {{1, -1}}, (struct i128) {{~0ull, 1}}));
    assert(!i128_above((struct i128) {{0, -1}}, (struct i128) {{1, -1}}));

    assert(!i128_below((struct i128) {{0, 0}}, (struct i128) {{0, 0}}));
    assert(!i128_below((struct i128) {{1, 0}}, (struct i128) {{0, 0}}));
    assert(!i128_below((struct i128) {{-1, 0}}, (struct i128) {{0, 0}}));
    assert(!i128_below((struct i128) {{-1, -1}}, (struct i128) {{0, 0}}));
    assert(i128_below((struct i128) {{0, 0}}, (struct i128) {{1, 0}}));
    assert(i128_below((struct i128) {{0, 0}}, (struct i128) {{-1, 0}}));
    assert(i128_below((struct i128) {{0, 0}}, (struct i128) {{-1, -1}}));
    assert(!i128_below((struct i128) {{1, 1}}, (struct i128) {{~0ull, 0}}));
    assert(i128_below((struct i128) {{~0ull, 0}}, (struct i128) {{1, 1}}));
    assert(!i128_below((struct i128) {{1, -1}}, (struct i128) {{~0ull, 1}}));
    assert(i128_below((struct i128) {{~0ull, 1}}, (struct i128) {{1, -1}}));
    assert(i128_below((struct i128) {{0, -1}}, (struct i128) {{1, -1}}));
    assert(!i128_below((struct i128) {{1, -1}}, (struct i128) {{0, -1}}));

    assert(i128_greater_eq((struct i128) {{0, 0}}, (struct i128) {{0, 0}}));
    assert(i128_greater_eq((struct i128) {{1, 0}}, (struct i128) {{0, 0}}));
    assert(i128_greater_eq((struct i128) {{-1, 0}}, (struct i128) {{0, 0}}));
    assert(!i128_greater_eq((struct i128) {{-1, -1}}, (struct i128) {{0, 0}}));
    assert(i128_greater_eq((struct i128) {{-1, -1}}, (struct i128) {{-2, -1}}));
    assert(i128_greater_eq((struct i128) {{0, 1}}, (struct i128) {{~0ull, 0}}));
    assert(!i128_greater_eq((struct i128) {{0, 1}}, (struct i128) {{~0ull, 1}}));
    assert(i128_greater_eq((struct i128) {{~0ull, 1}}, (struct i128) {{~0ull, 1}}));
    assert(i128_greater_eq((struct i128) {{~0ull, 2}}, (struct i128) {{~0ull, 1}}));
    assert(i128_greater_eq((struct i128) {{0, -1}}, (struct i128) {{0, -1}}));
    assert(i128_greater_eq((struct i128) {{1, -1}}, (struct i128) {{0, -1}}));

    assert(i128_less_eq((struct i128) {{0, 0}}, (struct i128) {{0, 0}}));
    assert(!i128_less_eq((struct i128) {{1, 0}}, (struct i128) {{0, 0}}));
    assert(i128_less_eq((struct i128) {{0, 0}}, (struct i128) {{1, 0}}));
    assert(!i128_less_eq((struct i128) {{-1, 0}}, (struct i128) {{0, 0}}));
    assert(i128_less_eq((struct i128) {{0, 0}}, (struct i128) {{-1, 0}}));
    assert(i128_less_eq((struct i128) {{-1, -1}}, (struct i128) {{0, 0}}));
    assert(!i128_less_eq((struct i128) {{0, 0}}, (struct i128) {{-1, -1}}));
    assert(!i128_less_eq((struct i128) {{-1, -1}}, (struct i128) {{-2, -1}}));
    assert(i128_less_eq((struct i128) {{-2, -1}}, (struct i128) {{-1, -1}}));
    assert(!i128_less_eq((struct i128) {{0, 1}}, (struct i128) {{~0ull, 0}}));
    assert(i128_less_eq((struct i128) {{~0ull, 0}}, (struct i128) {{0, 1}}));
    assert(i128_less_eq((struct i128) {{0, 1}}, (struct i128) {{~0ull, 1}}));
    assert(i128_less_eq((struct i128) {{~0ull, 1}}, (struct i128) {{~0ull, 1}}));
    assert(!i128_less_eq((struct i128) {{~0ull, 2}}, (struct i128) {{~0ull, 1}}));
    assert(i128_less_eq((struct i128) {{~0ull, 1}}, (struct i128) {{~0ull, 2}}));
    assert(i128_less_eq((struct i128) {{0, -1}}, (struct i128) {{0, -1}}));
    assert(!i128_less_eq((struct i128) {{1, -1}}, (struct i128) {{0, -1}}));
    assert(i128_less_eq((struct i128) {{0, -1}}, (struct i128) {{1, -1}}));

    assert(i128_above_eq((struct i128) {{0, 0}}, (struct i128) {{0, 0}}));
    assert(i128_above_eq((struct i128) {{1, 0}}, (struct i128) {{0, 0}}));
    assert(i128_above_eq((struct i128) {{-1, 0}}, (struct i128) {{0, 0}}));
    assert(i128_above_eq((struct i128) {{-1, -1}}, (struct i128) {{0, 0}}));
    assert(!i128_above_eq((struct i128) {{0, 0}}, (struct i128) {{1, 0}}));
    assert(!i128_above_eq((struct i128) {{0, 0}}, (struct i128) {{-1, 0}}));
    assert(!i128_above_eq((struct i128) {{0, 0}}, (struct i128) {{-1, -1}}));
    assert(i128_above_eq((struct i128) {{1, 1}}, (struct i128) {{~0ull, 0}}));
    assert(i128_above_eq((struct i128) {{1, -1}}, (struct i128) {{~0ull, 1}}));
    assert(!i128_above_eq((struct i128) {{0, -1}}, (struct i128) {{1, -1}}));
    assert(i128_above_eq((struct i128) {{1, -1}}, (struct i128) {{1, -1}}));

    assert(i128_below_eq((struct i128) {{0, 0}}, (struct i128) {{0, 0}}));
    assert(!i128_below_eq((struct i128) {{1, 0}}, (struct i128) {{0, 0}}));
    assert(!i128_below_eq((struct i128) {{-1, 0}}, (struct i128) {{0, 0}}));
    assert(!i128_below_eq((struct i128) {{-1, -1}}, (struct i128) {{0, 0}}));
    assert(i128_below_eq((struct i128) {{0, 0}}, (struct i128) {{1, 0}}));
    assert(i128_below_eq((struct i128) {{0, 0}}, (struct i128) {{-1, 0}}));
    assert(i128_below_eq((struct i128) {{0, 0}}, (struct i128) {{-1, -1}}));
    assert(!i128_below_eq((struct i128) {{1, 1}}, (struct i128) {{~0ull, 0}}));
    assert(i128_below_eq((struct i128) {{~0ull, 0}}, (struct i128) {{1, 1}}));
    assert(!i128_below_eq((struct i128) {{1, -1}}, (struct i128) {{~0ull, 1}}));
    assert(i128_below_eq((struct i128) {{~0ull, 1}}, (struct i128) {{1, -1}}));
    assert(i128_below_eq((struct i128) {{0, -1}}, (struct i128) {{1, -1}}));
    assert(!i128_below_eq((struct i128) {{1, -1}}, (struct i128) {{0, -1}}));
    assert(i128_below_eq((struct i128) {{1, -1}}, (struct i128) {{1, -1}}));
    return EXIT_SUCCESS;
}
