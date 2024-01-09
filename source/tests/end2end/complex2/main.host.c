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
    for (double real = -100.0; real < 100.0; real += 0.1) {
        _Complex float f32 = cfloat_new((float) real);
        _Complex double f64 = cdouble_new(real);
        _Complex long double ld = cldouble_new((long double) real);

        assert(fabs(crealf(f32) - (float) real) < EPSILON_F);
        assert(fabs(cimagf(f32)) < EPSILON_F);
        assert(fabs(creal(f64) - real) < EPSILON_D);
        assert(fabs(cimag(f64)) < EPSILON_D);
        assert(fabsl(creall(ld) - (long double) real) < EPSILON_LD);
        assert(fabsl(cimagl(ld)) < EPSILON_LD);

        _Complex float f32_from_long = cfloat_from_long((long) real);
        assert(fabs(crealf(f32_from_long) - (float) (long) real) < EPSILON_F);
        assert(fabs(cimagf(f32_from_long)) < EPSILON_F);

        _Complex float f32_from_double = cfloat_from_double(real);
        assert(fabs(crealf(f32_from_double) - (float) real) < EPSILON_F);
        assert(fabs(cimagf(f32_from_double)) < EPSILON_F);

        _Complex float f32_from_ldouble = cfloat_from_ldouble((long double) real);
        assert(fabs(crealf(f32_from_ldouble) - (float) (long double) real) < EPSILON_F);
        assert(fabs(cimagf(f32_from_ldouble)) < EPSILON_F);

        _Complex double f64_from_long = cdouble_from_long((long) real);
        assert(fabs(creal(f64_from_long) - (double) (long) real) < EPSILON_D);
        assert(fabs(cimag(f64_from_long)) < EPSILON_D);

        _Complex double f64_from_float = cdouble_from_float((float) real);
        assert(fabs(creal(f64_from_float) - (double) (float) real) < EPSILON_D);
        assert(fabs(cimag(f64_from_float)) < EPSILON_D);

        _Complex double f64_from_ldouble = cdouble_from_ldouble((long double) real);
        assert(fabs(creal(f64_from_ldouble) - (double) (long double) real) < EPSILON_D);
        assert(fabs(cimag(f64_from_ldouble)) < EPSILON_D);

        _Complex long double ld_from_long = cldouble_from_long((long) real);
        assert(fabsl(creall(ld_from_long) - (long double) (long) real) < EPSILON_LD);
        assert(fabsl(cimagl(ld_from_long)) < EPSILON_LD);

        _Complex long double ld_from_float = cldouble_from_float((float) real);
        assert(fabsl(creall(ld_from_float) - (long double) (float) real) < EPSILON_LD);
        assert(fabsl(cimagl(ld_from_float)) < EPSILON_LD);

        _Complex long double ld_from_double = cldouble_from_double(real);
        assert(fabsl(creall(ld_from_double) - (long double) real) < EPSILON_LD);
        assert(fabsl(cimagl(ld_from_double)) < EPSILON_LD);

        for (double imag = -10.0; imag < 10.0; imag += 0.1) {
            _Complex float f32 = (float) real + I * (float) imag;
            _Complex float f64 = real + I * imag;
            _Complex long double ld = (long double) real + I * (long double) imag;

            _Complex float cf32_from_cf32 = cfloat_from_cfloat(f32);
            assert(fabs(crealf(f32) - crealf(cf32_from_cf32)) < EPSILON_F);
            assert(fabs(cimagf(f32) - cimagf(cf32_from_cf32)) < EPSILON_F);

            _Complex float cf32_from_cf64 = cfloat_from_cdouble(f64);
            assert(fabs((float) creal(f64) - crealf(cf32_from_cf64)) < EPSILON_F);
            assert(fabs((float) cimag(f64) - cimagf(cf32_from_cf64)) < EPSILON_F);

            _Complex float cf32_from_cld = cfloat_from_cldouble(ld);
            assert(fabs((float) creall(ld) - crealf(cf32_from_cld)) < EPSILON_F);
            assert(fabs((float) cimagl(ld) - cimagf(cf32_from_cld)) < EPSILON_F);

            _Complex double cf64_from_cf32 = cdouble_from_cfloat(f32);
            assert(fabs((double) creal(f32) - creal(cf64_from_cf32)) < EPSILON_D);
            assert(fabs((double) cimag(f32) - cimag(cf64_from_cf32)) < EPSILON_D);

            _Complex double cf64_from_cf64 = cdouble_from_cdouble(f64);
            assert(fabs(creal(f64) - creal(cf64_from_cf64)) < EPSILON_D);
            assert(fabs(cimag(f64) - cimag(cf64_from_cf64)) < EPSILON_D);

            _Complex double cf64_from_cld = cdouble_from_cldouble(ld);
            assert(fabs((double) creall(ld) - creal(cf64_from_cld)) < EPSILON_D);
            assert(fabs((double) cimagl(ld) - cimag(cf64_from_cld)) < EPSILON_D);

            _Complex long double cld_from_cf32 = cldouble_from_cfloat(f32);
            assert(fabsl((long double) creal(f32) - creall(cld_from_cf32)) < EPSILON_LD);
            assert(fabsl((long double) cimag(f32) - cimagl(cld_from_cf32)) < EPSILON_LD);

            _Complex long double cld_from_cf64 = cldouble_from_cdouble(f64);
            assert(fabsl((long double) creall(f64) - creall(cld_from_cf64)) < EPSILON_LD);
            assert(fabsl((long double) cimagl(f64) - cimagl(cld_from_cf64)) < EPSILON_LD);

            _Complex long double cld_from_cld = cldouble_from_cldouble(ld);
            assert(fabsl(creall(ld) - creall(cld_from_cld)) < EPSILON_LD);
            assert(fabsl(cimagl(ld) - cimagl(cld_from_cld)) < EPSILON_LD);

            assert(long_from_cfloat(f32) == (long) f32);
            assert(long_from_cdouble(f64) == (long) f64);
            assert(long_from_cldouble(ld) == (long) ld);
            if (real >= 0.0) {
                assert(ulong_from_cfloat(f32) == (unsigned long) f32);
                assert(ulong_from_cdouble(f64) == (unsigned long) f64);
                assert(ulong_from_cldouble(ld) == (unsigned long) ld);
            }

            assert(bool_from_cfloat(f32) == (_Bool) f32);
            assert(bool_from_cdouble(f64) == (_Bool) f64);
            assert(bool_from_cldouble(ld) == (_Bool) ld);
        }
    }
    return EXIT_SUCCESS;
}
