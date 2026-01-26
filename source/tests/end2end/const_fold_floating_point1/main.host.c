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
    assert(fabs(neg32() + 3.14159) < 1e-5);
    assert(fabs(neg64() + 3.14159e1) < 1e-6);
    assert(fabsl(neg80() + 3.14159e2l) < 1e-7);
    assert(fabs(add32() - (3.14159f + 2.71828f)) < 1e-5);
    assert(fabs(add64() - (3.14159e1 + 2.71828e-1)) < 1e-6);
    assert(fabsl(add80() - (3.14159e-2 + 2.71828e2)) < 1e-7);
    assert(fabs(sub32() - (3.14159f - 2.71828f)) < 1e-5);
    assert(fabs(sub64() - (3.14159e1 - 2.71828e-1)) < 1e-6);
    assert(fabsl(sub80() - (3.14159e-2 - 2.71828e2)) < 1e-7);
    assert(fabs(mul32() - (3.14159f * 2.71828f)) < 1e-5);
    assert(fabs(mul64() - (3.14159e1 * 2.71828e-1)) < 1e-6);
    assert(fabsl(mul80() - (3.14159e-2 * 2.71828e2)) < 1e-7);
    assert(fabs(div32() - (3.14159f / 2.71828f)) < 1e-5);
    assert(fabs(div64() - (3.14159e1 / 2.71828e-1)) < 1e-6);
    assert(fabsl(div80() - (3.14159e-2 / 2.71828e2)) < 1e-7);
    assert(to_int_32() == -3);
    assert(to_int_64() == -31);
    assert(to_int_80() == -314);
    assert(to_uint_32() == 3);
    assert(to_uint_64() == 31);
    assert(to_uint_80() == 314);
    assert(fabs(int_to_float32() + 314159) < 1e-5);
    assert(fabs(int_to_float64() + 31415926) < 1e-6);
    assert(fabsl(int_to_float80() + 31415926) < 1e-7);
    assert(fabs(uint_to_float32() - 314159) < 1e-5);
    assert(fabs(uint_to_float64() - 31415926) < 1e-6);
    assert(fabsl(uint_to_float80() - 31415926) < 1e-7);
    assert(fabs(float32_to_float64() - 3.14159) < 1e-6);
    assert(fabsl(float32_to_float80() - 3.14159e1l) < 1e-5);
    assert(fabs(float64_to_float32() - 3.14159e2f) < 1e-5);
    assert(fabsl(float64_to_float80() - 2.71828e3l) < 1e-6);
    assert(fabs(float80_to_float32() - 2.71828e4f) < 1e-5);
    assert(fabs(float80_to_float64() - 2.71828e5f) < 1e-6);
    assert(!float32_eq_float32());
    assert(!float64_eq_float64());
    assert(!float80_eq_float80());
    assert(float32_gt_float32());
    assert(float64_gt_float64());
    assert(float80_gt_float80());
    assert(float32_lt_float32());
    assert(float64_lt_float64());
    assert(float80_lt_float80());
    assert(float32_eq2_float32());
    assert(float64_eq2_float64());
    assert(float80_eq2_float80());
    assert(!float32_gt2_float32());
    assert(!float64_gt2_float64());
    assert(!float80_gt2_float80());
    assert(!float32_lt2_float32());
    assert(!float64_lt2_float64());
    assert(!float80_lt2_float80());
    return EXIT_SUCCESS;
}
