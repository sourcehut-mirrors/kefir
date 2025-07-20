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
    assert(*getx() == 1000);
    assert(fabs(*gety() - 3.1415926f) < 1e-5);
    assert(fabs(creal(*getz()) - 100.0f) < 1e-5);
    assert(fabs(cimag(*getz()) + 200.1f) < 1e-5);
    assert(*get_w() == getx());
    assert(geta()->a == 100);
    assert(geta()->b == 0xcafeb00dull);
    assert(strcmp(geta()->c, "Hello, world!") == 0);
    return EXIT_SUCCESS;
}
