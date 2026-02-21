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

int test1(int x, int y) {
    int arr[100];
    arr[90] = 0;
    arr[50] = x;
    for (int i = 0; i < y; i++)
        arr[90] += arr[50];
    return arr[90];
}

int test2(int x, int y) {
    int arr[100] = {0};
    arr[50] = x;
    for (int i = 0; i < y; i++)
        arr[90] += arr[50];
    return arr[90];
}

int test3(int x, int y) {
    volatile int arr[100];
    arr[90] = 0;
    arr[50] = x;
    for (int i = 0; i < y; i++)
        arr[90] += arr[50];
    return arr[90];
}

int test4(int x, int y) {
    _Atomic int arr[100];
    arr[90] = 0;
    arr[50] = x;
    for (int i = 0; i < y; i++)
        arr[90] += arr[50];
    return arr[90];
}

int test5(int x, int y) {
    static int *leak;
    int arr[100];
    leak = arr;
    arr[90] = 0;
    arr[50] = x;
    for (int i = 0; i < y; i++)
        arr[90] += arr[50];
    return arr[90];
}

int test6(int x, int y) {
    struct point {
        int x, y;
    } p;
    p.x = y;
    p.y = x;
    for (int i = 0; i < 10; i++)
        p.x ^= p.y;
    return (-p.x) ^ p.y;
}

int test7(int x, int y) {
    union {
        int x;
        int y[1];
    } p;
    p.x = -y;
    p.y[0] ^= x;
    for (int i = 0; i < 10; i++)
        p.x ^= 0xc0c0;
    return p.y[0];
}

int test8() {
    struct point {
        int x, y;
    } p;
    for (int i = 0; i < 10; i++)
        p.x ^= p.y;
    return (-p.x) ^ p.y;
}

float test9(int x) {
    union {
        int x;
        float y;
    } p;
    p.x = x;
    return p.y;
}

float test10(int x) {
    union {
        int x;
        float y;
    } p;
    for (int i = 0; i < 10; i++)
        p.x = x;
    return p.y;
}
