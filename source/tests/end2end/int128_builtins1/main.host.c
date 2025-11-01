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
    struct i128 res;
    assert(!int128_add_overflow((struct i128){{0, 0}}, (struct i128){{0, 0}}, &res));
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    assert(!int128_add_overflow((struct i128){{1234, 5678}}, (struct i128){{9876, 5643}}, &res));
    assert(res.arr[0] == 1234 + 9876);
    assert(res.arr[1] == 5678 + 5643);

    assert(!int128_add_overflow((struct i128){{~0ull, ~0ull}}, (struct i128){{1, 0}}, &res));
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    assert(int128_add_overflow((struct i128){{~0ull, (1ull << 63) - 1}}, (struct i128){{1, 0}}, &res));
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 1ull << 63);

    assert(!int128_add_overflow((struct i128){{0, 1ull << 63}}, (struct i128){{1, 0}}, &res));
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 1ull << 63);

    assert(int128_add_overflow((struct i128){{0, 1ull << 63}}, (struct i128){{~0ull, ~0ull}}, &res));
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == (1ull << 63) - 1);

    assert(!uint128_add_overflow((struct i128){{0, 0}}, (struct i128){{0, 0}}, &res));
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    assert(!uint128_add_overflow((struct i128){{1234, 5678}}, (struct i128){{9876, 5643}}, &res));
    assert(res.arr[0] == 1234 + 9876);
    assert(res.arr[1] == 5678 + 5643);

    assert(uint128_add_overflow((struct i128){{~0ull, ~0ull}}, (struct i128){{1, 0}}, &res));
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    assert(!uint128_add_overflow((struct i128){{~0ull, (1ull << 63) - 1}}, (struct i128){{1, 0}}, &res));
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 1ull << 63);

    assert(!uint128_add_overflow((struct i128){{0, 1ull << 63}}, (struct i128){{1, 0}}, &res));
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 1ull << 63);

    assert(uint128_add_overflow((struct i128){{0, 1ull << 63}}, (struct i128){{~0ull, ~0ull}}, &res));
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == (1ull << 63) - 1);

    assert(!int128_sub_overflow((struct i128){{0, 0}}, (struct i128){{0, 0}}, &res));
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    assert(!int128_sub_overflow((struct i128){{1234, 5678}}, (struct i128){{1, 1235}}, &res));
    assert(res.arr[0] == 1234 - 1);
    assert(res.arr[1] == 5678 - 1235);

    assert(!int128_sub_overflow((struct i128){{0, 0}}, (struct i128){{1, 0}}, &res));
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == ~0ull);

    assert(!int128_sub_overflow((struct i128){{0, 0}}, (struct i128){{~0ull, ~0ull}}, &res));
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    assert(int128_sub_overflow((struct i128){{~0ull, (1ull << 63) - 1}}, (struct i128){{~0ull, ~0ull}}, &res));
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 1ull << 63);

    assert(int128_sub_overflow((struct i128){{0, 1ull << 63}}, (struct i128){{1, 0}}, &res));
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == (1ull << 63) - 1);

    assert(!uint128_sub_overflow((struct i128){{0, 0}}, (struct i128){{0, 0}}, &res));
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    assert(!uint128_sub_overflow((struct i128){{1234, 5678}}, (struct i128){{1, 1235}}, &res));
    assert(res.arr[0] == 1234 - 1);
    assert(res.arr[1] == 5678 - 1235);

    assert(uint128_sub_overflow((struct i128){{0, 0}}, (struct i128){{1, 0}}, &res));
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == ~0ull);

    assert(uint128_sub_overflow((struct i128){{0, 0}}, (struct i128){{~0ull, ~0ull}}, &res));
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    assert(uint128_sub_overflow((struct i128){{~0ull, (1ull << 63) - 1}}, (struct i128){{~0ull, ~0ull}}, &res));
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 1ull << 63);

    assert(!uint128_sub_overflow((struct i128){{0, 1ull << 63}}, (struct i128){{1, 0}}, &res));
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == (1ull << 63) - 1);

    assert(!int128_mul_overflow((struct i128){{0, 0}}, (struct i128){{0, 0}}, &res));
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    assert(!int128_mul_overflow((struct i128){{1000, 2000}}, (struct i128){{0, 0}}, &res));
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    assert(!int128_mul_overflow((struct i128){{0, 0}}, (struct i128){{1000, 2000}}, &res));
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    assert(!int128_mul_overflow((struct i128){{2, 0}}, (struct i128){{1000, 2000}}, &res));
    assert(res.arr[0] == 2000);
    assert(res.arr[1] == 4000);

    assert(!int128_mul_overflow((struct i128){{~0ull, ~0ull}}, (struct i128){{~0ull, ~0ull}}, &res));
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    assert(int128_mul_overflow((struct i128){{~0ull, (1ull << 63) - 1}}, (struct i128){{2, 0}}, &res));
    assert(res.arr[0] == ~0ull - 1);
    assert(res.arr[1] == ~0ull);

    assert(!uint128_mul_overflow((struct i128){{0, 0}}, (struct i128){{0, 0}}, &res));
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    assert(!uint128_mul_overflow((struct i128){{1000, 2000}}, (struct i128){{0, 0}}, &res));
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    assert(!uint128_mul_overflow((struct i128){{0, 0}}, (struct i128){{1000, 2000}}, &res));
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    assert(!uint128_mul_overflow((struct i128){{2, 0}}, (struct i128){{1000, 2000}}, &res));
    assert(res.arr[0] == 2000);
    assert(res.arr[1] == 4000);

    assert(uint128_mul_overflow((struct i128){{~0ull, ~0ull}}, (struct i128){{~0ull, ~0ull}}, &res));
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    assert(!uint128_mul_overflow((struct i128){{~0ull, (1ull << 63) - 1}}, (struct i128){{2, 0}}, &res));
    assert(res.arr[0] == ~0ull - 1);
    assert(res.arr[1] == ~0ull);
    return EXIT_SUCCESS;
}
