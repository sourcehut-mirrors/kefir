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
#include <math.h>
#include <string.h>
#include "./definitions.h"

int map1(int x) {
    return -x;
}

int map2(int x) {
    return x / 2;
}

int reduce1(int x, int y) {
    return x + y;
}

int fn1(int a, int b, int c, int d, int e, int f, int g, int h) {
    int res = a + 1 == b && b + 1 == c && c + 1 == d && d + 1 == e && e + 1 == f && f + 1 == g && g + 1 == h;
    if (!res) {
        abort();
    }
    return a + b + c + d + e + f + g + h;
}

int main(void) {
    for (int i = -100; i <= 100; i++) {
        int arr[6] = {i, i + 1, i + 2, i + 3, i + 4, i + 5};
        iarr_map(arr, sizeof(arr) / sizeof(arr[0]), map1);
        assert(arr[0] == -i && arr[1] == -(i + 1) && arr[2] == -(i + 2) && arr[3] == -(i + 3) && arr[4] == -(i + 4) &&
               arr[5] == -(i + 5));
        iarr_map(arr, sizeof(arr) / sizeof(arr[0]), map2);
        assert(arr[0] == -i / 2 && arr[1] == -(i + 1) / 2 && arr[2] == -(i + 2) / 2 && arr[3] == -(i + 3) / 2 &&
               arr[4] == -(i + 4) / 2 && arr[5] == -(i + 5) / 2);
        assert(iarr_reduce(arr, sizeof(arr) / sizeof(arr[0]), reduce1) ==
               (-i / 2 - (i + 1) / 2 - (i + 2) / 2 - (i + 3) / 2 - (i + 4) / 2 - (i + 5) / 2));
        assert(test_large(fn1, i) == (1 + 2 + i * 8 + 1 + 2 + 3 + 4 + 5 + 6 + 7));
    }
    return EXIT_SUCCESS;
}
