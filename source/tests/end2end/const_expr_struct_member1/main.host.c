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
#include "./definitions.h"

struct Str1 structure1;

int main(void) {
    assert(arr1_1 == structure1.arr1);
    assert(arr2_1 == structure1.arr2);
    assert(arr3_1 == structure1.arr3);

    struct Str1 *structure2 = get_structure2();
    assert(arr1_2 == structure2->arr1);
    assert(arr2_2 == structure2->arr2);
    assert(arr3_2 == structure2->arr3);
    return EXIT_SUCCESS;
}
