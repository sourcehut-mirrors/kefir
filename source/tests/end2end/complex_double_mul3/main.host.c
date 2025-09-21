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
#include <complex.h>
#include "./definitions.h"

static void test(_Complex double x, _Complex double y) {
    _Complex double expected = x * y;
    _Complex double res = mul(x, y);

    double exp_real = creal(expected);
    double exp_imag = cimag(expected);
    double res_real = creal(res);
    double res_imag = cimag(res);
    assert(fabs(exp_real - res_real) < 1e-8 || (isnan(exp_real) && isnan(res_real)) ||
           (isinf(exp_real) && isinf(res_real) && signbit(exp_real) == signbit(res_real)));
    assert(fabs(exp_imag - res_imag) < 1e-8 || (isnan(exp_imag) && isnan(res_imag)) ||
           (isinf(exp_imag) && isinf(res_imag) && signbit(exp_imag) == signbit(res_imag)));
}

int main(void) {
    test(3.14 + 2.718 * I, 4.52 - 2.42 * I);
    test(3.14 + 2.718 * I, nanl(""));
    test(nanl(""), 4.52 - 2.42 * I);
    test(nanl(""), nanl(""));
    test(3.14 + 2.718 * I, INFINITY);
    test(3.14 + 2.718 * I, -INFINITY);
    test(INFINITY, 4.52 - 2.42 * I);
    test(-INFINITY, 4.52 - 2.42 * I);
    test(INFINITY, INFINITY);
    test(INFINITY, -INFINITY);
    test(-INFINITY, INFINITY);
    test(-INFINITY, -INFINITY);
    test(3.14 + 2.718 * I, 0.0 * I);
    test(3.14 + 2.718 * I, 0.0 * I + 2.0);
    test(0.0 * I, 4.52 - 2.42 * I);
    test(0.0 * I + 2.0, 4.52 - 2.42 * I);
    test(INFINITY, 0.0);
    test(INFINITY, 0.0 * I);
    test(INFINITY, -1.0 * I);
    test(INFINITY, nanl(""));
    test(-INFINITY, 0.0);
    test(-INFINITY, 0.0 * I);
    test(-INFINITY, -1.0 * I);
    test(-INFINITY, nanl(""));
    test(0.0, INFINITY);
    test(0.0 * I, INFINITY);
    test(-1.0 * I, INFINITY);
    test(nanl(""), INFINITY);
    test(0.0, -INFINITY);
    test(0.0 * I, -INFINITY);
    test(-1.0 * I, -INFINITY);
    test(nanl(""), -INFINITY);
    return EXIT_SUCCESS;
}
