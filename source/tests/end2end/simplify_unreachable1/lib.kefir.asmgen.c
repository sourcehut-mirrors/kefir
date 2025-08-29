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

char u8;
short u16;
int u32;
long u64;
long double ld;
_Complex float c32;
_Complex double c64;
_Complex long double c80;
_BitInt(128) u128;

void test1(void) {
    u8 = 1;
    __builtin_unreachable();
}

void test2(void) {
    u16 = 1;
    __builtin_unreachable();
}

void test3(void) {
    u32 = 1;
    __builtin_unreachable();
}

void test4(void) {
    u64 = 1;
    __builtin_unreachable();
}

void test5(void) {
    ld = 1;
    __builtin_unreachable();
}

void test6(void) {
    c32 = 1;
    __builtin_unreachable();
}

void test7(void) {
    c64 = 1;
    __builtin_unreachable();
}

void test8(void) {
    c80 = 1;
    __builtin_unreachable();
}

void test9(void) {
    *(volatile char *) &u8 = 1;
    __builtin_unreachable();
}

void test10(void) {
    *(volatile short *) &u16 = 1;
    __builtin_unreachable();
}

void test11(void) {
    *(volatile int *) &u32 = 1;
    __builtin_unreachable();
}

void test12(void) {
    *(volatile long *) &u64 = 1;
    __builtin_unreachable();
}

void test13(void) {
    *(volatile long double *) &ld = 1;
    __builtin_unreachable();
}

void test14(void) {
    *(volatile _Complex float *) &c32 = 1;
    __builtin_unreachable();
}

void test15(void) {
    *(volatile _Complex double *) &c64 = 1;
    __builtin_unreachable();
}

void test16(void) {
    *(volatile _Complex long double *) &c80 = 1;
    __builtin_unreachable();
}

void test17(void) {
    u128 = 1;
    __builtin_unreachable();
}

void test18(void) {
    *(volatile _BitInt(128) *) &u128 = 1;
    __builtin_unreachable();
}

void test19(...) {
    __builtin_va_list args, arg2;
    __builtin_c23_va_start(args);
    int x = __builtin_va_arg(args, int);
    __builtin_va_copy(args, arg2);
    __builtin_va_end(args);
    __builtin_unreachable();
}
