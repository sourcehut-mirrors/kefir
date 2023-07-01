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
#include <math.h>
#include <string.h>
#include <limits.h>
#include "./definitions.h"

#define EPSILON_LD 1e-8
#define EPSILON_D 1e-6
#define EPSILON_F 1e-3

int main(void) {
    assert(fabsl(get_pi() - PI_LD) < EPSILON_LD);
    assert(fabsl(get_e() - E_LD) < EPSILON_LD);

    for (long double x = -100.0L; x < 100.0L; x += 0.1L) {
        assert(ldequals(x, x));
        assert(!ldequals(x, x + 0.001L));
        for (long double y = -5.0L; y < 5.0L; y += 0.05L) {
            assert(fabsl(addld(x, y) - (x + y)) < EPSILON_LD);
            assert(fabsl(subld(x, y) - (x - y)) < EPSILON_LD);
            assert(fabsl(mulld(x, y) - (x * y)) < EPSILON_LD);
            assert(fabsl(divld(x, y) - (x / y)) < EPSILON_LD);
            assert(ldgreater(x, y) == (x > y));
            assert(ldlesser(x, y) == (x < y));
        }
        assert(fabsl(negld(x) + x) < EPSILON_LD);

        assert(long_double_to_long(x) == (long) x);
        if (x >= 0.0L) {
            assert(long_double_to_ulong(x) == (unsigned long) x);
        }
        assert(long_double_trunc(x) == (x ? 1 : 0));
        _Bool b = long_double_to_bool(x);
        assert((b && (_Bool) x) || (!b && !(_Bool) x));

        assert(fabs(long_double_to_float(x) - (float) x) < EPSILON_F);
        assert(fabs(long_double_to_double(x) - (double) x) < EPSILON_D);
    }

    assert(long_double_to_ulong((long double) (ULONG_MAX - 1000000)) ==
           (unsigned long) (volatile long double){(long double) (ULONG_MAX - 1000000)});

    for (long i = -1000; i < 1000; i++) {
        assert(fabsl(long_to_long_double(i) - (long double) i) < EPSILON_LD);
    }

    for (unsigned long i = 0; i < 1000; i++) {
        assert(fabsl(ulong_to_long_double(i) - (long double) i) < EPSILON_LD);
        assert(fabsl(ulong_to_long_double(~i - 0xffff) - (long double) (~i - 0xffff)) < EPSILON_LD);
    }

    for (float i = -100.0f; i < 100.0f; i += 0.1f) {
        assert(fabsl(float_to_long_double(i) - (long double) i) < EPSILON_LD);
        assert(fabsl(double_to_long_double((double) i) - (long double) (double) i) < EPSILON_LD);
    }

    return EXIT_SUCCESS;
}
