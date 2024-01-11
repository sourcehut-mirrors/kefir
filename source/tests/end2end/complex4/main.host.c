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
#include <complex.h>
#include "./definitions.h"

#define EPSILON_F 1e-3
#define EPSILON_D 1e-6
#define EPSILON_LD 1e-8

int main(void) {
    for (double a = -10.0; a < 10.0; a += 0.1) {
        for (double b = -10.0; b < 10.0; b += 0.1) {
            _Complex float f32x = (float) a + I * (float) b;
            _Complex float f32y = (float) b + I * (float) a;
            _Complex double f64x = (double) a + I * (double) b;
            _Complex double f64y = (double) b + I * (double) a;
            _Complex long double ldx = (long double) a + I * (long double) b;
            _Complex long double ldy = (long double) b + I * (long double) a;

            _Complex float f32addres = cmpf32add(f32x, f32y);
            _Complex float f32add = f32x + f32y;
            assert(fabs(crealf(f32addres) - crealf(f32add)) < EPSILON_F);
            assert(fabs(cimagf(f32addres) - cimagf(f32add)) < EPSILON_F);

            _Complex double f64addres = cmpf64add(f64x, f64y);
            _Complex double f64add = f64x + f64y;
            assert(fabs(creal(f64addres) - creal(f64add)) < EPSILON_D);
            assert(fabs(cimag(f64addres) - cimag(f64add)) < EPSILON_D);

            _Complex long double ldaddres = cmpldadd(ldx, ldy);
            _Complex long double ldadd = ldx + ldy;
            assert(fabsl(creall(ldaddres) - creall(ldadd)) < EPSILON_LD);
            assert(fabsl(cimagl(ldaddres) - cimagl(ldadd)) < EPSILON_LD);

            _Complex float f32subres = cmpf32sub(f32x, f32y);
            _Complex float f32sub = f32x - f32y;
            assert(fabs(crealf(f32subres) - crealf(f32sub)) < EPSILON_F);
            assert(fabs(cimagf(f32subres) - cimagf(f32sub)) < EPSILON_F);

            _Complex double f64subres = cmpf64sub(f64x, f64y);
            _Complex double f64sub = f64x - f64y;
            assert(fabs(creal(f64subres) - creal(f64sub)) < EPSILON_D);
            assert(fabs(cimag(f64subres) - cimag(f64sub)) < EPSILON_D);

            _Complex long double ldsubres = cmpldsub(ldx, ldy);
            _Complex long double ldsub = ldx - ldy;
            assert(fabsl(creall(ldsubres) - creall(ldsub)) < EPSILON_LD);
            assert(fabsl(cimagl(ldsubres) - cimagl(ldsub)) < EPSILON_LD);

            _Complex float f32mulres = cmpf32mul(f32x, f32y);
            _Complex float f32mul = f32x * f32y;
            assert(fabs(crealf(f32mulres) - crealf(f32mul)) < EPSILON_F);
            assert(fabs(cimagf(f32mulres) - cimagf(f32mul)) < EPSILON_F);

            _Complex double f64mulres = cmpf64mul(f64x, f64y);
            _Complex double f64mul = f64x * f64y;
            assert(fabs(creal(f64mulres) - creal(f64mul)) < EPSILON_D);
            assert(fabs(cimag(f64mulres) - cimag(f64mul)) < EPSILON_D);

            _Complex long double ldmulres = cmpldmul(ldx, ldy);
            _Complex long double ldmul = ldx * ldy;
            assert(fabsl(creall(ldmulres) - creall(ldmul)) < EPSILON_LD);
            assert(fabsl(cimagl(ldmulres) - cimagl(ldmul)) < EPSILON_LD);

            _Complex float f32divres = cmpf32div(f32x, f32y);
            _Complex float f32div = f32x / f32y;
            assert(fabs(crealf(f32divres) - crealf(f32div)) < EPSILON_F);
            assert(fabs(cimagf(f32divres) - cimagf(f32div)) < EPSILON_F);

            _Complex double f64divres = cmpf64div(f64x, f64y);
            _Complex double f64div = f64x / f64y;
            assert(fabs(creal(f64divres) - creal(f64div)) < EPSILON_D);
            assert(fabs(cimag(f64divres) - cimag(f64div)) < EPSILON_D);

            _Complex long double lddivres = cmplddiv(ldx, ldy);
            _Complex long double lddiv = ldx / ldy;
            assert(fabsl(creall(lddivres) - creall(lddiv)) < EPSILON_LD);
            assert(fabsl(cimagl(lddivres) - cimagl(lddiv)) < EPSILON_LD);

            _Complex float f32negres = cmpf32neg(f32x);
            _Complex float f32neg = -f32x;
            assert(fabs(crealf(f32negres) - crealf(f32neg)) < EPSILON_F);
            assert(fabs(cimagf(f32negres) - cimagf(f32neg)) < EPSILON_F);

            _Complex double f64negres = cmpf64neg(f64x);
            _Complex double f64neg = -f64x;
            assert(fabs(creal(f64negres) - creal(f64neg)) < EPSILON_D);
            assert(fabs(cimag(f64negres) - cimag(f64neg)) < EPSILON_D);

            _Complex long double ldnegres = cmpldneg(ldx);
            _Complex long double ldneg = -ldx;
            assert(fabsl(creall(ldnegres) - creall(ldneg)) < EPSILON_LD);
            assert(fabsl(cimagl(ldnegres) - cimagl(ldneg)) < EPSILON_LD);

            _Complex float f32add_assign = f32x;
            cmpf32add_assign(&f32add_assign, f32y);
            assert(fabs(crealf(f32add_assign) - crealf(f32x + f32y)) < EPSILON_F);
            assert(fabs(cimagf(f32add_assign) - cimagf(f32x + f32y)) < EPSILON_F);

            _Complex double f64add_assign = f64x;
            cmpf64add_assign(&f64add_assign, f64y);
            assert(fabs(creal(f64add_assign) - creal(f64x + f64y)) < EPSILON_D);
            assert(fabs(cimag(f64add_assign) - cimag(f64x + f64y)) < EPSILON_D);

            _Complex long double ldadd_assign = ldx;
            cmpldadd_assign(&ldadd_assign, ldy);
            assert(fabsl(creall(ldadd_assign) - creall(ldx + ldy)) < EPSILON_LD);
            assert(fabsl(cimagl(ldadd_assign) - cimagl(ldx + ldy)) < EPSILON_LD);

            _Complex float f32sub_assign = f32x;
            cmpf32sub_assign(&f32sub_assign, f32y);
            assert(fabs(crealf(f32sub_assign) - crealf(f32x - f32y)) < EPSILON_F);
            assert(fabs(cimagf(f32sub_assign) - cimagf(f32x - f32y)) < EPSILON_F);

            _Complex double f64sub_assign = f64x;
            cmpf64sub_assign(&f64sub_assign, f64y);
            assert(fabs(creal(f64sub_assign) - creal(f64x - f64y)) < EPSILON_D);
            assert(fabs(cimag(f64sub_assign) - cimag(f64x - f64y)) < EPSILON_D);

            _Complex long double ldsub_assign = ldx;
            cmpldsub_assign(&ldsub_assign, ldy);
            assert(fabsl(creall(ldsub_assign) - creall(ldx - ldy)) < EPSILON_LD);
            assert(fabsl(cimagl(ldsub_assign) - cimagl(ldx - ldy)) < EPSILON_LD);

            _Complex float f32mul_assign = f32x;
            cmpf32mul_assign(&f32mul_assign, f32y);
            assert(fabs(crealf(f32mul_assign) - crealf(f32x * f32y)) < EPSILON_F);
            assert(fabs(cimagf(f32mul_assign) - cimagf(f32x * f32y)) < EPSILON_F);

            _Complex double f64mul_assign = f64x;
            cmpf64mul_assign(&f64mul_assign, f64y);
            assert(fabs(creal(f64mul_assign) - creal(f64x * f64y)) < EPSILON_D);
            assert(fabs(cimag(f64mul_assign) - cimag(f64x * f64y)) < EPSILON_D);

            _Complex long double ldmul_assign = ldx;
            cmpldmul_assign(&ldmul_assign, ldy);
            assert(fabsl(creall(ldmul_assign) - creall(ldx * ldy)) < EPSILON_LD);
            assert(fabsl(cimagl(ldmul_assign) - cimagl(ldx * ldy)) < EPSILON_LD);

            _Complex float f32div_assign = f32x;
            cmpf32div_assign(&f32div_assign, f32y);
            assert(fabs(crealf(f32div_assign) - crealf(f32x / f32y)) < EPSILON_F);
            assert(fabs(cimagf(f32div_assign) - cimagf(f32x / f32y)) < EPSILON_F);

            _Complex double f64div_assign = f64x;
            cmpf64div_assign(&f64div_assign, f64y);
            assert(fabs(creal(f64div_assign) - creal(f64x / f64y)) < EPSILON_D);
            assert(fabs(cimag(f64div_assign) - cimag(f64x / f64y)) < EPSILON_D);

            _Complex long double lddiv_assign = ldx;
            cmplddiv_assign(&lddiv_assign, ldy);
            assert(fabsl(creall(lddiv_assign) - creall(ldx / ldy)) < EPSILON_LD);
            assert(fabsl(cimagl(lddiv_assign) - cimagl(ldx / ldy)) < EPSILON_LD);
        }
    }
    return EXIT_SUCCESS;
}
