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

#ifndef KEFIR_CODEGEN_AMD64_SYMBOLIC_LABELS_H_
#define KEFIR_CODEGEN_AMD64_SYMBOLIC_LABELS_H_

#include "kefir/core/basic-types.h"

#define KEFIR_AMD64_TLS_GET_ADDR "__tls_get_addr"
#define KEFIR_AMD64_EMUTLS_V "__emutls_v.%s"
#define KEFIR_AMD64_EMUTLS_T "__emutls_t.%s"
#define KEFIR_AMD64_EMUTLS_GET_ADDR "__emutls_get_address"

#define KEFIR_AMD64_TEXT_SECTION_BEGIN "__kefir_text_section_begin"
#define KEFIR_AMD64_TEXT_SECTION_END "__kefir_text_section_end"

#define KEFIR_AMD64_FUNCTION_BEGIN "__kefir_text_func_%s_begin"
#define KEFIR_AMD64_FUNCTION_END "__kefir_text_func_%s_end"

#define KEFIR_AMD64_VARARG_SAVE_INT "__kefir_text_func_%s_vararg_save_int"

#define KEFIR_AMD64_STRING_LITERAL "__kefir_string_literal%" KEFIR_ID_FMT
#define KEFIR_AMD64_LABEL "_kefir_func_%s_label%" KEFIR_SIZE_FMT
#define KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT32_NEG "__kefir_opt_complex_float32_neg"
#define KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT64_NEG "__kefir_opt_complex_float64_neg"

#define KEFIR_AMD64_RUNTIME_COMPLEX_LONG_DOUBLE_EQUALS "__kefirrt_opt_complex_long_double_equals"
#define KEFIR_AMD64_RUNTIME_COMPLEX_LONG_DOUBLE_TRUNCATE_1BIT "__kefirrt_opt_complex_long_double_truncate_1bit"
#define KEFIR_AMD64_RUNTIME_COMPLEX_FLOAT32_MUL "__kefirrt_opt_complex_float32_mul"
#define KEFIR_AMD64_RUNTIME_COMPLEX_FLOAT32_DIV "__kefirrt_opt_complex_float32_div"
#define KEFIR_AMD64_RUNTIME_COMPLEX_FLOAT64_MUL "__kefirrt_opt_complex_float64_mul"
#define KEFIR_AMD64_RUNTIME_COMPLEX_FLOAT64_DIV "__kefirrt_opt_complex_float64_div"
#define KEFIR_AMD64_RUNTIME_COMPLEX_LONG_DOUBLE_MUL "__kefirrt_opt_complex_long_double_mul"
#define KEFIR_AMD64_RUNTIME_COMPLEX_LONG_DOUBLE_DIV "__kefirrt_opt_complex_long_double_div"
#define KEFIR_AMD64_RUNTIME_FENV_UPDATE "__kefirrt_fenv_update"

#endif
