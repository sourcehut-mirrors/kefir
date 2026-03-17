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

#include "./definitions.h"

int test1() {
    _Alignas(128) volatile int x = 0;
    return ((unsigned long) &x) % 128 == 0;
}

long test2(struct S1 arg) {
    _Alignas(128) volatile int x = 0;
    if (((unsigned long) &x) % 128 == 0) {
        return arg.arr[0] + arg.arr[1] + arg.arr[2] + arg.arr[3] + arg.arr[4] + arg.arr[5] + arg.arr[6] + arg.arr[7];
    } else {
        return 0;
    }
}

long test3(struct S1 arg0, struct S1 arg) {
    _Alignas(128) volatile int x = 0;
    if (((unsigned long) &x) % 128 == 0) {
        return arg.arr[0] + arg.arr[1] + arg.arr[2] + arg.arr[3] + arg.arr[4] + arg.arr[5] + arg.arr[6] + arg.arr[7];
    } else {
        return 0;
    }
}

long test4(struct S1 arg0, struct S1 arg1, struct S1 arg) {
    _Alignas(128) volatile int x = 0;
    if (((unsigned long) &x) % 128 == 0) {
        return arg.arr[0] * arg0.arr[0] + arg.arr[1] * arg0.arr[0] + arg.arr[2] * arg0.arr[0] +
               arg.arr[3] * arg0.arr[0] + arg.arr[4] * arg0.arr[0] + arg.arr[5] * arg0.arr[0] +
               arg.arr[6] * arg0.arr[0] + arg.arr[7] * arg0.arr[0];
    } else {
        return 0;
    }
}

long test5(int len, struct S1 arg0, ...) {
    _Alignas(128) volatile int x = 0;
    if (((unsigned long) &x) % 128 == 0) {
        __builtin_va_list args;
        __builtin_va_start(args, arg0);
        long sum = 0;
        while (len--) {
            struct S1 arg = __builtin_va_arg(args, struct S1);
            sum +=
                arg.arr[0] + arg.arr[1] + arg.arr[2] + arg.arr[3] + arg.arr[4] + arg.arr[5] + arg.arr[6] + arg.arr[7];
        }
        return sum;
    } else {
        return 0;
    }
}

void *test6(int level) {
    _Alignas(128) volatile int x = 0;
    return __builtin_frame_address(level);
}

void *test7(int level) {
    _Alignas(128) volatile int x = 0;
    return __builtin_return_address(level);
}

void *test8(int level) {
    _Alignas(128) volatile int x = 0;
    static volatile int *ptr;
    ptr = &x;
    return test6(level);
}

void *test9(int level) {
    _Alignas(128) volatile int x = 0;
    static volatile int *ptr;
    ptr = &x;
    return test7(level);
}
