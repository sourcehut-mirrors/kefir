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
    assert(f32x_size == sizeof(double));
    assert(f32x_alignment == _Alignof(double));
    assert(fabs(*f32x_const_ptr - 2.71828f) < 1e-6);
    assert(f32x_compat[0] == -1);
    assert(f32x_compat[1] == -1);
    assert(f32x_compat[2] == -1);
    assert(f32x_compat[3] == 2);
    assert(f32x_compat[4] == 3);
    assert(f32x_compat[5] == -1);
    assert(f32x_compat[6] == 2);
    assert(f32x_compat[7] == 3);

    int i = 0;
    assert(fabs(f32x[i++] - ((double) 3.14159f)) < 1e-6);
    assert(fabs(f32x[i++] - (-((double) 1902.318f))) < 1e-6);
    assert(fabs(f32x[i++] - (((double) 273.3f) + ((double) 1902.318f))) < 1e-6);
    assert(fabs(f32x[i++] - (((double) 273.3f) - ((double) 1902.318f))) < 1e-6);
    assert(fabs(f32x[i++] - (((double) 273.3f) * ((double) 1902.318f))) < 1e-6);
    assert(fabs(f32x[i++] - (((double) 273.3f) / ((double) 1902.318f))) < 1e-6);
    assert(fabs(f32x[i++] - ((273.3f) + ((double) 1902.318f))) < 1e-6);
    assert(fabs(f32x[i++] - ((273.3f) - ((double) 1902.318f))) < 1e-6);
    assert(fabs(f32x[i++] - ((273.3f) * ((double) 1902.318f))) < 1e-6);
    assert(fabs(f32x[i++] - ((273.3f) / ((double) 1902.318f))) < 1e-6);
    assert(fabs(f32x[i++] - (((double) 273.3f) + ((double) 1902.318f))) < 1e-4);
    assert(fabs(f32x[i++] - (((double) 273.3f) - ((double) 1902.318f))) < 1e-4);
    assert(fabs(f32x[i++] - (((double) 273.3f) * ((double) 1902.318f))) < 1e-1);
    assert(fabs(f32x[i++] - (((double) 273.3f) / ((double) 1902.318f))) < 1e-1);

    assert(fabs(get32x_1() - 5.428f) < 1e-5);
    assert(fabs(get32x_2() - 2.71828f) < 1e-5);
    assert(fabs(neg32x(-42842.31f) - 42842.31f) < 1e-5);
    assert(fabs(add32x(0.428f, 1.522f) - (0.428f + 1.522f)) < 1e-5);
    assert(fabs(sub32x(0.428f, 1.522f) - (0.428f - 1.522f)) < 1e-5);
    assert(fabs(mul32x(0.428f, 1.522f) - (0.428f * 1.522f)) < 1e-5);
    assert(fabs(div32x(0.428f, 1.522f) - (0.428f / 1.522f)) < 1e-5);
    assert(fabs(conv1(-42849) + 42849) < 1e-5);
    assert(fabs(conv2(130015) - 130015) < 1e-5);
    assert(fabs(conv3(9.130e1f) - 9.130e1) < 1e-5);
    assert(fabs(conv4(9.130e-1) - 9.130e-1) < 1e-5);
    assert(fabs(conv5(-1831.41L) + 1831.41) < 1e-5);
    return EXIT_SUCCESS;
}
