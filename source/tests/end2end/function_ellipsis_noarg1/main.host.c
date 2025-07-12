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
    assert(test1() == 0xcafe);
    for (int i = -100; i < 100; i++) {
        assert(test2(i) == -i);
        assert(test2(i, 1, 2, 3, ~i) == -i);
        assert(test2(~i, 1, 2, 3, ~i, -1000 * i, (long) i) == -(~i));
        assert(test3(i, i ^ 0xbad0) == (-i * (i ^ 0xbad0)));

        assert(test4(0) == 0);
        assert(test4(0, 1) == 0);
        assert(test4(1, 1) == 1);
        assert(test4(1, i, 2) == i);
        assert(test4(2, i, 2) == i + 2);
        assert(test4(10, i, 1, 2, 3, 4, 5, 6, 7, 8, 9) == i + 45);
    }
    return EXIT_SUCCESS;
}
