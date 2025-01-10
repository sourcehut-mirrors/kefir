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
#include <stdbool.h>
#include <limits.h>
#include "./definitions.h"

int main(void) {
    for (int i = 0; i < 4096; i++) {
#define TEST(_type, _shift, _value) \
    assert(_type##_shr##_shift((_type) (_value)) == (_type) (((_type) (_value)) >> (_shift)))

        TEST(uchar, 1, i);
        TEST(uchar, 2, i);
        TEST(uchar, 4, i);
        TEST(uchar, 8, i);
        TEST(uchar, 16, i);
        TEST(schar, 1, i % 128);
        TEST(schar, 2, i % 128);
        TEST(schar, 4, i % 128);
        TEST(schar, 8, i % 128);
        TEST(schar, 16, i % 128);
        TEST(ushort, 1, i);
        TEST(ushort, 2, i);
        TEST(ushort, 4, i);
        TEST(ushort, 8, i);
        TEST(ushort, 16, i);
        TEST(sshort, 1, i);
        TEST(sshort, 2, i);
        TEST(sshort, 4, i);
        TEST(sshort, 8, i);
        TEST(sshort, 16, i);
        TEST(uint, 1, i);
        TEST(uint, 2, i);
        TEST(uint, 4, i);
        TEST(uint, 8, i);
        TEST(uint, 16, i);
        TEST(sint, 1, i);
        TEST(sint, 2, i);
        TEST(sint, 4, i);
        TEST(sint, 8, i);
        TEST(sint, 16, i);
        TEST(ulong, 1, i);
        TEST(ulong, 2, i);
        TEST(ulong, 4, i);
        TEST(ulong, 8, i);
        TEST(ulong, 16, i);
        TEST(ulong, 32, i);
        TEST(slong, 1, i);
        TEST(slong, 2, i);
        TEST(slong, 4, i);
        TEST(slong, 8, i);
        TEST(slong, 16, i);
        TEST(slong, 32, i);

        assert(uchar_shr32(i) == 0);
        assert(schar_shr32(i) == 0);
        assert(uchar_shr64(i) == 0);
        assert(schar_shr64(i) == 0);
        assert(ushort_shr32(i) == 0);
        assert(sshort_shr32(i) == 0);
        assert(ushort_shr64(i) == 0);
        assert(sshort_shr64(i) == 0);
        assert(uint_shr32(i) == 0);
        assert(sint_shr32(i) == 0);
        assert(uint_shr64(i) == 0);
        assert(sint_shr64(i) == 0);
        assert(ulong_shr64(i) == 0);
        assert(slong_shr64(i) == 0);

        assert(int_sar31_1(i) == ((int) i >> 31));
        assert(int_sar31_1(-i) == ((int) -i >> 31));
        assert(long_sar63_1(i) == ((long) i >> 63));
        assert(long_sar63_1(-i) == ((long) -i >> 63));
    }
    return EXIT_SUCCESS;
}
