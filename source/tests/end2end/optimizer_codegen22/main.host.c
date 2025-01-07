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
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include "./definitions.h"

#define EPSILON_F 1e-3
#define EPSILON_D 1e-6
#define EPSILON_LD 1e-8

void *array[10];

void fn1(void) {}

int main(void) {
#ifdef __x86_64__
    for (double x = -100.0; x < 100.0; x += 0.1) {
        for (double y = -10.0; y < 10.0; y += 1.5) {
            assert(fabs(custom_hypot(x, y) - (x * x + y * y)) < EPSILON_D);
        }
    }

    struct S1 s1 = init_s1();
    assert(s1.l == 100);
    assert(s1.i == 200);
    assert(s1.s == 300);
    assert(s1.c == 'X');

    for (long l1 = 0; l1 < 5000; l1++) {
        for (long l2 = 0; l2 < 100; l2++) {
            long l = l1 + (l2 << 32);
            assert(clear8(l) == (l & ~0xffL));
            assert(clear16(l) == (l & ~0xffffL));
            assert(clear32(l) == 0);
            assert(clear64(l) == 0);

            assert(set8(l) == (l | 0xffL));
            assert(set16(l) == (l | 0xffffL));
            assert(set32(l) == (l | 0xffffffffL));
            assert(set64(l) == (long) 0xffffffffffffffffL);
        }
    }

    for (long x = -10; x <= 10; x++) {
        for (long y = -10; y <= 10; y++) {
            for (long z = -10; z <= 10; z++) {
                assert(sum3_one(x, y, z) == (x + y + z + 1));
            }
        }
    }

    for (unsigned int x = 0; x < 0x2ffff; x++) {
        struct S2 s2 = make_s2(x);
        assert(s2.shrt[0] == (x & 0xffff));
        assert(s2.shrt[1] == ((x >> 16) & 0xffff));
        assert(unwrap_s2(s2) == x);
    }

    for (unsigned long x = 0; x < 0xffff; x++) {
        for (unsigned long y = 0; y < 0x1ff; y++) {
            unsigned long l = (y << 32) | x;
            assert(cast_int(l) == (unsigned int) l);
        }
    }

    memset(array, 0, sizeof(array));

    assert(array[0] == NULL);
    assert(array[1] == NULL);
    assert(array[2] == NULL);
    assert(array[3] == NULL);
    assert(array[4] == NULL);
    init_array();

    void (*fn1_ptr)(void) = fn1;
    assert(array[0] == *(void **) &fn1_ptr);
    assert(array[1] == &array[5]);
    assert(strcmp(array[2], "llo") == 0);
    assert(array[3] == getx());
    union {
        void *ptr;
        double dbl;
    } u1 = {.ptr = array[4]};
    assert(u1.dbl - 4.21 <= 1e-6);
#endif
    return EXIT_SUCCESS;
}
