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

extern int A;

inline void test1(int x) {
    A += x;
}

inline void test2(int y) {
    A -= y;
}

void main() {
    test1(100);
    test2(50);
    test1(200);
    test2(30);
    test1(-500);
    test2(45);
    test1(123);
    test2(0);
    test1(-90);
    test2(-56);
    test1(1000);
    test2(2000);
}
