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

struct S {
    char a : 6;
    short b : 10;
    int c : 24;
    long d : 31;

    _BitInt(2) e : 1;
    _BitInt(10) f : 7;
    _BitInt(100) g : 31;
    _BitInt(__BITINT_MAXWIDTH__) h : 10000;
} x;

ASSERT_TYPE(x.a + 0wb, int);
ASSERT_TYPE(x.b + 0wb, int);
ASSERT_TYPE(x.c + 0wb, int);
ASSERT_TYPE(x.d + 0wb, int);
ASSERT_TYPE(x.e + 0wb, _BitInt(2));
ASSERT_TYPE(x.f + 0wb, _BitInt(10));
ASSERT_TYPE(x.g + 0wb, _BitInt(100));
ASSERT_TYPE(x.h + 0wb, _BitInt(__BITINT_MAXWIDTH__));

struct S2 {
    unsigned char a : 6;
    unsigned short b : 10;
    unsigned int c : 24;
    unsigned long d : 31;

    unsigned _BitInt(2) e : 1;
    unsigned _BitInt(10) f : 7;
    unsigned _BitInt(100) g : 31;
    unsigned _BitInt(__BITINT_MAXWIDTH__) h : 10000;
} x2;

ASSERT_TYPE(x2.a + 0wb, int);
ASSERT_TYPE(x2.b + 0wb, int);
ASSERT_TYPE(x2.c + 0wb, int);
ASSERT_TYPE(x2.d + 0wb, int);
ASSERT_TYPE(x2.e + 0wb, unsigned _BitInt(2));
ASSERT_TYPE(x2.f + 0wb, unsigned _BitInt(10));
ASSERT_TYPE(x2.g + 0wb, unsigned _BitInt(100));
ASSERT_TYPE(x2.h + 0wb, unsigned _BitInt(__BITINT_MAXWIDTH__));