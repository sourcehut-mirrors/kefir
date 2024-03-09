/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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
    for (long x = -4096; x < 4096; x++) {
        _Atomic char chr = x;
        _Atomic short shrt = x;
        _Atomic int integer = x;
        _Atomic long lng = x;

        assert(test_atomic_exchange8(&chr, (char) ~x) == (char) x);
        assert(chr == (char) ~x);

        assert(test_atomic_exchange16(&shrt, (short) ~x) == (short) x);
        assert(shrt == (short) ~x);

        assert(test_atomic_exchange32(&integer, (int) ~x) == (int) x);
        assert(integer == (int) ~x);

        assert(test_atomic_exchange64(&lng, (long) ~x) == (long) x);
        assert(lng == (long) ~x);
    }

    for (long x = -4096; x < 4096; x++) {
        _Atomic char chr = x;
        _Atomic short shrt = x;
        _Atomic int integer = x;
        _Atomic long lng = x;

        assert(test2_atomic_exchange8(&chr, (char) ~x) == (char) x);
        assert(chr == (char) ~x);

        assert(test2_atomic_exchange16(&shrt, (short) ~x) == (short) x);
        assert(shrt == (short) ~x);

        assert(test2_atomic_exchange32(&integer, (int) ~x) == (int) x);
        assert(integer == (int) ~x);

        assert(test2_atomic_exchange64(&lng, (long) ~x) == (long) x);
        assert(lng == (long) ~x);
    }
    return EXIT_SUCCESS;
}
