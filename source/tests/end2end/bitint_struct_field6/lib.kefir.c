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
    _BitInt(6) a;
    _BitInt(50) b;
    _BitInt(14) c;
    _BitInt(150) d;
    _BitInt(29) e;
    _BitInt(120) f;
};

struct S1 get1(int x) {
    return (struct S1) {.a = x, .b = x + 1, .c = x + 2, .d = x + 3, .e = x + 4, .f = x + 5};
}

long get2(struct S1 s) {
    return s.a;
}

long get3(struct S1 s) {
    return s.b;
}

long get4(struct S1 s) {
    return s.c;
}

long get5(struct S1 s) {
    return s.d;
}

long get6(struct S1 s) {
    return s.e;
}

long get7(struct S1 s) {
    return s.f;
}
