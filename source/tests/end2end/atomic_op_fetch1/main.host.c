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
    for (long i = -512; i < 512; i++) {
        for (long j = -512; j < 512; j++) {
            char x8 = i;
            short x16 = i;
            int x32 = i;
            long x64 = i;

            assert(add_fetch8(&x8, j) == (char) (i + j));
            assert(x8 == (char) (i + j));
            assert(add_fetch16(&x16, j) == (short) (i + j));
            assert(x16 == (short) (i + j));
            assert(add_fetch32(&x32, j) == (int) (i + j));
            assert(x32 == (int) (i + j));
            assert(add_fetch64(&x64, j) == (long) (i + j));
            assert(x64 == (long) (i + j));

            x8 = i;
            x16 = i;
            x32 = i;
            x64 = i;

            assert(sub_fetch8(&x8, j) == (char) (i - j));
            assert(x8 == (char) (i - j));
            assert(sub_fetch16(&x16, j) == (short) (i - j));
            assert(x16 == (short) (i - j));
            assert(sub_fetch32(&x32, j) == (int) (i - j));
            assert(x32 == (int) (i - j));
            assert(sub_fetch64(&x64, j) == (long) (i - j));
            assert(x64 == (long) (i - j));

            x8 = i;
            x16 = i;
            x32 = i;
            x64 = i;

            assert(and_fetch8(&x8, j) == (char) (i & j));
            assert(x8 == (char) (i & j));
            assert(and_fetch16(&x16, j) == (short) (i & j));
            assert(x16 == (short) (i & j));
            assert(and_fetch32(&x32, j) == (int) (i & j));
            assert(x32 == (int) (i & j));
            assert(and_fetch64(&x64, j) == (long) (i & j));
            assert(x64 == (long) (i & j));

            x8 = i;
            x16 = i;
            x32 = i;
            x64 = i;

            assert(or_fetch8(&x8, j) == (char) (i | j));
            assert(x8 == (char) (i | j));
            assert(or_fetch16(&x16, j) == (short) (i | j));
            assert(x16 == (short) (i | j));
            assert(or_fetch32(&x32, j) == (int) (i | j));
            assert(x32 == (int) (i | j));
            assert(or_fetch64(&x64, j) == (long) (i | j));
            assert(x64 == (long) (i | j));

            x8 = i;
            x16 = i;
            x32 = i;
            x64 = i;

            assert(xor_fetch8(&x8, j) == (char) (i ^ j));
            assert(x8 == (char) (i ^ j));
            assert(xor_fetch16(&x16, j) == (short) (i ^ j));
            assert(x16 == (short) (i ^ j));
            assert(xor_fetch32(&x32, j) == (int) (i ^ j));
            assert(x32 == (int) (i ^ j));
            assert(xor_fetch64(&x64, j) == (long) (i ^ j));
            assert(x64 == (long) (i ^ j));

            x8 = i;
            x16 = i;
            x32 = i;
            x64 = i;

            assert(nand_fetch8(&x8, j) == (char) ~(i & j));
            assert(x8 == (char) ~(i & j));
            assert(nand_fetch16(&x16, j) == (short) ~(i & j));
            assert(x16 == (short) ~(i & j));
            assert(nand_fetch32(&x32, j) == (int) ~(i & j));
            assert(x32 == (int) ~(i & j));
            assert(nand_fetch64(&x64, j) == (long) ~(i & j));
            assert(x64 == (long) ~(i & j));
        }
    }
    return EXIT_SUCCESS;
}
