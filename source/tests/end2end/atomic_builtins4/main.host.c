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
#include <math.h>
#include <complex.h>
#include "./definitions.h"

#define EPSILON_F 1e-3
#define EPSILON_D 1e-6
#define EPSILON_LD 1e-8

int main(void) {
    for (long x = -4096; x < 4096; x++) {
        _Atomic char chr = x;
        _Atomic short shrt = x;
        _Atomic int integer = x;
        _Atomic long lng = x;

        char expected_chr = x + 1;
        assert(!test_atomic_compare_exchange8(&chr, &expected_chr, (char) ~x));
        assert(chr == (char) x && expected_chr == (char) x);
        assert(test_atomic_compare_exchange8(&chr, &expected_chr, (char) ~x));
        assert(chr == (char) ~x && expected_chr == (char) x);

        short expected_shrt = x + 1;
        assert(!test_atomic_compare_exchange16(&shrt, &expected_shrt, (short) ~x));
        assert(shrt == (short) x && expected_shrt == (short) x);
        assert(test_atomic_compare_exchange16(&shrt, &expected_shrt, (short) ~x));
        assert(shrt == (short) ~x && expected_shrt == (short) x);

        int expected_int = x + 1;
        assert(!test_atomic_compare_exchange32(&integer, &expected_int, (int) ~x));
        assert(integer == (int) x && expected_int == (int) x);
        assert(test_atomic_compare_exchange32(&integer, &expected_int, (int) ~x));
        assert(integer == (int) ~x && expected_int == (int) x);

        long expected_lng = x + 1;
        assert(!test_atomic_compare_exchange64(&lng, &expected_lng, (long) ~x));
        assert(lng == (long) x && expected_lng == (long) x);
        assert(test_atomic_compare_exchange64(&lng, &expected_lng, (long) ~x));
        assert(lng == (long) ~x && expected_lng == (long) x);
    }

    for (long x = -4096; x < 4096; x++) {
        _Atomic char chr = x;
        _Atomic short shrt = x;
        _Atomic int integer = x;
        _Atomic long lng = x;

        char expected_chr = x + 1;
        assert(!test2_atomic_compare_exchange8(&chr, &expected_chr, (char) ~x));
        assert(chr == (char) x && expected_chr == (char) x);
        assert(test2_atomic_compare_exchange8(&chr, &expected_chr, (char) ~x));
        assert(chr == (char) ~x && expected_chr == (char) x);

        short expected_shrt = x + 1;
        assert(!test2_atomic_compare_exchange16(&shrt, &expected_shrt, (short) ~x));
        assert(shrt == (short) x && expected_shrt == (short) x);
        assert(test2_atomic_compare_exchange16(&shrt, &expected_shrt, (short) ~x));
        assert(shrt == (short) ~x && expected_shrt == (short) x);

        int expected_int = x + 1;
        assert(!test2_atomic_compare_exchange32(&integer, &expected_int, (int) ~x));
        assert(integer == (int) x && expected_int == (int) x);
        assert(test2_atomic_compare_exchange32(&integer, &expected_int, (int) ~x));
        assert(integer == (int) ~x && expected_int == (int) x);

        long expected_lng = x + 1;
        assert(!test2_atomic_compare_exchange64(&lng, &expected_lng, (long) ~x));
        assert(lng == (long) x && expected_lng == (long) x);
        assert(test2_atomic_compare_exchange64(&lng, &expected_lng, (long) ~x));
        assert(lng == (long) ~x && expected_lng == (long) x);
    }
    return EXIT_SUCCESS;
}
