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

#define DEF_OP(_type, _offset) \
    _type idiv_##_type##_offset(_type x) { \
        return x / (_type) (1ull << (_offset)); \
    } \
    unsigned _type div_##_type##_offset(unsigned _type x) { \
        return x / (unsigned _type) (1ull << (_offset)); \
    } \

DEF_OP(char, 0)
DEF_OP(char, 1)
DEF_OP(char, 2)
DEF_OP(char, 5)
DEF_OP(char, 7)

DEF_OP(short, 0)
DEF_OP(short, 1)
DEF_OP(short, 5)
DEF_OP(short, 8)
DEF_OP(short, 13)
DEF_OP(short, 15)

DEF_OP(int, 0)
DEF_OP(int, 1)
DEF_OP(int, 10)
DEF_OP(int, 16)
DEF_OP(int, 23)
DEF_OP(int, 31)

DEF_OP(long, 0)
DEF_OP(long, 1)
DEF_OP(long, 26)
DEF_OP(long, 32)
DEF_OP(long, 48)
DEF_OP(long, 63)
