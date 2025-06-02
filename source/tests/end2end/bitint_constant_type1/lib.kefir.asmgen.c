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

#define ASSERT_TYPE(_expr, _type) \
    _Static_assert(_Generic((_expr), _type: 1, default: 0), "Assertion on expression type has failed")

ASSERT_TYPE(0uwb, unsigned _BitInt(1));
ASSERT_TYPE(1uwb, unsigned _BitInt(1));
ASSERT_TYPE(2uwb, unsigned _BitInt(2));
ASSERT_TYPE(0xffuwb, unsigned _BitInt(8));
ASSERT_TYPE(0xfffuwb, unsigned _BitInt(12));
ASSERT_TYPE(0xffffuwb, unsigned _BitInt(16));
ASSERT_TYPE(0xfffffuwb, unsigned _BitInt(20));
ASSERT_TYPE(0xffffffuwb, unsigned _BitInt(24));
ASSERT_TYPE(0xffffffffffffffffuwb, unsigned _BitInt(64));
ASSERT_TYPE(0xffffffffffffffffffffffffuwb, unsigned _BitInt(96));
ASSERT_TYPE(0xfffffffffffffffffffffffffffffffuwb, unsigned _BitInt(124));
ASSERT_TYPE(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffuwb, unsigned _BitInt(256));

ASSERT_TYPE(0wb, signed _BitInt(2));
ASSERT_TYPE(1wb, signed _BitInt(2));
ASSERT_TYPE(2wb, signed _BitInt(3));
ASSERT_TYPE(0x7fwb, signed _BitInt(8));
ASSERT_TYPE(0x80wb, signed _BitInt(9));
ASSERT_TYPE(0xffwb, signed _BitInt(9));
ASSERT_TYPE(0x7ffwb, signed _BitInt(12));
ASSERT_TYPE(0xfffwb, signed _BitInt(13));
ASSERT_TYPE(0x7fffwb, signed _BitInt(16));
ASSERT_TYPE(0xffffwb, signed _BitInt(17));
ASSERT_TYPE(0x7ffffwb, signed _BitInt(20));
ASSERT_TYPE(0xfffffwb, signed _BitInt(21));
ASSERT_TYPE(0x7fffffwb, signed _BitInt(24));
ASSERT_TYPE(0xffffffwb, signed _BitInt(25));
ASSERT_TYPE(0x7fffffffffffffffwb, signed _BitInt(64));
ASSERT_TYPE(0xffffffffffffffffwb, signed _BitInt(65));
ASSERT_TYPE(0x7fffffffffffffffffffffffwb, signed _BitInt(96));
ASSERT_TYPE(0xffffffffffffffffffffffffwb, signed _BitInt(97));
ASSERT_TYPE(0x7ffffffffffffffffffffffffffffffwb, signed _BitInt(124));
ASSERT_TYPE(0xfffffffffffffffffffffffffffffffwb, signed _BitInt(125));
ASSERT_TYPE(0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffwb, signed _BitInt(256));
ASSERT_TYPE(0x8000000000000000000000000000000000000000000000000000000000000000wb, signed _BitInt(257));
ASSERT_TYPE(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffwb, signed _BitInt(257));
