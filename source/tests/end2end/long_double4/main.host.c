/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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
#include "./definitions.h"

int main(void) {
    for (long double x = -1000.0l; x < 1000.0l; x += 0.1l) {
        struct S s = init_s(x);
        assert(fabsl(s.x - x) < 1e-6l);
        assert(fabsl(s.y.a - (x + 1.0l)) < 1e-6l);
        assert(fabsl(s.y.b - (x - 1.0l)) < 1e-6l);
        assert(fabsl(s.arr[0] - (-x)) < 1e-6l);
        assert(fabsl(s.arr[1] - (x / 2.0l)) < 1e-6l);
        assert(fabsl(s.arr[2] - (x * 2.0l)) < 1e-6l);
        assert(fabsl(s.z - (x * x)) < 1e-6l);
    }
    return EXIT_SUCCESS;
}
