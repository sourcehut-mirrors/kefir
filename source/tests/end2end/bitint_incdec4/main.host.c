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
#include <float.h>
#include "./definitions.h"

int main(void) {
#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))
    for (unsigned long i = 0; i < 4096; i++) {
        unsigned char i8 = (unsigned char) i;
        unsigned short i16 = (unsigned short) i;
        unsigned int i32 = (unsigned int) i;
        unsigned long i64 = (unsigned long) i;

        assert(MASK(postdec1(&i8), 6) == MASK(i, 6));
        assert(MASK(i8, 6) == MASK(i - 1, 6));
        assert(MASK(postdec2(&i16), 14) == MASK(i, 14));
        assert(MASK(i16, 14) == MASK(i - 1, 14));
        assert(MASK(postdec3(&i32), 29) == MASK(i, 29));
        assert(MASK(i32, 29) == MASK(i - 1, 29));
        assert(MASK(postdec4(&i64), 58) == MASK(i, 58));
        assert(MASK(i64, 58) == MASK(i - 1, 58));

        struct S2 s2_orig = {{i, 0}};
        struct S2 s2 = postdec5(&s2_orig);
        if (i >= 1) {
            assert(s2.arr[0] == i);
            assert(MASK(s2.arr[1], 55) == 0);
            assert(s2_orig.arr[0] == i - 1);
            assert(MASK(s2_orig.arr[1], 55) == 0);
        } else {
            assert(s2.arr[0] == i);
            assert(MASK(s2.arr[1], 55) == 0);
            assert(s2_orig.arr[0] == ~0ull);
            assert(MASK(s2_orig.arr[1], 55) == MASK(~0ull, 55));
        }

        s2_orig = (struct S2) {{0, i}};
        s2 = postdec5(&s2_orig);
        if (i >= 1) {
            assert(s2.arr[0] == 0);
            assert(MASK(s2.arr[1], 55) == i);
            assert(s2_orig.arr[0] == ~0ull);
            assert(MASK(s2_orig.arr[1], 55) == i - 1);
        } else {
            assert(s2.arr[0] == 0);
            assert(MASK(s2.arr[1], 55) == i);
            assert(s2_orig.arr[0] == ~0ull);
            assert(MASK(s2_orig.arr[1], 55) == MASK(~0ull, 55));
        }

        struct S5 s5_orig = {{i, 0, 0, 0, 0}};
        struct S5 s5 = postdec6(&s5_orig);
        if (i >= 1) {
            assert(s5.arr[0] == i);
            assert(s5.arr[1] == 0);
            assert(s5.arr[2] == 0);
            assert(s5.arr[3] == 0);
            assert(MASK(s5.arr[4], 55) == 0);
            assert(s5_orig.arr[0] == i - 1);
            assert(s5_orig.arr[1] == 0);
            assert(s5_orig.arr[2] == 0);
            assert(s5_orig.arr[3] == 0);
            assert(MASK(s5_orig.arr[4], 55) == 0);
        } else {
            assert(s5.arr[0] == i);
            assert(s5.arr[1] == 0);
            assert(s5.arr[2] == 0);
            assert(s5.arr[3] == 0);
            assert(MASK(s5.arr[4], 55) == 0);
            assert(s5_orig.arr[0] == ~0ull);
            assert(s5_orig.arr[1] == ~0ull);
            assert(s5_orig.arr[2] == ~0ull);
            assert(s5_orig.arr[3] == ~0ull);
            assert(MASK(s5_orig.arr[4], 55) == MASK(~0ull, 55));
        }

        s5_orig = (struct S5) {{0ull, 0ull, 0ull, 0ull, i}};
        s5 = postdec6(&s5_orig);
        if (i >= 1) {
            assert(s5.arr[0] == 0);
            assert(s5.arr[1] == 0);
            assert(s5.arr[2] == 0);
            assert(s5.arr[3] == 0);
            assert(MASK(s5.arr[4], 55) == i);
            assert(s5_orig.arr[0] == ~0ull);
            assert(s5_orig.arr[1] == ~0ull);
            assert(s5_orig.arr[2] == ~0ull);
            assert(s5_orig.arr[3] == ~0ull);
            assert(MASK(s5_orig.arr[4], 55) == i - 1);
        } else {
            assert(s5.arr[0] == 0);
            assert(s5.arr[1] == 0);
            assert(s5.arr[2] == 0);
            assert(s5.arr[3] == 0);
            assert(MASK(s5.arr[4], 55) == i);
            assert(s5_orig.arr[0] == ~0ull);
            assert(s5_orig.arr[1] == ~0ull);
            assert(s5_orig.arr[2] == ~0ull);
            assert(s5_orig.arr[3] == ~0ull);
            assert(MASK(s5_orig.arr[4], 55) == MASK(~0ull, 55));
        }
    }
    return EXIT_SUCCESS;
}
