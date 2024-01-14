/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

int sum1(struct Test1 t) {
    return t.x + t.y + t.z;
}

int sum2(struct Test2 t) {
    return -t.x - t.y;
}

long sum3(struct Test3 t) {
    return t.x + t.z;
}

long sum4(long a, long b, long c, long d, long e, long f, long g, long h, long i) {
    return a + b + c + d + e + f + g + h + i;
}

long sum5(long a, struct Test4 b, long c, struct Test5 d, long e, struct Test4 f) {
    return a + b.x + b.y + c + d.arr[0] + d.arr[1] + d.arr[2] + d.arr[3] + e + f.x + f.y;
}
