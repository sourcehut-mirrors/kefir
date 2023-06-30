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
#include "./definitions.h"

#define EPSILON_LD 1e-8

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
    }

    return EXIT_SUCCESS;
}
