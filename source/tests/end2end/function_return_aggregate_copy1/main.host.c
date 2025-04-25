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

#include "./definitions.h"
#include <stdlib.h>
#include <assert.h>

struct S1 get(int seed) {
    struct S1 s;
    for (int i = 0; i < 64; i++) {
        s.arr[i] = seed + i;
    }
    return s;
}

int main(void) {
    for (int i = -1000; i < 1000; i++) {
        for (int j = 0; j < 64; j++) {
            assert(get2(i, j) == i + j);
            assert(get3(i, j) == i + j);
        }
    }
    return EXIT_SUCCESS;
}
