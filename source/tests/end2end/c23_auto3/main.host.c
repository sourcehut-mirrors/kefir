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
    assert(a == 0xcafebabe0l);
    assert(fabs(b - 3.14159f) < 1e-5);
    assert(fabs(creal(c)) < 1e-5);
    assert(fabs(cimag(c) + 2.71) < 1e-5);
    assert(d[0] == 1);
    assert(d[1] == 2);
    assert(d[2] == 3);
    assert(d[3] == 4);
    assert(d[4] == -5);
    assert(e.a == 100);
    assert(fabs(e.b - 200.2) < 1e-6);
    assert(e.c == 300);
    assert(f->a == -300);
    assert(fabs(f->b + 200.2) < 1e-6);
    assert(f->c == -100);
    return EXIT_SUCCESS;
}
