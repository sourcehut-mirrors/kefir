/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

struct Struct1 getstruct1(long seed) {
    return (struct Struct1){.a = seed, .b = seed / 2, .c = ~seed, .d = seed | 17251};
}

struct Struct2 getstruct2(int seed) {
    return (struct Struct2) {.arr = { seed, seed / 2, ~seed }};
}

int main(void) {
    for (long x = -1000; x < 1000; x++) {
        assert(struct1sum(x) == (int) x + (long) x / 2 + (char) ~x + (long) (x | 17251));
        assert(struct2sum((int) x) == (int) x + ((int) x) / 2 + ~(int) x);
    }
    return EXIT_SUCCESS;
}
