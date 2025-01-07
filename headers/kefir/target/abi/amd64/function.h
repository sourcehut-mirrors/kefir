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

#ifndef KEFIR_TARGET_ABI_AMD64_FUNCTION_H_
#define KEFIR_TARGET_ABI_AMD64_FUNCTION_H_

#include "kefir/ir/function.h"
#include "kefir/target/abi/amd64/parameters.h"

kefir_result_t kefir_abi_amd64_get_callee_preserved_general_purpose_register(kefir_abi_amd64_variant_t, kefir_size_t,
                                                                             kefir_asm_amd64_xasmgen_register_t *);
kefir_size_t kefir_abi_amd64_num_of_callee_preserved_general_purpose_registers(kefir_abi_amd64_variant_t);

kefir_result_t kefir_abi_amd64_get_caller_preserved_general_purpose_register(kefir_abi_amd64_variant_t, kefir_size_t,
                                                                             kefir_asm_amd64_xasmgen_register_t *);
kefir_size_t kefir_abi_amd64_num_of_caller_preserved_general_purpose_registers(kefir_abi_amd64_variant_t);

kefir_result_t kefir_abi_amd64_get_caller_preserved_sse_register(kefir_abi_amd64_variant_t, kefir_size_t,
                                                                 kefir_asm_amd64_xasmgen_register_t *);
kefir_size_t kefir_abi_amd64_num_of_caller_preserved_sse_registers(kefir_abi_amd64_variant_t);

typedef struct kefir_abi_amd64_function_decl {
    void *payload;
} kefir_abi_amd64_function_decl_t;

kefir_result_t kefir_abi_amd64_function_decl_alloc(struct kefir_mem *, kefir_abi_amd64_variant_t,
                                                   const struct kefir_ir_function_decl *,
                                                   struct kefir_abi_amd64_function_decl *);
kefir_result_t kefir_abi_amd64_function_decl_free(struct kefir_mem *, struct kefir_abi_amd64_function_decl *);

kefir_result_t kefir_abi_amd64_function_decl_ir(const struct kefir_abi_amd64_function_decl *,
                                                const struct kefir_ir_function_decl **);
kefir_result_t kefir_abi_amd64_function_decl_parameters(const struct kefir_abi_amd64_function_decl *,
                                                        const struct kefir_abi_amd64_function_parameters **);
kefir_result_t kefir_abi_amd64_function_decl_parameters_layout(const struct kefir_abi_amd64_function_decl *,
                                                               const struct kefir_abi_amd64_type_layout **);
kefir_result_t kefir_abi_amd64_function_decl_parameters_sse_reqs(const struct kefir_abi_amd64_function_decl *,
                                                                 kefir_bool_t *, kefir_asm_amd64_xasmgen_register_t *);
kefir_result_t kefir_abi_amd64_function_decl_returns(const struct kefir_abi_amd64_function_decl *,
                                                     const struct kefir_abi_amd64_function_parameters **);
kefir_result_t kefir_abi_amd64_function_decl_returns_layout(const struct kefir_abi_amd64_function_decl *,
                                                            const struct kefir_abi_amd64_type_layout **);
kefir_result_t kefir_abi_amd64_function_decl_returns_implicit_parameter(const struct kefir_abi_amd64_function_decl *,
                                                                        kefir_bool_t *,
                                                                        kefir_asm_amd64_xasmgen_register_t *);

#endif
