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

#include "definitions.h"

static struct S1 {
    int a : 7;
    unsigned int b : 7;
} s1;

_Bool test1(int x, int y) {
    return __builtin_mul_overflow_p(x, y, (int) 0);
}

_Bool test2(int x, int y) {
    return __builtin_mul_overflow_p(x, y, (unsigned int) 0);
}

_Bool test3(int x, int y) {
    return __builtin_mul_overflow_p(x, y, s1.a);
}

_Bool test4(int x, int y) {
    return __builtin_mul_overflow_p(x, y, s1.b);
}
