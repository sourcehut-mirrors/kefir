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

_Complex float add_f32(_Atomic const _Complex float *ptr) {
    return f32_1 + *ptr;
}

_Complex float sub_f32(_Atomic const _Complex float *ptr) {
    return f32_1 - *ptr;
}

_Complex float mul_f32(_Atomic const _Complex float *ptr) {
    return f32_1 * *ptr;
}

_Complex float div_f32(_Atomic const _Complex float *ptr) {
    return f32_1 / *ptr;
}

_Complex float neg_f32(void) {
    return -f32_1;
}

_Complex double add_f64(_Atomic const _Complex double *ptr) {
    return f64_1 + *ptr;
}

_Complex double sub_f64(_Atomic const _Complex double *ptr) {
    return f64_1 - *ptr;
}

_Complex double mul_f64(_Atomic const _Complex double *ptr) {
    return f64_1 * *ptr;
}

_Complex double div_f64(_Atomic const _Complex double *ptr) {
    return f64_1 / *ptr;
}

_Complex double neg_f64(void) {
    return -f64_1;
}

_Complex long double add_ld(_Atomic const _Complex long double *ptr) {
    return ld_1 + *ptr;
}

_Complex long double sub_ld(_Atomic const _Complex long double *ptr) {
    return ld_1 - *ptr;
}

_Complex long double mul_ld(_Atomic const _Complex long double *ptr) {
    return ld_1 * *ptr;
}

_Complex long double div_ld(_Atomic const _Complex long double *ptr) {
    return ld_1 / *ptr;
}

_Complex long double neg_ld(void) {
    return -ld_1;
}
