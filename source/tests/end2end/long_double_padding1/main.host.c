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

#define EPSILON_LD 1e-8

int main(void) {
    for (unsigned long x = 0; x < 4096; x++) {
        add_ld1((long double) x);
        add_cld1(x - 1.0 * I);
    }

    long double expected_ld = 4096 * 4095 / 2;
    assert(fabsl(ld1 - expected_ld) < EPSILON_LD);

    long double expected_cld_real = 4096 * 4095 / 2;
    long double expected_cld_imag = -4096;
    assert(fabsl(creall(cld1) - expected_cld_real) < EPSILON_LD);
    assert(fabsl(cimagl(cld1) - expected_cld_imag) < EPSILON_LD);
    return EXIT_SUCCESS;
}
