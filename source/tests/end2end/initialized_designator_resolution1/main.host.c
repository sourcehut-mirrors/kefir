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
#include "./definitions.h"

int main(void) {
    assert(s1.arr[0] == 1234);
    assert(s1.arr[1] == 0);
    assert(s1.arr[2] == 0);
    assert(s1.arr[3] == 0);
    assert(s1.arr[4] == 0);
    assert(s1.arr3[0] == 'H');
    assert(s1.arr3[1] == 0);
    assert(s1.arr3[2] == 0);
    assert(s1.arr3[3] == 0);
    assert(s1.arr3[4] == 0);
    assert(s1.arr4[0] == 0xcac0);
    assert(s1.arr4[1] == 0);

    assert(s2.arr[0].arr[0] == 9876);
    assert(s2.arr[0].arr[1] == 0);
    assert(s2.arr[0].arr[2] == 0);
    assert(s2.arr[0].arr[3] == 0);
    assert(s2.arr[0].arr[4] == 0);
    assert(s2.arr[0].arr3[0] == 0);
    assert(s2.arr[0].arr3[1] == 0);
    assert(s2.arr[0].arr3[2] == 0);
    assert(s2.arr[0].arr3[3] == 0);
    assert(s2.arr[0].arr3[4] == 0);
    assert(s2.arr[0].arr4[0] == 0);
    assert(s2.arr[0].arr4[1] == 0);
    assert(s2.arr[1].arr[0] == 0);
    assert(s2.arr[1].arr[1] == 0);
    assert(s2.arr[1].arr[2] == 0);
    assert(s2.arr[1].arr[3] == 0);
    assert(s2.arr[1].arr[4] == 0);
    assert(s2.arr[1].arr3[0] == 0);
    assert(s2.arr[1].arr3[1] == 0);
    assert(s2.arr[1].arr3[2] == 0);
    assert(s2.arr[1].arr3[3] == 0);
    assert(s2.arr[1].arr3[4] == 0);
    assert(s2.arr[1].arr4[0] == 0);
    assert(s2.arr[1].arr4[1] == 0);
    assert(s2.arr2[0] == 1098);
    assert(s2.arr2[1] == 0);
    return EXIT_SUCCESS;
}
