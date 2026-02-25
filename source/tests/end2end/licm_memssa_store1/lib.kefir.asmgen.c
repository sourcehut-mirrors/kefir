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

void test() {
    extern int a;
    for (int i = 0; i < 10; i++)
        a = 1;
}

void test2() {
    extern int b[10];
    extern int c[10];
    for (int i = 0; i < 10; i++)
        b[i] = c[1];
}

void test3() {
    extern int b[10];
    extern int c[10];
    for (int i = 0; i < 10; i++)
        b[1] = c[2];
}

void test4() {
    extern int a;
    for (;;)
        a = 1000;
}

void test5() {
    extern volatile int d;
    for (;;)
        d = 1234;
}

void test6() {
    extern int b[10];
    extern int c[10];
    extern void fn(void);
    for (int i = 0; i < 10; i++) {
        fn();
        b[1] = c[2];
    }
}
