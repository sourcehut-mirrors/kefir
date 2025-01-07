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
#include <string.h>
#include "./definitions.h"

int main(void) {
    assert(arr[0][0].a == 1);
    assert(strcmp(arr[0][0].b, "Hello") == 0);
    assert(arr[0][1].a == 2);
    assert(strcmp(arr[0][1].b, "world") == 0);
    assert(arr[0][2].a == 3);
    assert(strcmp(arr[0][2].b, "!") == 0);
    assert(arr[0][3].a == 0);
    assert(arr[0][3].b == NULL);

    assert(arr[1][0].a == 1);
    assert(strcmp(arr[1][0].b, "Goodbye, world!") == 0);
    assert(arr[1][1].a == 0);
    assert(arr[1][1].b == NULL);

    assert(arr[2] == NULL);
    return EXIT_SUCCESS;
}
