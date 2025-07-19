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
    assert(a.arr[0] == 1);
    assert(a.arr[1] == 2);
    assert(a.arr[2] == 3);
    assert(fabs(a.b - 3.14f) < 1e-5);
    assert(a.c == &a);
    assert(a.d.x == 1000);

    assert(getone() == 1);
    assert(getzero() == 0);
    assert(b[0] == 100);
    assert(b[1] == 200);
    assert(b[2] == -300);
    assert(b[3] == 400);
    return EXIT_SUCCESS;
}
