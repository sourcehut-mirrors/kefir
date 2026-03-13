/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

int main(void) {
    _Atomic char chr;
    _Atomic short shrt;
    _Atomic int integer;
    _Atomic long lng;

    test_atomic_store8(&chr);
    assert(chr == 1);
    chr = 100;
    assert(test_atomic_load8(&chr) == 100);

    test_atomic_store16(&shrt);
    assert(shrt == 1);
    shrt = 100;
    assert(test_atomic_load16(&shrt) == 100);

    test_atomic_store32(&integer);
    assert(integer == 1);
    integer = 100;
    assert(test_atomic_load32(&integer) == 100);

    test_atomic_store64(&lng);
    assert(lng == 1);
    lng = 100;
    assert(test_atomic_load64(&lng) == 100);

    assert(test_atomic_exchange8(&chr) == 100);
    assert(chr == 123);

    assert(test_atomic_exchange16(&shrt) == 100);
    assert(shrt == 123);

    assert(test_atomic_exchange32(&integer) == 100);
    assert(integer == 123);

    assert(test_atomic_exchange64(&lng) == 100);
    assert(lng == 123);

    assert(test_atomic_compare_exchange8(&chr));
    assert(chr == 23);
    assert(!test_atomic_compare_exchange8(&chr));
    assert(chr == 23);

    assert(test_atomic_compare_exchange16(&shrt));
    assert(shrt == 23);
    assert(!test_atomic_compare_exchange16(&shrt));
    assert(shrt == 23);

    assert(test_atomic_compare_exchange32(&integer));
    assert(integer == 23);
    assert(!test_atomic_compare_exchange32(&integer));
    assert(integer == 23);

    assert(test_atomic_compare_exchange64(&lng));
    assert(lng == 23);
    assert(!test_atomic_compare_exchange64(&lng));
    assert(lng == 23);

    assert(test_atomic_fetch_add8(&chr) == 23);
    assert(chr == 63);

    assert(test_atomic_fetch_add16(&shrt) == 23);
    assert(shrt == 63);

    assert(test_atomic_fetch_add32(&integer) == 23);
    assert(integer == 63);

    assert(test_atomic_fetch_add64(&lng) == 23);
    assert(lng == 63);
    return EXIT_SUCCESS;
}
