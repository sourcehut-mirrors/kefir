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

#include "./definitions.h"

#define DEF(_type, _name, _op)                      \
    void _name##_##_type(_type *ptr, _type value) { \
        *ptr _op## = value;                         \
    }

#define DEF_OP(_name, _op)  \
    DEF(char, _name, _op)   \
    DEF(short, _name, _op)  \
    DEF(int, _name, _op)    \
    DEF(long, _name, _op)   \
    DEF(uchar, _name, _op)  \
    DEF(ushort, _name, _op) \
    DEF(uint, _name, _op)   \
    DEF(ulong, _name, _op)

DEF_OP(add, +)
DEF_OP(sub, -)
DEF_OP(and, &)
DEF_OP(or, |)
DEF_OP(xor, ^)
DEF_OP(shl, <<)
DEF_OP(shr, >>)

void neg_char(char *ptr) {
    *ptr *= -1;
}

void neg_short(short *ptr) {
    *ptr *= -1;
}

void neg_int(int *ptr) {
    *ptr *= -1;
}

void neg_long(long *ptr) {
    *ptr *= -1;
}

void not_char(char *ptr) {
    *ptr = ~*ptr;
}

void not_short(short *ptr) {
    *ptr = ~*ptr;
}

void not_int(int *ptr) {
    *ptr = ~*ptr;
}

void not_long(long *ptr) {
    *ptr = ~*ptr;
}
