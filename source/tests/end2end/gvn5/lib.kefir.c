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

#include "./definitions.h"

#define DEF_OP(_name, _type, _op, _op2) \
    _Bool _name##_##_type(_Bool c, _type a, _type b) { \
        _Bool r; \
        if (c) { \
            r = a _op b; \
        } else { \
            r = b _op2 a; \
        } \
        return r; \
    }
#define DEF_OPS(_name, _op, _op2) \
    DEF_OP(_name, schar, _op, _op2) \
    DEF_OP(_name, sshort, _op, _op2) \
    DEF_OP(_name, sint, _op, _op2) \
    DEF_OP(_name, slong, _op, _op2) \
    DEF_OP(_name, uchar, _op, _op2) \
    DEF_OP(_name, ushort, _op, _op2) \
    DEF_OP(_name, uint, _op, _op2) \
    DEF_OP(_name, ulong, _op, _op2) \

DEF_OPS(equal, ==, ==)
DEF_OPS(not_equal, !=, !=)
DEF_OPS(greater, >, <)
DEF_OPS(greater_or_equal, >=, <=)
DEF_OPS(less, <, >)
DEF_OPS(less_or_equal, <=, >=)

#define DEF_TEST(_type) \
    _Bool test_##_type(enum test_op op, _type a, _type b) { \
        switch (op) { \
            case OP_EQ: return a == b; \
            case OP_NE: return a != b; \
            case OP_GT: return a > b; \
            case OP_GE: return a >= b; \
            case OP_LT: return a < b; \
            case OP_LE: return a <= b; \
        } \
        __builtin_trap(); \
    }

DEF_TEST(schar)
DEF_TEST(sshort)
DEF_TEST(sint)
DEF_TEST(slong)
DEF_TEST(uchar)
DEF_TEST(ushort)
DEF_TEST(uint)
DEF_TEST(ulong)
