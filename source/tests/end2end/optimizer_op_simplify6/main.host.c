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
    for (long x = -1000; x <= 1000; x++) {
        assert(int_add(x) == ((1 + x) * (x + (12345 - 1))));
        assert(int_sub(x) == (x - (0 - 100)));
        assert(int_mul(x) == ((2 + 3) * x + x * 6));
        assert(int_and(x) == ((-381 & x) | (x & 12356)));
        assert(int_or(x) == ((10291 | x) & (x | 81712)));
        assert(int_xor(x) == ((x ^ 76) + (7894 ^ x)));
    }

    for (unsigned long x = 0; x <= 2000ul; x++) {
        assert(uint_add(x) == ((1ul + x) * (x + (12345ul - 1ul))));
        assert(uint_sub(x) == (x - 257ul));
        assert(uint_mul(x) == ((2ul + 3ul) * x + x * 6ul));
        assert(uint_and(x) == ((381ul & x) | (x & 12356ul)));
        assert(uint_or(x) == ((10291ul | x) & (x | 81712ul)));
        assert(uint_xor(x) == ((x ^ 76ul) + (7894ul ^ x)));
    }
    return EXIT_SUCCESS;
}
