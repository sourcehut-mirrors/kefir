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

long sum(long x, long y) {
    return x + y;
}

long sump(struct Pair p) {
    return p.a + p.b;
}

long sumh(struct HugePair p) {
    return p.a + p.b;
}

long test_hypot(long x, long y) {
    return sum(mul(x, x), mul(y, y));
}

long test_hypotp(struct Pair p) {
    struct Pair pa, pb, pm;
    pa.a = p.a;
    pa.b = p.a;
    pb.a = p.b;
    pb.b = p.b;
    pm.a = mulp(pa);
    pm.b = mulp(pb);
    return sump(pm);
}

long test_hypoth(struct HugePair p) {
    struct HugePair pa, pb, pm;
    pa.a = p.a;
    pa.b = p.a;
    pb.a = p.b;
    pb.b = p.b;
    pm.a = mulh(pa);
    pm.b = mulh(pb);
    return sumh(pm);
}

int dummy_test(void) {
    return dummy_fun(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17);
}

int dummy_test2(void) {
    struct IPair i1;
    i1.a = 1;
    i1.b = 2;
    struct Pair i2;
    i2.a = 4;
    i2.b = 5;
    return dummy_fun2(i1, 3, i2, 6);
}

int dummy_test3(void) {
    struct IPair i1;
    i1.a = 1;
    i1.b = 2;
    struct HugePair i2;
    i2.a = 5;
    i2.b = 6;
    struct IPair i3;
    i3.a = 8;
    i3.b = 9;
    return dummy_fun3(i1, 3, 4, i2, 7, i3);
}
