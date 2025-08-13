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
    for (int i = -100; i < 100; i++) {
#define TEST_OP(_name, _type, _op)                      \
    do {                                                \
        struct S1 res = _name##_type(i);                \
        assert(res._type##1 == (_type) _op((_type) i)); \
        assert(res._type##2 == (_type) _op((_type) i)); \
    } while (0)
#define TEST_OPS(_name, _op)    \
    TEST_OP(_name, char, _op);  \
    TEST_OP(_name, short, _op); \
    TEST_OP(_name, int, _op);   \
    TEST_OP(_name, long, _op)

        TEST_OPS(neg, -);
        TEST_OPS(not, ~);
        TEST_OPS(bnot, !);
        TEST_OPS(bool, (_Bool));
        TEST_OPS(conv8, (signed char) );
        TEST_OPS(conv16, (signed short) );
        TEST_OPS(conv32, (signed int) );
        TEST_OPS(uconv8, (unsigned char) );
        TEST_OPS(uconv16, (unsigned short) );
        TEST_OPS(uconv32, (unsigned int) );
    }
    return EXIT_SUCCESS;
}
