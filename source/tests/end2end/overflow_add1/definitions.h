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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

typedef signed char schar;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef signed long long sllong;
typedef unsigned long long ullong;

#define DECLARE(_type1, _type2, _type3) _Bool overflow_add_##_type1##_##_type2##_##_type3(_type1, _type2, _type3 *);

DECLARE(schar, schar, schar)
DECLARE(schar, schar, uchar)
DECLARE(schar, uchar, uchar)
DECLARE(uchar, schar, uchar)
DECLARE(int, short, uint)
DECLARE(uint, short, ushort)
DECLARE(long, int, int)
DECLARE(long, long, long)
DECLARE(ulong, long, long)
DECLARE(long, ulong, long)
DECLARE(ulong, ulong, long)
DECLARE(ulong, ulong, short)
DECLARE(ulong, ulong, ulong)
DECLARE(long, ulong, ulong)
DECLARE(ulong, long, ulong)
DECLARE(long, long, ulong)

#undef DECLARE

#endif
