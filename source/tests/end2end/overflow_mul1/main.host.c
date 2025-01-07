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
#include <limits.h>
#include "./definitions.h"

int main(void) {
    schar schr;
    uchar uchr;
    //     short shrt;
    ushort ushrt;
    uint uinteger;
    int integer;
    long lng;
    ulong ulng;

    assert(!overflow_mul_schar_schar_schar(8, -3, &schr) && schr == -24);
    assert(overflow_mul_schar_schar_schar(8, 30, &schr) && schr == (char) 240);
    assert(!overflow_mul_schar_schar_schar(-1, CHAR_MAX, &schr) && schr == -CHAR_MAX);
    assert(overflow_mul_schar_schar_schar(CHAR_MIN, -1, &schr) && schr == CHAR_MIN);

    assert(!overflow_mul_schar_schar_uchar(8, 30, &uchr) && uchr == 240);
    assert(overflow_mul_schar_schar_uchar(8, -1, &uchr) && uchr == (unsigned char) -8);
    assert(!overflow_mul_schar_schar_uchar(-1, CHAR_MIN, &uchr) && uchr == -(int) CHAR_MIN);
    assert(overflow_mul_schar_schar_uchar(CHAR_MAX, -1, &uchr) && uchr == (unsigned char) -CHAR_MAX);

    assert(!overflow_mul_schar_uchar_uchar(CHAR_MAX, 2, &uchr) && uchr == 254);
    assert(overflow_mul_schar_uchar_uchar(-1, CHAR_MAX, &uchr) && uchr == (unsigned char) -CHAR_MAX);
    assert(!overflow_mul_schar_uchar_uchar(1, UCHAR_MAX, &uchr) && uchr == UCHAR_MAX);
    assert(overflow_mul_schar_uchar_uchar(CHAR_MIN, 2, &uchr) && uchr == (unsigned char) (CHAR_MIN * 2));

    assert(!overflow_mul_uchar_schar_uchar(2, CHAR_MAX, &uchr) && uchr == 254);
    assert(overflow_mul_uchar_schar_uchar(1, CHAR_MIN, &uchr) && uchr == (unsigned char) CHAR_MIN);
    assert(!overflow_mul_uchar_schar_uchar(2, 64, &uchr) && uchr == 128);
    assert(overflow_mul_uchar_schar_uchar(3, CHAR_MAX, &uchr) && uchr == (unsigned char) (CHAR_MAX * 3));

    assert(!overflow_mul_int_short_uint(INT_MAX, 2, &uinteger) && uinteger == 2 * (unsigned int) INT_MAX);
    assert(overflow_mul_int_short_uint(INT_MAX, -1, &uinteger) && uinteger == (unsigned int) -INT_MAX);
    assert(!overflow_mul_int_short_uint(-8, SHRT_MIN, &uinteger) && uinteger == (unsigned int) -8 * SHRT_MIN);
    assert(overflow_mul_int_short_uint(2, SHRT_MIN, &uinteger) && uinteger == (unsigned int) 2 * SHRT_MIN);

    assert(!overflow_mul_uint_short_ushort(SHRT_MAX, 2, &ushrt) && ushrt == 2 * SHRT_MAX);
    assert(overflow_mul_uint_short_ushort(1, SHRT_MIN, &ushrt) && ushrt == (unsigned short) SHRT_MIN);
    assert(!overflow_mul_uint_short_ushort(UCHAR_MAX, UCHAR_MAX, &ushrt) && ushrt == UCHAR_MAX * UCHAR_MAX);
    assert(overflow_mul_uint_short_ushort(UCHAR_MAX, CHAR_MIN, &ushrt) &&
           ushrt == (unsigned short) (UCHAR_MAX * CHAR_MIN));

    assert(!overflow_mul_long_int_int(-INT_MAX, -1, &integer) && integer == INT_MAX);
    assert(overflow_mul_long_int_int(LONG_MAX, 1, &integer) && integer == (int) LONG_MAX);
    assert(!overflow_mul_long_int_int(-(long) INT_MIN, -1, &integer) && integer == INT_MIN);
    assert(overflow_mul_long_int_int(INT_MIN, -1, &integer) && integer == (int) -(long) INT_MIN);

    assert(!overflow_mul_long_long_long(LONG_MAX, -1, &lng) && lng == -LONG_MAX);
    assert(overflow_mul_long_long_long(-1, LONG_MIN, &lng) && lng == LONG_MIN);
    assert(!overflow_mul_long_long_long(LONG_MAX, -1, &lng) && lng == -LONG_MAX);
    assert(overflow_mul_long_long_long(LONG_MAX, 10, &lng) && lng == -10);

    assert(!overflow_mul_ulong_long_long(LONG_MAX, -1, &lng) && lng == -LONG_MAX);
    assert(overflow_mul_ulong_long_long(2 + (unsigned long) LONG_MAX, -1, &lng) && lng == LONG_MAX);
    assert(!overflow_mul_ulong_long_long(INT_MAX, -10, &lng) && lng == -10 * (long) INT_MAX);
    assert(overflow_mul_ulong_long_long(ULONG_MAX, 1, &lng) && lng == (long) ULONG_MAX);

    assert(!overflow_mul_long_ulong_long(LONG_MAX / 2, 2, &lng) && lng == LONG_MAX / 2 * 2);
    assert(overflow_mul_long_ulong_long(LONG_MAX, 2, &lng) && lng == (long) ((unsigned long) LONG_MAX * 2));
    assert(!overflow_mul_long_ulong_long(1, LONG_MAX, &lng) && lng == LONG_MAX);
    assert(overflow_mul_long_ulong_long(1, ULONG_MAX, &lng) && lng == (long) ULONG_MAX);

    assert(!overflow_mul_ulong_ulong_long(4, ULONG_MAX / LONG_MAX, &lng) && lng == ULONG_MAX / LONG_MAX * 4);
    assert(overflow_mul_ulong_ulong_long(4, ULONG_MAX, &lng) && lng == (long) (ULONG_MAX * 4));
    assert(!overflow_mul_ulong_ulong_long(((unsigned long) LONG_MAX) / 3, 3, &lng) && lng == (long) (LONG_MAX / 3 * 3));
    assert(overflow_mul_ulong_ulong_long(ULONG_MAX / 10, 10, &lng) && lng == (long) (ULONG_MAX / 10 * 10));

    assert(!overflow_mul_ulong_ulong_ulong(LONG_MAX, 2, &ulng) && ulng == (unsigned long) LONG_MAX * 2);
    assert(overflow_mul_ulong_ulong_ulong(3, ULONG_MAX, &ulng) && ulng == 3 * ULONG_MAX);
    assert(!overflow_mul_ulong_ulong_ulong(ULONG_MAX / 2, 2, &ulng) && ulng == ULONG_MAX / 2 * 2);
    assert(overflow_mul_ulong_ulong_ulong(4, ULONG_MAX / 3, &ulng) && ulng == ULONG_MAX / 3 * 4);

    assert(!overflow_mul_ulong_long_ulong(LONG_MAX, 2, &ulng) && ulng == (unsigned long) LONG_MAX * 2);
    assert(overflow_mul_ulong_long_ulong(LONG_MAX, -1, &ulng) && ulng == (unsigned long) -LONG_MAX);
    assert(!overflow_mul_ulong_long_ulong(ULONG_MAX, 1, &ulng) && ulng == (unsigned long) ULONG_MAX);
    assert(overflow_mul_ulong_long_ulong(1, LONG_MIN, &ulng) && ulng == (unsigned long) LONG_MIN);

    assert(!overflow_mul_long_ulong_ulong(2, LONG_MAX, &ulng) && ulng == (unsigned long) LONG_MAX * 2);
    assert(overflow_mul_long_ulong_ulong(-1, LONG_MAX, &ulng) && ulng == (unsigned long) -LONG_MAX);
    assert(!overflow_mul_long_ulong_ulong(1, ULONG_MAX, &ulng) && ulng == (unsigned long) ULONG_MAX);
    assert(overflow_mul_long_ulong_ulong(LONG_MIN, 1, &ulng) && ulng == (unsigned long) LONG_MIN);

    assert(!overflow_mul_long_long_ulong(-1, LONG_MIN, &ulng) && ulng == (unsigned long) LONG_MAX + 1);
    assert(overflow_mul_long_long_ulong(3, LONG_MAX, &ulng) && ulng == (unsigned long) LONG_MAX * 3);
    assert(!overflow_mul_long_long_ulong(LONG_MAX, 2, &ulng) && ulng == (unsigned long) LONG_MAX * 2);
    assert(overflow_mul_long_long_ulong(LONG_MIN, 1, &ulng) && ulng == (unsigned long) LONG_MIN);
    return EXIT_SUCCESS;
}
