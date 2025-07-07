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

extern unsigned long x[3], y[3], z[3], a[3];

int main(void) {
#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))
    assert(arr[0] == 1000);
    assert(arr[1] == 0x1000);
    assert(arr[2] == 8);
    assert(arr[3] == 512);
    assert(arr[4] == 123456789);
    assert(arr[5] == 0xabcd12349876ull);
    assert(arr[6] == 61680);
    assert(arr[7] == 07771115553333l);
    assert(arr[8] == 1239871236541230ull);
    assert(arr[9] == 0x1239871236541230ull);

    assert(x[0] == 0x44321defa0000011ull);
    assert(x[1] == 0xd1234efdc0987123ull);
    assert(MASK(x[2], 52) == 0xabc);

    assert(y[0] == (unsigned long) -1148669705926492162ll);
    assert(y[1] == 240);
    assert(MASK(y[2], 52) == 0);

    assert(z[0] == (unsigned long) -3290091646667557752ll);
    assert(z[1] == 22976149868412);
    assert(MASK(z[2], 52) == 16368);

    assert(a[0] == (unsigned long) 8312942291372959420ull);
    assert(a[1] == 2959064529720148950ull);
    assert(MASK(a[2], 52) == 567829143ull);
    return EXIT_SUCCESS;
}
