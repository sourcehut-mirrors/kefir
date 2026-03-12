/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#define EPSILON_F 1e-3
#define EPSILON_D 1e-6

int main(void) {
    _Atomic float f32;
    _Atomic double f64;
    for (double i = -100.0; i < 100.0; i += 0.1) {
        float_store(&f32, i);
        double_store(&f64, i);

        assert(fabs(f32 - i) < 1e-5);
        assert(fabs(f64 - i) < 1e-6);

        assert(fabs(float_exchange(&f32, -i) - i) < 1e-5);
        assert(fabs(double_exchange(&f64, -i) - i) < 1e-6);

        assert(fabs(f32 + i) < 1e-5);
        assert(fabs(f64 + i) < 1e-6);

        assert(!float_compare_exchange(&f32, -1000000, 136713));
        assert(float_compare_exchange(&f32, -i, 136713));
        assert(fabs(f32 - 136713) < 1e-5);

        assert(!double_compare_exchange(&f64, -1000000, 136713));
        assert(double_compare_exchange(&f64, -i, 136713));
        assert(fabs(f64 - 136713) < 1e-5);
    }
    return EXIT_SUCCESS;
}
