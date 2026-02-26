/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

void test(int a) {
    extern int x;
    extern int y;
    for (int i = 0; i < a; i++)
        x += y;
}

void test2(int a) {
    extern int x;
    extern int z[10];
    for (int i = 0; i < a; i++)
        x += z[5];
}

void test3(int a) {
    extern int x;
    extern int z[10];
    for (int i = 0; i < a; i++)
        x += z[i];
}

void test4(int a) {
    extern int x;
    extern int z[10];
    extern void fn(void);
    for (int i = 0; i < a; i++) {
        fn();
        x += z[5];
    }
}

void test5(int a) {
    extern _Thread_local int w;
    extern _Thread_local int k[10];
    for (int i = 0; i < a; i++) {
        w += k[5];
    }
}

void test6(int a) {
    extern _Thread_local int w;
    extern _Thread_local int k[10];
    extern void fn(void);
    for (int i = 0; i < a; i++) {
        w += k[5];
        fn();
    }
}

void test7(int a) {
    extern int x;
    extern int y;
    for (int i = 0; i < a; i++)
        if (i == 3)
            x += y;
}

void test8(int a, int *b) {
    extern int x;
    for (int i = 0; i < a; i++) {
        x += *b;
    }
}

void test9(int *b) {
    extern int x;
    for (int i = 0; i < 10; i++) {
        x += *b;
    }
}
