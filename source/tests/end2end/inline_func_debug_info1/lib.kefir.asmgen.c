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

volatile int x = 0;

inline void test1(int a) {
    int xval = x;
    xval += a;
    x = xval;
}

inline void test2(int b) {
    int xval = x;
    xval += b;
    x = xval;
}

int main() {
    int inc = 1;
    test1(inc);
    inc = 3;
    test2(inc);
    inc = -1;
    test1(inc);
    return 0;
}
