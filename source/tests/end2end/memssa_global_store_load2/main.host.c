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
    for (int i = -4096; i < 4096; i++) {
        assert(test_char(i) == (char) (((char) i) + ((char) (i + 1)) + ((char) (i * 2))));
        assert(c1 == (char) i);
        assert(c2 == (char) (i + 1));
        assert(c3 == (char) (i * 2));

        assert(test_short(i) == (short) (((short) i) + ((short) (i + 1)) + ((short) (i * 2))));
        assert(s1 == (short) i);
        assert(s2 == (short) (i + 1));
        assert(s3 == (short) (i * 2));

        assert(test_int(i) == (int) (((int) i) + ((int) (i + 1)) + ((int) (i * 2))));
        assert(i1 == (int) i);
        assert(i2 == (int) (i + 1));
        assert(i3 == (int) (i * 2));

        assert(test_long(i) == (long) (((long) i) + ((long) (i + 1)) + ((long) (i * 2))));
        assert(l1 == (long) i);
        assert(l2 == (long) (i + 1));
        assert(l3 == (long) (i * 2));

        assert(fabs(test_float(i) - (float) (((float) i) + ((float) (i + 1)) + ((float) (i * 2)))) < 1e-5);
        assert(fabs(f1 - (float) i) < 1e-5);
        assert(fabs(f2 - (float) (i + 1)) < 1e-5);
        assert(fabs(f3 - (float) (i * 2)) < 1e-5);

        assert(fabs(test_double(i) - (double) (((double) i) + ((double) (i + 1)) + ((double) (i * 2)))) < 1e-5);
        assert(fabs(d1 - (double) i) < 1e-5);
        assert(fabs(d2 - (double) (i + 1)) < 1e-5);
        assert(fabs(d3 - (double) (i * 2)) < 1e-5);

        assert(fabsl(test_ldouble(i) -
                     (long double) (((long double) i) + ((long double) (i + 1)) + ((long double) (i * 2)))) < 1e-5);
        assert(fabsl(ld1 - (long double) i) < 1e-5);
        assert(fabsl(ld2 - (long double) (i + 1)) < 1e-5);
        assert(fabsl(ld3 - (long double) (i * 2)) < 1e-5);

        _Complex float res = test_cfloat(i);
        assert(fabs(creal(res) - (float) (((float) i) + ((float) (i + 1)) + ((float) (i * 2)))) < 1e-5);
        assert(fabs(cimag(res)) < 1e-5);
        assert(fabs(creal(cf1) - (float) i) < 1e-5);
        assert(fabs(cimag(cf1)) < 1e-5);
        assert(fabs(creal(cf2) - (float) (i + 1)) < 1e-5);
        assert(fabs(cimag(cf2)) < 1e-5);
        assert(fabs(creal(cf3) - (float) (i * 2)) < 1e-5);
        assert(fabs(cimag(cf3)) < 1e-5);

        _Complex double res64 = test_cdouble(i);
        assert(fabs(creal(res64) - (double) (((double) i) + ((double) (i + 1)) + ((double) (i * 2)))) < 1e-5);
        assert(fabs(cimag(res64)) < 1e-5);
        assert(fabs(creal(cd1) - (double) i) < 1e-5);
        assert(fabs(cimag(cd1)) < 1e-5);
        assert(fabs(creal(cd2) - (double) (i + 1)) < 1e-5);
        assert(fabs(cimag(cd2)) < 1e-5);
        assert(fabs(creal(cd3) - (double) (i * 2)) < 1e-5);
        assert(fabs(cimag(cd3)) < 1e-5);

        _Complex long double res80 = test_cldouble(i);
        assert(fabsl(creall(res80) -
                     (long double) (((long double) i) + ((long double) (i + 1)) + ((long double) (i * 2)))) < 1e-5);
        assert(fabsl(cimagl(res80)) < 1e-5);
        assert(fabsl(creall(cld1) - (long double) i) < 1e-5);
        assert(fabsl(cimagl(cld1)) < 1e-5);
        assert(fabsl(creall(cld2) - (long double) (i + 1)) < 1e-5);
        assert(fabsl(cimagl(cld2)) < 1e-5);
        assert(fabsl(creall(cld3) - (long double) (i * 2)) < 1e-5);
        assert(fabsl(cimagl(cld3)) < 1e-5);
    }
    return EXIT_SUCCESS;
}
