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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "./definitions.h"

int main(void) {
    char arr[8] = {0};
    test(arr, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    assert(arr[0] == 1);
    test(arr, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1);
    assert(arr[0] == 3);
    test(arr, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8);
    assert(arr[0] == 3);
    assert(arr[1] == 1);
    test(arr, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10);
    assert(arr[0] == 3);
    assert(arr[1] == 5);
    return EXIT_SUCCESS;
}
