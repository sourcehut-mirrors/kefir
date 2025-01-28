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
    assert(((int *) ptr1)[0] == 1);
    assert(((int *) ptr1)[1] == 2);
    assert(((int *) ptr1)[2] == 3);
    assert(((int *) ptr1)[3] == 4);
    assert(((int *) ptr1)[4] == 5);
    assert(*ptr2 == ptr1);
    assert(scalar1 == -1234);
    assert(fabs(scalar2 - 3.14159f) < 1e-3);
    assert(fabs(creal(complex1) - 2.1828) < 1e-5);
    assert(fabs(cimag(complex1)) < 1e-5);
    return EXIT_SUCCESS;
}
