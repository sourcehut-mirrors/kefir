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
#include "./definitions.h"

int main(void) {
    for (long i = -1000; i <= 1000; i++) {
        assert(long_double_to_bool((long double) i) == (_Bool) (long double) i);
        assert(double_to_bool((double) i) == (_Bool) (double) i);
        assert(float_to_bool((float) i) == (_Bool) (float) i);
        assert(long_to_bool(i) == (_Bool) i);
        if (i >= 0) {
            assert(ulong_to_bool((unsigned long) i) == (_Bool) (unsigned long) i);
        }
    }

    assert(long_double_to_bool(-0.0L) == (_Bool) -0.0L);
    assert(long_double_to_bool(0.0L) == (_Bool) 0.0L);
    assert(long_double_to_bool(+0.0L) == (_Bool) + 0.0L);
    assert(long_double_to_bool(-INFINITY) == (_Bool) -INFINITY);
    assert(long_double_to_bool(INFINITY) == (_Bool) INFINITY);
    assert(long_double_to_bool(NAN) == (_Bool) NAN);

    assert(double_to_bool(-0.0) == (_Bool) -0.0);
    assert(double_to_bool(0.0) == (_Bool) 0.0);
    assert(double_to_bool(+0.0) == (_Bool) + 0.0);
    assert(double_to_bool(-INFINITY) == (_Bool) -INFINITY);
    assert(double_to_bool(INFINITY) == (_Bool) INFINITY);
    assert(double_to_bool(NAN) == (_Bool) NAN);

    assert(float_to_bool(-0.0f) == (_Bool) -0.0f);
    assert(float_to_bool(0.0f) == (_Bool) 0.0f);
    assert(float_to_bool(+0.0f) == (_Bool) + 0.0f);
    assert(float_to_bool(-INFINITY) == (_Bool) -INFINITY);
    assert(float_to_bool(INFINITY) == (_Bool) INFINITY);
    assert(float_to_bool(NAN) == (_Bool) NAN);
    return EXIT_SUCCESS;
}
