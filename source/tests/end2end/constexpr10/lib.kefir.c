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

struct S0 {
    short a;
    long b;
    float c;
};

struct S1 {
    int a;
    const char *b;
    _BitInt(41) c;
    double d;
    _Complex float e;
    int f[3];
    struct S0 g;
};

struct S1 get1(void) {
    return (struct S1) {58472, "Test123", 0xe4d4wb, 5.42482, -5838.1if, {1, 2, -1}, {50, -0x43edca, 5.424f}};
}
