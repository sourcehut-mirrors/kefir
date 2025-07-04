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
    unsigned _BitInt(6) a : 5;
    unsigned _BitInt(50) b : 45;
    unsigned _BitInt(14) c : 10;
    unsigned _BitInt(29) e : 25;
};

long get2(struct S1 s) {
    return s.a;
}

long get3(struct S1 s) {
    return s.b;
}

long get4(struct S1 s) {
    return s.c;
}

long get6(struct S1 s) {
    return s.e;
}

struct S2 {
    unsigned _BitInt(150) a : 10, b : 120, c : 30;
};

long get5(struct S2 s) {
    return s.b;
}

long get7(struct S2 s) {
    return s.c;
}
