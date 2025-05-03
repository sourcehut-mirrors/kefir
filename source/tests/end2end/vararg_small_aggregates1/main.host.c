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
#include "./definitions.h"

#define EPSILON_F 1e-3
#define EPSILON_D 1e-6

int main(void) {
    assert(sum(0) == 0);
    assert(sum(0, (struct S1) {1, 2}) == 0);
    assert(sum(1, (struct S1) {1, 2}) == 2);
    assert(sum(2, (struct S1) {1, 2}, (struct S1) {3, 4}) == 14);
    assert(sum(3, (struct S1) {1, 2}, (struct S1) {3, 4}, (struct S1) {5, 6}) == 44);
    return EXIT_SUCCESS;
}
