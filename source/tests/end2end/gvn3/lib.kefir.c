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

#define DEF_OP(_name, _type, _op)                          \
    struct S1 _name##_type(_type a) {                      \
        const _type x = _op a;                             \
        const _type y = _op a;                             \
        return (struct S1) {._type##1 = x, ._type##2 = y}; \
    }
#define DEF_OPS(_name, _op)   \
    DEF_OP(_name, char, _op)  \
    DEF_OP(_name, short, _op) \
    DEF_OP(_name, int, _op)   \
    DEF_OP(_name, long, _op)
DEF_OPS(neg, -)
DEF_OPS(not, ~)
DEF_OPS(bnot, !)
DEF_OPS(bool, (_Bool))
DEF_OPS(conv8, (signed char) )
DEF_OPS(conv16, (signed short) )
DEF_OPS(conv32, (signed int) )
DEF_OPS(uconv8, (unsigned char) )
DEF_OPS(uconv16, (unsigned short) )
DEF_OPS(uconv32, (unsigned int) )
