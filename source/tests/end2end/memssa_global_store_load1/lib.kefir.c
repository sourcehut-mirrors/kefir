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

char c1, c2, c3;
short s1, s2, s3;
int i1, i2, i3;
long l1, l2, l3;

char test_char(char x) {
    c1 = x;
    c2 = x + 1;
    c3 = x * 2;
    return c1 + c2 + c3;
}

short test_short(short x) {
    s1 = x;
    s2 = x + 1;
    s3 = x * 2;
    return s1 + s2 + s3;
}

int test_int(int x) {
    i1 = x;
    i2 = x + 1;
    i3 = x * 2;
    return i1 + i2 + i3;
}

long test_long(long x) {
    l1 = x;
    l2 = x + 1;
    l3 = x * 2;
    return l1 + l2 + l3;
}
