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

#ifndef KEFIR_UTIL_CHAR32_H_
#define KEFIR_UTIL_CHAR32_H_

#include "kefir/core/basic-types.h"

#define KEFIR_STRCMP32_ERROR KEFIR_INT_MAX

kefir_size_t kefir_strlen32(const kefir_char32_t *);
kefir_bool_t kefir_isspace32(kefir_char32_t);
kefir_bool_t kefir_isdigit32(kefir_char32_t);
kefir_bool_t kefir_isoctdigit32(kefir_char32_t);
kefir_bool_t kefir_ishexdigit32(kefir_char32_t);
kefir_bool_t kefir_isnondigit32(kefir_char32_t);
kefir_int_t kefir_strcmp32(const kefir_char32_t *, const kefir_char32_t *);
kefir_uint32_t kefir_hex32todec(kefir_char32_t);
kefir_char32_t kefir_dectohex32(kefir_uint64_t);

#endif
