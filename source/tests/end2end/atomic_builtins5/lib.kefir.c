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

#include "./definitions.h"

#define DEFINE_FETCH(_id, _op, _type)                              \
    _type test_fetch_##_id(_Atomic _type *ptr, _type value) {      \
        return __atomic_fetch_##_op(ptr, value, __ATOMIC_SEQ_CST); \
    }

DEFINE_FETCH(add_char, add, char)
DEFINE_FETCH(add_short, add, short)
DEFINE_FETCH(add_int, add, int)
DEFINE_FETCH(add_long, add, long)

DEFINE_FETCH(sub_char, sub, char)
DEFINE_FETCH(sub_short, sub, short)
DEFINE_FETCH(sub_int, sub, int)
DEFINE_FETCH(sub_long, sub, long)

DEFINE_FETCH(or_char, or, char)
DEFINE_FETCH(or_short, or, short)
DEFINE_FETCH(or_int, or, int)
DEFINE_FETCH(or_long, or, long)

DEFINE_FETCH(and_char, and, char)
DEFINE_FETCH(and_short, and, short)
DEFINE_FETCH(and_int, and, int)
DEFINE_FETCH(and_long, and, long)

DEFINE_FETCH(xor_char, xor, char)
DEFINE_FETCH(xor_short, xor, short)
DEFINE_FETCH(xor_int, xor, int)
DEFINE_FETCH(xor_long, xor, long)
