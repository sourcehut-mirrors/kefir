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

#define EPSILON_F 1e-3f
#define EPSILON_D 1e-6

int main(void) {
    assert(fabs(PI_float() - PI_F) < EPSILON_F);
    assert(fabs(PI_double() - PI_D) < EPSILON_D);
    assert(fabs(E_float() - E_F) < EPSILON_F);
    assert(fabs(E_double() - E_D) < EPSILON_D);

    for (long i = -1000; i <= 1000; i++) {
        assert(fabs(long_to_float(i) - (float) i) < EPSILON_F);
        assert(fabs(long_to_double(i) - (double) i) < EPSILON_D);
        assert(fabs(ulong_to_float((unsigned long) i) - (float) (unsigned long) i) < EPSILON_F);
        assert(fabs(ulong_to_double((unsigned long) i) - (double) (unsigned long) i) < EPSILON_D);
    }

    for (double i = -100.0; i < 100.0; i += 0.1) {
        assert(float_to_long((float) i) == (long) (float) i);
        assert(double_to_long(i) == (long) i);
        assert(float_to_ulong((float) i) == (unsigned long) (float) i);
        assert(double_to_ulong(i) == (unsigned long) i);
        assert(fabs(double_to_float(i) - (float) i) < EPSILON_F);
        assert(fabs(float_to_double((float) i) - (double) (float) i) < EPSILON_D);
    }

    return EXIT_SUCCESS;
}
