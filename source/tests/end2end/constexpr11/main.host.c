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
    assert(get0() == 12345);

    struct S1 s1 = get1();
    assert(s1.a == -12345);

    struct S2 s2 = get2();
    assert(s2.a == -12345);
    assert(s2.b == 0xcafe);

    struct S3 s3 = get3();
    assert(s3.a == -12345);
    assert(s3.b == 0xcafe);
    assert(fabs(s3.c - 4.532) < 1e-6);

    struct S4 s4 = get4();
    assert(s4.a == -12345);
    assert(s4.b == 0xcafe);
    assert(fabs(s4.c - 4.532) < 1e-6);
    assert(s4.d == NULL);

    struct S5 s5 = get5();
    assert(s5.a == -12345);
    assert(s5.b == 0xcafe);
    assert(fabs(s5.c - 4.532) < 1e-6);
    assert(s5.d == NULL);
    assert(s5.e == -1);

    _Complex long double x = getx();
    assert(fabsl(creall(x) - 3.14L) < 1e-7L);
    assert(fabsl(cimagl(x) + 2.71L) < 1e-7L);
    return EXIT_SUCCESS;
}
