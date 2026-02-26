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

int x;
int y;

void test() {
    for (int i = 0; i < 10; i++)
        y += x;
}

void test0() {
    for (int i = 0; i < 10; i++)
        y -= x;
}

void test2() {
    extern int a[10];
    extern int b[10];
    for (int i = 0; i < 10; i++)
        a[i] += b[1];
}

void test3() {
    extern int a[10];
    for (int i = 0; i < 10; i++)
        a[i] += a[1];
}

void test4() {
    extern int a[10];
    extern struct {
        int arr[10];
        int value;
    } c;
    for (int i = 0; i < 10; i++)
        a[i] += c.value;
}

void test5(int x) {
    extern int a[10];
    extern int d;
    for (int i = 0; i < x; i++)
        a[i] += d;
}

void test6(int x) {
    extern int a[10];
    extern int d;
    for (int i = 0; i < 10; i = i % x + 1)
        a[i] += d;
}

void test7() {
    extern int a[10];
    extern int d;
    for (;;)
        a[1] += d;
}

void test8() {
    extern void fn(void);
    volatile extern int e[10];
    extern int d;
    fn();
    for (;;)
        e[1] += d;
}

void test9(int x) {
    extern int a[10];
    extern int d;
    for (; x;)
        a[1] += d;
}

void test11(int x) {
    extern int a[10];
    extern int d;
    extern void fn(void);
    for (int i = 0; i < 10; i = i % x + 1) {
        fn();
        a[i] += d;
    }
}

void test12() {
    extern int a[10];
    for (int i = 0; i < 10; i++) {
        a[i] += a[i + 1];
    }
}

void test13() {
    extern int a[10];
    int b[10];
    for (int i = 0; i < 10; i++) {
        a[i] += b[i];
    }
}

void test14(int x) {
    extern int a[10];
    extern int b[10];
    for (int i = 0; i < 10; i++) {
        (&a[x])[i] += (&b[x])[i];
    }
}

void test15(int x) {
    extern int a[10];
    for (int i = 0; i < 10; i++) {
        (&a[x])[i] += (&a[x])[i + 1];
    }
}
