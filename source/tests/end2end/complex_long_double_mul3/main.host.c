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

static void test(_Complex long double x, _Complex long double y) {
    _Complex long double expected = x * y;
    _Complex long double res = mul(x, y);

    long double exp_real = creall(expected);
    long double exp_imag = cimagl(expected);
    long double res_real = creall(res);
    long double res_imag = cimagl(res);
    assert(fabsl(exp_real - res_real) < 1e-9L || (isnan(exp_real) && isnan(res_real)) ||
           (isinf(exp_real) && isinf(res_real) && signbit(exp_real) == signbit(res_real)));
    assert(fabsl(exp_imag - res_imag) < 1e-9L || (isnan(exp_imag) && isnan(res_imag)) ||
           (isinf(exp_imag) && isinf(res_imag) && signbit(exp_imag) == signbit(res_imag)));
}

int main(void) {
    test(3.14L + 2.718 * I, 4.52L - 2.42L * I);
    test(3.14L + 2.718 * I, nanl(""));
    test(nanl(""), 4.52L - 2.42L * I);
    test(nanl(""), nanl(""));
    test(3.14L + 2.718 * I, INFINITY);
    test(3.14L + 2.718 * I, -INFINITY);
    test(INFINITY, 4.52L - 2.42L * I);
    test(-INFINITY, 4.52L - 2.42L * I);
    test(INFINITY, INFINITY);
    test(INFINITY, -INFINITY);
    test(-INFINITY, INFINITY);
    test(-INFINITY, -INFINITY);
    test(3.14L + 2.718 * I, 0.0L * I);
    test(3.14L + 2.718 * I, 0.0L * I + 2.0L);
    test(0.0L * I, 4.52L - 2.42L * I);
    test(0.0L * I + 2.0L, 4.52L - 2.42L * I);
    test(INFINITY, 0.0L);
    test(INFINITY, 0.0L * I);
    test(INFINITY, -1.0L * I);
    test(INFINITY, nanl(""));
    test(-INFINITY, 0.0L);
    test(-INFINITY, 0.0L * I);
    test(-INFINITY, -1.0L * I);
    test(-INFINITY, nanl(""));
    test(0.0L, INFINITY);
    test(0.0L * I, INFINITY);
    test(-1.0L * I, INFINITY);
    test(nanl(""), INFINITY);
    test(0.0L, -INFINITY);
    test(0.0L * I, -INFINITY);
    test(-1.0L * I, -INFINITY);
    test(nanl(""), -INFINITY);
    return EXIT_SUCCESS;
}
