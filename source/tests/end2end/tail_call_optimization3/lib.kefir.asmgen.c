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

extern int *someptr;

int test(int x) {
    someptr = &x;
    return test(x + 1);
}

int test2(int x) {
    return test2(x + 1);
}

extern void somefn(void *);
int test3(int x) {
    somefn(&x);
    return test3(x + 1);
}

extern _Atomic int someothervar;

int test4(int x) {
    someothervar += &x - &someothervar;
    return test4(x + 1);
}

int test5(int x) {
    asm("" : : "r"(&x));
    return test5(x + 1);
}

int test6(int x) {
    asm("" : : "r"(x));
    return test5(x + 1);
}

int test7(int x) {
    somefn(x > 0 ? &x : 0);
    return test7(x + 1);
}

int test8(int x) {
    somefn(x > 0 ? x : 0);
    return test8(x + 1);
}

extern void __attribute__((returns_twice)) somejmp(void);
extern void somejmp2(void);

int test9(int x) {
    somejmp();
    return test9(x + 1);
}

int test10(int x) {
    somejmp2();
    return test10(x + 1);
}

struct S1 {
    char arr[128];
};

struct S1 test11(int x) {
    return test11(x + 1);
}

struct S2 {
    long a;
    long b;
};

struct S2 test12(int x) {
    return test12(x + 1);
}

int test13(int x) {
    volatile int arr[1024];
    arr[0] = 0;
    return test13(x);
}

int test14(int x) {
    volatile int arr[1024];
    return test14(x + arr[0]);
}
