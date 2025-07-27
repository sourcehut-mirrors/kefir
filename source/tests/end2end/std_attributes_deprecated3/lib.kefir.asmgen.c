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

struct S1 {
    int a [[deprecated]];
    int b [[deprecated("DEPRECATED")]];
    int c;
} s1;

union U1 {
    int a [[deprecated]];
    int b [[deprecated("DEPRECATED")]];
    int c;
} u1;

int get1() {
    return s1.a;
}

int get2() {
    return s1.b;
}

int get3() {
    return s1.c;
}

int get4() {
    return u1.a;
}

int get5() {
    return u1.b;
}

int get6() {
    return u1.c;
}
