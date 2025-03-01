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
    for (int i = -4; i < 4; i++) {
        for (int j = -4; j < 4; j++) {
            assert(char_short_or(i, j) == (i || j));
            assert(char_short_and(i, j) == (i && j));
            assert(char_int_or(i, j) == (i || j));
            assert(char_int_and(i, j) == (i && j));
            assert(char_long_or(i, j) == (i || j));
            assert(char_long_and(i, j) == (i && j));

            assert(short_char_or(i, j) == (i || j));
            assert(short_char_and(i, j) == (i && j));
            assert(short_int_or(i, j) == (i || j));
            assert(short_int_and(i, j) == (i && j));
            assert(short_long_or(i, j) == (i || j));
            assert(short_long_and(i, j) == (i && j));

            assert(int_char_or(i, j) == (i || j));
            assert(int_char_and(i, j) == (i && j));
            assert(int_short_or(i, j) == (i || j));
            assert(int_short_and(i, j) == (i && j));
            assert(int_long_or(i, j) == (i || j));
            assert(int_long_and(i, j) == (i && j));

            assert(long_char_or(i, j) == (i || j));
            assert(long_char_and(i, j) == (i && j));
            assert(long_short_or(i, j) == (i || j));
            assert(long_short_and(i, j) == (i && j));
            assert(long_int_or(i, j) == (i || j));
            assert(long_int_and(i, j) == (i && j));
        }
    }
    return EXIT_SUCCESS;
}
