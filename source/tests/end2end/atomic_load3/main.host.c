/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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
        const struct S str = {
            .i8 = (char) x,
            .i16 = (short) x,
            .i32 = (int) x,
            .i64 = (long) x,
            .f32 = (float) x,
            .f64 = (double) x,
            .a1 = a1,
            .arr1 = {~a1.buf[0], ~a1.buf[1], ~a1.buf[2], ~a1.buf[3], ~a1.buf[4], ~a1.buf[5], ~a1.buf[6], ~a1.buf[7]},
            .arr2 = {a1}};

        assert(get_i8(&str) == (char) x);
        assert(get_i16(&str) == (short) x);
        assert(get_i32(&str) == (int) x);
        assert(get_i64(&str) == (long) x);
        assert(fabs(get_f32(&str) - (float) x) < EPSILON_F);
        assert(fabs(get_f64(&str) - (double) x) < EPSILON_D);

        struct X a1_loaded = get_a1(&str);
        assert(memcmp(&a1_loaded, &a1, sizeof(struct X)) == 0);

        for (int i = 0; i < 8; i++) {
            assert(get_arr1_el(&str, i) == ~a1.buf[i]);
        }

        struct X arr2_0_loaded = get_arr2_el(&str, 0);
        assert(memcmp(&arr2_0_loaded, &a1, sizeof(struct X)) == 0);

        struct X arr2_1_loaded = get_arr2_el(&str, 1);
        assert(memcmp(&arr2_1_loaded, &(struct X){0}, sizeof(struct X)) == 0);
    }
    return EXIT_SUCCESS;
}
