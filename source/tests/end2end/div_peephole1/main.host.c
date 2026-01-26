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
#include "./definitions.h"

int main(void) {
    for (int i = -100; i < 100; i++) {
#define TEST_OP(_type, _offset) \
        do { \
            assert(idiv_##_type##_offset((_type) i) == (_type) (((_type) i ) / (_type) (1ull << (_offset)))); \
            assert(div_##_type##_offset((unsigned _type) i) == (unsigned _type) (((unsigned _type) i ) / (unsigned _type) (1ull << (_offset)))); \
        } while (0);

        TEST_OP(char, 0)
        TEST_OP(char, 1)
        TEST_OP(char, 2)
        TEST_OP(char, 5)
        TEST_OP(char, 7)

        TEST_OP(short, 0)
        TEST_OP(short, 1)
        TEST_OP(short, 5)
        TEST_OP(short, 8)
        TEST_OP(short, 13)
        TEST_OP(short, 15)

        TEST_OP(int, 0)
        TEST_OP(int, 1)
        TEST_OP(int, 10)
        TEST_OP(int, 16)
        TEST_OP(int, 23)
        TEST_OP(int, 31)

        TEST_OP(long, 0)
        TEST_OP(long, 1)
        TEST_OP(long, 26)
        TEST_OP(long, 32)
        TEST_OP(long, 48)
        TEST_OP(long, 63)
    }
    return EXIT_SUCCESS;
}
