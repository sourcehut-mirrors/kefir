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

static int flag = 0;
int callback() {
    return flag++;
}

int main(void) {
    assert(!test1(INT32_MAX, INT32_MIN));
    assert(test1(INT32_MAX, INT32_MAX));
    assert(test1(INT32_MIN, INT32_MIN));
    assert(test1(INT32_MAX, 1));
    assert(flag == 4);

    assert(test2(INT32_MAX, INT32_MIN));
    assert(!test2(INT32_MAX, INT32_MAX));
    assert(test2(INT32_MIN, INT32_MIN));
    assert(!test2(INT32_MAX, 1));

    assert(!test3(60, 3));
    assert(test3(60, 4));
    assert(!test3(-65, 1));
    assert(test3(-64, -1));

    assert(!test4(60, 4));
    assert(!test4(120, 7));
    assert(test4(120, 8));
    assert(test4(-65, 1));
    assert(!test4(-65, 65));
    return EXIT_SUCCESS;
}
