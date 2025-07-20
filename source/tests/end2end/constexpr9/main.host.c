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

int main(void) {
#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))
    assert(MASK(get_a(), 42) == MASK(0xcafebabeull, 42));

    struct S1 b = get_b();
    assert(b.arr[0] == 0xeeeeffff00001111ull);
    assert(b.arr[1] == 0xaaaabbbbccccddddull);
    assert(MASK(b.arr[2], 22) == 5);

    assert(MASK(*(unsigned long *) get_c(), 42) == MASK(0xcafebabeull, 42));
    assert(get_d().a == -1005);
    assert(fabs(get_d().b - 4.3238f) < 1e-5);
    assert(strcmp(get_d().c, "Goodbye") == 0);
    return EXIT_SUCCESS;
}
