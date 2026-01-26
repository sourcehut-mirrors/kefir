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

float f32[6] = {
    __builtin_copysignf(3.14f, 1.0f),
    __builtin_copysignf(3.14f, -1.0f),
    __builtin_copysignf(-3.14f, 1.0f),
    __builtin_copysignf(-3.14f, -1.0f),
    __builtin_copysignf(0.0f, 0.0f),
    __builtin_copysignf(0.0f, -0.0f)
};

double f64[6] = {
    __builtin_copysign(3.14, 1.0),
    __builtin_copysign(3.14, -1.0),
    __builtin_copysign(-3.14, 1.0),
    __builtin_copysign(-3.14, -1.0),
    __builtin_copysign(0.0, 0.0),
    __builtin_copysign(0.0, -0.0)
};

long double f80[6] = {
    __builtin_copysignl(3.14l, 1.0l),
    __builtin_copysignl(3.14l, -1.0l),
    __builtin_copysignl(-3.14l, 1.0l),
    __builtin_copysignl(-3.14l, -1.0l),
    __builtin_copysignl(0.0l, 0.0l),
    __builtin_copysignl(0.0l, -0.0l)
};

float my_copysignf(float x, float y) {
    return __builtin_copysignf(x, y);
}

double my_copysign(double x, double y) {
    return __builtin_copysign(x, y);
}

long double my_copysignl(long double x, long double y) {
    return __builtin_copysignl(x, y);
}
