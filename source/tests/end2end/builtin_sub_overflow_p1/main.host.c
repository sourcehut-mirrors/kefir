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
    assert(!test1(0, INT32_MAX));
    assert(!test1(-1, INT32_MAX));
    assert(test1(-2, INT32_MAX));
    assert(!test1(-1, INT32_MIN));
    assert(test1(0, INT32_MIN));
    assert(!test1(INT32_MIN, -INT32_MAX));

    assert(!test2(0, INT32_MIN));
    assert(!test2(INT32_MAX, INT32_MIN));
    assert(test2(0, INT32_MAX));
    assert(test2(INT32_MIN, -INT32_MAX));

    assert(!test3(65, 2));
    assert(test3(65, 1));
    assert(!test3(0, 64));
    assert(test3(0, 65));

    assert(!test4(60, -4));
    assert(!test4(7, -120));
    assert(test4(7, -121));
    assert(test4(0, 1));
    return EXIT_SUCCESS;
}
