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

struct S1 *gs1 = &(static struct S1) {100, 200, 300, 400, 500, 600, 700, 800};

struct S1 *get1(void) {
    return &(static struct S1) {1, 2, 3, 4, 5, 6, 7, 8};
}

struct S1 *get2(void) {
    return &(static _Thread_local struct S1) {8, 7, 6, 5, 4, 3, 2, 1};
}

struct S1 get3(void) {
    return (auto struct S1) {-1, -2, -3, -4, -5, -6, -7, -8};
}

struct S1 get4(void) {
    return (register struct S1) {-8, -7, -6, -5, -4, -3, -2, -1};
}

struct S1 *get5(int c) {
    return c ? &(static struct S1) {-100, -200, -300, -400, -500, -600, -700, -800}
             : &(static struct S1) {-800, -700, -600, -500, -400, -300, -200, -100};
}

int test1(int x[(int) {100}]);
int test1(int x[(int) {100}]) {
    return sizeof(x);
}
