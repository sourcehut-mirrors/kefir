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

static long mask(unsigned long x, int width) {
    const unsigned long msb = (x >> (width - 1)) & 1;
    if (msb) {
        return (x << (sizeof(unsigned long) * CHAR_BIT - width) >> (sizeof(unsigned long) * CHAR_BIT - width)) |
               ~((1ull << width) - 1);
    } else {
        return (x << (sizeof(unsigned long) * CHAR_BIT - width) >> (sizeof(unsigned long) * CHAR_BIT - width));
    }
}

#define MASK(_x, _y) mask(_x, _y)
#define UMASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))

int main(void) {
    for (int i = -4096; i < 4096; i++) {
        assert(get1(1, i) == MASK(i, 11));
        assert(get2(1, i) == (int) UMASK(i, 11));
    }
    return EXIT_SUCCESS;
}
