/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#include "./definitions.h"

void iarr_map(int *arr, unsigned int len, int (*map)(int)) {
    for (unsigned int i = 0; i < len; i++) {
        arr[i] = map(arr[i]);
    }
}

int iarr_reduce(int *arr, unsigned int len, int (*reduce)(int, int)) {
    int res = reduce(0, arr[0]);
    for (unsigned int i = 1; i < len; i++) {
        res = reduce(res, arr[i]);
    }
    return res;
}

int test_large(int (*fn)(int, int, int, int, int, int, int, int), int seed) {
    return 1 + 2 + fn(seed, seed + 1, seed + 2, seed + 3, seed + 4, seed + 5, seed + 6, seed + 7);
}
