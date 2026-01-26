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
    assert(f80_size == sizeof(_Complex long double));
    assert(f80_alignment == _Alignof(_Complex long double));
    assert(fabsl(creall(*f80_const_ptr) - 2.71828f) < 1e-6);
    assert(fabsl(cimagl(*f80_const_ptr) - 9.831f) < 1e-6);
    assert(f80_compat[0] == -1);
    assert(f80_compat[1] == -1);
    assert(f80_compat[2] == -1);
    assert(f80_compat[3] == -1);
    assert(f80_compat[4] == -1);
    assert(f80_compat[5] == -1);
    assert(f80_compat[6] == -1);
    assert(f80_compat[7] == -1);

    int i = 0;
    assert(fabsl(creall(f80[i]) - ((long double) 3.14159f)) < 1e-6);
    assert(fabsl(cimagl(f80[i++]) - ((long double) -123.1f)) < 1e-6);
    assert(fabsl(creall(f80[i]) - (-((long double) 1902.318f))) < 1e-6);
    assert(fabsl(cimagl(f80[i++]) - ((long double) 90.4)) < 1e-5);
    _Complex long double exp =
        ((_Complex long double) 273.3f - 638.1 * I) + ((_Complex long double) 1902.318f + 90.31 * I);
    assert(fabsl(creall(f80[i]) - creall(exp)) < 1e-6);
    assert(fabsl(cimagl(f80[i++]) - cimagl(exp)) < 1e-4);
    exp = ((_Complex long double) 273.3f - 638.1 * I) - ((_Complex long double) 1902.318f + 90.31 * I);
    assert(fabsl(creall(f80[i]) - creall(exp)) < 1e-6);
    assert(fabsl(cimagl(f80[i++]) - cimagl(exp)) < 1e-4);
    exp = ((_Complex long double) 273.3f - 638.1 * I) * ((_Complex long double) 1902.318f + 90.31 * I);
    assert(fabsl(creall(f80[i]) - creall(exp)) < 1e-2);
    assert(fabsl(cimagl(f80[i++]) - cimagl(exp)) < 1e-1);
    exp = ((_Complex long double) 273.3f - 638.1 * I) / ((_Complex long double) 1902.318f + 90.31 * I);
    assert(fabsl(creall(f80[i]) - creall(exp)) < 1e-6);
    assert(fabsl(cimagl(f80[i++]) - cimagl(exp)) < 1e-6);
    exp = (273.3f + 1.21 * I) + ((_Complex long double) 1902.318f - 99.131 * I),
    assert(fabsl(creall(f80[i]) - creall(exp)) < 1e-6);
    assert(fabsl(cimagl(f80[i++]) - cimagl(exp)) < 1e-4);
    exp = (273.3f + 1.21 * I) - ((_Complex long double) 1902.318f - 99.131 * I),
    assert(fabsl(creall(f80[i]) - creall(exp)) < 1e-6);
    assert(fabsl(cimagl(f80[i++]) - cimagl(exp)) < 1e-4);
    exp = (273.3f + 1.21 * I) * ((_Complex long double) 1902.318f - 99.131 * I),
    assert(fabsl(creall(f80[i]) - creall(exp)) < 1e-6);
    assert(fabsl(cimagl(f80[i++]) - cimagl(exp)) < 1e-2);
    exp = (273.3f + 1.21 * I) / ((_Complex long double) 1902.318f - 99.131 * I),
    assert(fabsl(creall(f80[i]) - creall(exp)) < 1e-6);
    assert(fabsl(cimagl(f80[i++]) - cimagl(exp)) < 1e-2);
    exp = ((long double) 273.3f - 99.3145 * I) + ((_Complex long double) 1902.318f + 1.0004e6 * I),
    assert(fabsl(creall(f80[i]) - creall(exp)) < 1e-6);
    assert(fabsl(cimagl(f80[i++]) - cimagl(exp)) < 1e-4);
    exp = ((long double) 273.3f - 99.3145 * I) - ((_Complex long double) 1902.318f + 1.0004e6 * I),
    assert(fabsl(creall(f80[i]) - creall(exp)) < 1e-6);
    assert(fabsl(cimagl(f80[i++]) - cimagl(exp)) < 1e-4);
    exp = ((long double) 273.3f - 99.3145 * I) * ((_Complex long double) 1902.318f + 1.0004e6 * I),
    assert(fabsl(creall(f80[i]) - creall(exp)) < 1e1);
    assert(fabsl(cimagl(f80[i++]) - cimagl(exp)) < 1e-2);
    exp = ((long double) 273.3f - 99.3145 * I) / ((_Complex long double) 1902.318f + 1.0004e6 * I);
    assert(fabsl(creall(f80[i]) - creall(exp)) < 1e-6);
    assert(fabsl(cimagl(f80[i++]) - cimagl(exp)) < 1e-2);

    _Complex long double res = get80_1();
    exp = 5.428f - 8842.3 * I;
    assert(fabsl(creall(res) - creall(exp)) < 1e-5);
    assert(fabsl(cimagl(res) - cimagl(exp)) < 1e-2);
    res = get80_2();
    exp = 2.71828 + 9.831 * I;
    assert(fabsl(creall(res) - creall(exp)) < 1e-5);
    assert(fabsl(cimagl(res) - cimagl(exp)) < 1e-5);
    res = neg80(-42842.31f + 90.56 * I);
    exp = -(-42842.31f + 90.56 * I);
    assert(fabsl(creall(res) - creall(exp)) < 1e-5);
    assert(fabsl(cimagl(res) - cimagl(exp)) < 1e-5);
    res = add80(-42842.31f + 90.56 * I, 3949.58f - 998.42 * I);
    exp = (-42842.31f + 90.56 * I) + (3949.58f - 998.42 * I);
    assert(fabsl(creall(res) - creall(exp)) < 1e-5);
    assert(fabsl(cimagl(res) - cimagl(exp)) < 1e-5);
    res = sub80(-42842.31f + 90.56 * I, 3949.58f - 998.42 * I);
    exp = (-42842.31f + 90.56 * I) - (3949.58f - 998.42 * I);
    assert(fabsl(creall(res) - creall(exp)) < 1e-5);
    assert(fabsl(cimagl(res) - cimagl(exp)) < 1e-5);
    res = mul80(-422.31f + 90.56 * I, 39.58f - 98.42 * I);
    exp = (-422.31f + 90.56 * I) * (39.58f - 98.42 * I);
    assert(fabsl(creall(res) - creall(exp)) < 1e-3);
    assert(fabsl(cimagl(res) - cimagl(exp)) < 1e-3);
    res = div80(-422.31f + 90.56 * I, 39.58f - 98.42 * I);
    exp = (-422.31f + 90.56 * I) / (39.58f - 98.42 * I);
    assert(fabsl(creall(res) - creall(exp)) < 1e-3);
    assert(fabsl(cimagl(res) - cimagl(exp)) < 1e-3);
    assert(fabsl(creall(conv1(-42849)) + 42849) < 1e-5);
    assert(fabsl(cimagl(conv1(-42849))) < 1e-5);
    assert(fabsl(creall(conv2(130015)) - 130015) < 1e-5);
    assert(fabsl(cimagl(conv2(130015))) < 1e-5);
    assert(fabsl(creall(conv3(9.130e1f)) - 9.130e1f) < 1e-5);
    assert(fabsl(cimagl(conv3(9.130e1f))) < 1e-5);
    assert(fabsl(creall(conv4(9.130e-1)) - 9.130e-1f) < 1e-5);
    assert(fabsl(cimagl(conv4(9.130e-1))) < 1e-5);
    assert(fabsl(creall(conv5(-1831.41L)) + 1831.41f) < 1e-2);
    assert(fabsl(cimagl(conv5(-1831.41L))) < 1e-5);
    exp = (_Complex long double) 4819.44 + 10031.4 * I;
    res = conv6(4819.44 + 10031.4 * I);
    assert(fabsl(creall(res) - creall(exp)) < 1e-2);
    assert(fabsl(cimagl(res) - cimagl(exp)) < 1e-2);
    exp = (_Complex double) 4819.44 + 10031.4 * I;
    res = conv7(4819.44 + 10031.4 * I);
    assert(fabsl(creall(res) - creall(exp)) < 1e-5);
    assert(fabsl(cimagl(res) - cimagl(exp)) < 1e-5);
    exp = (_Complex long double) 4819.44 + 10031.4 * I;
    res = conv8(4819.44 + 10031.4 * I);
    assert(fabsl(creall(res) - creall(exp)) < 1e-5);
    assert(fabsl(cimagl(res) - cimagl(exp)) < 1e-5);
    return EXIT_SUCCESS;
}
