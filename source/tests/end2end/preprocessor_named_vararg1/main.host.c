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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include "./definitions.h"

int sum(int count, ...) {
    va_list args;
    va_start(args, count);
    int sum = 0;
    while (count--) {
        sum += va_arg(args, int);
    }
    va_end(args);
    return sum;
}

int main(void) {
    for (int i = -10; i < 10; i++) {
        for (int j = -10; j < 10; j++) {
            for (int k = -10; k < 10; k++) {
                assert(add3(i, j, k) == (i + j + k));
            }
        }
    }
    return EXIT_SUCCESS;
}
