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

int main(void) {
    char i8;
    short i16;
    int i32;
    long i64;

    for (long i = -4096; i < 4096; i++) {
        for (long j = -4096; j < 4096; j++) {
            i8 = i;
            add8(&i8, j);
            assert(i8 == (char) (i + j));

            i16 = i;
            add16(&i16, j);
            assert(i16 == (short) (i + j));

            i32 = i;
            add32(&i32, j);
            assert(i32 == (int) (i + j));

            i64 = i;
            add64(&i64, j);
            assert(i64 == (i + j));

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

            i8 = i;
            and8(&i8, j);
            assert(i8 == (char) (i & j));

            i16 = i;
            and16(&i16, j);
            assert(i16 == (short) (i & j));

            i32 = i;
            and32(&i32, j);
            assert(i32 == (int) (i & j));

            i64 = i;
            and64(&i64, j);
            assert(i64 == (i & j));

            i8 = i;
            or8(&i8, j);
            assert(i8 == (char) (i | j));

            i16 = i;
            or16(&i16, j);
            assert(i16 == (short) (i | j));

            i32 = i;
            or32(&i32, j);
            assert(i32 == (int) (i | j));

            i64 = i;
            or64(&i64, j);
            assert(i64 == (i | j));

            i8 = i;
            xor8(&i8, j);
            assert(i8 == (char) (i ^ j));

            i16 = i;
            xor16(&i16, j);
            assert(i16 == (short) (i ^ j));

            i32 = i;
            xor32(&i32, j);
            assert(i32 == (int) (i ^ j));

            i64 = i;
            xor64(&i64, j);
            assert(i64 == (i ^ j));
        }

        i8 = i;
        neg8(&i8);
        assert(i8 == (char) -i);

        i16 = i;
        neg16(&i16);
        assert(i16 == (short) -i);

        i32 = i;
        neg32(&i32);
        assert(i32 == (int) -i);

        i64 = i;
        neg64(&i64);
        assert(i64 == -i);

        i8 = i;
        not8(&i8);
        assert(i8 == (char) ~i);

        i16 = i;
        not16(&i16);
        assert(i16 == (short) ~i);

        i32 = i;
        not32(&i32);
        assert(i32 == (int) ~i);

        i64 = i;
        not64(&i64);
        assert(i64 == ~i);
    }
    return EXIT_SUCCESS;
}
