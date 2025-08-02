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

int main(void) {
    assert(test1(0, -1000) == -1000);
    assert(test1((unsigned char) (1ull << (CHAR_BIT * sizeof(char))), -1001) == -1001);
    assert(test2(0, -2000) == -2000);
    assert(test2((unsigned short) (1ull << CHAR_BIT * sizeof(short)), -2001) == -2001);
    assert(test3(0, -3000) == -3000);
    assert(test3((unsigned int) (1ull << CHAR_BIT * sizeof(int)), -3001) == -3001);
    assert(test4(0, -4000) == -4000);

    for (unsigned int i = 0; i < CHAR_BIT * sizeof(char); i++) {
        assert(test1(1 << i, -12345) == (int) (CHAR_BIT * sizeof(char) - i - 1));
    }

    for (unsigned int i = 0; i < CHAR_BIT * sizeof(short); i++) {
        assert(test2(1 << i, -58438) == (int) (CHAR_BIT * sizeof(short) - i - 1));
    }

    for (unsigned int i = 0; i < CHAR_BIT * sizeof(int); i++) {
        assert(test3(1ull << i, 482448) == (int) (CHAR_BIT * sizeof(int) - i - 1));
    }

    for (unsigned int i = 0; i < CHAR_BIT * sizeof(long); i++) {
        assert(test4(1ull << i, 8375) == (int) (CHAR_BIT * sizeof(long) - i - 1));
    }
    return EXIT_SUCCESS;
}
