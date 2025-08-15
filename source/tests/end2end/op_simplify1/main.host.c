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
#include <limits.h>
#include "./definitions.h"

int main(void) {
    for (int i = -100; i < 100; i++) {
        for (int j = -100; j < 100; j++) {
            assert(test1(1, i, j) == (i + j));
            assert(test1(0, i, j) == (i + j));
            assert(test2(1, i, j) == (i + j));
            assert(test2(0, i, j) == (i + j));
            assert(test3(1, i, j) == (i - j));
            assert(test4(1, i, j) == (i * j));
            assert(test4(0, i, j) == (i * j));
            assert(test5(1, i, j) == (i * j));
            assert(test5(0, i, j) == (i * j));
            if (j != 0) {
                assert(test6(1, i, j) == (i / j));
                assert(test6(0, i, j) == (i / j));
            }
            if (i >= 0 && j >= 0 && j < 8) {
                assert(test7(1, i, j) == (i << j));
                assert(test7(0, i, j) == (i << j));
                assert(test8(1, i, j) == (i << j));
                assert(test8(0, i, j) == (i << j));
            }
            if (j >= 0 && j < (int) (CHAR_BIT * sizeof(int))) {
                assert(test9(1, i, j) == (i >> j));
                assert(test9(0, i, j) == (i >> j));
                assert(test10(1, i, j) == (i >> j));
                assert(test10(0, i, j) == (i >> j));
                assert(test11(1, i, j) == (((unsigned int) i) >> j));
                assert(test11(0, i, j) == (((unsigned int) i) >> j));
                assert(test12(1, i, j) == (((unsigned int) i) >> j));
                assert(test12(0, i, j) == (((unsigned int) i) >> j));
            }
        }
    }
    return EXIT_SUCCESS;
}
