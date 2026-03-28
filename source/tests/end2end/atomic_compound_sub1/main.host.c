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
#include "./definitions.h"

#define EPSILON_F 1e-3
#define EPSILON_D 1e-6
#define EPSILON_LD 1e-8

int main(void) {
#if !defined(__DragonFly__)
    _Atomic char i8;
    _Atomic short i16;
    _Atomic int i32;
    _Atomic long i64;
    _Atomic float f32;
    _Atomic double f64;

    for (long i = -128; i < 128; i++) {
        for (long j = -4096; j < 4096; j++) {
            i8 = i;
            sub8(&i8, j);
            assert(i8 == (char) (i - j));

            i16 = i;
            sub16(&i16, j);
            assert(i16 == (short) (i - j));

            i32 = i;
            sub32(&i32, j);
            assert(i32 == (int) (i - j));

            i64 = i;
            sub64(&i64, j);
            assert(i64 == (i - j));

            f32 = i / 10.0f;
            subf32(&f32, j);
            assert(fabs(f32 - (i / 10.0f - j)) < 1e-3);

            f64 = i / 10.0f;
            subf64(&f64, j);
            assert(fabs(f64 - (i / 10.0f - j)) < 1e-3);

            i8 = i;
            assert(sub8r(&i8, j) == (char) (i - j));
            assert(i8 == (char) (i - j));

            i16 = i;
            assert(sub16r(&i16, j) == (short) (i - j));
            assert(i16 == (short) (i - j));

            i32 = i;
            assert(sub32r(&i32, j) == (int) (i - j));
            assert(i32 == (int) (i - j));

            i64 = i;
            assert(sub64r(&i64, j) == (i - j));
            assert(i64 == (i - j));

            f32 = i / 10.0f;
            assert(fabs(subf32r(&f32, j) - (i / 10.0f - j)) < 1e-3);
            assert(fabs(f32 - (i / 10.0f - j)) < 1e-3);

            f64 = i / 10.0f;
            assert(fabs(subf64r(&f64, j) - (i / 10.0f - j)) < 1e-3);
            assert(fabs(f64 - (i / 10.0f - j)) < 1e-3);
        }
    }
#endif
    return EXIT_SUCCESS;
}
