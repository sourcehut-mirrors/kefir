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
    short sshrt;
    ushort ushrt;
    uint uint;
    int sint;
    long slong;
    ulong ulng;
    assert(!overflow_add_schar_schar_schar(-2, 120, &schr) && schr == 118);
    assert(overflow_add_schar_schar_schar(9, 120, &schr) && schr == -127);
    assert(overflow_add_schar_schar_schar(-120, -10, &schr) && schr == 126);
    assert(!overflow_add_schar_schar_schar(0, 127, &schr) && schr == 127);
    assert(!overflow_add_schar_schar_uchar(127, 127, &uchr) && uchr == 254);
    assert(overflow_add_schar_uchar_uchar(127, 200, &uchr) && uchr == 71);
    assert(overflow_add_uchar_schar_uchar(255, 2, &uchr) && uchr == 1);

    assert(!overflow_add_int_short_uint(INT_MAX, 100, &uint) && uint == ((unsigned int) INT_MAX) + 100);
    assert(overflow_add_int_short_uint(10, -100, &uint) && uint == (unsigned int) -90);

    assert(!overflow_add_uint_short_ushort(SHRT_MAX + 1000, SHRT_MIN, &ushrt) &&
           ushrt == (ushort) (SHRT_MAX + SHRT_MIN + 1000));
    assert(overflow_add_uint_short_ushort(USHRT_MAX, SHRT_MAX / 2, &ushrt) &&
           ushrt == (ushort) (USHRT_MAX + SHRT_MAX / 2));

    assert(!overflow_add_long_int_int(UINT_MAX - 100, INT_MIN, &sint) && sint == (int) (UINT_MAX - 100 + INT_MIN));
    assert(overflow_add_long_int_int((long) UINT_MAX + 100, INT_MIN, &sint) &&
           sint == (int) (100 + (long) UINT_MAX + INT_MIN));

    assert(!overflow_add_long_long_long(LONG_MAX - 1000, 500, &slong) && slong == (long) (LONG_MAX - 500));
    assert(overflow_add_long_long_long(LONG_MAX / 2 + 2, LONG_MAX / 2, &slong) && slong == LONG_MIN);
    assert(!overflow_add_long_long_long(0, LONG_MIN, &slong) && slong == LONG_MIN);
    assert(overflow_add_long_long_long(-10, LONG_MIN, &slong) && slong == LONG_MAX - 9);
    assert(!overflow_add_long_long_long(LONG_MIN, 0, &slong) && slong == LONG_MIN);

    assert(!overflow_add_ulong_long_long(ULONG_MAX, (ULONG_MAX - LONG_MAX), &slong) && slong == LONG_MAX);
    assert(overflow_add_ulong_long_long(ULONG_MAX, 1, &slong) && slong == (long) (ULONG_MAX + 1));
    assert(overflow_add_ulong_long_long(100, LONG_MAX, &slong) && slong == (long) (LONG_MIN + 99));
    assert(overflow_add_ulong_long_long(LONG_MAX, LONG_MAX, &slong) && slong == (long) (LONG_MIN + LONG_MAX - 1));
    assert(!overflow_add_ulong_long_long((unsigned long) LONG_MIN, LONG_MIN, &slong) && slong == 0);

    assert(!overflow_add_long_ulong_long(123, LONG_MAX - 1000, &slong) && slong == (LONG_MAX - 1000 + 123));
    assert(overflow_add_long_ulong_long(123, (unsigned long) LONG_MAX + 1000, &slong) &&
           slong == (long) (LONG_MIN + 1122));
    assert(!overflow_add_long_ulong_long(LONG_MIN, 2 * (unsigned long) LONG_MAX, &slong) &&
           slong == (long) (LONG_MAX - 1));
    assert(overflow_add_long_ulong_long(LONG_MAX, (unsigned long) LONG_MAX, &slong) &&
           slong == (long) ((unsigned long) LONG_MAX + LONG_MAX));

    assert(!overflow_add_ulong_ulong_long(LONG_MAX / 2, LONG_MAX / 2, &slong) && slong == (LONG_MAX / 2) * 2);
    assert(overflow_add_ulong_ulong_long((unsigned long) LONG_MAX + 1, LONG_MAX, &slong) &&
           slong == (long) (2 * (unsigned long) LONG_MAX + 1));
    assert(!overflow_add_ulong_ulong_long(LONG_MAX, 0, &slong) && slong == LONG_MAX);
    assert(overflow_add_ulong_ulong_long(LONG_MIN, 1, &slong) && slong == (long) (1 + (unsigned long) LONG_MIN));

    assert(!overflow_add_ulong_ulong_short(SHRT_MAX - 10, 9, &sshrt) && sshrt == SHRT_MAX - 1);
    assert(overflow_add_ulong_ulong_short(SHRT_MAX, SHRT_MAX, &sshrt) && sshrt == (short) (SHRT_MAX * 2));

    assert(!overflow_add_ulong_ulong_ulong(LONG_MAX, LONG_MAX, &ulng));
    assert(overflow_add_ulong_ulong_ulong(ULONG_MAX - 100, 101, &ulng) && ulng == 0);
    assert(overflow_add_ulong_ulong_ulong(ULONG_MAX, ULONG_MAX / 2, &ulng) && ulng == ULONG_MAX / 2 - 1);

    assert(!overflow_add_long_ulong_ulong(LONG_MAX, 1000, &ulng) && ulng == ((unsigned long) LONG_MAX) + 1000);
    assert(overflow_add_long_ulong_ulong(LONG_MIN, 1, &ulng) && ulng == ((unsigned long) LONG_MIN) + 1);
    assert(overflow_add_long_ulong_ulong(10000, ULONG_MAX, &ulng) && ulng == 9999);

    assert(!overflow_add_ulong_long_ulong(1000, LONG_MAX, &ulng) && ulng == ((unsigned long) LONG_MAX) + 1000);
    assert(overflow_add_ulong_long_ulong(1000, LONG_MIN, &ulng) && ulng == ((unsigned long) LONG_MIN) + 1000);
    assert(overflow_add_ulong_long_ulong(ULONG_MAX, 1000, &ulng) && ulng == 999);

    assert(!overflow_add_long_long_ulong(LONG_MAX, LONG_MAX, &ulng) && ulng == ((unsigned long) LONG_MAX) * 2);
    assert(overflow_add_long_long_ulong(LONG_MIN, 1, &ulng) && ulng == ((unsigned long) LONG_MIN) + 1);
    assert(overflow_add_long_long_ulong(1000, LONG_MIN, &ulng) && ulng == ((unsigned long) LONG_MIN) + 1000);
    assert(overflow_add_long_long_ulong(LONG_MIN, LONG_MIN, &ulng) && ulng == ((unsigned long) LONG_MIN) * 2);
    assert(!overflow_add_long_long_ulong(LONG_MAX, -LONG_MAX, &ulng) && ulng == 0);
    return EXIT_SUCCESS;
}
