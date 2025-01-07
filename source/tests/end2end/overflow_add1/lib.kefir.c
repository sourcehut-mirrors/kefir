/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

    This file is part of Kefir project.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "./definitions.h"

#define DEFINE(_type1, _type2, _type3)                                                 \
    _Bool overflow_add_##_type1##_##_type2##_##_type3(_type1 a, _type2 b, _type3 *c) { \
        return __builtin_add_overflow(a, b, c);                                        \
    }

DEFINE(schar, schar, schar)
DEFINE(schar, schar, uchar)
DEFINE(schar, uchar, uchar)
DEFINE(uchar, schar, uchar)
DEFINE(int, short, uint)
DEFINE(uint, short, ushort)
DEFINE(long, int, int)
DEFINE(long, long, long)
DEFINE(ulong, long, long)
DEFINE(long, ulong, long)
DEFINE(ulong, ulong, long)
DEFINE(ulong, ulong, short)
DEFINE(ulong, ulong, ulong)
DEFINE(long, ulong, ulong)
DEFINE(ulong, long, ulong)
DEFINE(long, long, ulong)