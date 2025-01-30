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

_Complex float x;
float _Complex y;
_Complex double a;
double _Complex b;
_Complex long double i;
_Complex double long j;
long _Complex double k;
long double _Complex l;
double _Complex long m;
double long _Complex n;

#define EPSILON_F 1e-3
#define EPSILON_D 1e-6
#define EPSILON_LD 1e-8

#ifdef CMPLXF
#undef CMPLXF
#endif

#define CMPLXF(_a, _b) ((float) (_a) + I * (float) (b))

#ifdef CMPLX
#undef CMPLX
#endif
#define CMPLX(_a, _b) ((double) (_a) + I * (double) (b))

#ifdef CMPLXL
#undef CMPLXL
#endif
#define CMPLXL(_a, _b) ((long double) (_a) + I * (long double) (b))

int main(void) {
    for (double real = -10.0; real < 10.0; real += 0.1) {
        for (double imaginary = -10.0; imaginary < 10.0; imaginary += 0.1) {
            x = CMPLXF(real, imaginary);
            y = CMPLXF(real + 1.0, imaginary - 1.0);
            a = CMPLX(real + 2.0, imaginary - 2.0);
            b = CMPLX(real + 3.0, imaginary - 3.0);
            i = CMPLXL(real + 4.0, imaginary - 4.0);
            j = CMPLXL(real + 5.0, imaginary - 5.0);
            k = CMPLXL(real + 6.0, imaginary - 6.0);
            l = CMPLXL(real + 7.0, imaginary - 7.0);
            m = CMPLXL(real + 8.0, imaginary - 8.0);
            n = CMPLXL(real + 9.0, imaginary - 9.0);

            assert(fabs(crealf(x) - crealf(get_x())) < EPSILON_F);
            assert(fabs(cimagf(x) - cimagf(get_x())) < EPSILON_F);
            assert(fabs(crealf(y) - crealf(get_y())) < EPSILON_F);
            assert(fabs(cimagf(y) - cimagf(get_y())) < EPSILON_F);

            assert(fabs(creal(a) - creal(get_a())) < EPSILON_D);
            assert(fabs(cimag(a) - cimag(get_a())) < EPSILON_D);
            assert(fabs(creal(b) - creal(get_b())) < EPSILON_D);
            assert(fabs(cimag(b) - cimag(get_b())) < EPSILON_D);

            assert(fabsl(creall(i) - creall(get_i())) < EPSILON_LD);
            assert(fabsl(cimagl(i) - cimagl(get_i())) < EPSILON_LD);
            assert(fabsl(creall(j) - creall(get_j())) < EPSILON_LD);
            assert(fabsl(cimagl(j) - cimagl(get_j())) < EPSILON_LD);
            assert(fabsl(creall(k) - creall(get_k())) < EPSILON_LD);
            assert(fabsl(cimagl(k) - cimagl(get_k())) < EPSILON_LD);
            assert(fabsl(creall(l) - creall(get_l())) < EPSILON_LD);
            assert(fabsl(cimagl(l) - cimagl(get_l())) < EPSILON_LD);
            assert(fabsl(creall(m) - creall(get_m())) < EPSILON_LD);
            assert(fabsl(cimagl(m) - cimagl(get_m())) < EPSILON_LD);
            assert(fabsl(creall(n) - creall(get_n())) < EPSILON_LD);
            assert(fabsl(cimagl(n) - cimagl(get_n())) < EPSILON_LD);
        }
    }

    assert(fabs(crealf(nonef())) < EPSILON_F);
    assert(fabs(cimagf(nonef())) < EPSILON_F);
    assert(fabs(creal(none())) < EPSILON_D);
    assert(fabs(cimag(none())) < EPSILON_D);
    assert(fabsl(creall(nonel())) < EPSILON_LD);
    assert(fabsl(cimagl(nonel())) < EPSILON_LD);
    return EXIT_SUCCESS;
}
