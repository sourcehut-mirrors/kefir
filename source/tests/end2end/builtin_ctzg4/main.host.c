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
#include <limits.h>
#include "./definitions.h"

int main(void) {
    int i = 0;
    assert(arr[i++] == -1000);
    assert(arr[i++] == -1001);
    assert(arr[i++] == 0);
    assert(arr[i++] == 1);
    assert(arr[i++] == 0);
    assert(arr[i++] == 6);
    assert(arr[i++] == 7);

    assert(arr[i++] == -2000);
    assert(arr[i++] == -2001);
    assert(arr[i++] == 0);
    assert(arr[i++] == 1);
    assert(arr[i++] == 0);
    assert(arr[i++] == 8);
    assert(arr[i++] == 14);
    assert(arr[i++] == 15);

    assert(arr[i++] == -3000);
    assert(arr[i++] == -3001);
    assert(arr[i++] == 0);
    assert(arr[i++] == 1);
    assert(arr[i++] == 0);
    assert(arr[i++] == 8);
    assert(arr[i++] == 16);
    assert(arr[i++] == 30);
    assert(arr[i++] == 31);

    assert(arr[i++] == -4000);
    assert(arr[i++] == 0);
    assert(arr[i++] == 1);
    assert(arr[i++] == 0);
    assert(arr[i++] == 8);
    assert(arr[i++] == 16);
    assert(arr[i++] == 32);
    assert(arr[i++] == 62);
    assert(arr[i++] == 63);
    return EXIT_SUCCESS;
}
