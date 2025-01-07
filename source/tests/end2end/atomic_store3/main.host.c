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
#include <string.h>
#include "./definitions.h"

#define EPSILON_F 1e-3
#define EPSILON_D 1e-6

int main(void) {
    for (long x = -4096; x < 4096; x++) {
        const struct X a1 = {{x, ~x, x + 1, x - 1, -x, x / 2, !x, x ^ 128}};
        struct S str = {0};

        set_i8(&str, (char) x);
        set_i16(&str, (short) x);
        set_i32(&str, (int) x);
        set_i64(&str, (long) x);
        set_f32(&str, (float) x);
        set_f64(&str, (double) x);
        set_a1(&str, a1);

        for (int i = 0; i < 8; i++) {
            set_arr1_el(&str, i, ~a1.buf[i]);
        }

        set_arr2_el(&str, 0, a1);
        set_arr2_el(&str, 1, (struct X){0});

        assert(str.i8 == (char) x);
        assert(str.i16 == (short) x);
        assert(str.i32 == (int) x);
        assert(str.i64 == (long) x);
        assert(fabs(str.f32 - (float) x) < EPSILON_F);
        assert(fabs(str.f64 - (double) x) < EPSILON_D);

        assert(memcmp(&str.a1, &a1, sizeof(struct X)) == 0);

        for (int i = 0; i < 8; i++) {
            assert(str.arr1[i] == ~a1.buf[i]);
        }

        assert(memcmp(&str.arr2[0], &a1, sizeof(struct X)) == 0);
        assert(memcmp(&str.arr2[1], &(struct X){0}, sizeof(struct X)) == 0);
    }
    return EXIT_SUCCESS;
}
