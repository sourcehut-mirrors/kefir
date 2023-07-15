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
#include "./definitions.h"

int main(void) {
    for (double x = -100.0; x < 100.0; x += 0.5) {
        long double lx = (long double) x;
        for (double y = -5.0; y < 5.0; y += 0.1) {
            long double ly = (long double) y;
            assert(greater_or_equalsd(x, y) == (x >= y ? 1 : 0));
            assert(less_or_equalsd(x, y) == (x <= y ? 1 : 0));

            assert(greater_or_equalsld(lx, ly) == (lx >= ly ? 1 : 0));
            assert(less_or_equalsld(lx, ly) == (lx <= ly ? 1 : 0));
        }
    }
    return EXIT_SUCCESS;
}
