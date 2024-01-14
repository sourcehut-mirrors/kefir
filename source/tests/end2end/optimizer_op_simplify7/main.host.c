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
#include "./definitions.h"

int main(void) {
    for (long x = -1000; x <= 1000; x++) {
        assert(int_add(x) == (100 + (x - 50 + 5) - 400));
        assert(int_sub(x) == (x - 5 - 10 - 15 + 20 - 1));
        assert(int_sub2(x) == (x - 5 - 10 - 15 - 20 - 1));
        assert(int_mul(x) == (2 * x * 10 * 5));
        assert(int_and(x) == (0xfefefe & x & 0xcacaca & 0xfe));
        assert(int_or(x) == (0x12012 | x | 0x100000 | 0x1));
        assert(int_xor(x) == (0x10203040 ^ x ^ 0xfefefe3 ^ 0x11));
    }

    for (unsigned long x = 0; x <= 2000; x++) {
        assert(uint_and(x) == (0xfefefeu & x & 0xcacacau & 0xfeu));
        assert(uint_or(x) == (0x12012u | x | 0x100000u | 0x1u));
        assert(uint_xor(x) == (0x10203040u ^ x ^ 0xfefefe3u ^ 0x11u));
        assert(uint_add(x) == (100u + (x - 50u + 5u) - 400u));
        assert(uint_sub(x) == (x - 5u - 10u - 15u + 20u - 1u));
        assert(uint_sub2(x) == (x - 5u - 10u - 15u - 20u - 1u));
        assert(uint_mul(x) == (2u * x * 10u * 5u));
    }
    return EXIT_SUCCESS;
}
