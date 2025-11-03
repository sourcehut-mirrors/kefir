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

static struct S {
    __int128 a : 80,
             b : 60,
             c : 76,
             d : 35;
} value = {0};

__int128 get_a() {
        return value.a;
}

__int128 get_b() {
    return value.b;
}

__int128 get_c() {
    return value.c;
}

__int128 get_d() {
    return value.d;
}

void set_a(__int128 a) {
    value.a = a;
}

void set_b(__int128 b) {
    value.b = b;
}

void set_c(__int128 c) {
    value.c = c;
}

void set_d(__int128 d) {
    value.d = d;
}
