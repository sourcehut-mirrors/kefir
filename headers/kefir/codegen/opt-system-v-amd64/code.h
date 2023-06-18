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

#ifndef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_CODE_H_
#define KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_CODE_H_

#include "kefir/codegen/opt-system-v-amd64/function.h"

kefir_result_t kefir_codegen_opt_sysv_amd64_translate_code(struct kefir_mem *, struct kefir_codegen_opt_amd64 *,
                                                           struct kefir_opt_module *, const struct kefir_opt_function *,
                                                           const struct kefir_opt_code_analysis *,
                                                           struct kefir_opt_sysv_amd64_function *);

#ifdef KEFIR_CODEGEN_OPT_SYSTEM_V_AMD64_CODE_INTERNAL
typedef struct kefir_codegen_opt_sysv_amd64_translate_temporary_register {
    kefir_bool_t borrow;
    kefir_bool_t evicted;
    kefir_asm_amd64_xasmgen_register_t reg;
} kefir_codegen_opt_sysv_amd64_translate_temporary_register_t;

kefir_result_t kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain(
    struct kefir_mem *, struct kefir_codegen_opt_amd64 *,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *, struct kefir_opt_sysv_amd64_function *,
    struct kefir_codegen_opt_sysv_amd64_translate_temporary_register *,
    kefir_result_t (*)(kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *, void *), void *);

kefir_result_t kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain_specific(
    struct kefir_mem *, struct kefir_codegen_opt_amd64 *,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *, kefir_asm_amd64_xasmgen_register_t,
    struct kefir_opt_sysv_amd64_function *, struct kefir_codegen_opt_sysv_amd64_translate_temporary_register *);

kefir_result_t kefir_codegen_opt_sysv_amd64_temporary_register_free(
    struct kefir_mem *, struct kefir_codegen_opt_amd64 *, struct kefir_opt_sysv_amd64_function *,
    const struct kefir_codegen_opt_sysv_amd64_translate_temporary_register *);

const struct kefir_asm_amd64_xasmgen_operand *kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
    struct kefir_asm_amd64_xasmgen_operand *, struct kefir_codegen_opt_sysv_amd64_stack_frame_map *,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *);

kefir_result_t kefir_codegen_opt_sysv_amd64_load_reg_allocation(
    struct kefir_codegen_opt_amd64 *, struct kefir_codegen_opt_sysv_amd64_stack_frame_map *,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *, kefir_asm_amd64_xasmgen_register_t);

kefir_result_t kefir_codegen_opt_sysv_amd64_store_reg_allocation(
    struct kefir_codegen_opt_amd64 *, struct kefir_codegen_opt_sysv_amd64_stack_frame_map *,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *, kefir_asm_amd64_xasmgen_register_t);

#define DECLARE_TRANSLATOR(_id)                                                          \
    kefir_result_t kefir_codegen_opt_sysv_amd64_translate_##_id(                         \
        struct kefir_mem *, struct kefir_codegen_opt_amd64 *, struct kefir_opt_module *, \
        const struct kefir_opt_function *, const struct kefir_opt_code_analysis *,       \
        struct kefir_opt_sysv_amd64_function *, kefir_opt_instruction_ref_t)

DECLARE_TRANSLATOR(constant);
DECLARE_TRANSLATOR(get_argument);
DECLARE_TRANSLATOR(data_access);
DECLARE_TRANSLATOR(load);
DECLARE_TRANSLATOR(store);
DECLARE_TRANSLATOR(binary_op);
DECLARE_TRANSLATOR(div_mod);
DECLARE_TRANSLATOR(bitshift);
DECLARE_TRANSLATOR(unary_op);
DECLARE_TRANSLATOR(comparison);
DECLARE_TRANSLATOR(int_conv);
DECLARE_TRANSLATOR(return);
DECLARE_TRANSLATOR(jump);
DECLARE_TRANSLATOR(memory);
DECLARE_TRANSLATOR(invoke);
#undef DECLARE

#endif

#endif