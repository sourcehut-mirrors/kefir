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

#ifndef KEFIR_UTIL_SOFTFLOAT_H_
#define KEFIR_UTIL_SOFTFLOAT_H_

#include "kefir/core/basic-types.h"

typedef struct kefir_softfloat_complex_long_double {
    kefir_long_double_t real;
    kefir_long_double_t imaginary;
} kefir_softfloat_complex_long_double_t;

struct kefir_softfloat_complex_long_double kefir_softfloat_complex_long_double_mul(struct kefir_softfloat_complex_long_double, struct kefir_softfloat_complex_long_double);
struct kefir_softfloat_complex_long_double kefir_softfloat_complex_long_double_div(struct kefir_softfloat_complex_long_double, struct kefir_softfloat_complex_long_double);

#endif
