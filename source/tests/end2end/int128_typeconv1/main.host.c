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

#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))
int main(void) {
    assert(fabs(i128_to_f32((struct i128){{0, 0}})) < 1e-5);
    assert(fabs(i128_to_f32((struct i128){{1, 0}}) - 1.0f) < 1e-5);
    assert(fabs(i128_to_f32((struct i128){{1024, 0}}) - 1024.0f) < 1e-5);
    assert(fabs(i128_to_f32((struct i128){{0, 1}}) - (1 + (double) ~0ull)) < 1e-5);
    assert(fabs(i128_to_f32((struct i128){{-1, -1}}) + 1.0f) < 1e-5);
    assert(fabs(i128_to_f32((struct i128){{1, -1}}) + (1 + (double) ~0ull)) < 1e-5);

    assert(fabs(u128_to_f32((struct i128){{0, 0}})) < 1e-5);
    assert(fabs(u128_to_f32((struct i128){{1, 0}}) - 1.0f) < 1e-5);
    assert(fabs(u128_to_f32((struct i128){{1024, 0}}) - 1024.0f) < 1e-5);
    assert(fabs(u128_to_f32((struct i128){{0, 1}}) - (1 + (double) ~0ull)) < 1e-5);
    assert(isinf(u128_to_f32((struct i128){{-1, -1}})));
    assert(isinf(u128_to_f32((struct i128){{1, -1}})));

    assert(fabs(i128_to_f64((struct i128){{0, 0}})) < 1e-5);
    assert(fabs(i128_to_f64((struct i128){{1, 0}}) - 1.0f) < 1e-5);
    assert(fabs(i128_to_f64((struct i128){{1024, 0}}) - 1024.0f) < 1e-5);
    assert(fabs(i128_to_f64((struct i128){{0, 1}}) - (1 + (double) ~0ull)) < 1e-5);
    assert(fabs(i128_to_f64((struct i128){{-1, -1}}) + 1.0f) < 1e-5);
    assert(fabs(i128_to_f64((struct i128){{1, -1}}) + (1 + (double) ~0ull)) < 1e-5);

    assert(fabs(u128_to_f64((struct i128){{0, 0}})) < 1e-5);
    assert(fabs(u128_to_f64((struct i128){{1, 0}}) - 1.0f) < 1e-5);
    assert(fabs(u128_to_f64((struct i128){{1024, 0}}) - 1024.0f) < 1e-5);
    assert(fabs(u128_to_f64((struct i128){{0, 1}}) - (1 + (double) ~0ull)) < 1e-5);
    assert(fabs(u128_to_f64((struct i128){{-1, -1}}) - 3.4028236692093846e+38) < 1e-5);
    assert(fabs(u128_to_f64((struct i128){{1, -1}}) - 3.4028236692093846e+38) < 1e-5);

    assert(fabsl(i128_to_f80((struct i128){{0, 0}})) < 1e-5);
    assert(fabsl(i128_to_f80((struct i128){{1, 0}}) - 1.0f) < 1e-5);
    assert(fabsl(i128_to_f80((struct i128){{1024, 0}}) - 1024.0f) < 1e-5);
    assert(fabsl(i128_to_f80((struct i128){{0, 1}}) - (1 + (double) ~0ull)) < 1e-5);
    assert(fabsl(i128_to_f80((struct i128){{-1, -1}}) + 1.0f) < 1e-5);
    assert(fabsl(i128_to_f80((struct i128){{0, -1}}) + (1 + (long double) ~0ull)) < 1e-5);

    assert(fabsl(u128_to_f80((struct i128){{0, 0}})) < 1e-5);
    assert(fabsl(u128_to_f80((struct i128){{1, 0}}) - 1.0f) < 1e-5);
    assert(fabsl(u128_to_f80((struct i128){{1024, 0}}) - 1024.0f) < 1e-5);
    assert(fabsl(u128_to_f80((struct i128){{0, 1}}) - (1 + (long double) ~0ull)) < 1e-5);
    assert(fabsl(u128_to_f80((struct i128){{-1, -1}}) - 3.4028236692093846e+38) < 1e-5);
    assert(fabsl(u128_to_f80((struct i128){{1, -1}}) - 3.40282366920938463445e+38L) < 1e-5);

// unsigned short i128_to_i9(struct i128);
// unsigned short u128_to_i9(struct i128);
// unsigned short i128_to_u9(struct i128);
// unsigned short u128_to_u9(struct i128);

// struct i256 i128_to_i256(struct i128);
// struct i256 u128_to_i256(struct i128);
// struct i256 i128_to_u256(struct i128);
// struct i256 u128_to_u256(struct i128);
    assert(MASK(i128_to_i9((struct i128){{0, 0}}), 9) == MASK(0, 9));
    assert(MASK(i128_to_i9((struct i128){{1, 0}}), 9) == MASK(1, 9));
    assert(MASK(i128_to_i9((struct i128){{255, 0}}), 9) == MASK(255, 9));
    assert(MASK(i128_to_i9((struct i128){{-256, 0}}), 9) == MASK(-256, 9));
    assert(MASK(i128_to_i9((struct i128){{-1, -1}}), 9) == MASK(-1, 9));
    assert(MASK(i128_to_i9((struct i128){{1, -1}}), 9) == MASK(1, 9));
    assert(MASK(i128_to_i9((struct i128){{0, -1}}), 9) == MASK(0, 9));
    assert(MASK(i128_to_u9((struct i128){{0, 0}}), 9) == MASK(0, 9));
    assert(MASK(i128_to_u9((struct i128){{1, 0}}), 9) == MASK(1, 9));
    assert(MASK(i128_to_u9((struct i128){{255, 0}}), 9) == MASK(255, 9));
    assert(MASK(i128_to_u9((struct i128){{-256, 0}}), 9) == MASK(-256, 9));
    assert(MASK(i128_to_u9((struct i128){{-1, -1}}), 9) == MASK(-1, 9));
    assert(MASK(i128_to_u9((struct i128){{1, -1}}), 9) == MASK(1, 9));
    assert(MASK(i128_to_u9((struct i128){{0, -1}}), 9) == MASK(0, 9));
    assert(MASK(u128_to_i9((struct i128){{0, 0}}), 9) == MASK(0, 9));
    assert(MASK(u128_to_i9((struct i128){{1, 0}}), 9) == MASK(1, 9));
    assert(MASK(u128_to_i9((struct i128){{255, 0}}), 9) == MASK(255, 9));
    assert(MASK(u128_to_i9((struct i128){{-256, 0}}), 9) == MASK(-256, 9));
    assert(MASK(u128_to_i9((struct i128){{-1, -1}}), 9) == MASK(-1, 9));
    assert(MASK(u128_to_i9((struct i128){{1, -1}}), 9) == MASK(1, 9));
    assert(MASK(u128_to_i9((struct i128){{0, -1}}), 9) == MASK(0, 9));
    assert(MASK(u128_to_u9((struct i128){{0, 0}}), 9) == MASK(0, 9));
    assert(MASK(u128_to_u9((struct i128){{1, 0}}), 9) == MASK(1, 9));
    assert(MASK(u128_to_u9((struct i128){{255, 0}}), 9) == MASK(255, 9));
    assert(MASK(u128_to_u9((struct i128){{-256, 0}}), 9) == MASK(-256, 9));
    assert(MASK(u128_to_u9((struct i128){{-1, -1}}), 9) == MASK(-1, 9));
    assert(MASK(u128_to_u9((struct i128){{1, -1}}), 9) == MASK(1, 9));
    assert(MASK(u128_to_u9((struct i128){{0, -1}}), 9) == MASK(0, 9));

    struct i256 i256;
    i256 = i128_to_i256((struct i128){{0, 0}});
    assert(i256.arr[0] == 0);
    assert(i256.arr[1] == 0);
    assert(i256.arr[2] == 0);
    assert(i256.arr[3] == 0);

    i256 = i128_to_i256((struct i128){{1, 0}});
    assert(i256.arr[0] == 1);
    assert(i256.arr[1] == 0);
    assert(i256.arr[2] == 0);
    assert(i256.arr[3] == 0);

    i256 = i128_to_i256((struct i128){{-1, 1}});
    assert(i256.arr[0] == (unsigned long) -1);
    assert(i256.arr[1] == 1);
    assert(i256.arr[2] == 0);
    assert(i256.arr[3] == 0);

    i256 = i128_to_i256((struct i128){{-1, -1}});
    assert(i256.arr[0] == (unsigned long) -1);
    assert(i256.arr[1] == (unsigned long) -1);
    assert(i256.arr[2] == (unsigned long) -1);
    assert(i256.arr[3] == (unsigned long) -1);

    i256 = i128_to_i256((struct i128){{0, -1}});
    assert(i256.arr[0] == (unsigned long) 0);
    assert(i256.arr[1] == (unsigned long) -1);
    assert(i256.arr[2] == (unsigned long) -1);
    assert(i256.arr[3] == (unsigned long) -1);

    i256 = i128_to_u256((struct i128){{0, 0}});
    assert(i256.arr[0] == 0);
    assert(i256.arr[1] == 0);
    assert(i256.arr[2] == 0);
    assert(i256.arr[3] == 0);

    i256 = i128_to_u256((struct i128){{1, 0}});
    assert(i256.arr[0] == 1);
    assert(i256.arr[1] == 0);
    assert(i256.arr[2] == 0);
    assert(i256.arr[3] == 0);

    i256 = i128_to_u256((struct i128){{-1, 1}});
    assert(i256.arr[0] == (unsigned long) -1);
    assert(i256.arr[1] == 1);
    assert(i256.arr[2] == 0);
    assert(i256.arr[3] == 0);

    i256 = i128_to_u256((struct i128){{-1, -1}});
    assert(i256.arr[0] == (unsigned long) -1);
    assert(i256.arr[1] == (unsigned long) -1);
    assert(i256.arr[2] == (unsigned long) -1);
    assert(i256.arr[3] == (unsigned long) -1);

    i256 = i128_to_u256((struct i128){{0, -1}});
    assert(i256.arr[0] == (unsigned long) 0);
    assert(i256.arr[1] == (unsigned long) -1);
    assert(i256.arr[2] == (unsigned long) -1);
    assert(i256.arr[3] == (unsigned long) -1);

    i256 = u128_to_i256((struct i128){{0, 0}});
    assert(i256.arr[0] == 0);
    assert(i256.arr[1] == 0);
    assert(i256.arr[2] == 0);
    assert(i256.arr[3] == 0);

    i256 = u128_to_i256((struct i128){{1, 0}});
    assert(i256.arr[0] == 1);
    assert(i256.arr[1] == 0);
    assert(i256.arr[2] == 0);
    assert(i256.arr[3] == 0);

    i256 = u128_to_i256((struct i128){{-1, 1}});
    assert(i256.arr[0] == (unsigned long) -1);
    assert(i256.arr[1] == 1);
    assert(i256.arr[2] == 0);
    assert(i256.arr[3] == 0);

    i256 = u128_to_i256((struct i128){{-1, -1}});
    assert(i256.arr[0] == (unsigned long) -1);
    assert(i256.arr[1] == (unsigned long) -1);
    assert(i256.arr[2] == 0);
    assert(i256.arr[3] == 0);

    i256 = u128_to_i256((struct i128){{0, -1}});
    assert(i256.arr[0] == (unsigned long) 0);
    assert(i256.arr[1] == (unsigned long) -1);
    assert(i256.arr[2] == 0);
    assert(i256.arr[3] == 0);

    i256 = u128_to_u256((struct i128){{0, 0}});
    assert(i256.arr[0] == 0);
    assert(i256.arr[1] == 0);
    assert(i256.arr[2] == 0);
    assert(i256.arr[3] == 0);

    i256 = u128_to_u256((struct i128){{1, 0}});
    assert(i256.arr[0] == 1);
    assert(i256.arr[1] == 0);
    assert(i256.arr[2] == 0);
    assert(i256.arr[3] == 0);

    i256 = u128_to_u256((struct i128){{-1, 1}});
    assert(i256.arr[0] == (unsigned long) -1);
    assert(i256.arr[1] == 1);
    assert(i256.arr[2] == 0);
    assert(i256.arr[3] == 0);

    i256 = u128_to_u256((struct i128){{-1, -1}});
    assert(i256.arr[0] == (unsigned long) -1);
    assert(i256.arr[1] == (unsigned long) -1);
    assert(i256.arr[2] == 0);
    assert(i256.arr[3] == 0);

    i256 = u128_to_u256((struct i128){{0, -1}});
    assert(i256.arr[0] == (unsigned long) 0);
    assert(i256.arr[1] == (unsigned long) -1);
    assert(i256.arr[2] == 0);
    assert(i256.arr[3] == 0);
    return EXIT_SUCCESS;
}
