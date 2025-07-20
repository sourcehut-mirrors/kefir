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

constexpr struct X x = {1000, 2000, .c = -5, .e = "Hello"};

enum A {
    A1 = (constexpr int) {-100},
    A2 = (constexpr int) {3.14},
    A3 = (constexpr int) {5.0 - 4.0i},
    A4 = (constexpr int) {(int) (void *) 0},
    A5 = (constexpr int) {sizeof("hello")},
    A6 = (constexpr int) {x.c}
};

static_assert(A1 == -100);
static_assert(A2 == 3);
static_assert(A3 == 5);
static_assert(A4 == 0);
static_assert(A5 == 6);
static_assert(A6 == -5);

constexpr const struct X *x2 = &(constexpr struct X) {.b = ~123};

const struct X *getx2() {
    return x2;
}

const struct X *getx3() {
    return &(constexpr static struct X) {.a = 0xcafebadll, .e = "hehe"};
}

struct Y test2(void) {
    return (constexpr struct Y) {A1, A2, A3, A4, A5, A6};
}

int test3(int x) {
    int prod = 1;
    for (constexpr int d = -1; x > 0; x += d) {
        prod *= x;
    }
    return prod;
}
