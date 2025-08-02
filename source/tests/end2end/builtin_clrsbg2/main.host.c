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
    assert(test1(0) == (sizeof(char) * CHAR_BIT - 1));
    assert(test1(1) == (sizeof(char) * CHAR_BIT - 2));
    assert(test1(2) == (sizeof(char) * CHAR_BIT - 3));
    assert(test1(3) == (sizeof(char) * CHAR_BIT - 3));
    assert(test1(1ull << (sizeof(char) * CHAR_BIT - 3)) == 1);
    assert(test1(1ull << (sizeof(char) * CHAR_BIT - 2)) == 0);
    assert(test1((char) (1ull << (sizeof(char) * CHAR_BIT - 1))) == 0);
    assert(test1((char) (1ull << (sizeof(char) * CHAR_BIT))) == (sizeof(char) * CHAR_BIT - 1));

    assert(test2(0) == (sizeof(short) * CHAR_BIT - 1));
    assert(test2(1) == (sizeof(short) * CHAR_BIT - 2));
    assert(test2(2) == (sizeof(short) * CHAR_BIT - 3));
    assert(test2(3) == (sizeof(short) * CHAR_BIT - 3));
    assert(test2(1ull << (sizeof(short) * CHAR_BIT - 3)) == 1);
    assert(test2(1ull << (sizeof(short) * CHAR_BIT - 2)) == 0);
    assert(test2((short) (1ull << (sizeof(short) * CHAR_BIT - 1))) == 0);
    assert(test2((short) (1ull << (sizeof(short) * CHAR_BIT))) == (sizeof(short) * CHAR_BIT - 1));

    assert(test3(0) == (sizeof(int) * CHAR_BIT - 1));
    assert(test3(1) == (sizeof(int) * CHAR_BIT - 2));
    assert(test3(2) == (sizeof(int) * CHAR_BIT - 3));
    assert(test3(3) == (sizeof(int) * CHAR_BIT - 3));
    assert(test3(1ull << (sizeof(int) * CHAR_BIT - 3)) == 1);
    assert(test3(1ull << (sizeof(int) * CHAR_BIT - 2)) == 0);
    assert(test3((int) (1ull << (sizeof(int) * CHAR_BIT - 1))) == 0);
    assert(test3((int) (1ull << (sizeof(int) * CHAR_BIT))) == (sizeof(int) * CHAR_BIT - 1));

    assert(test4(0) == (sizeof(long) * CHAR_BIT - 1));
    assert(test4(1) == (sizeof(long) * CHAR_BIT - 2));
    assert(test4(2) == (sizeof(long) * CHAR_BIT - 3));
    assert(test4(3) == (sizeof(long) * CHAR_BIT - 3));
    assert(test4(1ull << (sizeof(long) * CHAR_BIT - 3)) == 1);
    assert(test4(1ull << (sizeof(long) * CHAR_BIT - 2)) == 0);
    assert(test4(1ull << (sizeof(long) * CHAR_BIT - 1)) == 0);
    return EXIT_SUCCESS;
}
