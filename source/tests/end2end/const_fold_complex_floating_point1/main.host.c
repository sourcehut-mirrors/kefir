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

int main(void) {
    assert(fabs(crealf(neg32()) + 1.1f) < 1e-5);
    assert(fabs(cimagf(neg32()) + 3.14f) < 1e-5);
    assert(fabs(creal(neg64()) - 1.1) < 1e-7);
    assert(fabs(cimag(neg64()) - 3.14) < 1e-7);
    assert(fabsl(creall(neg80()) + 1.1l) < 1e-9);
    assert(fabsl(cimagl(neg80()) - 3.14l) < 1e-9l);

    assert(fabs(crealf(add32()) - (1.1f + 2.718f)) < 1e-5);
    assert(fabs(cimagf(add32()) - (3.14f - 1.1f)) < 1e-5);
    assert(fabs(creal(add64()) - (2.718 - 1.1)) < 1e-7);
    assert(fabs(cimag(add64()) - (-3.14 + 1.1)) < 1e-7);
    assert(fabsl(creall(add80()) - (1.1l + 2.718l)) < 1e-9l);
    assert(fabsl(cimagl(add80()) - (3.14l - 1.1l)) < 1e-9l);

    assert(fabs(crealf(sub32()) - (1.1f - 2.718f)) < 1e-5);
    assert(fabs(cimagf(sub32()) - (3.14f + 1.1f)) < 1e-5);
    assert(fabs(creal(sub64()) - (-2.718 - 1.1)) < 1e-7);
    assert(fabs(cimag(sub64()) - (-3.14 - 1.1)) < 1e-7);
    assert(fabsl(creall(sub80()) - (1.1l - 2.718l)) < 1e-9l);
    assert(fabsl(cimagl(sub80()) - (3.14l + 1.1l)) < 1e-9l);

#define MUL_REAL(a, b, c, d) ((a) * (c) - (b) * (d))
#define MUL_IMAG(a, b, c, d) ((a) * (d) + (b) * (c))
    assert(fabs(crealf(mul32()) - MUL_REAL(1.1f, 3.14f, 2.718f, -1.1f)) < 1e-5);
    assert(fabs(cimagf(mul32()) - MUL_IMAG(1.1f, 3.14f, 2.718f, -1.1f)) < 1e-5);
    assert(fabs(creal(mul64()) - MUL_REAL(-1.1, -3.14, 2.718, 1.1)) < 1e-7);
    assert(fabs(cimag(mul64()) - MUL_IMAG(-1.1, -3.14, 2.718, 1.1)) < 1e-7);
    assert(fabsl(creall(mul80()) - MUL_REAL(1.1L, 3.14L, 2.718L, -1.1L)) < 1e-8l);
    assert(fabsl(cimagl(mul80()) - MUL_IMAG(1.1L, 3.14L, 2.718L, -1.1L)) < 1e-8l);

#define DIV_REAL(a, b, c, d) (((a) * (c) + (b) * (d)) / ((c) * (c) + (d) * (d)))
#define DIV_IMAG(a, b, c, d) (((b) * (c) - (a) * (d)) / ((c) * (c) + (d) * (d)))
    assert(fabs(crealf(div32()) - DIV_REAL(1.1f, 3.14f, 2.718f, -1.1f)) < 1e-5);
    assert(fabs(cimagf(div32()) - DIV_IMAG(1.1f, 3.14f, 2.718f, -1.1f)) < 1e-5);
    assert(fabs(creal(div64()) - DIV_REAL(-1.1, -3.14, 2.718, 1.1)) < 1e-7);
    assert(fabs(cimag(div64()) - DIV_IMAG(-1.1, -3.14, 2.718, 1.1)) < 1e-7);
    assert(fabsl(creall(div80()) - DIV_REAL(1.1L, 3.14L, 2.718L, -1.1L)) < 1e-8l);
    assert(fabsl(cimagl(div80()) - DIV_IMAG(1.1L, 3.14L, 2.718L, -1.1L)) < 1e-8l);
    return EXIT_SUCCESS;
}
