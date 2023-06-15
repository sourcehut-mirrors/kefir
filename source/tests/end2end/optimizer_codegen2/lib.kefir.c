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

#include "./definitions.h"

#define DEFINE_BIN_FN(_id, _type, _op) \
    _type op_##_id##_##_type(_type x, _type y) { return OP_##_op(x, y); }

#define DEFINE_BIN_FNS(_id, _op)    \
    DEFINE_BIN_FN(_id, char, _op)   \
    DEFINE_BIN_FN(_id, uchar, _op)  \
    DEFINE_BIN_FN(_id, short, _op)  \
    DEFINE_BIN_FN(_id, ushort, _op) \
    DEFINE_BIN_FN(_id, int, _op)    \
    DEFINE_BIN_FN(_id, uint, _op)   \
    DEFINE_BIN_FN(_id, long, _op)   \
    DEFINE_BIN_FN(_id, ulong, _op)  \
    DEFINE_BIN_FN(_id, llong, _op)  \
    DEFINE_BIN_FN(_id, ullong, _op)

DEFINE_BIN_FNS(equals, EQUALS)
DEFINE_BIN_FNS(greater, GREATER)
DEFINE_BIN_FNS(lesser, LESSER)