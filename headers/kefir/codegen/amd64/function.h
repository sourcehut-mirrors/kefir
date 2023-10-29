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

#ifndef KEFIR_CODEGEN_AMD64_FUNCTION_H_
#define KEFIR_CODEGEN_AMD64_FUNCTION_H_

#include "kefir/codegen/amd64/codegen.h"
#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/codegen/amd64/register_allocator.h"
#include "kefir/codegen/amd64/stack_frame.h"
#include "kefir/target/abi/amd64/function.h"
#include "kefir/optimizer/module.h"

typedef struct kefir_codegen_amd64_function {
    struct kefir_codegen_amd64 *codegen;
    const struct kefir_opt_module *module;
    const struct kefir_opt_function *function;
    const struct kefir_opt_code_analysis *function_analysis;
    struct kefir_abi_amd64_function_decl abi_function_declaration;
    struct kefir_asmcmp_amd64 code;
    struct kefir_codegen_amd64_register_allocator register_allocator;
    struct kefir_abi_amd64_type_layout locals_layout;
    struct kefir_codegen_amd64_stack_frame stack_frame;

    struct kefir_hashtree instructions;
    struct kefir_hashtree labels;
    struct kefir_hashtree virtual_registers;
    kefir_asmcmp_instruction_index_t argument_touch_instr;
} kefir_codegen_amd64_function_t;

kefir_result_t kefir_codegen_amd64_function_translate(struct kefir_mem *, struct kefir_codegen_amd64 *,
                                                      const struct kefir_opt_module *,
                                                      const struct kefir_opt_function *,
                                                      const struct kefir_opt_code_analysis *);

#ifdef KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
kefir_result_t kefir_codegen_amd64_function_assign_vreg(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                        kefir_opt_instruction_ref_t,
                                                        kefir_asmcmp_virtual_register_index_t);
kefir_result_t kefir_codegen_amd64_function_vreg_of(struct kefir_codegen_amd64_function *, kefir_opt_instruction_ref_t,
                                                    kefir_asmcmp_virtual_register_index_t *);

// clang-format off
#define KEFIR_CODEGEN_AMD64_INSTRUCTIONS(_def, _separator)                                               \
    _def(get_argument, KEFIR_OPT_OPCODE_GET_ARGUMENT) _separator \
    _def(return, KEFIR_OPT_OPCODE_RETURN) _separator \
    _def(int_const, KEFIR_OPT_OPCODE_INT_CONST) _separator \
    _def(uint_const, KEFIR_OPT_OPCODE_UINT_CONST) _separator \
    _def(get_local, KEFIR_OPT_OPCODE_GET_LOCAL) _separator \
    _def(int8_store, KEFIR_OPT_OPCODE_INT8_STORE) _separator \
    _def(int16_store, KEFIR_OPT_OPCODE_INT16_STORE) _separator \
    _def(int32_store, KEFIR_OPT_OPCODE_INT32_STORE) _separator \
    _def(int64_store, KEFIR_OPT_OPCODE_INT64_STORE) _separator \
    _def(int8_load_signed, KEFIR_OPT_OPCODE_INT8_LOAD_SIGNED) _separator \
    _def(int8_load_unsigned, KEFIR_OPT_OPCODE_INT8_LOAD_UNSIGNED) _separator \
    _def(int16_load_signed, KEFIR_OPT_OPCODE_INT16_LOAD_SIGNED) _separator \
    _def(int16_load_unsigned, KEFIR_OPT_OPCODE_INT16_LOAD_UNSIGNED) _separator \
    _def(int32_load_signed, KEFIR_OPT_OPCODE_INT32_LOAD_SIGNED) _separator \
    _def(int32_load_unsigned, KEFIR_OPT_OPCODE_INT32_LOAD_UNSIGNED) _separator \
    _def(int64_load, KEFIR_OPT_OPCODE_INT64_LOAD) _separator \
    _def(int_add, KEFIR_OPT_OPCODE_INT_ADD) _separator                                                   \
    _def(int_sub, KEFIR_OPT_OPCODE_INT_SUB) _separator                                                   \
    _def(int_mul, KEFIR_OPT_OPCODE_INT_MUL) _separator                                                   \
    _def(int_and, KEFIR_OPT_OPCODE_INT_AND) _separator                                                   \
    _def(int_or, KEFIR_OPT_OPCODE_INT_OR) _separator                                                     \
    _def(int_xor, KEFIR_OPT_OPCODE_INT_XOR) _separator                                                   \
    _def(int_zero_extend8, KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS) _separator                          \
    _def(int_zero_extend16, KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS) _separator                        \
    _def(int_zero_extend32, KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) _separator                        \
    _def(int_sign_extend8, KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS) _separator                          \
    _def(int_sign_extend16, KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS) _separator                        \
    _def(int_sign_extend32, KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS)
// clang-format on

#define KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(_id) kefir_codegen_amd64_translate_##_id

#define DECL_INSTR(_id, _opcode)                              \
    kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(_id)( \
        struct kefir_mem *, struct kefir_codegen_amd64_function *, const struct kefir_opt_instruction *)
KEFIR_CODEGEN_AMD64_INSTRUCTIONS(DECL_INSTR, ;);
#undef DECL_INSTR
#endif

#endif
