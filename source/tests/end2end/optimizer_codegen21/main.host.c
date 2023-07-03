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
#include <math.h>
#include "./definitions.h"

#define EPSILON_F 1e-3
#define EPSILON_D 1e-6
#define EPSILON_LD 1e-8

static long sum(int n, ...) {
    va_list args;
    va_start(args, n);
    long result = 0;
    while (n--) {
        result += va_arg(args, long);
    }
    va_end(args);
    return result;
}

extern long double sumld(long x, long double y) {
    return ((long double) x) + y;
}

int main(void) {
    for (long i = -100; i < 100; i++) {
        assert(invoke(sum, i) == (i + i + 1 + i * 2 + i / 2 + (i ^ 0x0badbabe)));

        assert(fabsl(testsumld(i) - ((long double) ~i + (long double) i)) < EPSILON_LD);
    }

    assert(zerofnl() == 0);
    assert(fabs(zerofnf()) < EPSILON_F);
    assert(fabs(zerofnd()) < EPSILON_D);
    assert(fabsl(zerofnld()) < EPSILON_LD);
    return EXIT_SUCCESS;
}
