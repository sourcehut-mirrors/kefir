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
    struct S1 s;
    test1(&s);
    assert(s.a == 0);
    assert(fabs(s.b) < 1e-3);
    assert(fabs(s.c) < 1e-5);
    assert(fabsl(s.d) < 1e-7);
    assert(fabs(creal(s.e)) < 1e-3);
    assert(fabs(cimag(s.e)) < 1e-3);
    assert(fabs(creal(s.f)) < 1e-5);
    assert(fabs(cimag(s.f)) < 1e-5);
    assert(fabsl(creall(s.g)) < 1e-7);
    assert(fabsl(cimagl(s.g)) < 1e-7);
    assert(memcmp(&s.h, &(struct S2) {0}, sizeof(struct S2)) == 0);
    return EXIT_SUCCESS;
}
