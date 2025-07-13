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

int test1(int *);

int main(void) {
    struct S1 *s1 = get1();
    struct S1 *s2 = get2();
    struct S1 s3 = get3();
    struct S1 s4 = get4();
    struct S1 *s5_0 = get5(0);
    struct S1 *s5_1 = get5(1);
    for (int i = 0; i < 8; i++) {
        assert(s1->arr[i] == (i + 1));
        assert(s2->arr[i] == (8 - i));
        assert(s3.arr[i] == -(i + 1));
        assert(s4.arr[i] == -(8 - i));
        assert(gs1->arr[i] == 100 * (i + 1));
        assert(s5_0->arr[i] == -100 * (8 - i));
        assert(s5_1->arr[i] == -100 * (i + 1));
    }
    assert(test1(NULL) == sizeof(int *));
    return EXIT_SUCCESS;
}
