/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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
#define BEGIN -300
#define END 300
    for (int i = BEGIN; i <= END; i++) {
        for (int j = BEGIN; j <= END; j++) {
#define ASSERT_OP(_id, _type, _op, _x, _y) \
    assert(op_##_id##_##_type((_type) (_x), (_type) (_y)) == (_type) OP_##_op((_type) (_x), (_type) (_y)))
#define ASSERT_OP1(_id, _type, _op, _x) assert(op_##_id##_##_type((_type) (_x)) == (_type) OP_##_op((_type) (_x)))
#define ASSERT_OPS_SIGNED(_id, _op, _x, _y) \
    ASSERT_OP(_id, char, _op, _x, _y);      \
    ASSERT_OP(_id, short, _op, _x, _y);     \
    ASSERT_OP(_id, int, _op, _x, _y);       \
    ASSERT_OP(_id, long, _op, _x, _y);      \
    ASSERT_OP(_id, llong, _op, _x, _y)
#define ASSERT_OPS_UNSIGNED(_id, _op, _x, _y) \
    ASSERT_OP(_id, uchar, _op, _x, _y);       \
    ASSERT_OP(_id, ushort, _op, _x, _y);      \
    ASSERT_OP(_id, uint, _op, _x, _y);        \
    ASSERT_OP(_id, ulong, _op, _x, _y);       \
    ASSERT_OP(_id, ullong, _op, _x, _y)
#define ASSERT_OPS(_id, _op, _x, _y)     \
    ASSERT_OPS_SIGNED(_id, _op, _x, _y); \
    ASSERT_OPS_UNSIGNED(_id, _op, _x, _y)

#define ASSERTS(_a, _b)                   \
    ASSERT_OPS(equals, EQUALS, _a, _b);   \
    ASSERT_OPS(greater, GREATER, _a, _b); \
    ASSERT_OPS(lesser, LESSER, _a, _b)

            ASSERTS(i, j);
            ASSERTS(i * 100, j * 100);
            ASSERTS(i - 10000, j + 500);
        }
    }
    return EXIT_SUCCESS;
}
