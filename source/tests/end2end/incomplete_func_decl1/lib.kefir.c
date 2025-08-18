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

struct A get1(struct B, struct C);
struct B get2(struct A, struct C);
struct C get3(struct A, struct B);
struct B get4(struct B);

struct B {
    int a;
    int b;
};

int test(int a, int b) {
    struct B x = get4((struct B){a, b});
    return x.a ^ x.b;
}

struct B get4(struct B x) {
    return (struct B) {
        .a = x.a + x.b,
        .b = x.a - x.b
    };
}
