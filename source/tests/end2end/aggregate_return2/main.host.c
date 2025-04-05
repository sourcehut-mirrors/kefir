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

struct small_struct getsmall(int x) {
    return (struct small_struct) {.a = x, .b = x * 3.14f};
}

struct large_struct getlarge(int x) {
    struct large_struct s = {0};
    for (int i = 0; i < 128; i++) {
        s.x[i] = ~(i + x);
    }
    return s;
}

_Bool my_memcmp(void *x, void *y, unsigned long long sz) {
    return memcmp(x, y, (size_t) sz) == 0;
}

int main(void) {
    for (int i = -100; i < 100; i++) {
        float res = testsmall(i);
        assert(fabs(res - (float) i - i * 3.14f) < 1e-3);
    }
    for (int i = 0; i < 128; i++) {
        assert(testlarge(i) == ~(i + i));
        for (int j = 0; j < 128; j++) {
            assert(testlarge2(i, j) == (i == j));
        }
    }
    return EXIT_SUCCESS;
}
