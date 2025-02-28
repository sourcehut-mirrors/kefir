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
#include "./definitions.h"

int main(void) {
    const long x = 0xcafe;
    const long y = 0xbabe;
    for (long a = -100; a < 100; a++) {
        for (long b = -1000; b < 1000; b++) {
            assert(select_equals(a, b, x, y) == (a == b ? x : y));
            assert(select_not_equals(a, b, x, y) == (a != b ? x : y));
            assert(select_greater(a, b, x, y) == (a > b ? x : y));
            assert(select_greater_or_equals(a, b, x, y) == (a >= b ? x : y));
            assert(select_lesser(a, b, x, y) == (a < b ? x : y));
            assert(select_lesser_or_equals(a, b, x, y) == (a <= b ? x : y));

            unsigned long ua = a;
            unsigned long ub = b;
            assert(select_above(ua, ub, x, y) == (ua > ub ? x : y));
            assert(select_above_or_equals(ua, ub, x, y) == (ua >= ub ? x : y));
            assert(select_below(ua, ub, x, y) == (ua < ub ? x : y));
            assert(select_below_or_equals(ua, ub, x, y) == (ua <= ub ? x : y));
        }
    }
    return EXIT_SUCCESS;
}
