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

#define EPSILON_F 1e-3

int main(void) {
    _Complex float r = get();
    assert(fabs(crealf(r)) < EPSILON_F);
    assert(fabs(cimagf(r) - 3.14159f) < EPSILON_F);
    r = get2();
    assert(fabs(crealf(r)) < EPSILON_F);
    assert(fabs(cimagf(r) + 3.14159f) < EPSILON_F);

    assert(fabs(crealf(arr[0])) < EPSILON_F);
    assert(fabs(cimagf(arr[0]) - 2.71828f) < EPSILON_F);
    assert(fabs(crealf(arr[1])) < EPSILON_F);
    assert(fabs(cimagf(arr[1]) + 2.71828f) < EPSILON_F);
    return EXIT_SUCCESS;
}
