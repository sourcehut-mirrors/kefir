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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

typedef signed char schar;
typedef signed short sshort;
typedef signed int sint;
typedef signed long slong;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#define DECL_OP(_name, _type) \
    _Bool _name##_##_type(_Bool, _type, _type)
#define DECL_OPS(_name) \
    DECL_OP(_name, schar); \
    DECL_OP(_name, sshort); \
    DECL_OP(_name, sint); \
    DECL_OP(_name, slong); \
    DECL_OP(_name, uchar); \
    DECL_OP(_name, ushort); \
    DECL_OP(_name, uint); \
    DECL_OP(_name, ulong)

DECL_OPS(equal);
DECL_OPS(not_equal);
DECL_OPS(greater);
DECL_OPS(greater_or_equal);
DECL_OPS(less);
DECL_OPS(less_or_equal);

enum test_op {
    OP_EQ,
    OP_NE,
    OP_GT,
    OP_GE,
    OP_LT,
    OP_LE
};

#define DECL_TEST(_type) \
    _Bool test_##_type(enum test_op, _type, _type) \

DECL_TEST(schar);
DECL_TEST(sshort);
DECL_TEST(sint);
DECL_TEST(slong);
DECL_TEST(uchar);
DECL_TEST(ushort);
DECL_TEST(uint);
DECL_TEST(ulong);

#undef DECL_OPS
#undef DECL_OP
#undef DECL_TEST

#endif
