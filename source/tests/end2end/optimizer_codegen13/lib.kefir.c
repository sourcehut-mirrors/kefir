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

#include "./definitions.h"

int test1(int n, int (*callback)(int *, int)) {
    int array[n];
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
    return callback(array, n);
}

int test2(int n, int (*callback)(int *, int)) {
    int *array = __builtin_alloca(sizeof(int) * n);
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
    return callback(array, n);
}

void test3(int n) {
    for (int i = 0; i < n; i++) {
        int arr[4096 + n];
        {
            char buf[n];
            dummy_fn(arr, buf);
        }
        for (int j = 0; j < 12; j++) {
            char buf[j << 2 + 1];
            dummy_fn(arr, buf);
        }
    }
}
