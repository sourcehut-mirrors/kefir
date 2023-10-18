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

#ifndef KEFIR_CODEGEN_SYSTEM_V_AMD64_BUILTINS_H_
#define KEFIR_CODEGEN_SYSTEM_V_AMD64_BUILTINS_H_

#include "kefir/ir/builtins.h"
#include "kefir/codegen/system-v-amd64/abi.h"
#include "kefir/codegen/system-v-amd64.h"

typedef struct kefir_codegen_amd64_sysv_builtin_type {
    kefir_result_t (*load_function_argument)(const struct kefir_codegen_amd64_sysv_builtin_type *,
                                             const struct kefir_ir_typeentry *, struct kefir_codegen_amd64 *,
                                             const struct kefir_abi_amd64_function_parameter *);
    kefir_result_t (*store_function_return)(const struct kefir_codegen_amd64_sysv_builtin_type *,
                                            const struct kefir_ir_typeentry *, struct kefir_codegen_amd64 *,
                                            const struct kefir_abi_amd64_function_parameter *);
    kefir_result_t (*store_function_argument)(const struct kefir_codegen_amd64_sysv_builtin_type *,
                                              const struct kefir_ir_typeentry *, struct kefir_codegen_amd64 *,
                                              const struct kefir_abi_amd64_function_parameter *, kefir_size_t);
    kefir_result_t (*load_function_return)(const struct kefir_codegen_amd64_sysv_builtin_type *,
                                           const struct kefir_ir_typeentry *, struct kefir_codegen_amd64 *,
                                           const struct kefir_abi_amd64_function_parameter *);
    kefir_result_t (*load_vararg)(struct kefir_mem *, const struct kefir_codegen_amd64_sysv_builtin_type *,
                                  const struct kefir_ir_typeentry *, struct kefir_codegen_amd64 *,
                                  struct kefir_amd64_sysv_function *, const char *,
                                  const struct kefir_abi_amd64_function_parameter *);
} kefir_codegen_amd64_sysv_builtin_type_t;

extern const struct kefir_codegen_amd64_sysv_builtin_type KEFIR_CODEGEN_SYSTEM_V_AMD64_SYSV_BUILIN_VARARG_TYPE;
extern const struct kefir_codegen_amd64_sysv_builtin_type *KEFIR_CODEGEN_SYSTEM_V_AMD64_SYSV_BUILTIN_TYPES[];

#endif
