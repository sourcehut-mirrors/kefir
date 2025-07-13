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
#include <math.h>
#include "./definitions.h"

int main(void) {
    assert(b == 4);
    assert(a[0] == 100);
    assert(a[1] == 20);
    assert(a[2] == 3);
    assert(a[3] == -4);

    assert(d == 10);
    assert(c[0] == -1);
    assert(c[1] == -2);
    assert(c[2] == -3);
    for (int i = 3; i < 10; i++) {
        assert(c[i] == 0);
    }

    assert(f == 3);
    assert(fabs(e[0].a + 3.14f) < 1e-5);
    assert(e[0].b == 4);
    assert(fabs(e[1].a) < 1e-5);
    assert(e[1].b == 0);
    assert(fabs(e[2].a) < 1e-5);
    assert(e[2].b == 0);
    return EXIT_SUCCESS;
}
