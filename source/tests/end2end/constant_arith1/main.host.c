/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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
#include <stddef.h>
#include "./definitions.h"

int main(void) {
    for (long x = -4096; x < 4096; x++) {
        assert(test1(x) == (x + 1) + 2);
        assert(test2(x) == 4 + (x + 3));
        assert(test3(x) == (x - 5) + 10);
        assert(test4(x) == (5 - x) + 13);
        assert(test5(x) == 20 + (x - 13));
        assert(test6(x) == 30 + (12 - x));
        assert(test7(x) == (x + 49) - 150);
        assert(test8(x) == 150 - (x + 29));
        assert(test9(x) == (x - 30) - 25);
        assert(test10(x) == (300 - x) - 50);
        assert(test11(x) == 55 - (x - 40));
        assert(test12(x) == 350 - (25 - x));
    }
    return EXIT_SUCCESS;
}
