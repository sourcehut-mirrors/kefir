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
#include <math.h>
#include "./definitions.h"

int main(void) {
    for (int i = -256; i <= 256; i++) {
        for (int j = 0; j < (i >= 0 ? 32 : 1); j++) {
            long x = i >= 0 ? ((long) i) << j : i;
            assert(char_to_long((char) x) == (long) (char) x);
            assert(short_to_long((short) x) == (long) (short) x);
            assert(int_to_long((int) x) == (long) (int) x);

            assert(uchar_to_ulong((unsigned char) x) == (unsigned long) (unsigned char) x);
            assert(ushort_to_ulong((unsigned short) x) == (unsigned long) (unsigned short) x);
            assert(uint_to_ulong((unsigned int) x) == (unsigned long) (unsigned int) x);

            assert(long_to_bool(x) == (_Bool) (long) x);
            assert(long_to_char(x) == (char) (long) x);
            assert(long_to_short(x) == (short) (long) x);
            assert(long_to_int(x) == (int) (long) x);

            assert(ulong_to_bool((unsigned long) x) == (_Bool) (unsigned long) x);
            assert(ulong_to_uchar((unsigned long) x) == (unsigned char) (unsigned long) x);
            assert(ulong_to_ushort((unsigned long) x) == (unsigned short) (unsigned long) x);
            assert(ulong_to_uint((unsigned long) x) == (unsigned int) (unsigned long) x);
        }
    }
    return EXIT_SUCCESS;
}
