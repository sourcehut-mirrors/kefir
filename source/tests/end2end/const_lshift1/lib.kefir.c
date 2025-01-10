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

#define DEF(_type, _shift)                   \
    _type _type##_shl##_shift(_type value) { \
        return value << (_shift);            \
    }
#define DEF_SHIFTS(_type) \
    DEF(_type, 1)         \
    DEF(_type, 2)         \
    DEF(_type, 4)         \
    DEF(_type, 8)         \
    DEF(_type, 16)        \
    DEF(_type, 32);       \
    DEF(_type, 64)

DEF_SHIFTS(schar)
DEF_SHIFTS(uchar)
DEF_SHIFTS(sshort)
DEF_SHIFTS(ushort)
DEF_SHIFTS(sint)
DEF_SHIFTS(uint)
DEF_SHIFTS(slong)
DEF_SHIFTS(ulong)
