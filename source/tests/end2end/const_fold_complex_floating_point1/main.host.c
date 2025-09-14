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
    return EXIT_SUCCESS;
}
