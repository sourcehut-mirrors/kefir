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
    unsigned int idx = 0;
    assert(arr[idx++] == 1);
    assert(arr[idx++] == 1);
    assert(arr[idx++] == sizeof(char));
    assert(arr[idx++] == sizeof(char));

    assert(arr[idx++] == 2);
    assert(arr[idx++] == 2);
    assert(arr[idx++] == sizeof(char));
    assert(arr[idx++] == sizeof(char));

    assert(arr[idx++] == 3);
    assert(arr[idx++] == 3);
    assert(arr[idx++] == sizeof(char));
    assert(arr[idx++] == sizeof(char));

    assert(arr[idx++] == 4);
    assert(arr[idx++] == 4);
    assert(arr[idx++] == sizeof(short));
    assert(arr[idx++] == sizeof(short));

    assert(arr[idx++] == 5);
    assert(arr[idx++] == 5);
    assert(arr[idx++] == sizeof(short));
    assert(arr[idx++] == sizeof(short));

    assert(arr[idx++] == 6);
    assert(arr[idx++] == 6);
    assert(arr[idx++] == sizeof(int));
    assert(arr[idx++] == sizeof(int));

    assert(arr[idx++] == 7);
    assert(arr[idx++] == 7);
    assert(arr[idx++] == sizeof(int));
    assert(arr[idx++] == sizeof(int));

    assert(arr[idx++] == 8);
    assert(arr[idx++] == 8);
    assert(arr[idx++] == sizeof(long));
    assert(arr[idx++] == sizeof(long));

    assert(arr[idx++] == 9);
    assert(arr[idx++] == 9);
    assert(arr[idx++] == sizeof(long));
    assert(arr[idx++] == sizeof(long));

    assert(arr[idx++] == 10);
    assert(arr[idx++] == 10);
    assert(arr[idx++] == sizeof(long long));
    assert(arr[idx++] == sizeof(long long));

    assert(arr[idx++] == 11);
    assert(arr[idx++] == 11);
    assert(arr[idx++] == sizeof(long long));
    assert(arr[idx++] == sizeof(long long));

    assert(arr[idx++] == 4);
    assert(arr[idx++] == 4);
    assert(arr[idx++] == sizeof(short));
    assert(arr[idx++] == sizeof(short));

    assert(arr[idx++] == 1);
    assert(arr[idx++] == 1);
    assert(arr[idx++] == sizeof(char));
    assert(arr[idx++] == sizeof(char));
    return EXIT_SUCCESS;
}
