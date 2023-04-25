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
#include "kefir/core/util.h"
#include "kefir/test/unit_test.h"

long double ldouble_sum(long double x, long double y) {
    return x + y;
}

long double ldouble_sum_proxy(long double, long double);

int main(int argc, const char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    for (long double x = -10.0; x < 10.0; x += 0.1) {
        for (long double y = -10.0; y < 10.0; y += 0.1) {
            ASSERT(LONG_DOUBLE_EQUALS(ldouble_sum_proxy(x, y), x + y, LONG_DOUBLE_EPSILON));
        }
    }
    return EXIT_SUCCESS;
}
