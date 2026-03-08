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

int test1(int x) {
    int arr[] = {x, x + 1, x + 2, x + 3};
    return callback1(arr);
}

int test2(int x) {
    int arr[] = {x, x + 1, [3] = x + 3};
    return callback1(arr);
}

int test3(int x) {
    int arr[4];
    arr[0] = x;
    arr[1] = x + 1;
    arr[2] = x + 2;
    arr[3] = x + 3;
    return callback1(arr);
}

int test4(int x) {
    int arr[4] = {};
    arr[0] = x;
    arr[1] = x + 1;
    arr[2] = x + 2;
    arr[3] = x + 3;
    return callback1(arr);
}

int test5(int x) {
    int arr[4] = {};
    arr[0] = x;
    arr[1] = x + 1;
    arr[3] = x + 3;
    return callback1(arr);
}

int test6(int x) {
    int arr[4] = {};
    arr[0] = x;
    arr[1] = x + 1;
    arr[2] = x + 2;
    arr[3] = x + 3;
    if (x)
        return callback1(arr);
    else
        return 0;
}

int test7(int x) {
    int arr[4] = {};
    arr[0] = x;
    arr[1] = x + 1;
    arr[3] = x + 3;
    if (x) {
        arr[2] = x + 2;
        return callback1(arr);
    } else {
        arr[2] = x + 2;
        return 0;
    }
}

int test8(int x) {
    struct S1 s1 = {x, x * 2};
    return callback2(s1);
}

int test9(int x) {
    struct S1 s1 = {.b = x * 2};
    return callback2(s1);
}

int test10(int x) {
    struct S1 s1 = {.b = x * 2};
    s1.a = x;
    return callback2(s1);
}

int test11(int x) {
    struct S1 s1 = {.b = x * 2};
    s1.a = x;
    if (x)
        return callback2(s1);
    else
        return 0;
}

int test12(int x) {
    struct S1 s1 = {.b = x * 2};
    if (x) {
        s1.a = x;
        return callback2(s1);
    } else {
        s1.a = x;
        return 0;
    }
}
