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

int test1(int arg) {
    struct S1 s = {.x = arg, .y = arg + 1, .z = arg * 2};
    s.z *= -1;
    dummy(0);
    s.x++;
    return s.x + s.y + s.z;
}

int test2(int arg) {
    struct S1 s = {.x = arg, .y = arg + 1, .z = arg * 2};
    s.z *= -1;
    dummy(&s);
    s.x++;
    return s.x + s.y + s.z;
}

int test3(int arg) {
    int arr[3];
    arr[0] = arg;
    arr[1] = arg + 1;
    arr[2] = arg * 2;
    arr[2] *= -1;
    dummy(0);
    arr[0]++;
    return arr[0] + arr[1] + arr[2];
}

int test4(int arg) {
    int arr[3];
    arr[0] = arg;
    arr[1] = arg + 1;
    arr[2] = arg * 2;
    arr[2] *= -1;
    dummy(arr);
    arr[0]++;
    return arr[0] + arr[1] + arr[2];
}

int test5(int arg) {
    struct S1 s = {.x = arg, .y = arg + 1, .z = arg * 2};
    dummy(&s);
    s.z *= -1;
    s.x++;
    return s.x + s.y + s.z;
}

int test6(int arg) {
    int arr[3];
    dummy(arr);
    arr[0] = arg;
    arr[1] = arg + 1;
    arr[2] = arg * 2;
    arr[2] *= -1;
    arr[0]++;
    return arr[0] + arr[1] + arr[2];
}
