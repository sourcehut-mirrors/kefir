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
#include "./definitions.h"

#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))

static void *ptr = &ptr;
static struct S16 s16 = {0};

void test1(void *x, struct S16 y) {
    ptr = x;
    s16 = y;
}

int main(void) {
    test2(-1);
    assert(ptr == NULL);
    for (int i = 0; i < 15; i++) {
        assert(s16.arr[i] == ~0ull);
    }
    assert(MASK(s16.arr[15], 63) == MASK(~0ull, 63));
    return EXIT_SUCCESS;
}
