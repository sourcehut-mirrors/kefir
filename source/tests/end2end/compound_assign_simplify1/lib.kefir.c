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

#include "./definitions.h"

#define DEF(_name, _type, _op)      \
    void _name(_type *x, _type y) { \
        *x _op y;                   \
    }

#define DEF2(_name, _type, _op) \
    void _name(_type *x) {      \
        *x = _op * x;           \
    }

DEF(add8, char, +=)
DEF(add16, short, +=)
DEF(add32, int, +=)
DEF(add64, long, +=)

DEF(sub8, char, -=)
DEF(sub16, short, -=)
DEF(sub32, int, -=)
DEF(sub64, long, -=)

DEF(and8, char, &=)
DEF(and16, short, &=)
DEF(and32, int, &=)
DEF(and64, long, &=)

DEF(or8, char, |=)
DEF(or16, short, |=)
DEF(or32, int, |=)
DEF(or64, long, |=)

DEF(xor8, char, ^=)
DEF(xor16, short, ^=)
DEF(xor32, int, ^=)
DEF(xor64, long, ^=)

DEF2(neg8, char, -)
DEF2(neg16, short, -)
DEF2(neg32, int, -)
DEF2(neg64, long, -)

DEF2(not8, char, ~)
DEF2(not16, short, ~)
DEF2(not32, int, ~)
DEF2(not64, long, ~)
