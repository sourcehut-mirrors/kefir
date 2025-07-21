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

int get0(void) {
    constexpr int a = 12345;
    return a;
}

struct S1 get1(void) {
    constexpr struct S1 a = {-12345};
    return a;
}

struct S2 get2(void) {
    constexpr struct S2 a = {-12345, 0xcafe};
    return a;
}

struct S3 get3(void) {
    constexpr struct S3 a = {-12345, 0xcafe, 4.532};
    return a;
}

struct S4 get4(void) {
    constexpr struct S4 a = {-12345, 0xcafe, 4.532, (void *) 0};
    return a;
}

struct S5 get5(void) {
    constexpr struct S5 a = {-12345, 0xcafe, 4.532, (void *) 0, .e = -1};
    return a;
}

_Complex long double getx(void) {
    constexpr _Complex long double a = 3.14l - 2.71il;
    return a;
}
