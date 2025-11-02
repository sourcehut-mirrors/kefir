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

    res = int128_zero_extend64();
    assert(res.arr[0] == (unsigned long) -0xcafebabell);
    assert(res.arr[1] == 0);

    res = int128_sign_extend64();
    assert(res.arr[0] == (unsigned long) -0xcafebabell);
    assert(res.arr[1] == (unsigned long) -1);

    assert(int128_truncate_int64() == ~((1ull << 32) - 1));
    assert(int128_to_bool());
    assert(!int128_bool_not());

    res = int128_neg();
    assert(res.arr[0] == (unsigned long) -0x0bad0c0ffeell);
    assert(res.arr[1] == (unsigned long) -1);

    res = int128_not();
    assert(res.arr[0] == (unsigned long) ~0x0bad0c0ffeell);
    assert(res.arr[1] == (unsigned long) -1);

    struct i192 i192 = int128_sign_to_bitint();
    assert(i192.arr[0] == (unsigned long) -0x0bad0c0ffeell);
    assert(i192.arr[1] == (unsigned long) -1);
    assert(i192.arr[2] == (unsigned long) -1);

    i192 = int128_unsign_to_bitint();
    assert(i192.arr[0] == (unsigned long) -0x0bad0c0ffeell);
    assert(i192.arr[1] == (unsigned long) -1);
    assert(i192.arr[2] == (unsigned long) 0);

    res = int128_from_bitint_sign();
    assert(res.arr[0] == (unsigned long) -0x0bad0c0ffeell);
    assert(res.arr[1] == (unsigned long) -1);

    res = int128_from_bitint_unsign();
    assert(res.arr[0] == (unsigned long) 0x0bad0c0ffeell);
    assert(res.arr[1] == (unsigned long) 0);

    assert(fabs(int128_signed_to_float() + 314159) < 1e-5);
    assert(fabs(int128_unsigned_to_float() - 314159) < 1e-5);
    assert(fabs(int128_signed_to_double() + 314159) < 1e-5);
    assert(fabs(int128_unsigned_to_double() - 314159) < 1e-5);
    assert(fabsl(int128_signed_to_long_double() + 314159) < 1e-5);
    assert(fabsl(int128_unsigned_to_long_double() - 314159) < 1e-5);

    res = int128_signed_from_float();
    assert(res.arr[0] == (unsigned long) -31415);
    assert(res.arr[1] == (unsigned long) -1);

    res = int128_unsigned_from_float();
    assert(res.arr[0] == (unsigned long) 31415);
    assert(res.arr[1] == (unsigned long) 0);

    res = int128_signed_from_double();
    assert(res.arr[0] == (unsigned long) -31415);
    assert(res.arr[1] == (unsigned long) -1);

    res = int128_unsigned_from_double();
    assert(res.arr[0] == (unsigned long) 31415);
    assert(res.arr[1] == (unsigned long) 0);

    res = int128_signed_from_long_double();
    assert(res.arr[0] == (unsigned long) -31415);
    assert(res.arr[1] == (unsigned long) -1);

    res = int128_unsigned_from_long_double();
    assert(res.arr[0] == (unsigned long) 31415);
    assert(res.arr[1] == (unsigned long) 0);
    return EXIT_SUCCESS;
}
