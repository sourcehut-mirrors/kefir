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

int f32[6] = {__builtin_isgreater(3.14f, 0.0f),    __builtin_isgreater(3.14f, 4.14f),
              __builtin_isgreater(-3.13f, -3.14f), __builtin_isgreater(-3.13f, -3.10f),
              __builtin_isgreater(1.0f, 1.0f),     __builtin_isgreater(-1.0f, -1.0f)};

int f64[6] = {__builtin_isgreater(3.14, 0.0),    __builtin_isgreater(3.14, 4.14), __builtin_isgreater(-3.13, -3.14),
              __builtin_isgreater(-3.13, -3.10), __builtin_isgreater(1.0, 1.0),   __builtin_isgreater(-1.0, -1.0)};

int f80[6] = {__builtin_isgreater(3.14L, 0.0L),    __builtin_isgreater(3.14L, 4.14L),
              __builtin_isgreater(-3.13L, -3.14L), __builtin_isgreater(-3.13L, -3.10L),
              __builtin_isgreater(1.0L, 1.0L),     __builtin_isgreater(-1.0L, -1.0L)};

int is_greater_f32(float x, float y) {
    return __builtin_isgreater(x, y);
}

int is_greater_f64(double x, double y) {
    return __builtin_isgreater(x, y);
}

int is_greater_f80(long double x, long double y) {
    return __builtin_isgreater(x, y);
}
