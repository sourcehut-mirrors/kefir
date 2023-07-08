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

extern void abort(void);

int arr[] = {__builtin_constant_p(1),         __builtin_constant_p(abort()),
             __builtin_constant_p(2 + 2 * 2), __builtin_constant_p(*((int *) 0)),
             __builtin_constant_p("hello"),   __builtin_constant_p(&abort)};

int arr_len = sizeof(arr) / sizeof(arr[0]);

int test(int x) {
    switch (x) {
        case 0:
            return __builtin_constant_p(1);
        case 1:
            return __builtin_constant_p(abort());
        case 2:
            return __builtin_constant_p(2 + 2 * 2);
        case 3:
            return __builtin_constant_p(*((int *) 0));
        case 4:
            return __builtin_constant_p("hello");
        case 5:
            return __builtin_constant_p(&abort);
        default:
            return -1;
    }
}
