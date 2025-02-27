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
    assert(select1(1, 100, -100) == 100);
    assert(select1(0, 100, -100) == -100);
    assert(select2(1, 1000, -200) == 1000);
    assert(select2(0, 1000, -200) == -200);
    assert(fabs(select3(1, 3.14f, 2.71) - 3.14f) < 1e-3);
    assert(fabs(select3(0, 3.14f, 2.71) - 2.71) < 1e-3);
    assert(fabs(select4(1, 3.14, 2.71) - 3.14) < 1e-5);
    assert(fabs(select4(0, 3.14, 2.71) - 2.71) < 1e-5);
    assert(fabsl(select5(1, 3.14L, 2.71L) - 3.14L) < 1e-7);
    assert(fabsl(select5(0, 3.14L, 2.71L) - 2.71L) < 1e-7);

    _Complex float cf1 = select6(1, CMPLXF(1, 2), CMPLXF(3, 4));
    assert(fabs(creal(cf1) - 1) < 1e-3);
    assert(fabs(cimag(cf1) - 2) < 1e-3);
    _Complex float cf2 = select6(0, CMPLXF(1, 2), CMPLXF(3, 4));
    assert(fabs(creal(cf2) - 3) < 1e-3);
    assert(fabs(cimag(cf2) - 4) < 1e-3);

    _Complex double cd1 = select7(1, CMPLXF(5, 6), CMPLXF(7, 8));
    assert(fabs(creal(cd1) - 5) < 1e-5);
    assert(fabs(cimag(cd1) - 6) < 1e-5);
    _Complex double cd2 = select7(0, CMPLXF(5, 6), CMPLXF(7, 8));
    assert(fabs(creal(cd2) - 7) < 1e-5);
    assert(fabs(cimag(cd2) - 8) < 1e-5);

    _Complex long double cld1 = select8(1, CMPLXF(-1, 1), CMPLXF(10, 20));
    assert(fabsl(creall(cld1) + 1) < 1e-5);
    assert(fabsl(cimagl(cld1) - 1) < 1e-5);
    _Complex long double cld2 = select8(0, CMPLXF(-1, 1), CMPLXF(10, 20));
    assert(fabsl(creall(cld2) - 10) < 1e-5);
    assert(fabsl(cimagl(cld2) - 20) < 1e-5);

    struct S1 s1 = select9(1, (struct S1) {"HELLO"}, (struct S1) {"GOODBYE"});
    assert(strcmp(s1.buf, "HELLO") == 0);
    struct S1 s2 = select9(0, (struct S1) {"HELLO"}, (struct S1) {"GOODBYE"});
    assert(strcmp(s2.buf, "GOODBYE") == 0);
    return EXIT_SUCCESS;
}
