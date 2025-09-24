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
#include "./definitions.h"

_Complex long double a = 0.0L, b = 0.0L, c = 0.0L;

int main(void) {
    a = 1.0L;
    b = -2.0L * I;
    c = 4.0L + 10.0L * I;

    test1();
    assert(fabsl(creall(a) - 5.0L) < 1e-9L);
    assert(fabsl(cimagl(a) - 8.0L) < 1e-9L);

    b = 20.0L + 1.0L * I;
    c = -5.0L + -5.0L * I;
    test1();
    assert(fabsl(creall(a) - 20.0L) < 1e-9L);
    assert(fabsl(cimagl(a) - 4.0L) < 1e-9L);
    return EXIT_SUCCESS;
}
