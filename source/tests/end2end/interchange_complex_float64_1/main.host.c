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
#include <complex.h>
#include "./definitions.h"

int main(void) {
    assert(f64_size == sizeof(_Complex double));
    assert(f64_alignment == _Alignof(_Complex double));
    assert(fabs(creal(*f64_const_ptr) - 2.71828f) < 1e-6);
    assert(fabs(cimag(*f64_const_ptr) - 9.831f) < 1e-6);
    assert(f64_compat[0] == -1);
    assert(f64_compat[1] == -1);
    assert(f64_compat[2] == -1);
    assert(f64_compat[3] == -1);
    assert(f64_compat[4] == 3);
    assert(f64_compat[5] == -1);
    assert(f64_compat[6] == -1);
    assert(f64_compat[7] == 3);

    int i = 0;
    assert(fabs(creal(f64[i]) - ((double) 3.14159f)) < 1e-6);
    assert(fabs(cimag(f64[i++]) - ((double) -123.1f)) < 1e-6);
    assert(fabs(creal(f64[i]) - (-((double) 1902.318f))) < 1e-6);
    assert(fabs(cimag(f64[i++]) - ((double) 90.4)) < 1e-5);
    _Complex double exp = ((_Complex double) 273.3f - 638.1 * I) + ((_Complex double) 1902.318f + 90.31 * I);
    assert(fabs(creal(f64[i]) - creal(exp)) < 1e-6);
    assert(fabs(cimag(f64[i++]) - cimag(exp)) < 1e-4);
    exp = ((_Complex double) 273.3f - 638.1 * I) - ((_Complex double) 1902.318f + 90.31 * I);
    assert(fabs(creal(f64[i]) - creal(exp)) < 1e-6);
    assert(fabs(cimag(f64[i++]) - cimag(exp)) < 1e-4);
    exp = ((_Complex double) 273.3f - 638.1 * I) * ((_Complex double) 1902.318f + 90.31 * I);
    assert(fabs(creal(f64[i]) - creal(exp)) < 1e-2);
    assert(fabs(cimag(f64[i++]) - cimag(exp)) < 1e-1);
    exp = ((_Complex double) 273.3f - 638.1 * I) / ((_Complex double) 1902.318f + 90.31 * I);
    assert(fabs(creal(f64[i]) - creal(exp)) < 1e-6);
    assert(fabs(cimag(f64[i++]) - cimag(exp)) < 1e-6);
    exp = (273.3f + 1.21 * I) + ((_Complex double) 1902.318f - 99.131 * I),
    assert(fabs(creal(f64[i]) - creal(exp)) < 1e-6);
    assert(fabs(cimag(f64[i++]) - cimag(exp)) < 1e-4);
    exp = (273.3f + 1.21 * I) - ((_Complex double) 1902.318f - 99.131 * I),
    assert(fabs(creal(f64[i]) - creal(exp)) < 1e-6);
    assert(fabs(cimag(f64[i++]) - cimag(exp)) < 1e-4);
    exp = (273.3f + 1.21 * I) * ((_Complex double) 1902.318f - 99.131 * I),
    assert(fabs(creal(f64[i]) - creal(exp)) < 1e-6);
    assert(fabs(cimag(f64[i++]) - cimag(exp)) < 1e-2);
    exp = (273.3f + 1.21 * I) / ((_Complex double) 1902.318f - 99.131 * I),
    assert(fabs(creal(f64[i]) - creal(exp)) < 1e-6);
    assert(fabs(cimag(f64[i++]) - cimag(exp)) < 1e-2);
    exp = ((double) 273.3f - 99.3145 * I) + ((_Complex double) 1902.318f + 1.0004e6 * I),
    assert(fabs(creal(f64[i]) - creal(exp)) < 1e-6);
    assert(fabs(cimag(f64[i++]) - cimag(exp)) < 1e-4);
    exp = ((double) 273.3f - 99.3145 * I) - ((_Complex double) 1902.318f + 1.0004e6 * I),
    assert(fabs(creal(f64[i]) - creal(exp)) < 1e-6);
    assert(fabs(cimag(f64[i++]) - cimag(exp)) < 1e-4);
    exp = ((double) 273.3f - 99.3145 * I) * ((_Complex double) 1902.318f + 1.0004e6 * I),
    assert(fabs(creal(f64[i]) - creal(exp)) < 1e1);
    assert(fabs(cimag(f64[i++]) - cimag(exp)) < 1e-2);
    exp = ((double) 273.3f - 99.3145 * I) / ((_Complex double) 1902.318f + 1.0004e6 * I);
    assert(fabs(creal(f64[i]) - creal(exp)) < 1e-6);
    assert(fabs(cimag(f64[i++]) - cimag(exp)) < 1e-2);

    _Complex double res = get64_1();
    exp = 5.428f - 8842.3 * I;
    assert(fabs(creal(res) - creal(exp)) < 1e-5);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-2);
    res = get64_2();
    exp = 2.71828 + 9.831 * I;
    assert(fabs(creal(res) - creal(exp)) < 1e-5);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-5);
    res = neg64(-42842.31f + 90.56 * I);
    exp = -(-42842.31f + 90.56 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-5);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-5);
    res = add64(-42842.31f + 90.56 * I, 3949.58f - 998.42 * I);
    exp = (-42842.31f + 90.56 * I) + (3949.58f - 998.42 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-5);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-5);
    res = sub64(-42842.31f + 90.56 * I, 3949.58f - 998.42 * I);
    exp = (-42842.31f + 90.56 * I) - (3949.58f - 998.42 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-5);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-5);
    res = mul64(-422.31f + 90.56 * I, 39.58f - 98.42 * I);
    exp = (-422.31f + 90.56 * I) * (39.58f - 98.42 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-3);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-3);
    res = div64(-422.31f + 90.56 * I, 39.58f - 98.42 * I);
    exp = (-422.31f + 90.56 * I) / (39.58f - 98.42 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-3);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-3);
    assert(fabs(creal(conv1(-42849)) + 42849) < 1e-5);
    assert(fabs(cimag(conv1(-42849))) < 1e-5);
    assert(fabs(creal(conv2(130015)) - 130015) < 1e-5);
    assert(fabs(cimag(conv2(130015))) < 1e-5);
    assert(fabs(creal(conv3(9.130e1f)) - 9.130e1f) < 1e-5);
    assert(fabs(cimag(conv3(9.130e1f))) < 1e-5);
    assert(fabs(creal(conv4(9.130e-1)) - 9.130e-1f) < 1e-5);
    assert(fabs(cimag(conv4(9.130e-1))) < 1e-5);
    assert(fabs(creal(conv5(-1831.41L)) + 1831.41f) < 1e-2);
    assert(fabs(cimag(conv5(-1831.41L))) < 1e-5);
    exp = (_Complex double) 4819.44 + 10031.4 * I;
    res = conv6(4819.44 + 10031.4 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-2);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-2);
    exp = (_Complex double) 4819.44 + 10031.4 * I;
    res = conv7(4819.44 + 10031.4 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-5);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-5);
    exp = (_Complex long double) 4819.44 + 10031.4 * I;
    res = conv8(4819.44 + 10031.4 * I);
    assert(fabs(creal(res) - creal(exp)) < 1e-5);
    assert(fabs(cimag(res) - cimag(exp)) < 1e-5);
    return EXIT_SUCCESS;
}
