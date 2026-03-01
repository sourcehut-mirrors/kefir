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
    for (int i = -4096; i < 4096; i++) {
        for (int j = -4096; j < 4096; j++) {
#define TEST_OP(_type, _name, _op)                                    \
    do {                                                              \
        _type val = i;                                                \
        _name##_##_type(&val, j);                                     \
        assert(val == (_type) (((_type) i) _op((_type) j)));          \
        u##_type uval = i;                                            \
        _name##_u##_type(&uval, j);                                   \
        assert(uval == (u##_type)(((u##_type) i) _op((u##_type) j))); \
    } while (0)

#define TEST_OPS(_name, _op)    \
    TEST_OP(char, _name, _op);  \
    TEST_OP(short, _name, _op); \
    TEST_OP(int, _name, _op);   \
    TEST_OP(long, _name, _op)

            TEST_OPS(add, +);
            TEST_OPS(sub, -);
            TEST_OPS(xor, ^);
            TEST_OPS(or, |);
            TEST_OPS(and, &);
            if (i >= 0 && j >= 0 && j < 8) {
                TEST_OPS(shl, <<);
                TEST_OPS(shr, >>);
            }

#undef TEST_OP
        }

        char i8 = i;
        neg_char(&i8);
        assert(i8 == (char) -i);

        short i16 = i;
        neg_short(&i16);
        assert(i16 == (short) -i);

        int i32 = i;
        neg_int(&i32);
        assert(i32 == (int) -i);

        long i64 = i;
        neg_long(&i64);
        assert(i64 == (long) -i);

        i8 = i;
        not_char(&i8);
        assert(i8 == (char) ~i);

        i16 = i;
        not_short(&i16);
        assert(i16 == (short) ~i);

        i32 = i;
        not_int(&i32);
        assert(i32 == (int) ~i);

        i64 = i;
        not_long(&i64);
        assert(i64 == (long) ~i);
    }
    return EXIT_SUCCESS;
}
