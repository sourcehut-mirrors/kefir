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

int callback1(struct S1 s1) {
    return s1.a + s1.b;
}

int main(void) {
    for (int i = -4096; i < 4096; i++) {
        assert(test1(i) == (((char) i) + i * 2));
        assert(test2(i) == (i * 2));
        assert(test3(i) == (((char) i) + i * 2));
        assert(test4(i) == (((char) i) + i * 2));
        assert(test5(i) == (((char) i) + i * 2));
        assert(test6(i) == (((char) i) + i * 2));
        if (i != 0) {
            assert(test7(i) == (((char) i) + i * 2));
            assert(test8(i) == (((char) i) + i * 2));
        } else {
            assert(test7(i) == 0);
            assert(test8(i) == 0);
        }
        assert(test9(i) == (i * 2));
        assert(test10(i) == (((char) i) + i * 2));
    }
    return EXIT_SUCCESS;
}
