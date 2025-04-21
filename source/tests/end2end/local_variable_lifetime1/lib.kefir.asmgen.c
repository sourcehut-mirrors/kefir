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

extern void test(int, ...);

inline void test3(volatile int x) {}

void test2(int a) {
    volatile int x = 1;
    volatile int y = 2;
    test3(a);
    test(2, &x, &y);
    if (a) {
        volatile int z = 3;
        volatile int w = 4;
        test(4, &x, &y, &z, &w);
        for (volatile int j = 0; j < 10; j++) {
            test(5, &x, &y, &z, &w, &j);
        }
    } else {
        volatile int z2 = 10;
        volatile int w2 = 11;
        volatile int j2 = 12;
        test(5, &x, &y, &z2, &w2, &j2);
    }
    volatile int i = 100;
    test(3, &x, &y, &i);
}