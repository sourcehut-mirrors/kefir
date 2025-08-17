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

struct S1 get1(unsigned long x) {
    struct S1 s = {0};
    s.arr[1] = x;
    s.arr[2] = x + 1;
    s.arr[3] = x + 2;
    s.arr[4] = x + 3;
    s.arr[5] = x + 4;
    s.arr[6] = x + 5;
    s.arr[7] = x + 6;
    return s;
}

struct S1 get2(int a, unsigned long x) {
    struct S1 s1 = {0}, s2 = {0}, s3 = {0};
    for (int i = 0; i < 7; i++) {
        s1.arr[i + 1] = x + i;
        s2.arr[i + 1] = -(x + i);
        s3.arr[i + 1] = ~(x + i);
    }

    if (a > 0 && a != 1) {
        return s1;
    } else if (a < 0) {
        return s2;
    } else if (a != 1) {
        return s3;
    } else {
        return s1;
    }
}
