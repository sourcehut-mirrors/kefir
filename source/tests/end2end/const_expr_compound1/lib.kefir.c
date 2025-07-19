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

struct S1 a = (struct S1) {.arr = {1, 2, 3}, .b = 3.14f, .c = &a, .d = (struct S2) {1000}};

long b[] = (long[]) {100, 200, -300, 400};

_Static_assert(__builtin_constant_p((struct S1) {.arr = {1, 2, 3}}));
_Static_assert(!__builtin_constant_p((struct S1) {.arr = {1, 2, 3}, .c = &a}));

int getone() {
    return __builtin_constant_p((struct S1) {.arr = {1, 2, 3}});
}

int getzero() {
    return __builtin_constant_p((struct S1) {.arr = {1, 2, 3}, .c = &a});
}
