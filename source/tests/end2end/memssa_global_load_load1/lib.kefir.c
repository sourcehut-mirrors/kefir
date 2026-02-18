/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

int test1(int *ptr) {
    return *ptr + *ptr;
}

int test2(volatile int *ptr) {
    return *ptr + *ptr;
}

int test3(_Atomic int *ptr) {
    return *ptr + *ptr;
}

int test4(int x) {
    int arr[1] = {fn(x)};
    return arr[0] * fn(-x) + arr[0];
}

int test5(int x) {
    return global1 * fn(x) + global1;
}

int test6(int x) {
    return global1 * x + global1;
}

int test7(_Atomic int *ptr) {
    int x = global1;
    int y = *ptr;
    int z = global1;
    return x + y + z;
}

int test8(_Atomic int *ptr) {
    int arr[1] = {fn(1000)};
    int x = arr[0];
    int y = *ptr;
    int z = arr[0];
    return x + y + z;
}

int test9(int *ptr) {
    int x = *ptr;
    if (x) {
        return x + *ptr;
    } else {
        return -1;
    }
}

int test10(int *ptr) {
    int x = *ptr;
    for (int i = 0; i < 10; i++) {
        x += *ptr;
    }
    return x;
}

int test11(volatile int *ptr) {
    int x = *ptr;
    for (int i = 0; i < 10; i++) {
        x += *ptr;
    }
    return x;
}

int test12(int *ptr, _Atomic int *ptr2) {
    int x = *ptr;
    for (int i = 0; i < 10; i++) {
        (void) *ptr2;
        x += *ptr;
    }
    return x;
}
