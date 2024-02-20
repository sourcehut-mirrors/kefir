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

#define DEF(_op, _sz, _type, _oper)                \
    _type _op##_##_sz(_Atomic _type *a, _type b) { \
        return (*a) _oper## = b;                   \
    }

#define DEF_ALL_SIZES(_op, _oper) \
    DEF(_op, i8, char, _oper)     \
    DEF(_op, i16, short, _oper)   \
    DEF(_op, i32, int, _oper)     \
    DEF(_op, i64, long, _oper)    \
    DEF(_op, f32, float, _oper)   \
    DEF(_op, f64, double, _oper)  \
    DEF(_op, ld, long double, _oper)

DEF_ALL_SIZES(multiply, *)
