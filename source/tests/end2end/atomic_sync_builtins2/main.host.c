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
    for (long x = -260; x < 260; x++) {
        for (long y = -260; y < 260; y++) {
            _Atomic char chr = x;
            _Atomic short shrt = x;
            _Atomic int integer = x;
            _Atomic long lng = x;

            assert(test_fetch_add_char(&chr, (char) y) == (char) (x + y));
            assert(chr == (char) (x + y));
            assert(test_fetch_add_short(&shrt, (short) y) == (short) (x + y));
            assert(shrt == (short) (x + y));
            assert(test_fetch_add_int(&integer, (int) y) == (int) (x + y));
            assert(integer == (int) (x + y));
            assert(test_fetch_add_long(&lng, (long) y) == (long) (x + y));
            assert(lng == (long) (x + y));

            chr = x;
            shrt = x;
            integer = x;
            lng = x;

            assert(test_fetch_sub_char(&chr, (char) y) == (char) (x - y));
            assert(chr == (char) (x - y));
            assert(test_fetch_sub_short(&shrt, (short) y) == (short) (x - y));
            assert(shrt == (short) (x - y));
            assert(test_fetch_sub_int(&integer, (int) y) == (int) (x - y));
            assert(integer == (int) (x - y));
            assert(test_fetch_sub_long(&lng, (long) y) == (long) (x - y));
            assert(lng == (long) (x - y));

            chr = x;
            shrt = x;
            integer = x;
            lng = x;

            assert(test_fetch_or_char(&chr, (char) y) == (char) (x | y));
            assert(chr == (char) (x | y));
            assert(test_fetch_or_short(&shrt, (short) y) == (short) (x | y));
            assert(shrt == (short) (x | y));
            assert(test_fetch_or_int(&integer, (int) y) == (int) (x | y));
            assert(integer == (int) (x | y));
            assert(test_fetch_or_long(&lng, (long) y) == (long) (x | y));
            assert(lng == (long) (x | y));

            chr = x;
            shrt = x;
            integer = x;
            lng = x;

            assert(test_fetch_and_char(&chr, (char) y) == (char) (x & y));
            assert(chr == (char) (x & y));
            assert(test_fetch_and_short(&shrt, (short) y) == (short) (x & y));
            assert(shrt == (short) (x & y));
            assert(test_fetch_and_int(&integer, (int) y) == (int) (x & y));
            assert(integer == (int) (x & y));
            assert(test_fetch_and_long(&lng, (long) y) == (long) (x & y));
            assert(lng == (long) (x & y));

            chr = x;
            shrt = x;
            integer = x;
            lng = x;

            assert(test_fetch_xor_char(&chr, (char) y) == (char) (x ^ y));
            assert(chr == (char) (x ^ y));
            assert(test_fetch_xor_short(&shrt, (short) y) == (short) (x ^ y));
            assert(shrt == (short) (x ^ y));
            assert(test_fetch_xor_int(&integer, (int) y) == (int) (x ^ y));
            assert(integer == (int) (x ^ y));
            assert(test_fetch_xor_long(&lng, (long) y) == (long) (x ^ y));
            assert(lng == (long) (x ^ y));
        }
    }
    return EXIT_SUCCESS;
}
