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

#define DEFINE_FN(_type, _id) \
    _type get_##_id(void) {   \
        return _id;           \
    }

DEFINE_FN(_Complex float, x)
DEFINE_FN(float _Complex, y)
DEFINE_FN(_Complex double, a)
DEFINE_FN(double _Complex, b)
DEFINE_FN(_Complex long double, i)
DEFINE_FN(_Complex double long, j)
DEFINE_FN(long _Complex double, k)
DEFINE_FN(long double _Complex, l)
DEFINE_FN(double _Complex long, m)
DEFINE_FN(double long _Complex, n)

_Complex float nonef(void) {}
_Complex double none(void) {}
_Complex long double nonel(void) {}
