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

int f32[6] = {
    __builtin_isfinite((float) 0.0f),
    __builtin_isfinite((float) 1.0f),
    __builtin_isfinite((float) -3.14f),
    __builtin_isfinite((float) __builtin_inff()),
    __builtin_isfinite((float) -__builtin_inff()),
    __builtin_isfinite((float) __builtin_nanf(""))
};

int f64[6] = {
    __builtin_isfinite((double) 0.0),
    __builtin_isfinite((double) 1.0),
    __builtin_isfinite((double) -3.14),
    __builtin_isfinite((double) __builtin_inf()),
    __builtin_isfinite((double) -__builtin_inf()),
    __builtin_isfinite((double) __builtin_nan(""))
};

int f80[6] = {
    __builtin_isfinite((long double) 0.0L),
    __builtin_isfinite((long double) 1.0L),
    __builtin_isfinite((long double) -3.14L),
    __builtin_isfinite((long double) __builtin_infl()),
    __builtin_isfinite((long double) -__builtin_infl()),
    __builtin_isfinite((long double) __builtin_nanl(""))
};

_Bool is_finite_f32(float x) {
    return __builtin_isfinite(x);
}

_Bool is_finite_f64(double x) {
    return __builtin_isfinite(x);
}

_Bool is_finite_f80(long double x) {
    return __builtin_isfinite(x);
}
