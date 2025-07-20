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
    struct S1 s1 = get1();

    assert(s1.a == 58472);
    assert(strcmp(s1.b, "Test123") == 0);
    assert(MASK(s1.c, 41) == 0xe4d4);
    assert(fabs(s1.d - 5.42482) < 1e-6);
    assert(fabs(creal(s1.e)) < 1e-5);
    assert(fabs(cimag(s1.e) + 5838.1f) < 1e-5);
    assert(s1.f[0] == 1);
    assert(s1.f[1] == 2);
    assert(s1.f[2] == -1);
    assert(s1.g.a == 50);
    assert(s1.g.b == -0x43edca);
    assert(fabs(s1.g.c - 5.424f) < 1e-5);
    return EXIT_SUCCESS;
}
