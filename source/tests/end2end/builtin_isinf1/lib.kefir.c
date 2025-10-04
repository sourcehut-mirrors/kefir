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

int f32[3] = {
    __builtin_isinf((float) __builtin_inff()),
    __builtin_isinf(-(float) __builtin_inff()),
    __builtin_isinf((float) 3.14f)
};

int f64[3] = {
    __builtin_isinf((double) __builtin_inf()),
    __builtin_isinf(-(double) __builtin_inf()),
    __builtin_isinf((double) 3.14)
};

int f80[3] = {
    __builtin_isinf((long double) __builtin_infl()),
    __builtin_isinf(-(long double) __builtin_infl()),
    __builtin_isinf((long double) 3.14L)
};

int is_inf_f32(float x) {
    return __builtin_isinf(x);
}

int is_inf_f64(double x) {
    return __builtin_isinf(x);
}

int is_inf_f80(long double x) {
    return __builtin_isinf(x);
}
