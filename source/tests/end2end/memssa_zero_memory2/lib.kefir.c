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

int test1(int a) {
    struct S1 x = {a, a * 2};
    return callback1(x);
}

int test2(int a) {
    struct S1 x = {.b = a * 2};
    return callback1(x);
}

int test3(int a) {
    struct S1 x = {};
    x.a = a;
    x.b = a * 2;
    return callback1(x);
}

int test4(int a) {
    struct S1 x[] = {{1, 2}, {3, 4}, {a, a * 2}};
    return callback1(x[2]);
}

int test5(int a) {
    struct S1 x[] = {{1, 2}, {}, {a, a * 2}};
    x[1].a = 3;
    x[1].b = 4;
    return callback1(x[2]);
}

int test6(int a) {
    struct S1 x[] = {{1, 2}, {}, {a, a * 2}};
    return callback1(x[2]);
}

int test7(int a) {
    struct S1 x[] = {{1, 2}, {}, {a, a * 2}};
    x[1].a = 3;
    x[1].b = 4;
    if (a)
        return callback1(x[2]);
    return 0;
}

int test8(int a) {
    struct S1 x[] = {{1, 2}, {}, {a, a * 2}};
    if (a) {
        x[1].a = 3;
        x[1].b = 4;
        return callback1(x[2]);
    } else {
        x[1].a = 3;
        x[1].b = 4;
        return 0;
    }
}

int test9(int a) {
    struct S1 x[] = {{1, 2}, {3, 4}, {.b = a * 2}};
    return callback1(x[2]);
}

int test10(int a) {
    struct S1 x[] = {{1, 2}, {3, 4}, {.b = a * 2}};
    x[2].a = a;
    return callback1(x[2]);
}
