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
    struct S3 s3 = get1();
    assert(s3.arr[0] == 3141);
    assert(s3.arr[1] == 0);
    assert((s3.arr[2] & ((1ull << 56) - 1)) == 0);

    s3 = get2();
    assert(s3.arr[0] == (unsigned long) -31415);
    assert(s3.arr[1] == (unsigned long) -1);
    assert((s3.arr[2] & ((1ull << 56) - 1)) == (1ull << 56) - 1);

    s3 = get3();
    assert(s3.arr[0] == 314159);
    assert(s3.arr[1] == 0);
    assert((s3.arr[2] & ((1ull << 56) - 1)) == 0);

    s3 = get4();
    assert(s3.arr[0] == 3141);
    assert(s3.arr[1] == 0);
    assert((s3.arr[2] & ((1ull << 56) - 1)) == 0);

    s3 = get5();
    assert(s3.arr[0] == (unsigned long) -31415);
    assert(s3.arr[1] == (unsigned long) -1);
    assert((s3.arr[2] & ((1ull << 56) - 1)) == (1ull << 56) - 1);

    s3 = get6();
    assert(s3.arr[0] == 314159);
    assert(s3.arr[1] == 0);
    assert((s3.arr[2] & ((1ull << 56) - 1)) == 0);

    s3 = get7();
    assert(s3.arr[0] == 3141);
    assert(s3.arr[1] == 0);
    assert((s3.arr[2] & ((1ull << 56) - 1)) == 0);

    s3 = get8();
    assert(s3.arr[0] == (unsigned long) -31415);
    assert(s3.arr[1] == (unsigned long) -1);
    assert((s3.arr[2] & ((1ull << 56) - 1)) == (1ull << 56) - 1);

    s3 = get9();
    assert(s3.arr[0] == 314159);
    assert(s3.arr[1] == 0);
    assert((s3.arr[2] & ((1ull << 56) - 1)) == 0);
    return EXIT_SUCCESS;
}
