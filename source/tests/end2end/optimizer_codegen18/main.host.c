/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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
#include <string.h>
#include "./definitions.h"

#define EPSILON_D 1e-6

int main(void) {
    assert(sum(0) == 0);
    assert(sum(0, 1) == 0);
    assert(sum(0, 1, 2, 3) == 0);
    assert(sum(1, 1, 2, 3) == 1);
    assert(sum(2, 1, 2, 3) == 3);
    assert(sum(3, 1, 2, 3) == 6);
    assert(sum(10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10) == 55);

    assert(fabs(sumd(0)) < EPSILON_D);
    assert(fabs(sumd(0, 1.0)) < EPSILON_D);
    assert(fabs(sumd(0, 1.0, 2.0, 3.0)) < EPSILON_D);
    assert(fabs(sumd(1, 1.0, 2.0, 3.0) - 1.0) < EPSILON_D);
    assert(fabs(sumd(2, 1.0, 2.0, 3.0) - 3.0) < EPSILON_D);
    assert(fabs(sumd(3, 1.0, 2.0, 3.0) - 6.0) < EPSILON_D);
    assert(fabs(sumd(10, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0) - 55.0) < EPSILON_D);

    assert(sum1(0) == 0);
    assert(sum1(0, (struct Struct1){{0}}) == 0);
    assert(sum1(0, (struct Struct1){{0}}, (struct Struct1){{0}}) == 0);
    assert(sum1(1, (struct Struct1){{0}}, (struct Struct1){{0}}) == 0);
    assert(sum1(1, (struct Struct1){{1, 2, 3, 4, 5}}, (struct Struct1){{0}}) == 15);
    assert(sum1(1, (struct Struct1){{1, 2, 3, 4, 5}}, (struct Struct1){{6, 7, 8, 9, 10}}) == 15);
    assert(sum1(2, (struct Struct1){{1, 2, 3, 4, 5}}, (struct Struct1){{6, 7, 8, 9, 10}}) == 55);
    assert(sum1(3, (struct Struct1){{1, 2, 3, 4, 5}}, (struct Struct1){{6, 7, 8, 9, 10}},
                (struct Struct1){{100, 0, 1}}) == 156);

    return EXIT_SUCCESS;
}
