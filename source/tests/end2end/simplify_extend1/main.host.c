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
#include "./definitions.h"

int main(void) {
    for (long x = -100; x < 100; x++) {
        assert(uchar_uchar(x) == (unsigned char) (unsigned char) x);
        assert(uchar_ushort(x) == (unsigned short) (unsigned char) x);
        assert(uchar_uint(x) == (unsigned int) (unsigned char) x);
        assert(uchar_char(x) == (char) (unsigned char) x);
        assert(uchar_short(x) == (short) (unsigned char) x);
        assert(uchar_int(x) == (int) (unsigned char) x);

        assert(uchar_uchar(~x) == (unsigned char) (unsigned char) ~x);
        assert(uchar_ushort(~x) == (unsigned short) (unsigned char) ~x);
        assert(uchar_uint(~x) == (unsigned int) (unsigned char) ~x);
        assert(uchar_char(~x) == (char) (unsigned char) ~x);
        assert(uchar_short(~x) == (short) (unsigned char) ~x);
        assert(uchar_int(~x) == (int) (unsigned char) ~x);

        assert(ushort_ushort(x) == (unsigned short) (unsigned short) x);
        assert(ushort_uint(x) == (unsigned int) (unsigned short) x);
        assert(ushort_short(x) == (short) (unsigned short) x);
        assert(ushort_int(x) == (int) (unsigned short) x);

        assert(ushort_ushort(~x) == (unsigned short) (unsigned short) ~x);
        assert(ushort_uint(~x) == (unsigned int) (unsigned short) ~x);
        assert(ushort_short(~x) == (short) (unsigned short) ~x);
        assert(ushort_int(~x) == (int) (unsigned short) ~x);

        assert(char_uchar(x) == (unsigned char) (char) x);
        assert(char_ushort(x) == (unsigned short) (char) x);
        assert(char_uint(x) == (unsigned int) (char) x);
        assert(char_char(x) == (char) (char) x);
        assert(char_short(x) == (short) (char) x);
        assert(char_int(x) == (int) (char) x);

        assert(char_uchar(~x) == (unsigned char) (char) ~x);
        assert(char_ushort(~x) == (unsigned short) (char) ~x);
        assert(char_uint(~x) == (unsigned int) (char) ~x);
        assert(char_char(~x) == (char) (char) ~x);
        assert(char_short(~x) == (short) (char) ~x);
        assert(char_int(~x) == (int) (char) ~x);

        assert(short_ushort(x) == (unsigned short) (short) x);
        assert(short_uint(x) == (unsigned int) (short) x);
        assert(short_short(x) == (short) (short) x);
        assert(short_int(x) == (int) (short) x);

        assert(short_ushort(~x) == (unsigned short) (short) ~x);
        assert(short_uint(~x) == (unsigned int) (short) ~x);
        assert(short_short(~x) == (short) (short) ~x);
        assert(short_int(~x) == (int) (short) ~x);

        assert(uint_uint(x) == (unsigned int) (unsigned int) x);
        assert(uint_int(x) == (int) (unsigned int) x);
        assert(int_uint(x) == (unsigned int) (int) x);
        assert(int_int(x) == (int) (int) x);
    }
    return EXIT_SUCCESS;
}
