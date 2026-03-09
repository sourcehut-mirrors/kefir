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
#include <complex.h>
#include "./definitions.h"

int callback1(int *arr) {
    return arr[0] + arr[1] + arr[2] + arr[3];
}

int callback2(struct S1 s1) {
    return s1.a + s1.b;
}

int main(void) {
    for (int i = -4096; i < 4096; i++) {
        assert(test1(i) == (i + i + 1 + i + 2 + i + 3));
        assert(test2(i) == (i + i + 1 + i + 3));
        assert(test3(i) == (i + i + 1 + i + 2 + i + 3));
        assert(test4(i) == (i + i + 1 + i + 2 + i + 3));
        assert(test5(i) == (i + i + 1 + i + 3));

        assert(test8(i) == (i + i * 2));
        assert(test9(i) == (i * 2));
        assert(test10(i) == (i + i * 2));

        if (i) {
            assert(test6(i) == (i + i + 1 + i + 2 + i + 3));
            assert(test7(i) == (i + i + 1 + i + 2 + i + 3));
            assert(test11(i) == (i + i * 2));
            assert(test12(i) == (i + i * 2));
            assert(test13(i) == (i + i * 2));
        } else {
            assert(test6(i) == 0);
            assert(test7(i) == 0);
            assert(test11(i) == 0);
            assert(test12(i) == 0);
            assert(test13(i) == 0);
        }
    }
    return EXIT_SUCCESS;
}
