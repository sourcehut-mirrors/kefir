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
    assert(arr[i++] == 0);
    assert(arr[i++] == 1);
    assert(arr[i++] == 1);
    assert(arr[i++] == 2);
    assert(arr[i++] == 1);
    assert(arr[i++] == 1);
    assert(arr[i++] == 1);
    assert(arr[i++] == 3);
    assert(arr[i++] == (sizeof(char) * CHAR_BIT - 2));
    assert(arr[i++] == (sizeof(char) * CHAR_BIT - 1));
    assert(arr[i++] == (sizeof(char) * CHAR_BIT - 1));
    assert(arr[i++] == (sizeof(char) * CHAR_BIT));

    assert(arr[i++] == 0);
    assert(arr[i++] == 1);
    assert(arr[i++] == 1);
    assert(arr[i++] == 2);
    assert(arr[i++] == 1);
    assert(arr[i++] == 1);
    assert(arr[i++] == 1);
    assert(arr[i++] == 3);
    assert(arr[i++] == (sizeof(short) * CHAR_BIT - 2));
    assert(arr[i++] == (sizeof(short) * CHAR_BIT - 1));
    assert(arr[i++] == (sizeof(short) * CHAR_BIT - 1));
    assert(arr[i++] == (sizeof(short) * CHAR_BIT));

    assert(arr[i++] == 0);
    assert(arr[i++] == 1);
    assert(arr[i++] == 1);
    assert(arr[i++] == 2);
    assert(arr[i++] == 1);
    assert(arr[i++] == 1);
    assert(arr[i++] == 1);
    assert(arr[i++] == 3);
    assert(arr[i++] == (sizeof(int) * CHAR_BIT - 2));
    assert(arr[i++] == (sizeof(int) * CHAR_BIT - 1));
    assert(arr[i++] == (sizeof(int) * CHAR_BIT - 1));
    assert(arr[i++] == (sizeof(int) * CHAR_BIT));

    assert(arr[i++] == 0);
    assert(arr[i++] == 1);
    assert(arr[i++] == 1);
    assert(arr[i++] == 2);
    assert(arr[i++] == 1);
    assert(arr[i++] == 1);
    assert(arr[i++] == 1);
    assert(arr[i++] == 3);
    assert(arr[i++] == (sizeof(long) * CHAR_BIT - 2));
    assert(arr[i++] == (sizeof(long) * CHAR_BIT - 1));
    assert(arr[i++] == (sizeof(long) * CHAR_BIT - 1));
    assert(arr[i++] == (sizeof(long) * CHAR_BIT));
    return EXIT_SUCCESS;
}
