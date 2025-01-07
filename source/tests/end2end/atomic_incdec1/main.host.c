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

#define EPSILON_F 1e-3
#define EPSILON_D 1e-6
#define EPSILON_LD 1e-8

_Atomic char i8;
_Atomic short i16;
_Atomic int i32;
_Atomic long i64;
_Atomic(long *) ptr;
_Atomic float f32;
_Atomic double f64;
_Atomic long double ld;

int main(void) {
    long *const BASE_PTR = (long *) 0xbadcafe0;

    i8 = 0;
    i16 = 0;
    i32 = 0;
    i64 = 0;
    ptr = BASE_PTR;
    f32 = 0.0f;
    f64 = 0.0;
    ld = 0.0L;

    for (long x = 0; x < 4096; x++) {
        assert(preinc_i8() == (char) (x + 1));
        assert(i8 == (char) (x + 1));

        assert(preinc_i16() == (short) (x + 1));
        assert(i16 == (short) (x + 1));

        assert(preinc_i32() == (int) (x + 1));
        assert(i32 == (int) (x + 1));

        assert(preinc_i64() == (long) (x + 1));
        assert(i64 == (long) (x + 1));

        assert(preinc_ptr() == BASE_PTR + x + 1);
        assert(ptr == BASE_PTR + x + 1);

        assert(fabs(preinc_f32() - (float) (x + 1)) < EPSILON_F);
        assert(fabs(f32 - (float) (x + 1)) < EPSILON_F);

        assert(fabs(preinc_f64() - (double) (x + 1)) < EPSILON_D);
        assert(fabs(f64 - (double) (x + 1)) < EPSILON_D);

        assert(fabsl(preinc_ld() - (long double) (x + 1)) < EPSILON_LD);
        assert(fabsl(ld - (long double) (x + 1)) < EPSILON_LD);
    }

    for (long x = 4096; x > 0; x--) {
        assert(predec_i8() == (char) (x - 1));
        assert(i8 == (char) (x - 1));

        assert(predec_i16() == (short) (x - 1));
        assert(i16 == (short) (x - 1));

        assert(predec_i32() == (int) (x - 1));
        assert(i32 == (int) (x - 1));

        assert(predec_i64() == (long) (x - 1));
        assert(i64 == (long) (x - 1));

        assert(predec_ptr() == BASE_PTR + x - 1);
        assert(ptr == BASE_PTR + x - 1);

        assert(fabs(predec_f32() - (float) (x - 1)) < EPSILON_F);
        assert(fabs(f32 - (float) (x - 1)) < EPSILON_F);

        assert(fabs(predec_f64() - (double) (x - 1)) < EPSILON_D);
        assert(fabs(f64 - (double) (x - 1)) < EPSILON_D);

        assert(fabsl(predec_ld() - (long double) (x - 1)) < EPSILON_LD);
        assert(fabsl(ld - (long double) (x - 1)) < EPSILON_LD);
    }
    return EXIT_SUCCESS;
}
