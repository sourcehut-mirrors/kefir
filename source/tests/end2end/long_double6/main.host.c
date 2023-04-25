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
    for (int i = -1000; i < 1000; i++) {
        struct S s = init_s(i, i + 2000, i * 0.25);
        assert(fabsl(s.ldl[0] - (long double) i) < 1e-6);
        assert(fabsl(s.ldl[1] - (long double) (i + 2000)) < 1e-6);
        assert(fabsl(s.ldl[2] - (long double) (i * 0.25)) < 1e-6);
    }
    return EXIT_SUCCESS;
}
