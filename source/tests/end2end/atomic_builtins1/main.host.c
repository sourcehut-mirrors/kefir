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
#include <math.h>
#include <complex.h>
#include "./definitions.h"

#define EPSILON_F 1e-3
#define EPSILON_D 1e-6
#define EPSILON_LD 1e-8

int main(void) {
    for (long x = -4096; x < 4096; x++) {
        _Atomic char chr;
        _Atomic short shrt;
        _Atomic int integer;
        _Atomic long lng;
        _Atomic long double ldbl;
        _Atomic _Complex long double cldbl;

        test_atomic_store8(&chr, (char) x);
        assert(chr == (char) x);

        test_atomic_store16(&shrt, (short) x);
        assert(shrt == (short) x);

        test_atomic_store32(&integer, (int) x);
        assert(integer == (int) x);

        test_atomic_store64(&lng, (long) x);
        assert(lng == (long) x);

        test_atomic_store128(&ldbl, (long double) x);
        assert(fabsl(ldbl - (long double) x) < EPSILON_LD);

        test_atomic_store256(&cldbl, (_Complex long double) x + I);
        assert(fabsl(creall(cldbl) - (long double) x) < EPSILON_LD);
        assert(fabsl(cimagl(cldbl) - (long double) 1) < EPSILON_LD);
    }

    for (long x = -4096; x < 4096; x++) {
        _Atomic char chr;
        _Atomic short shrt;
        _Atomic int integer;
        _Atomic long lng;
        _Atomic long double ldbl;
        _Atomic _Complex long double cldbl;

        test2_atomic_store8(&chr, (char) x);
        assert(chr == (char) x);

        test2_atomic_store16(&shrt, (short) x);
        assert(shrt == (short) x);

        test2_atomic_store32(&integer, (int) x);
        assert(integer == (int) x);

        test2_atomic_store64(&lng, (long) x);
        assert(lng == (long) x);

        test2_atomic_store128(&ldbl, (long double) x);
        assert(fabsl(ldbl - (long double) x) < EPSILON_LD);

        test2_atomic_store256(&cldbl, (_Complex long double) x + I);
        assert(fabsl(creall(cldbl) - (long double) x) < EPSILON_LD);
        assert(fabsl(cimagl(cldbl) - (long double) 1) < EPSILON_LD);
    }
    return EXIT_SUCCESS;
}
