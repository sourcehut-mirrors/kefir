/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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
    assert(fabs(creal(get(0, 0, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get(0, 1, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get(1, 0, 3.14f, 2.71f)) - 2.71f) < 1.0e-5f);
    assert(fabs(creal(get(1, 1, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);

    assert(fabs(creal(get2(0, 0, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get2(0, 1, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get2(1, 2, 3.14f, 2.71f)) - 2.71f) < 1.0e-5f);
    assert(fabs(creal(get2(1, 0, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);

    assert(fabs(creal(get3(0, 0, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get3(0, 1, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get3(1, 3, 3.14f, 2.71f)) - 2.71f) < 1.0e-5f);
    assert(fabs(creal(get3(1, 4, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);

    assert(fabs(creal(get4(0, 0, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get4(0, 1, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get4(1, 3, 3.14f, 2.71f)) - 2.71f) < 1.0e-5f);
    assert(fabs(creal(get4(1, 4, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);

    assert(fabs(creal(get5(0, 0, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get5(0, 1, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get5(1, -3, 3.14f, 2.71f)) - 2.71f) < 1.0e-5f);
    assert(fabs(creal(get5(1, -4, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);

    assert(fabs(creal(get6(0, 0, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get6(0, 1, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get6(1, -3, 3.14f, 2.71f)) - 2.71f) < 1.0e-5f);
    assert(fabs(creal(get6(1, -5, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);

    assert(fabs(creal(get7(0, 0, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get7(0, 1, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get7(1, 30, 3.14f, 2.71f)) - 2.71f) < 1.0e-5f);
    assert(fabs(creal(get7(1, 31, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);

    assert(fabs(creal(get8(0, 0, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get8(0, 1, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get8(1, 39, 3.14f, 2.71f)) - 2.71f) < 1.0e-5f);
    assert(fabs(creal(get8(1, 40, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);

    assert(fabs(creal(get9(0, 0, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get9(0, 1, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get9(1, 100, 3.14f, 2.71f)) - 2.71f) < 1.0e-5f);
    assert(fabs(creal(get9(1, 99, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);

    assert(fabs(creal(get10(0, 0, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get10(0, 1, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    assert(fabs(creal(get10(1, 201, 3.14f, 2.71f)) - 2.71f) < 1.0e-5f);
    assert(fabs(creal(get10(1, 200, 3.14f, 2.71f)) - 3.14f) < 1.0e-5f);
    return EXIT_SUCCESS;
}
