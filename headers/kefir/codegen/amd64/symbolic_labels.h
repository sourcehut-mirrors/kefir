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

#ifndef KEFIR_CODEGEN_AMD64_SYMBOLIC_LABELS_H_
#define KEFIR_CODEGEN_AMD64_SYMBOLIC_LABELS_H_

#include "kefir/core/basic-types.h"

#define KEFIR_AMD64_TLS_GET_ADDR "__tls_get_addr"
#define KEFIR_AMD64_EMUTLS_V "__emutls_v.%s"
#define KEFIR_AMD64_EMUTLS_T "__emutls_t.%s"
#define KEFIR_AMD64_EMUTLS_GET_ADDR "__emutls_get_address"

#define KEFIR_AMD64_TEXT_SECTION_BEGIN "%s_text_section_begin"
#define KEFIR_AMD64_TEXT_SECTION_END "%s_text_section_end"

#define KEFIR_AMD64_FUNCTION_BEGIN "%s_text_func_%s_begin"
#define KEFIR_AMD64_FUNCTION_END "%s_text_func_%s_end"

#define KEFIR_AMD64_STRING_LITERAL "%s_string_literal%" KEFIR_ID_FMT
#define KEFIR_AMD64_LABEL "%s_func_%s_label%" KEFIR_SIZE_FMT
#define KEFIR_AMD64_CONSTANT_FLOAT32_TO_UINT "%s_constant_float32_to_uint"
#define KEFIR_AMD64_CONSTANT_FLOAT64_TO_UINT "%s_constant_float64_to_uint"
#define KEFIR_AMD64_CONSTANT_LONG_DOUBLE_TO_UINT "%s_constant_long_double_to_uint"
#define KEFIR_AMD64_CONSTANT_UINT_TO_LONG_DOUBLE "%s_constant_uint_to_long_double"
#define KEFIR_AMD64_CONSTANT_FLOAT32_NEG "%s_constant_float32_neg"
#define KEFIR_AMD64_CONSTANT_FLOAT64_NEG "%s_constant_float64_neg"
#define KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT32_NEG "%s_constant_complex_float32_neg"
#define KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT64_NEG "%s_constant_complex_float64_neg"
#define KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT32_MUL "%s_constant_complex_float32_mul"
#define KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT32_DIV "%s_constant_complex_float32_div"
#define KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT64_MUL "%s_constant_complex_float64_mul"
#define KEFIR_AMD64_CONSTANT_COMPLEX_LONG_DOUBLE_DIV "%s_constant_complex_long_double_div"
#define KEFIR_AMD64_CONSTANT_COPYSIGNF "%s_constant_copysignf"
#define KEFIR_AMD64_CONSTANT_COPYSIGN "%s_constant_copysign"
#define KEFIR_AMD64_CONSTANT_ISFINITEF32_MASK "%s_constant_isfinitef32_mask"
#define KEFIR_AMD64_CONSTANT_ISFINITEF32_CMP "%s_constant_isfinitef32_cmp"
#define KEFIR_AMD64_CONSTANT_ISFINITEF64_MASK "%s_constant_isfinitef64_mask"
#define KEFIR_AMD64_CONSTANT_ISFINITEF64_CMP "%s_constant_isfinitef64_cmp"
#define KEFIR_AMD64_CONSTANT_ISFINITEL_CMP "%s_constant_isfinitel_cmp"

#endif
