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
#include <math.h>
#include <assert.h>
#include "./definitions.h"

int main(void) {
    for (int i = -1000; i < 1000; i++) {
        long double res = get(i, (long double) i, (long double) ~i);
        if (i > 0) {
            if (i % 2 == 0) {
                assert(fabsl(res - (long double) ~i) < 1e-8);
            } else {
                assert(fabsl(res - (long double) i) < 1e-8);
            }
        } else {
            assert(fabsl(res + 1.0L) < 1e-8);
        }
    }
    return EXIT_SUCCESS;
}
