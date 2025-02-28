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

#define DEF(_op, _oper) \
    long select_##_op(long a, long b, long x, long y) { \
        return a _oper b ? x : y; \
    }

DEF(equals, ==)
DEF(not_equals, !=)
DEF(greater, >)
DEF(greater_or_equals, >=)
DEF(lesser, <)
DEF(lesser_or_equals, <=)

#undef DEF

#define DEF(_op, _oper) \
    long select_##_op(unsigned long a, unsigned long b, long x, long y) { \
        return a _oper b ? x : y; \
    }

DEF(above, >)
DEF(above_or_equals, >=)
DEF(below, <)
DEF(below_or_equals, <=)

#undef DEF
