/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#ifndef KEFIR_CODEGEN_NAIVE_SYSTEM_V_AMD64_SYMBOLIC_LABELS_H_
#define KEFIR_CODEGEN_NAIVE_SYSTEM_V_AMD64_SYMBOLIC_LABELS_H_

#include "kefir/core/basic-types.h"

#define KEFIR_AMD64_SYSTEM_V_RUNTIME_PRESERVE_STATE "__kefirrt_preserve_state"
#define KEFIR_AMD64_SYSTEM_V_RUNTIME_GENERIC_PROLOGUE "__kefirrt_generic_prologue"
#define KEFIR_AMD64_SYSTEM_V_RUNTIME_RESTORE_STATE "__kefirrt_restore_state"
#define KEFIR_AMD64_SYSTEM_V_RUNTIME_SAVE_REGISTERS "__kefirrt_save_registers"
#define KEFIR_AMD64_SYSTEM_V_RUNTIME_VARARG_INT "__kefirrt_load_integer_vararg"
#define KEFIR_AMD64_SYSTEM_V_RUNTIME_VARARG_SSE "__kefirrt_load_sse_vararg"
#define KEFIR_AMD64_SYSTEM_V_RUNTIME_VARARG_LONG_DOUBLE "__kefirrt_load_long_double_vararg"
#define KEFIR_AMD64_SYSTEM_V_RUNTIME_VARARG_COPY "__kefirrt_copy_vararg"
#define KEFIR_AMD64_SYSTEM_V_RUNTIME_STRING_LITERAL "__kefirrt_string_literal%" KEFIR_ID_FMT
#define KEFIR_AMD64_SYSTEM_V_RUNTIME_INLINE_ASSEMBLY_FRAGMENT "__kefirrt_inline_assembly%" KEFIR_ID_FMT
#define KEFIR_AMD64_SYSTEM_V_RUNTIME_INLINE_ASSEMBLY_JUMP_TRAMPOLINE \
    "__kefirrt_inline_assembly%" KEFIR_ID_FMT "_jump%" KEFIR_ID_FMT
#define KEFIR_AMD64_SYSTEM_V_RUNTIME_INLINE_ASSEMBLY_JUMP_TRAMPOLINE_END \
    "__kefirrt_inline_assembly%" KEFIR_ID_FMT "_jump_end"

extern const char *KEFIR_AMD64_SYSTEM_V_RUNTIME_SYMBOLS[];
extern kefir_size_t KEFIR_AMD64_SYSTEM_V_RUNTIME_SYMBOL_COUNT;

#define KEFIR_AMD64_SYSV_PROCEDURE_LABEL "%s"
#define KEFIR_AMD64_SYSV_PROCEDURE_BODY_LABEL "__%s_body"
#define KEFIR_AMD64_SYSV_PROCEDURE_EPILOGUE_LABEL "__%s_epilogue"
#define KEFIR_AMD64_SYSV_FUNCTION_GATE_NAMED_LABEL "__kefirrt_sfunction_%s_gate%" KEFIR_ID_FMT
#define KEFIR_AMD64_SYSV_FUNCTION_GATE_ID_LABEL "__kefirrt_ifunction_gate%" KEFIR_ID_FMT
#define KEFIR_AMD64_SYSV_FUNCTION_VIRTUAL_GATE_NAMED_LABEL "__kefirrt_sfunction_%s_vgate"
#define KEFIR_AMD64_SYSV_FUNCTION_VIRTUAL_GATE_ID_LABEL "__kefirrt_ifunction_vgate%" KEFIR_ID_FMT
#define KEFIR_AMD64_SYSV_FUNCTION_TLS_ENTRY "__kefirrt_tls_entry_%s"
#define KEFIR_AMD64_SYSV_FUNCTION_VARARG_START_LABEL "__%s_vararg_start"
#define KEFIR_AMD64_SYSV_FUNCTION_VARARG_END_LABEL "__%s_vararg_end"
#define KEFIR_AMD64_SYSV_FUNCTION_VARARG_ARG_LABEL "__%s_vararg_type%" KEFIR_UINT32_FMT "_%" KEFIR_UINT32_FMT
#define KEFIR_AMD64_SYSV_FUNCTION_VARARG_ARG_SCALAR_LABEL "__%s_vararg_%" KEFIR_INT_FMT "_type"

#define KEFIR_AMD64_THREAD_LOCAL "%s@tpoff"
#define KEFIR_AMD64_THREAD_LOCAL_GOT "%s@gottpoff"
#define KEFIR_AMD64_EMUTLS_V "__emutls_v.%s"
#define KEFIR_AMD64_EMUTLS_T "__emutls_t.%s"
#define KEFIR_AMD64_EMUTLS_GOT "__emutls_v.%s@GOTPCREL"
#define KEFIR_AMD64_EMUTLS_GET_ADDR "__emutls_get_address@PLT"

#endif
