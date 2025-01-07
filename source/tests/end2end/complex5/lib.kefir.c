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

#include "./definitions.h"

_Complex float test32(_Complex float x) {
    return sum32(x + 0, x + 1, x + 2, x + 3, x + 4, x + 5, x + 6, x + 7, x + 8, x + 9, x + 10, x + 11);
}

_Complex double test64(_Complex double x) {
    return sum64(x + 0, x + 1, x + 2, x + 3, x + 4, x + 5, x + 6, x + 7, x + 8, x + 9, x + 10, x + 11);
}

_Complex long double testld(_Complex long double x) {
    return sumld(x + 0, x + 1, x + 2, x + 3, x + 4, x + 5, x + 6, x + 7, x + 8, x + 9, x + 10, x + 11);
}

union Union1 test_struct(struct Struct1 s) {
    return (union Union1){.c = s.a + s.b + s.c + s.x + s.y + s.z};
}

_Complex float vsum32(int n, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, n);

    _Complex float sum = 0.0f;
    while (n--) {
        sum += __builtin_va_arg(args, _Complex float);
    }
    __builtin_va_end(args);
    return sum;
}

_Complex double vsum64(int n, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, n);

    _Complex double sum = 0.0;
    while (n--) {
        sum += __builtin_va_arg(args, _Complex double);
    }
    __builtin_va_end(args);
    return sum;
}

_Complex long double vsumld(int n, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, n);

    _Complex double sum = 0.0;
    while (n--) {
        sum += __builtin_va_arg(args, _Complex long double);
    }
    __builtin_va_end(args);
    return sum;
}
