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

#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))
int main(void) {
#ifndef __NetBSD__
    struct i128 res;

    res = i128_from_f32(0.0f);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_from_f32(1.0f);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i128_from_f32(3.14f);
    assert(res.arr[0] == 3);
    assert(res.arr[1] == 0);

    res = i128_from_f32(-1.0f);
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_f32(-3.14f);
    assert(res.arr[0] == (unsigned long) -3);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_f32((float) ~0ull);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 1);

    res = i128_from_f32(2 * (float) ~0ull);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 2);

    res = i128_from_f32(-(float) ~0ull);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_f32(-2 * (float) ~0ull);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == (unsigned long) -2);

    res = i128_from_f64(0.0f);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_from_f64(1.0f);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i128_from_f64(3.14f);
    assert(res.arr[0] == 3);
    assert(res.arr[1] == 0);

    res = i128_from_f64(-1.0f);
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_f64(-3.14f);
    assert(res.arr[0] == (unsigned long) -3);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_f64((double) ~0ull);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 1);

    res = i128_from_f64(2 * (double) ~0ull);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 2);

    res = i128_from_f64(-(double) ~0ull);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_f64(-2 * (double) ~0ull);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == (unsigned long) -2);

    res = i128_from_f80(0.0f);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_from_f80(1.0f);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i128_from_f80(3.14f);
    assert(res.arr[0] == 3);
    assert(res.arr[1] == 0);

    res = i128_from_f80(-1.0f);
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_f80(-3.14f);
    assert(res.arr[0] == (unsigned long) -3);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_f80((long double) ~0ull);
    assert(res.arr[0] == ~0ull);
    assert(res.arr[1] == 0);

    res = i128_from_f80(2 * (long double) ~0ull);
    assert(res.arr[0] == ~0ull - 1);
    assert(res.arr[1] == 1);

    res = i128_from_f80(-(long double) ~0ull);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_f80(-2 * (long double) ~0ull);
    assert(res.arr[0] == 2);
    assert(res.arr[1] == (unsigned long) -2);

    res = i128_from_i9(0);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_from_i9(1);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i128_from_i9(-1);
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_i9(255);
    assert(res.arr[0] == 255);
    assert(res.arr[1] == 0);

    res = i128_from_i9(-256);
    assert(res.arr[0] == (unsigned long) -256);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_i9(256);
    assert(res.arr[0] == (unsigned long) -256);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_i9(-257);
    assert(res.arr[0] == (unsigned long) 255);
    assert(res.arr[1] == (unsigned long) 0);

    res = u128_from_i9(0);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_from_i9(1);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u128_from_i9(-1);
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_from_i9(255);
    assert(res.arr[0] == 255);
    assert(res.arr[1] == 0);

    res = u128_from_i9(-256);
    assert(res.arr[0] == (unsigned long) -256);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_from_i9(256);
    assert(res.arr[0] == (unsigned long) -256);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_from_i9(-257);
    assert(res.arr[0] == (unsigned long) 255);
    assert(res.arr[1] == (unsigned long) 0);

    res = i128_from_u9(0);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_from_u9(1);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i128_from_u9(-1);
    assert(res.arr[0] == 511);
    assert(res.arr[1] == 0);

    res = i128_from_u9(255);
    assert(res.arr[0] == 255);
    assert(res.arr[1] == 0);

    res = i128_from_u9(-256);
    assert(res.arr[0] == 256);
    assert(res.arr[1] == 0);

    res = i128_from_u9(256);
    assert(res.arr[0] == 256);
    assert(res.arr[1] == 0);

    res = i128_from_u9(-257);
    assert(res.arr[0] == 255);
    assert(res.arr[1] == 0);

    res = u128_from_u9(0);
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_from_u9(1);
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u128_from_u9(-1);
    assert(res.arr[0] == 511);
    assert(res.arr[1] == 0);

    res = u128_from_u9(255);
    assert(res.arr[0] == 255);
    assert(res.arr[1] == 0);

    res = u128_from_u9(-256);
    assert(res.arr[0] == 256);
    assert(res.arr[1] == 0);

    res = u128_from_u9(256);
    assert(res.arr[0] == 256);
    assert(res.arr[1] == 0);

    res = u128_from_u9(-257);
    assert(res.arr[0] == 255);
    assert(res.arr[1] == 0);

    res = i128_from_i256((struct i256){{0, 0, 0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_from_i256((struct i256){{1, 0, 0, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i128_from_i256((struct i256){{-1, -1, -1, -1}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_i256((struct i256){{-1, -1, 0, 0}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_i256((struct i256){{1, -1, 0, 0}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_u256((struct i256){{0, 0, 0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = i128_from_u256((struct i256){{1, 0, 0, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = i128_from_u256((struct i256){{-1, -1, -1, -1}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_u256((struct i256){{-1, -1, 0, 0}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = i128_from_u256((struct i256){{1, -1, 0, 0}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_from_i256((struct i256){{0, 0, 0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_from_i256((struct i256){{1, 0, 0, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u128_from_i256((struct i256){{-1, -1, -1, -1}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_from_i256((struct i256){{-1, -1, 0, 0}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_from_i256((struct i256){{1, -1, 0, 0}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_from_u256((struct i256){{0, 0, 0, 0}});
    assert(res.arr[0] == 0);
    assert(res.arr[1] == 0);

    res = u128_from_u256((struct i256){{1, 0, 0, 0}});
    assert(res.arr[0] == 1);
    assert(res.arr[1] == 0);

    res = u128_from_u256((struct i256){{-1, -1, -1, -1}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_from_u256((struct i256){{-1, -1, 0, 0}});
    assert(res.arr[0] == (unsigned long) -1);
    assert(res.arr[1] == (unsigned long) -1);

    res = u128_from_u256((struct i256){{1, -1, 0, 0}});
    assert(res.arr[0] == (unsigned long) 1);
    assert(res.arr[1] == (unsigned long) -1);
#endif
    return EXIT_SUCCESS;
}
