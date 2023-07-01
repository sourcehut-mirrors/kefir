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

#define BOOL_SAME(_x, _y) (((_x) && (_y)) || (!(_x) && !(_y)))

int main(void) {
    for (long i = -100; i <= 100; i++) {
        _Bool b = long_to_bool(i);
        assert(BOOL_SAME(b, (_Bool) i));
        if (i >= 0) {
            b = ulong_to_bool((unsigned long) i);
            assert(BOOL_SAME(b, (_Bool) (unsigned long) i));
        }
    }

    for (float i = -100.0f; i < 100.0f; i += 0.1f) {
        _Bool b = float_to_bool(i);
        assert(BOOL_SAME(b, (_Bool) i));

        b = double_to_bool((double) i);
        assert(BOOL_SAME(b, (_Bool) (double) i));

        b = long_double_to_bool((long double) i);
        assert(BOOL_SAME(b, (_Bool) (long double) i));
    }
    return EXIT_SUCCESS;
}
