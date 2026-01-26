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

int main(void) {
    assert(f64x_size == sizeof(long double));
    assert(f64x_alignment == _Alignof(long double));
    assert(fabsl(*f64x_const_ptr - 2.71828f) < 1e-6);
    assert(f64x_compat[0] == -1);
    assert(f64x_compat[1] == -1);
    assert(f64x_compat[2] == -1);
    assert(f64x_compat[3] == -1);
    assert(f64x_compat[4] == 3);
    assert(f64x_compat[5] == -1);
    assert(f64x_compat[6] == -1);
    assert(f64x_compat[7] == 3);

    int i = 0;
    assert(fabsl(f64x[i++] - ((long double) 3.14159f)) < 1e-6);
    assert(fabsl(f64x[i++] - (-((long double) 1902.318f))) < 1e-6);
    assert(fabsl(f64x[i++] - (((long double) 273.3f) + ((long double) 1902.318f))) < 1e-6);
    assert(fabsl(f64x[i++] - (((long double) 273.3f) - ((long double) 1902.318f))) < 1e-6);
    assert(fabsl(f64x[i++] - (((long double) 273.3f) * ((long double) 1902.318f))) < 1e-6);
    assert(fabsl(f64x[i++] - (((long double) 273.3f) / ((long double) 1902.318f))) < 1e-6);
    assert(fabsl(f64x[i++] - ((273.3f) + ((long double) 1902.318f))) < 1e-6);
    assert(fabsl(f64x[i++] - ((273.3f) - ((long double) 1902.318f))) < 1e-6);
    assert(fabsl(f64x[i++] - ((273.3f) * ((long double) 1902.318f))) < 1e-6);
    assert(fabsl(f64x[i++] - ((273.3f) / ((long double) 1902.318f))) < 1e-6);
    assert(fabsl(f64x[i++] - (((long double) 273.3f) + ((long double) 1902.318f))) < 1e-4);
    assert(fabsl(f64x[i++] - (((long double) 273.3f) - ((long double) 1902.318f))) < 1e-4);
    assert(fabsl(f64x[i++] - (((long double) 273.3f) * ((long double) 1902.318f))) < 1e-1);
    assert(fabsl(f64x[i++] - (((long double) 273.3f) / ((long double) 1902.318f))) < 1e-1);

    assert(fabsl(get64x_1() - 5.428f) < 1e-5);
    assert(fabsl(get64x_2() - 2.71828f) < 1e-5);
    assert(fabsl(neg64x(-42842.31f) - 42842.31f) < 1e-5);
    assert(fabsl(add64x(0.428f, 1.522f) - (0.428f + 1.522f)) < 1e-5);
    assert(fabsl(sub64x(0.428f, 1.522f) - (0.428f - 1.522f)) < 1e-5);
    assert(fabsl(mul64x(0.428f, 1.522f) - (0.428f * 1.522f)) < 1e-5);
    assert(fabsl(div64x(0.428f, 1.522f) - (0.428f / 1.522f)) < 1e-5);
    assert(fabsl(conv1(-42849) + 42849) < 1e-5);
    assert(fabsl(conv2(130015) - 130015) < 1e-5);
    assert(fabsl(conv3(9.130e1f) - 9.130e1) < 1e-5);
    assert(fabsl(conv4(9.130e-1) - 9.130e-1) < 1e-5);
    assert(fabsl(conv5(-1831.41L) + 1831.41) < 1e-5);
    return EXIT_SUCCESS;
}
