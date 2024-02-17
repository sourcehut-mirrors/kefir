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
#include "./definitions.h"

_Atomic long double ld1;

#define EPSILON_LD 1e-8

int main(void) {
    for (long double i = -100.0L; i < 100.0L; i += 0.2L) {
        set_ld1(i);
        assert(fabsl(ld1 - i) < EPSILON_LD);

        ld1 = -i / 2;
        assert(fabsl(get_ld1() + i / 2) < EPSILON_LD);
    }
    return EXIT_SUCCESS;
}
