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

int main(void) {
    assert(arr[0] == 1);
    assert(arr[1] == 2);
    assert(arr[2] == 0);
    assert(arr[3] == 3);
    assert(arr[4] == 1);
    assert(arr[5] == 2);
    assert(arr[6] == 1);

    assert(test1(0) == 1);
    assert(test1(1) == 2);
    assert(test1(2) == 0);
    assert(test1(3) == 3);
    assert(test1(4) == 1);
    assert(test1(5) == 2);
    assert(test1(6) == 1);
    return EXIT_SUCCESS;
}
