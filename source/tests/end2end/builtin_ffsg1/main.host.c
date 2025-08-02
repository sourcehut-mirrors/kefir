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
#include <math.h>
#include "./definitions.h"

int main(void) {
    assert(test1(0) == 0);
    assert(test1(1) == 1);
    assert(test1(2) == 2);
    assert(test1(3) == 1);
    assert(test1((unsigned char) (1ull << 7)) == 8);
    assert(test1((unsigned char) ((1ull << 7) | (1ull << 6))) == 7);
    assert(test1((unsigned char) (1ull << 8)) == 0);

    assert(test2(0) == 0);
    assert(test2(1) == 1);
    assert(test2(2) == 2);
    assert(test2(3) == 1);
    assert(test2((unsigned short) (1ull << 7)) == 8);
    assert(test2((unsigned short) ((1ull << 7) | (1ull << 6))) == 7);
    assert(test2((unsigned short) (1ull << 8)) == 9);
    assert(test2((unsigned short) (1ull << 15)) == 16);
    assert(test2((unsigned short) ((1ull << 15) | (1ull << 10))) == 11);
    assert(test2((unsigned short) (1ull << 16)) == 0);

    assert(test3(0) == 0);
    assert(test3(1) == 1);
    assert(test3(2) == 2);
    assert(test3(3) == 1);
    assert(test3((unsigned int) (1ull << 7)) == 8);
    assert(test3((unsigned int) ((1ull << 7) | (1ull << 6))) == 7);
    assert(test3((unsigned int) (1ull << 8)) == 9);
    assert(test3((unsigned int) (1ull << 15)) == 16);
    assert(test3((unsigned int) (1ull << 16)) == 17);
    assert(test3((unsigned int) (1ull << 31)) == 32);
    assert(test3((unsigned int) ((1ull << 31) | (1ull << 29))) == 30);
    assert(test3((unsigned int) (1ull << 32)) == 0);

    assert(test4(0) == 0);
    assert(test4(1) == 1);
    assert(test4(2) == 2);
    assert(test4(3) == 1);
    assert(test4((unsigned long) (1ull << 7)) == 8);
    assert(test4((unsigned long) ((1ull << 7) | (1ull << 6))) == 7);
    assert(test4((unsigned long) (1ull << 8)) == 9);
    assert(test4((unsigned long) (1ull << 15)) == 16);
    assert(test4((unsigned long) (1ull << 16)) == 17);
    assert(test4((unsigned long) (1ull << 31)) == 32);
    assert(test4((unsigned long) ((1ull << 31) | (1ull << 29))) == 30);
    assert(test4((unsigned long) (1ull << 32)) == 33);
    assert(test4((unsigned long) (1ull << 63)) == 64);
    return EXIT_SUCCESS;
}
