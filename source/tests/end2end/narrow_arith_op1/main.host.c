/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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
#include <math.h>
#include "./definitions.h"

int main(void) {
    char i8;
    short i16;

    for (long i = -4096; i < 4096; i++) {
        for (long j = -4096; j < 4096; j++) {
            test1(&i8, i, j, (char) 0xcafe);
            assert(i8 == (char) -(i + j - (char) 0xcafe));

            test2(&i16, i, j, (short) 0xcafe);
            assert(i16 == (short) -(i + j - (short) 0xcafe));
        }
    }
    return EXIT_SUCCESS;
}
