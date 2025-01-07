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
#include "./definitions.h"

int main(void) {
    for (long double x = -100.0l; x < 100.0l; x += 0.1l) {
        assert(eq(x, x));
        assert(neq(x, x + 1));
        for (long double y = -100.0l; y < 100.0l; y += 0.1l) {
            assert(gr(x, y) == (x > y));
            assert(ge(x, y) == (x >= y));
            assert(lt(x, y) == (x < y));
            assert(le(x, y) == (x <= y));
        }
    }
    return EXIT_SUCCESS;
}
