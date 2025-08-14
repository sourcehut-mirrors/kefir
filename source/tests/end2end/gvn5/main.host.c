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
    for (int i = -512; i < 512; i++) {
        for (int j = -64; j < 64; j++) {
#define TEST_OP(_name, _type, _op)                      \
        do {                                                \
            assert(_name##_##_type(1, i, j) == (((_type) i) _op ((_type) j))); \
            assert(_name##_##_type(0, i, j) == (((_type) i) _op ((_type) j))); \
        } while (0)
#define TEST_OPS(_name, _op)    \
        TEST_OP(_name, schar, _op);  \
        TEST_OP(_name, sshort, _op); \
        TEST_OP(_name, sint, _op);   \
        TEST_OP(_name, slong, _op); \
        TEST_OP(_name, uchar, _op);  \
        TEST_OP(_name, ushort, _op); \
        TEST_OP(_name, uint, _op);   \
        TEST_OP(_name, ulong, _op)

            TEST_OPS(equal, ==);
            TEST_OPS(not_equal, !=);
            TEST_OPS(greater, >);
            TEST_OPS(greater_or_equal, >=);
            TEST_OPS(less, <);
            TEST_OPS(less_or_equal, <=);

#define TEST(_type) \
            do { \
                assert(test##_##_type(OP_EQ, i, j) == (((_type) i) == ((_type) j))); \
                assert(test##_##_type(OP_NE, i, j) == (((_type) i) != ((_type) j))); \
                assert(test##_##_type(OP_GT, i, j) == (((_type) i) > ((_type) j))); \
                assert(test##_##_type(OP_GE, i, j) == (((_type) i) >= ((_type) j))); \
                assert(test##_##_type(OP_LT, i, j) == (((_type) i) < ((_type) j))); \
                assert(test##_##_type(OP_LE, i, j) == (((_type) i) <= ((_type) j))); \
            } while (0)

            TEST(schar);
            TEST(sshort);
            TEST(sint);
            TEST(slong);
            TEST(uchar);
            TEST(ushort);
            TEST(uint);
            TEST(ulong);
        }
    }
    return EXIT_SUCCESS;
}
