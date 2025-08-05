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
#include <stdint.h>
#include "./definitions.h"

int main(void) {
    assert(!test1(INT32_MAX / 16, 16));
    assert(test1(INT32_MAX / 16, 17));
    assert(!test1(2, INT32_MIN / 2));
    assert(test1(2, -(INT32_MIN / 2)));

    assert(!test2(INT32_MAX / 16, 16));
    assert(!test2(INT32_MAX / 16, 17));
    assert(!test2(UINT32_MAX / 16, 16));
    assert(test2(UINT32_MAX / 16, 17));
    assert(!test2(INT32_MIN, -1));
    assert(test2(INT32_MIN, 1));

    assert(!test3(7, 9));
    assert(test3(8, 8));
    assert(!test3(-7, 9));
    assert(!test3(8, -8));
    assert(test3(5, -13));

    assert(!test4(7, 9));
    assert(!test4(8, 8));
    assert(!test4(42, 3));
    assert(test4(64, 2));
    assert(test4(1, -1));
    assert(!test4(-127, -1));
    assert(test4(-128, -1));
    return EXIT_SUCCESS;
}
