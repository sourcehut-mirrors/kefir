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

int main(void) {
    for (int i = -100; i < 100; i++) {
        struct S1 s = get1(i);
        assert(s.arr[0] == 0);
        for (int j = 0; j < 7; j++) {
            assert(s.arr[j + 1] == (unsigned long) (i + j));
        }

        struct S1 s1 = get2(1, i);
        struct S1 s2 = get2(0, i);
        struct S1 s3 = get2(-1, i);

        assert(s1.arr[0] == 0);
        assert(s2.arr[0] == 0);
        assert(s3.arr[0] == 0);
        for (int j = 0; j < 7; j++) {
            assert(s1.arr[j + 1] == (unsigned long) (i + j));
            assert(s2.arr[j + 1] == (unsigned long) ~(i + j));
            assert(s3.arr[j + 1] == (unsigned long) -(i + j));
        }
    }
    return EXIT_SUCCESS;
}
