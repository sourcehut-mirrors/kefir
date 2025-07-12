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

int test1(...);
int test2(...);
int test3(...);
int test4(...);

int test1(...) {
    __builtin_va_list args, args2;
    __builtin_c23_va_start(args);
    __builtin_va_copy(args2, args);
    __builtin_va_end(args);
    return 0xcafe;
}

int test2(...) {
    __builtin_va_list args;
    __builtin_c23_va_start(args);
    int x = __builtin_va_arg(args, int);
    __builtin_va_end(args);
    return -x;
}

int test3(...) {
    __builtin_va_list args;
    __builtin_c23_va_start(args);
    int x = __builtin_va_arg(args, int);
    int y = __builtin_va_arg(args, int);
    __builtin_va_end(args);
    return -x * y;
}

int test4(...) {
    __builtin_va_list args;
    __builtin_c23_va_start(args);
    int x = __builtin_va_arg(args, int);
    int sum = 0;
    while (x--) {
        sum += __builtin_va_arg(args, int);
    }
    __builtin_va_end(args);
    return sum;
}
