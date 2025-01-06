/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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
    short shrt;
    ushort ushrt;
    uint uinteger;
    int integer;
    long lng;
    ulong ulng;

    assert(!overflow_sub_schar_schar_schar(120, -1, &schr) && schr == 121);
    assert(overflow_sub_schar_schar_schar(-2, CHAR_MAX, &schr) && schr == CHAR_MAX);
    assert(!overflow_sub_schar_schar_schar(CHAR_MIN, -CHAR_MAX, &schr) && schr == -1);
    assert(!overflow_sub_schar_schar_schar(CHAR_MAX, 100, &schr) && schr == CHAR_MAX - 100);
    assert(overflow_sub_schar_schar_schar(CHAR_MAX, -2, &schr) && schr == CHAR_MIN + 1);

    assert(!overflow_sub_schar_schar_uchar(125, -10, &uchr) && uchr == 135);
    assert(overflow_sub_schar_schar_uchar(0, 10, &uchr) && uchr == (unsigned char) -10);
    assert(overflow_sub_schar_schar_uchar(CHAR_MIN, 0, &uchr) && uchr == (unsigned char) CHAR_MIN);

    assert(!overflow_sub_schar_uchar_uchar(100, 99, &uchr) && uchr == 1);
    assert(overflow_sub_schar_uchar_uchar(100, 160, &uchr) && uchr == (unsigned char) -60);
    assert(overflow_sub_schar_uchar_uchar(-50, 40, &uchr) && uchr == (unsigned char) -90);

    assert(!overflow_sub_uchar_schar_uchar(255, 127, &uchr) && uchr == 128);
    assert(overflow_sub_uchar_schar_uchar(100, 126, &uchr) && uchr == (unsigned char) -26);
    assert(!overflow_sub_uchar_schar_uchar(10, -50, &uchr) && uchr == 60);
    assert(overflow_sub_uchar_schar_uchar(UCHAR_MAX, -5, &uchr) && uchr == 4);
    assert(!overflow_sub_uchar_schar_uchar(UCHAR_MAX, 5, &uchr) && uchr == UCHAR_MAX - 5);

    assert(!overflow_sub_int_short_uint(SHRT_MAX * 2 + 1, SHRT_MAX, &uinteger) && uinteger == SHRT_MAX + 1);
    assert(overflow_sub_int_short_uint(128, UCHAR_MAX, &uinteger) && uinteger == (unsigned int) (128 - UCHAR_MAX));
    assert(!overflow_sub_int_short_uint(INT_MAX, SHRT_MIN, &uinteger) &&
           uinteger == (unsigned int) ((long) INT_MAX - SHRT_MIN));

    assert(!overflow_sub_uint_short_ushort(USHRT_MAX / 2, 128, &ushrt) && ushrt == USHRT_MAX / 2 - 128);
    assert(overflow_sub_uint_short_ushort(50, 500, &ushrt) && ushrt == (unsigned short) -450);
    assert(!overflow_sub_uint_short_ushort(USHRT_MAX / 2, -100, &ushrt) &&
           ushrt == (unsigned short) (USHRT_MAX / 2 + 100));
    assert(overflow_sub_uint_short_ushort(USHRT_MAX - 50, -150, &ushrt) && ushrt == (unsigned short) (USHRT_MAX + 100));

    assert(!overflow_sub_long_int_int(INT_MAX, INT_MAX / 2, &integer) && integer == INT_MAX - INT_MAX / 2);
    assert(overflow_sub_long_int_int(LONG_MAX, INT_MAX / 2, &integer) && integer == (int) (LONG_MAX - INT_MAX / 2));
    assert(overflow_sub_long_int_int(INT_MIN, 2, &integer) && integer == INT_MAX - 1);
    assert(!overflow_sub_long_int_int(0, INT_MAX, &integer) && integer == -INT_MAX);

    assert(!overflow_sub_long_long_long(150, 500, &lng) && lng == -350);
    assert(!overflow_sub_long_long_long(LONG_MAX, LONG_MAX / 2, &lng) && lng == LONG_MAX - LONG_MAX / 2);
    assert(overflow_sub_long_long_long(LONG_MIN, 10, &lng) && lng == LONG_MAX - 9);
    assert(overflow_sub_long_long_long(0, LONG_MIN, &lng) && lng == LONG_MIN);
    assert(!overflow_sub_long_long_long(0, -LONG_MAX, &lng) && lng == LONG_MAX);

    assert(!overflow_sub_ulong_long_long(LONG_MAX, 1000, &lng) && lng == LONG_MAX - 1000);
    assert(overflow_sub_ulong_long_long(LONG_MAX, -1000, &lng) && lng == (long) ((unsigned long) LONG_MAX + 1000));
    assert(!overflow_sub_ulong_long_long((unsigned long) LONG_MAX + 10000, 20000, &lng) && lng == LONG_MAX - 10000);
    assert(overflow_sub_ulong_long_long((unsigned long) LONG_MAX + 10000, 2000, &lng) &&
           lng == (long) ((unsigned long) LONG_MAX + 8000));

    assert(!overflow_sub_long_ulong_long(0, 5000, &lng) && lng == -5000);
    assert(!overflow_sub_long_ulong_long(-5000, 5000, &lng) && lng == -10000);
    assert(overflow_sub_long_ulong_long(0, (unsigned long) LONG_MAX * 2, &lng) &&
           lng == -((unsigned long) LONG_MAX * 2));
    assert(overflow_sub_long_ulong_long(LONG_MIN, 1, &lng) && lng == LONG_MAX);

    assert(!overflow_sub_ulong_ulong_long(0, 1000, &lng) && lng == -1000);
    assert(overflow_sub_ulong_ulong_long(ULONG_MAX, 0, &lng) && lng == (long) ULONG_MAX);
    assert(!overflow_sub_ulong_ulong_long(ULONG_MAX, ULONG_MAX - 1, &lng) && lng == 1);
    assert(overflow_sub_ulong_ulong_long(0, ULONG_MAX, &lng) && lng == (long) -ULONG_MAX);

    assert(!overflow_sub_ulong_ulong_short(ULONG_MAX, ULONG_MAX - SHRT_MAX + 1, &shrt) && shrt == SHRT_MAX - 1);
    assert(overflow_sub_ulong_ulong_short(ULONG_MAX, ULONG_MAX - SHRT_MAX - 1, &shrt) && shrt == SHRT_MIN);
    assert(!overflow_sub_ulong_ulong_short(0, SHRT_MAX, &shrt) && shrt == -SHRT_MAX);
    assert(overflow_sub_ulong_ulong_short(0, SHRT_MIN, &shrt) && shrt == (short) -(int) SHRT_MIN);

    assert(!overflow_sub_ulong_ulong_ulong(ULONG_MAX, ULONG_MAX - 3, &ulng) && ulng == 3);
    assert(overflow_sub_ulong_ulong_ulong(ULONG_MAX - 10, ULONG_MAX - 3, &ulng) && ulng == ULONG_MAX - 6);
    assert(!overflow_sub_ulong_ulong_ulong(ULONG_MAX, 0, &ulng) && ulng == ULONG_MAX);
    assert(overflow_sub_ulong_ulong_ulong(0, ULONG_MAX, &ulng) && ulng == 1);

    assert(!overflow_sub_ulong_long_ulong(ULONG_MAX, LONG_MAX, &ulng) && ulng == ULONG_MAX - LONG_MAX);
    assert(overflow_sub_ulong_long_ulong(100, LONG_MAX, &ulng) && ulng == 100ull - LONG_MAX);
    assert(!overflow_sub_ulong_long_ulong(LONG_MAX, LONG_MIN, &ulng) && ulng == (unsigned long) LONG_MAX - LONG_MIN);
    assert(overflow_sub_ulong_long_ulong(ULONG_MAX - 100, -200, &ulng) && ulng == 99);

    assert(!overflow_sub_long_ulong_ulong(LONG_MAX, 1000, &ulng) && ulng == LONG_MAX - 1000);
    assert(overflow_sub_long_ulong_ulong(LONG_MAX, ULONG_MAX, &ulng) && ulng == (unsigned long) (LONG_MAX - ULONG_MAX));
    assert(overflow_sub_long_ulong_ulong(10, 20, &ulng) && ulng == (unsigned long) -10);
    assert(overflow_sub_long_ulong_ulong(LONG_MIN, 1, &ulng) && ulng == (unsigned long) LONG_MAX);

    assert(!overflow_sub_long_long_ulong(LONG_MAX, LONG_MIN, &ulng) && ulng == (unsigned long) LONG_MAX - LONG_MIN);
    assert(overflow_sub_long_long_ulong(LONG_MIN, 1, &ulng) && ulng == LONG_MAX);
    assert(overflow_sub_long_long_ulong(0, 1, &ulng) && ulng == (unsigned long) -1);
    assert(!overflow_sub_long_long_ulong(LONG_MAX, LONG_MAX / 2, &ulng) &&
           ulng == (unsigned long) LONG_MAX - LONG_MAX / 2);
    assert(!overflow_sub_long_long_ulong(-1000, -2000, &ulng) && ulng == 1000);
    return EXIT_SUCCESS;
}
