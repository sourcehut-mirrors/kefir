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
#include <float.h>
#include "./definitions.h"

int main(void) {
#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))
    struct S3 s3 = get1();
    assert(s3.arr[0] == ~0xbcdefedfebce3245ull);
    assert(s3.arr[1] == ~0x1234567890aull);
    assert(MASK(s3.arr[2], 22) == MASK(~0ull, 22));

    s3 = get2();
    assert(s3.arr[0] == ~0xbcdefedfebce3245ull);
    assert(s3.arr[1] == ~0x1234567890aull);
    assert(MASK(s3.arr[2], 22) == MASK(~0ull, 22));

    s3 = get3();
    assert(s3.arr[0] == 4837148712062012859ull);
    assert(s3.arr[1] == (unsigned long) -1250999896331ll);
    assert(MASK(s3.arr[2], 22) == MASK(~0ull, 22));

    s3 = get4();
    assert(s3.arr[0] == 4837148712062012859ull);
    assert(s3.arr[1] == (unsigned long) -1250999896331ll);
    assert(MASK(s3.arr[2], 22) == MASK(~0ull, 22));
    return EXIT_SUCCESS;
}
