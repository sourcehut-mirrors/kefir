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

int process(int (*callback)(int *, int), int x, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, x);
    if (x > 0) {
        int arr[x];
        for (int i = 0; i < x; i++) {
            arr[i] = __builtin_va_arg(args, int);
        }
        return callback(arr, x);
    } else {
        return 0;
    }
}
