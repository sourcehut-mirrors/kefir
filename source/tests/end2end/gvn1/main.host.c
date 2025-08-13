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
        for (int j = -100; j < 100; j++) {
#define TEST_OP(_name, _type, _op)                                      \
    do {                                                                \
        struct S1 res = _name##_type(i, j);                             \
        assert(res._type##1 == (_type) (((_type) i) _op((_type) j)));   \
        assert(res._type##2 == (_type) - (((_type) i) _op((_type) j))); \
    } while (0)
#define TEST_OPS(_name, _op)    \
    TEST_OP(_name, char, _op);  \
    TEST_OP(_name, short, _op); \
    TEST_OP(_name, int, _op);   \
    TEST_OP(_name, long, _op)
#define TEST_UOPS(_name, _op)    \
    TEST_OP(_name, uchar, _op);  \
    TEST_OP(_name, ushort, _op); \
    TEST_OP(_name, uint, _op);   \
    TEST_OP(_name, ulong, _op)

            TEST_OPS(add, +);
            TEST_OPS(sub, -);
            TEST_OPS(mul, *);
            if (j != 0) {
                TEST_OPS(div, /);
                TEST_OPS(mod, %);
            }
            TEST_OPS(and, &);
            TEST_OPS(or, |);
            TEST_OPS(xor, ^);
            if (j >= 0 && j <= 8) {
                if (i >= 0) {
                    TEST_OPS(shl, <<);
                }
                TEST_OPS(sar, >>);
            }
        }
    }

    for (unsigned int i = 0; i < 200; i++) {
        for (unsigned int j = 0; j < 200; j++) {
#define TEST_UOPS(_name, _op)    \
    TEST_OP(_name, uchar, _op);  \
    TEST_OP(_name, ushort, _op); \
    TEST_OP(_name, uint, _op);   \
    TEST_OP(_name, ulong, _op)

            TEST_UOPS(umul, *);
            if (j != 0) {
                TEST_UOPS(udiv, /);
                TEST_UOPS(umod, %);
            }
            if (j <= 8) {
                TEST_UOPS(shr, >>);
            }
        }
    }
    return EXIT_SUCCESS;
}
