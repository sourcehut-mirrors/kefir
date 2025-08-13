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

#define DEF_OP(_name, _type, _op)                           \
    struct S1 _name##_type(_type a, _type b) {              \
        const _type x = a _op b;                            \
        const _type y = a _op b;                            \
        return (struct S1) {._type##1 = x, ._type##2 = -y}; \
    }
#define DEF_OPS(_name, _op)   \
    DEF_OP(_name, char, _op)  \
    DEF_OP(_name, short, _op) \
    DEF_OP(_name, int, _op)   \
    DEF_OP(_name, long, _op)
#define DEF_UOPS(_name, _op)   \
    DEF_OP(_name, uchar, _op)  \
    DEF_OP(_name, ushort, _op) \
    DEF_OP(_name, uint, _op)   \
    DEF_OP(_name, ulong, _op)

DEF_OPS(add, +)
DEF_OPS(sub, -)
DEF_OPS(mul, *)
DEF_OPS(div, /)
DEF_OPS(mod, %)
DEF_OPS(and, &)
DEF_OPS(or, |)
DEF_OPS(xor, ^)
DEF_OPS(shl, <<)
DEF_OPS(sar, >>)
DEF_OPS(band, &&)
DEF_OPS(bor, ||)

DEF_UOPS(umul, *)
DEF_UOPS(udiv, /)
DEF_UOPS(umod, %)
DEF_UOPS(shr, >>)
