/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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
typedef long long llong;
typedef unsigned long long ullong;

#define NEG_DISCRIMINANT(_a, _b, _c) (0 - ((_b) * (_b) -4u * (_a) * (_c)))

#define DECL_NEG_DISCRIMINANT(_type) extern _type neg_discriminant_##_type(_type, _type, _type);

DECL_NEG_DISCRIMINANT(char)
DECL_NEG_DISCRIMINANT(uchar)
DECL_NEG_DISCRIMINANT(short)
DECL_NEG_DISCRIMINANT(ushort)
DECL_NEG_DISCRIMINANT(int)
DECL_NEG_DISCRIMINANT(uint)
DECL_NEG_DISCRIMINANT(long)
DECL_NEG_DISCRIMINANT(ulong)
DECL_NEG_DISCRIMINANT(llong)
DECL_NEG_DISCRIMINANT(ullong)

#endif
