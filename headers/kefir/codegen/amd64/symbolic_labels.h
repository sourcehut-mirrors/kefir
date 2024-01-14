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

#define KEFIR_AMD64_PLT "%s@PLT"
#define KEFIR_AMD64_GOTPCREL "%s@GOTPCREL"
#define KEFIR_AMD64_THREAD_LOCAL "%s@tpoff"
#define KEFIR_AMD64_THREAD_LOCAL_GOT "%s@gottpoff"
#define KEFIR_AMD64_TLSGD "%s@tlsgd"
#define KEFIR_AMD64_TLS_GET_ADDR "__tls_get_addr@PLT"
#define KEFIR_AMD64_EMUTLS_V "__emutls_v.%s"
#define KEFIR_AMD64_EMUTLS_T "__emutls_t.%s"
#define KEFIR_AMD64_EMUTLS_GOT "__emutls_v.%s@GOTPCREL"
#define KEFIR_AMD64_EMUTLS_GET_ADDR "__emutls_get_address@PLT"

#define KEFIR_AMD64_STRING_LITERAL "__kefir_string_literal%" KEFIR_ID_FMT
#define KEFIR_AMD64_LABEL "_kefir_func_%s_label%" KEFIR_SIZE_FMT
#define KEFIR_AMD64_CONSTANT_FLOAT32_NEG "__kefir_opt_float32_neg"
#define KEFIR_AMD64_CONSTANT_FLOAT64_NEG "__kefir_opt_float64_neg"
#define KEFIR_AMD64_CONSTANT_UINT_TO_LONG_DOUBLE "__kefir_opt_uint2long_double"

#define KEFIR_AMD64_SYSTEM_V_RUNTIME_VARARG_SAVE "__kefirrt_opt_amd64_sysv_vararg_save"
#define KEFIR_AMD64_SYSTEM_V_RUNTIME_LOAD_INT_VARARG "__kefirrt_opt_load_int_vararg"
#define KEFIR_AMD64_SYSTEM_V_RUNTIME_LOAD_SSE_VARARG "__kefirrt_opt_load_sse_vararg"
#define KEFIR_AMD64_RUNTIME_FLOAT32_TO_UINT "__kefirrt_opt_float32_to_uint"
#define KEFIR_AMD64_RUNTIME_FLOAT64_TO_UINT "__kefirrt_opt_float64_to_uint"
#define KEFIR_AMD64_RUNTIME_LONG_DOUBLE_TO_INT "__kefirrt_opt_long_double_to_int"
#define KEFIR_AMD64_RUNTIME_LONG_DOUBLE_TO_UINT "__kefirrt_opt_long_double_to_uint"

#endif
