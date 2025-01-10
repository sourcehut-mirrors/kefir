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
typedef unsigned char uchar;
typedef signed short sshort;
typedef unsigned short ushort;
typedef signed int sint;
typedef unsigned int uint;
typedef signed long slong;
typedef unsigned long ulong;

#define DECL(_type, _shift) _type _type##_shl##_shift(_type)
#define DECL_SHIFTS(_type) \
    DECL(_type, 1);        \
    DECL(_type, 2);        \
    DECL(_type, 4);        \
    DECL(_type, 8);        \
    DECL(_type, 16);       \
    DECL(_type, 32);       \
    DECL(_type, 64)

DECL_SHIFTS(schar);
DECL_SHIFTS(uchar);
DECL_SHIFTS(sshort);
DECL_SHIFTS(ushort);
DECL_SHIFTS(sint);
DECL_SHIFTS(uint);
DECL_SHIFTS(slong);
DECL_SHIFTS(ulong);

#endif
