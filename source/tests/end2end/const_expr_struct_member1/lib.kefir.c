/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#include "./definitions.h"

static struct Str1 structure2;

int *arr1_1 = structure1.arr1, *arr1_2 = structure2.arr1;
long *arr2_1 = (&structure1)->arr2, *arr2_2 = (&structure2)->arr2;
char *arr3_1 = structure1.arr3, *arr3_2 = structure2.arr3;

extern struct Str1 *get_structure2(void) {
    return &structure2;
}
