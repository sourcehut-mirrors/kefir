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
    assert(a == s1.b.d.arr);
    assert(fabs(b - 200.01) < 1e-6);
    assert(c == -250);
    assert(fabs(crealf(d) - 100.3) < 1e-5);
    assert(fabs(cimagf(d)) < 1e-5);
    assert(e == &a);
    assert(f == NULL);
    assert(g == 0);
    assert(fabsl(h) < 1e-7l);
    assert(fabs(creal(i)) < 1e-6);
    assert(fabs(cimag(i)) < 1e-6);
    assert(strncmp(j, "Hello", 5) == 0);
    return EXIT_SUCCESS;
}
