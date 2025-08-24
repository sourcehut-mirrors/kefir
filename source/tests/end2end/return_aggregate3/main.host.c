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
#include <string.h>
#include "./definitions.h"

void leak(void *x) {
    (void) x;
}

struct S1 get(unsigned long x) {
    return (struct S1) {
        {x, x + 1, x + 2, x + 3, -x, -x - 1, -x - 2, -x - 3}
    };
}

int main(void) {
    struct S1 s1 = test(0);
    assert(s1.arr[0] == 0);
    assert(s1.arr[1] == 0);
    assert(s1.arr[2] == 0);
    assert(s1.arr[3] == 0);
    assert(s1.arr[4] == 0);
    assert(s1.arr[5] == 0);
    assert(s1.arr[6] == 0);
    assert(s1.arr[7] == 0);

    for (unsigned long x = 1; x < 256; x++) {
        struct S1 s1 = test(x);
        assert(s1.arr[0] == ~x);
        assert(s1.arr[1] == ~x + 1);
        assert(s1.arr[2] == ~x + 2);
        assert(s1.arr[3] == ~x + 3);
        assert(s1.arr[4] == -(~x));
        assert(s1.arr[5] == -(~x) - 1);
        assert(s1.arr[6] == -(~x) - 2);
        assert(s1.arr[7] == -(~x) - 3);
    }
    return EXIT_SUCCESS;
}
