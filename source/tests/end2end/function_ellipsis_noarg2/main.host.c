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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include "./definitions.h"

static int fn(int count, ...) {
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (; count--;) {
        sum += va_arg(args, int);
    }
    va_end(args);
    return sum;
}

int main(void) {
    for (int i = -100; i < 100; i++) {
        assert(test1(fn, i) == (int) (i + 1 + 2 + 3 - 9 - 8 - 7 + (1ull << 18) + (1ull << 18) - 1));
    }
    return EXIT_SUCCESS;
}
