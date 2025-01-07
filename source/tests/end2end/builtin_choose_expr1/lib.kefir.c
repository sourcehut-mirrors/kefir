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

extern void abort(void);

int arr[] = {__builtin_choose_expr(1, 1, 100), __builtin_choose_expr(0, 265, 0),
             __builtin_choose_expr(2 + 2 == 4, sizeof(char), -10),
             __builtin_choose_expr(3 - 1 > 5, 1, sizeof(double) - sizeof(double)),
             sizeof(__builtin_choose_expr(1, volatile short, unsigned long)) / sizeof(short)};

int arr_len = sizeof(arr) / sizeof(arr[0]);

int test(int x) {
    __builtin_choose_expr(1, (void) 0, abort());
    switch (x) {
        case 0:
            return __builtin_choose_expr(1, 1, 100);
        case 1:
            return __builtin_choose_expr(0, 265, 0);
        case 2:
            return __builtin_choose_expr(2 + 2 == 4, sizeof(char), -10);
        case 3:
            return __builtin_choose_expr(3 - 1 > 5, 1, sizeof(double) - sizeof(double));
        case 4:
            return sizeof(__builtin_choose_expr(1, volatile short, unsigned long)) / sizeof(short);
        default:
            return -1;
    }
}
