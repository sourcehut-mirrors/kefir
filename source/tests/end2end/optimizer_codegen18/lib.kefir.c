/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

extern int sum(int n, ...) {
    __builtin_va_list args, args2;
    __builtin_va_start(args, n);
    __builtin_va_copy(args2, args);
    int result = 0;
    while (n--) {
        result += __builtin_va_arg(args2, int);
    }
    __builtin_va_end(args);
    __builtin_va_end(args2);
    return result;
}

extern double sumd(int n, ...) {
    __builtin_va_list args, args2;
    __builtin_va_start(args, n);
    __builtin_va_copy(args2, args);
    double result = 0.0;
    while (n--) {
        result += __builtin_va_arg(args2, double);
    }
    __builtin_va_end(args);
    __builtin_va_end(args2);
    return result;
}

extern long sum1(int n, ...) {
    __builtin_va_list args, args2;
    __builtin_va_start(args, n);
    __builtin_va_copy(args2, args);
    long result = 0;
    while (n--) {
        struct Struct1 arg = __builtin_va_arg(args2, struct Struct1);
        result += arg.arr[0] + arg.arr[1] + arg.arr[2] + arg.arr[3] + arg.arr[4];
    }
    __builtin_va_end(args);
    __builtin_va_end(args2);
    return result;
}

extern struct Struct2 sum2(int n, ...) {
    __builtin_va_list args, args2;
    __builtin_va_start(args, n);
    __builtin_va_copy(args2, args);
    struct Struct2 result = {0, 0.0};
    while (n--) {
        struct Struct2 arg = __builtin_va_arg(args2, struct Struct2);
        result.a += arg.a;
        result.b += arg.b;
    }
    __builtin_va_end(args);
    __builtin_va_end(args2);
    return result;
}

extern long sum3(int n, ...) {
    __builtin_va_list args, args2;
    __builtin_va_start(args, n);
    __builtin_va_copy(args2, args);
    long result = 0;
    while (n--) {
        struct Struct3 s3 = __builtin_va_arg(args2, struct Struct3);
        result += s3.a * s3.b;
    }
    __builtin_va_end(args);
    __builtin_va_end(args2);
    return result;
}

extern float sum4(int n, ...) {
    __builtin_va_list args, args2;
    __builtin_va_start(args, n);
    __builtin_va_copy(args2, args);
    float result = 0;
    while (n--) {
        struct Struct4 s4 = __builtin_va_arg(args2, struct Struct4);
        result += s4.a * s4.b;
    }
    __builtin_va_end(args);
    __builtin_va_end(args2);
    return result;
}

extern long sum5(int n, va_list args) {
    long result = 0;
    while (n--) {
        result += __builtin_va_arg(args, long);
    }
    return result;
}

extern long sum6_proxy(int n, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, n);
    long result = sum6(n, args);
    __builtin_va_end(args);
    return result;
}
