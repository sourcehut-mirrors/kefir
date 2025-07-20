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
#include <complex.h>
#include <string.h>
#include "./definitions.h"

int main(void) {
    assert(a.a == 100);
    assert(fabs(a.b - 3.14159f) < 1e-5);
    assert(fabs(creal(a.c) - 384.0) < 1e-6);
    assert(fabs(cimag(a.c) + 4.564) < 1e-6);
    assert(a.d[0] == 0xcafe);
    assert(a.d[1] == 0xbabe);
    assert(strcmp(a.x, "Hello") == 0);

    assert(b.b.a == ~100);
    assert(fabs(b.b.b + 3.14159f) < 1e-5);
    assert(fabs(creal(b.b.c) + 384.0) < 1e-6);
    assert(fabs(cimag(b.b.c) - 4.564) < 1e-6);
    assert(b.b.d[0] == ~0xcafe);
    assert(b.b.d[1] == ~0xbabe);
    assert(strcmp(b.b.x, "Goodbye") == 0);
    return EXIT_SUCCESS;
}
