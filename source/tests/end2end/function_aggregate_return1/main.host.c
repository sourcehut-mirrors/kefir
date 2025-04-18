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
#include <assert.h>
#include <math.h>
#include "./definitions.h"

struct S1 empty1(void) {
    return (struct S1) {0};
}

int main(void) {
    for (long i = -1000; i < 1000; i++) {
        struct S1 s = get1(i);
        if (i >= 0) {
            assert(s.arr[0] == i);
            assert(s.arr[1] == ~i);
            assert(s.arr[2] == i + 1);
            assert(s.arr[3] == -i);
            assert(s.arr[4] == i - 1);
            assert(s.arr[5] == i * 2);
            assert(s.arr[6] == i / 2);
            assert(s.arr[7] == (i ^ 0xcafebabe));
        } else {
            assert(s.arr[0] == 0);
            assert(s.arr[1] == 0);
            assert(s.arr[2] == 0);
            assert(s.arr[3] == 0);
            assert(s.arr[4] == 0);
            assert(s.arr[5] == 0);
            assert(s.arr[6] == 0);
            assert(s.arr[7] == 0);
        }
    }
    return EXIT_SUCCESS;
}
