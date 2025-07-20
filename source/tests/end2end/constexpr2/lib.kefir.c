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

static constexpr int x = 1000;
static constexpr double y = 3.1415926;
static constexpr _Complex float z = 100.0f - 200.1fi;
static constexpr const int *w = &x;
static constexpr struct S1 a = {.a = 100, .b = 0xcafeb00dull, .c = "Hello, world!"};

static_assert(x == 1000);
static_assert(y < 3.1415926 ? 3.1415926 - y < 1e-5 : y - 3.1415926 - y < 1e-5);
static_assert((int) z);
static_assert(w == &x);
static_assert(a.a == 100);
static_assert(a.b == 0xcafeb00dull);

const int *getx() {
    return &x;
}

const double *gety() {
    return &y;
}

const _Complex float *getz() {
    return &z;
}

const int *const *get_w() {
    return &w;
}

const struct S1 *geta() {
    return &a;
}
