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

#ifndef KEFIR_DFP_SHIM_H_
#define KEFIR_DFP_SHIM_H_

#include "kefir/core/basic-types.h"

typedef struct kefir_dfp_shim_decimal32 {
    kefir_uint32_t uvalue;
} kefir_dfp_shim_decimal32_t;

typedef struct kefir_dfp_shim_decimal64 {
    kefir_uint64_t uvalue;
} kefir_dfp_shim_decimal64_t;

typedef struct kefir_dfp_shim_decimal128 {
    kefir_uint64_t uvalue[2];
} kefir_dfp_shim_decimal128_t;

kefir_dfp_shim_decimal32_t kefir_dfp_shim_decimal32_from_int64(kefir_int64_t);
kefir_dfp_shim_decimal32_t kefir_dfp_shim_decimal32_from_uint64(kefir_uint64_t);
kefir_dfp_shim_decimal64_t kefir_dfp_shim_decimal64_from_int64(kefir_int64_t);
kefir_dfp_shim_decimal64_t kefir_dfp_shim_decimal64_from_uint64(kefir_uint64_t);
kefir_dfp_shim_decimal128_t kefir_dfp_shim_decimal128_from_int64(kefir_int64_t);
kefir_dfp_shim_decimal128_t kefir_dfp_shim_decimal128_from_uint64(kefir_uint64_t);

#endif
