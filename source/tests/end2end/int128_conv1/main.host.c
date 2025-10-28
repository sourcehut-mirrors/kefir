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
#include <math.h>
#include "./definitions.h"

int main(void) {
    assert(!i128_to_bool((struct i128){{0, 0}}));
    assert(i128_to_bool((struct i128){{0, 1}}));
    assert(i128_to_bool((struct i128){{1, 0}}));
    assert(i128_to_bool((struct i128){{-1, -1}}));
    assert(!u128_to_bool((struct i128){{0, 0}}));
    assert(u128_to_bool((struct i128){{0, 1}}));
    assert(u128_to_bool((struct i128){{1, 0}}));
    assert(u128_to_bool((struct i128){{-1, -1}}));

    assert(i128_to_i64((struct i128){{0, 0}}) == 0);
    assert(i128_to_i64((struct i128){{1, 0}}) == 1);
    assert(i128_to_i64((struct i128){{-1, 0}}) == -1);
    assert(i128_to_i64((struct i128){{0, 0}}) == 0);
    assert(i128_to_i64((struct i128){{0, 1}}) == 0);
    assert(i128_to_i64((struct i128){{0, -1}}) == 0);
    assert(i128_to_i64((struct i128){{-1, -1}}) == -1);
    assert(i128_to_i64((struct i128){{1, -1}}) == 1);

    assert(i128_to_u64((struct i128){{0, 0}}) == 0);
    assert(i128_to_u64((struct i128){{1, 0}}) == 1);
    assert(i128_to_u64((struct i128){{-1, 0}}) == (unsigned long) -1);
    assert(i128_to_u64((struct i128){{0, 0}}) == 0);
    assert(i128_to_u64((struct i128){{0, 1}}) == 0);
    assert(i128_to_u64((struct i128){{0, -1}}) == 0);
    assert(i128_to_u64((struct i128){{-1, -1}}) == (unsigned long) -1);
    assert(i128_to_u64((struct i128){{1, -1}}) == 1);

    assert(u128_to_i64((struct i128){{0, 0}}) == 0);
    assert(u128_to_i64((struct i128){{1, 0}}) == 1);
    assert(u128_to_i64((struct i128){{-1, 0}}) == -1);
    assert(u128_to_i64((struct i128){{0, 0}}) == 0);
    assert(u128_to_i64((struct i128){{0, 1}}) == 0);
    assert(u128_to_i64((struct i128){{0, -1}}) == 0);
    assert(u128_to_i64((struct i128){{-1, -1}}) == -1);
    assert(u128_to_i64((struct i128){{1, -1}}) == 1);

    assert(u128_to_u64((struct i128){{0, 0}}) == 0);
    assert(u128_to_u64((struct i128){{1, 0}}) == 1);
    assert(u128_to_u64((struct i128){{-1, 0}}) == (unsigned long) -1);
    assert(u128_to_u64((struct i128){{0, 0}}) == 0);
    assert(u128_to_u64((struct i128){{0, 1}}) == 0);
    assert(u128_to_u64((struct i128){{0, -1}}) == 0);
    assert(u128_to_u64((struct i128){{-1, -1}}) == (unsigned long) -1);
    assert(u128_to_u64((struct i128){{1, -1}}) == 1);


// struct i128 i64_to_int128(long);
// struct i128 u64_to_int128(unsigned long);
// struct i128 i64_to_unt128(long);
// struct i128 u64_to_unt128(unsigned long);
    struct i128 res = i64_to_int128(0);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i64_to_int128(1);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i64_to_int128(0xcafebab0);
    assert(res.arr[0] == 0xcafebab0);
    assert(res.arr[1] == 0);

    res = i64_to_int128((1ull << 63) - 1);
    assert(res.arr[0] == (1ull << 63) - 1);
    assert(res.arr[1] == 0);

    res = i64_to_int128(-1);
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i64_to_int128(-10);
    assert(res.arr[0] == (unsigned long) -10);
    assert(res.arr[1] == (unsigned long) -1);

    res = u64_to_int128(0);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u64_to_int128(1);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u64_to_int128(0xcafebab0);
    assert(res.arr[0] == 0xcafebab0);
    assert(res.arr[1] == 0);

    res = u64_to_int128((1ull << 63) - 1);
    assert(res.arr[0] == (1ull << 63) - 1);
    assert(res.arr[1] == 0);

    res = u64_to_int128(-1);
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == 0);

    res = u64_to_int128(-10);
    assert(res.arr[0] == (unsigned long) -10);
    assert(res.arr[1] == 0);

    res = i64_to_unt128(0);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i64_to_unt128(1);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i64_to_unt128(0xcafebab0);
    assert(res.arr[0] == 0xcafebab0);
    assert(res.arr[1] == 0);

    res = i64_to_unt128((1ull << 63) - 1);
    assert(res.arr[0] == (1ull << 63) - 1);
    assert(res.arr[1] == 0);

    res = i64_to_unt128(-1);
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i64_to_unt128(-10);
    assert(res.arr[0] == (unsigned long) -10);
    assert(res.arr[1] == (unsigned long) -1);

    res = u64_to_unt128(0);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u64_to_unt128(1);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u64_to_unt128(0xcafebab0);
    assert(res.arr[0] == 0xcafebab0);
    assert(res.arr[1] == 0);

    res = u64_to_unt128((1ull << 63) - 1);
    assert(res.arr[0] == (1ull << 63) - 1);
    assert(res.arr[1] == 0);

    res = u64_to_unt128(-1);
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == 0);

    res = u64_to_unt128(-10);
    assert(res.arr[0] == (unsigned long) -10);
    assert(res.arr[1] == 0);
    return EXIT_SUCCESS;
}
