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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#define DECL(_type, _name) void _name##_##_type(_type *, _type)

#define DECL_OP(_name)   \
    DECL(char, _name);   \
    DECL(short, _name);  \
    DECL(int, _name);    \
    DECL(long, _name);   \
    DECL(uchar, _name);  \
    DECL(ushort, _name); \
    DECL(uint, _name);   \
    DECL(ulong, _name)

DECL_OP(add);
DECL_OP(sub);
DECL_OP(and);
DECL_OP(or);
DECL_OP(xor);
DECL_OP(shl);
DECL_OP(shr);

#undef DECL_OP
#undef DECL

#endif
