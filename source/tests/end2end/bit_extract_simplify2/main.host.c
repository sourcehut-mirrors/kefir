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

static long mask(unsigned long x, int y) {
    unsigned long masked = (x & ((1ull << y) - 1));
    unsigned char sign = (masked >> (y - 1)) & 1;
    if (sign) {
        masked |= ~((1ull << y) - 1);
    }
    return masked;
}

int main(void) {
    for (int i = -256; i < 256; i++) {
        assert(test1(i) == mask(i, 4));
        assert(test2(i) == mask(i, 5));
    }
    return EXIT_SUCCESS;
}
