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
    for (unsigned int i = 0; i < CHAR_BIT * sizeof(char); i++) {
        assert(test1(1 << i) == (int) (CHAR_BIT * sizeof(char) - i - 1));
    }

    for (unsigned int i = 0; i < CHAR_BIT * sizeof(short); i++) {
        assert(test2(1 << i) == (int) (CHAR_BIT * sizeof(short) - i - 1));
    }

    for (unsigned int i = 0; i < CHAR_BIT * sizeof(int); i++) {
        assert(test3(1ull << i) == (int) (CHAR_BIT * sizeof(int) - i - 1));
    }

    for (unsigned int i = 0; i < CHAR_BIT * sizeof(long); i++) {
        assert(test4(1ull << i) == (int) (CHAR_BIT * sizeof(long) - i - 1));
    }
    return EXIT_SUCCESS;
}
