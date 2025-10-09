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
    assert(f80_size == sizeof(long double));
    assert(f80_alignment == _Alignof(long double));
    assert(fabsl(*f80_const_ptr - 2.71828f) < 1e-6);
    assert(f80_compat[0] == -1);
    assert(f80_compat[1] == -1);
    assert(f80_compat[2] == -1);
    assert(f80_compat[3] == -1);
    assert(f80_compat[4] == -1);
    assert(f80_compat[5] == -1);
    assert(f80_compat[6] == -1);
    assert(f80_compat[7] == -1);

    int i = 0;
    assert(fabsl(f80[i++] - ((long double) 3.14159f)) < 1e-6);
    assert(fabsl(f80[i++] - (-((long double) 1902.318f))) < 1e-6);
    assert(fabsl(f80[i++] - (((long double) 273.3f) + ((long double) 1902.318f))) < 1e-6);
    assert(fabsl(f80[i++] - (((long double) 273.3f) - ((long double) 1902.318f))) < 1e-6);
    assert(fabsl(f80[i++] - (((long double) 273.3f) * ((long double) 1902.318f))) < 1e-6);
    assert(fabsl(f80[i++] - (((long double) 273.3f) / ((long double) 1902.318f))) < 1e-6);
    assert(fabsl(f80[i++] - ((273.3f) + ((long double) 1902.318f))) < 1e-6);
    assert(fabsl(f80[i++] - ((273.3f) - ((long double) 1902.318f))) < 1e-6);
    assert(fabsl(f80[i++] - ((273.3f) * ((long double) 1902.318f))) < 1e-6);
    assert(fabsl(f80[i++] - ((273.3f) / ((long double) 1902.318f))) < 1e-6);
    assert(fabsl(f80[i++] - (((long double) 273.3f) + ((long double) 1902.318f))) < 1e-4);
    assert(fabsl(f80[i++] - (((long double) 273.3f) - ((long double) 1902.318f))) < 1e-4);
    assert(fabsl(f80[i++] - (((long double) 273.3f) * ((long double) 1902.318f))) < 1e-1);
    assert(fabsl(f80[i++] - (((long double) 273.3f) / ((long double) 1902.318f))) < 1e-1);

    assert(fabsl(get80_1() - 5.428f) < 1e-5);
    assert(fabsl(get80_2() - 2.71828f) < 1e-5);
    assert(fabsl(neg80(-42842.31f) - 42842.31f) < 1e-5);
    assert(fabsl(add80(0.428f, 1.522f) - (0.428f + 1.522f)) < 1e-5);
    assert(fabsl(sub80(0.428f, 1.522f) - (0.428f - 1.522f)) < 1e-5);
    assert(fabsl(mul80(0.428f, 1.522f) - (0.428f * 1.522f)) < 1e-5);
    assert(fabsl(div80(0.428f, 1.522f) - (0.428f / 1.522f)) < 1e-5);
    assert(fabsl(conv1(-42849) + 42849) < 1e-5);
    assert(fabsl(conv2(130015) - 130015) < 1e-5);
    assert(fabsl(conv3(9.130e1f) - 9.130e1) < 1e-5);
    assert(fabsl(conv4(9.130e-1) - 9.130e-1) < 1e-5);
    assert(fabsl(conv5(-1831.41L) + 1831.41) < 1e-5);
    return EXIT_SUCCESS;
}
