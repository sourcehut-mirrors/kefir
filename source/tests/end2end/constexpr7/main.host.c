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
#include <complex.h>
#include <string.h>
#include "./definitions.h"

static int fact(int x) {
    return x > 1 ? fact(x - 1) * x : 1;
}

int main(void) {
    assert(getx2()->a == 0);
    assert(getx2()->b == ~123);
    assert(getx2()->c == 0);
    assert(getx2()->e[0] == '\0');
    assert(getx2()->e[1] == '\0');
    assert(getx2()->e[2] == '\0');
    assert(getx2()->e[3] == '\0');

    assert(getx3()->a == 0xcafebadll);
    assert(getx3()->b == 0);
    assert(getx3()->c == 0);
    assert(getx3()->e[0] == 'h');
    assert(getx3()->e[1] == 'e');
    assert(getx3()->e[2] == 'h');
    assert(getx3()->e[3] == 'e');

    struct Y y = test2();
    assert(y.arr[0] == -100);
    assert(y.arr[1] == 3);
    assert(y.arr[2] == 5);
    assert(y.arr[3] == 0);
    assert(y.arr[4] == 6);
    assert(y.arr[5] == -5);

    for (int i = 0; i < 10; i++) {
        assert(test3(i) == fact(i));
    }
    return EXIT_SUCCESS;
}
