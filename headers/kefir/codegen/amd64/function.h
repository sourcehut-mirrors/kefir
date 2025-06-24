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

#ifndef KEFIR_CODEGEN_AMD64_FUNCTION_H_
#define KEFIR_CODEGEN_AMD64_FUNCTION_H_

#include "kefir/codegen/amd64/codegen.h"
#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/codegen/amd64/xregalloc.h"
#include "kefir/codegen/amd64/stack_frame.h"
#include "kefir/codegen/variable_allocator.h"
#include "kefir/optimizer/schedule.h"
#include "kefir/target/abi/amd64/function.h"
#include "kefir/optimizer/module.h"

typedef struct kefir_codegen_amd64_module kefir_codegen_amd64_module_t;

typedef struct kefir_codegen_amd64_function {
    struct kefir_codegen_amd64 *codegen;
    struct kefir_codegen_amd64_module *codegen_module;
    const struct kefir_opt_module *module;
    const struct kefir_opt_function *function;
    struct kefir_opt_code_analysis function_analysis;
    struct kefir_abi_amd64_function_decl abi_function_declaration;
    struct kefir_asmcmp_amd64 code;
    struct kefir_codegen_amd64_xregalloc xregalloc;
    struct kefir_codegen_amd64_stack_frame stack_frame;
    struct kefir_codegen_local_variable_allocator variable_allocator;
    struct kefir_opt_code_schedule schedule;

    struct kefir_hashtreeset translated_instructions;
    struct kefir_hashtree labels;
    struct kefir_hashtree virtual_registers;
    struct kefir_hashtree constants;
    struct kefir_hashtree type_layouts;
    struct kefir_hashtreeset preserve_vregs;

    struct kefir_list x87_stack;

    kefir_asmcmp_instruction_index_t argument_touch_instr;
    kefir_asmcmp_instruction_index_t prologue_tail;
    kefir_asmcmp_virtual_register_index_t return_address_vreg;
    kefir_asmcmp_virtual_register_index_t dynamic_scope_vreg;
    kefir_asmcmp_virtual_register_index_t vararg_area;

    struct {
        struct kefir_hashtree opt_instruction_location_labels;
        struct kefir_hashtree ir_instructions;
        struct kefir_hashtree function_parameters;
        struct kefir_hashtree occupied_x87_stack_slots;
    } debug;
} kefir_codegen_amd64_function_t;

kefir_result_t kefir_codegen_amd64_function_init(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                 struct kefir_codegen_amd64_module *, const struct kefir_opt_module *,
                                                 const struct kefir_opt_function *);
kefir_result_t kefir_codegen_amd64_function_free(struct kefir_mem *, struct kefir_codegen_amd64_function *);
kefir_result_t kefir_codegen_amd64_function_translate(struct kefir_mem *, struct kefir_codegen_amd64_function *);

#ifdef KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
kefir_result_t kefir_codegen_amd64_function_assign_vreg(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                        kefir_opt_instruction_ref_t,
                                                        kefir_asmcmp_virtual_register_index_t);
kefir_result_t kefir_codegen_amd64_function_vreg_of(struct kefir_codegen_amd64_function *, kefir_opt_instruction_ref_t,
                                                    kefir_asmcmp_virtual_register_index_t *);
kefir_result_t kefir_codegen_amd64_function_map_phi_outputs(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                            kefir_opt_block_id_t, kefir_opt_block_id_t);
kefir_result_t kefir_codegen_amd64_function_generate_debug_instruction_locations(struct kefir_mem *,
                                                                                 struct kefir_codegen_amd64_function *,
                                                                                 kefir_opt_instruction_ref_t);

kefir_result_t kefir_codegen_amd64_function_find_instruction_lifetime(const struct kefir_codegen_amd64_function *,
                                                                      kefir_opt_instruction_ref_t,
                                                                      kefir_asmcmp_label_index_t *,
                                                                      kefir_asmcmp_label_index_t *);
kefir_result_t kefir_codegen_amd64_function_find_code_range_labels(const struct kefir_codegen_amd64_function *,
                                                                   kefir_size_t, kefir_size_t,
                                                                   kefir_asmcmp_label_index_t *,
                                                                   kefir_asmcmp_label_index_t *);
kefir_result_t kefir_codegen_amd64_function_find_instruction_linear_index_label(
    const struct kefir_codegen_amd64_function *, kefir_size_t, kefir_asmcmp_label_index_t *);

kefir_result_t kefir_codegen_amd64_return_from_function(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                        kefir_opt_instruction_ref_t,
                                                        kefir_asmcmp_virtual_register_index_t);

// clang-format off
#define KEFIR_CODEGEN_AMD64_INSTRUCTIONS(_def, _separator)                                               \
    _def(get_argument, KEFIR_OPT_OPCODE_GET_ARGUMENT) _separator \
    _def(return, KEFIR_OPT_OPCODE_RETURN) _separator \
    _def(select, KEFIR_OPT_OPCODE_SELECT) _separator \
    _def(select_compare, KEFIR_OPT_OPCODE_SELECT_COMPARE) _separator \
    _def(int_const, KEFIR_OPT_OPCODE_INT_CONST) _separator \
    _def(uint_const, KEFIR_OPT_OPCODE_UINT_CONST) _separator \
    _def(float32_const, KEFIR_OPT_OPCODE_FLOAT32_CONST) _separator \
    _def(float64_const, KEFIR_OPT_OPCODE_FLOAT64_CONST) _separator \
    _def(long_double_const, KEFIR_OPT_OPCODE_LONG_DOUBLE_CONST) _separator \
    _def(string_ref, KEFIR_OPT_OPCODE_STRING_REF) _separator \
    _def(block_label, KEFIR_OPT_OPCODE_BLOCK_LABEL) _separator \
    _def(int_placeholder, KEFIR_OPT_OPCODE_INT_PLACEHOLDER) _separator \
    _def(float_placeholder, KEFIR_OPT_OPCODE_FLOAT32_PLACEHOLDER) _separator \
    _def(float_placeholder, KEFIR_OPT_OPCODE_FLOAT64_PLACEHOLDER) _separator \
    _def(temporary_object, KEFIR_OPT_OPCODE_TEMPORARY_OBJECT) _separator \
    _def(pair, KEFIR_OPT_OPCODE_PAIR) _separator \
    _def(local_lifetime_mark, KEFIR_OPT_OPCODE_LOCAL_LIFETIME_MARK) _separator \
    _def(alloc_local, KEFIR_OPT_OPCODE_ALLOC_LOCAL) _separator \
    _def(ref_local, KEFIR_OPT_OPCODE_REF_LOCAL) _separator \
    _def(get_global, KEFIR_OPT_OPCODE_GET_GLOBAL) _separator \
    _def(thread_local, KEFIR_OPT_OPCODE_GET_THREAD_LOCAL) _separator \
    _def(phi, KEFIR_OPT_OPCODE_PHI) _separator \
    _def(jump, KEFIR_OPT_OPCODE_JUMP) _separator \
    _def(branch, KEFIR_OPT_OPCODE_BRANCH) _separator \
    _def(branch_compare, KEFIR_OPT_OPCODE_BRANCH_COMPARE) _separator \
    _def(ijump, KEFIR_OPT_OPCODE_IJUMP) _separator \
    _def(int8_store, KEFIR_OPT_OPCODE_INT8_STORE) _separator \
    _def(int16_store, KEFIR_OPT_OPCODE_INT16_STORE) _separator \
    _def(int32_store, KEFIR_OPT_OPCODE_INT32_STORE) _separator \
    _def(int64_store, KEFIR_OPT_OPCODE_INT64_STORE) _separator \
    _def(long_double_store, KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE) _separator \
    _def(int8_load, KEFIR_OPT_OPCODE_INT8_LOAD) _separator \
    _def(int16_load, KEFIR_OPT_OPCODE_INT16_LOAD) _separator \
    _def(int32_load, KEFIR_OPT_OPCODE_INT32_LOAD) _separator \
    _def(int64_load, KEFIR_OPT_OPCODE_INT64_LOAD) _separator \
    _def(long_double_load, KEFIR_OPT_OPCODE_LONG_DOUBLE_LOAD) _separator \
    _def(int_to_float32, KEFIR_OPT_OPCODE_INT_TO_FLOAT32) _separator \
    _def(int_to_float64, KEFIR_OPT_OPCODE_INT_TO_FLOAT64) _separator \
    _def(uint_to_float, KEFIR_OPT_OPCODE_UINT_TO_FLOAT32) _separator \
    _def(uint_to_float, KEFIR_OPT_OPCODE_UINT_TO_FLOAT64) _separator \
    _def(float32_to_int, KEFIR_OPT_OPCODE_FLOAT32_TO_INT) _separator \
    _def(float64_to_int, KEFIR_OPT_OPCODE_FLOAT64_TO_INT) _separator \
    _def(float_to_uint, KEFIR_OPT_OPCODE_FLOAT32_TO_UINT) _separator \
    _def(float_to_uint, KEFIR_OPT_OPCODE_FLOAT64_TO_UINT) _separator \
    _def(float_to_float, KEFIR_OPT_OPCODE_FLOAT32_TO_FLOAT64) _separator \
    _def(float_to_float, KEFIR_OPT_OPCODE_FLOAT64_TO_FLOAT32) _separator \
    _def(float32_arith_op, KEFIR_OPT_OPCODE_FLOAT32_ADD) _separator \
    _def(float32_arith_op, KEFIR_OPT_OPCODE_FLOAT32_SUB) _separator \
    _def(float32_arith_op, KEFIR_OPT_OPCODE_FLOAT32_MUL) _separator \
    _def(float32_arith_op, KEFIR_OPT_OPCODE_FLOAT32_DIV) _separator \
    _def(float64_arith_op, KEFIR_OPT_OPCODE_FLOAT64_ADD) _separator \
    _def(float64_arith_op, KEFIR_OPT_OPCODE_FLOAT64_SUB) _separator \
    _def(float64_arith_op, KEFIR_OPT_OPCODE_FLOAT64_MUL) _separator \
    _def(float64_arith_op, KEFIR_OPT_OPCODE_FLOAT64_DIV) _separator \
    _def(float_unary_op, KEFIR_OPT_OPCODE_FLOAT32_NEG) _separator \
    _def(float_unary_op, KEFIR_OPT_OPCODE_FLOAT64_NEG) _separator \
    _def(long_double_binary_op, KEFIR_OPT_OPCODE_LONG_DOUBLE_ADD) _separator \
    _def(long_double_binary_op, KEFIR_OPT_OPCODE_LONG_DOUBLE_SUB) _separator \
    _def(long_double_binary_op, KEFIR_OPT_OPCODE_LONG_DOUBLE_MUL) _separator \
    _def(long_double_binary_op, KEFIR_OPT_OPCODE_LONG_DOUBLE_DIV) _separator \
    _def(long_double_neg, KEFIR_OPT_OPCODE_LONG_DOUBLE_NEG) _separator \
    _def(long_double_equals, KEFIR_OPT_OPCODE_LONG_DOUBLE_EQUALS) _separator \
    _def(long_double_greater, KEFIR_OPT_OPCODE_LONG_DOUBLE_GREATER) _separator \
    _def(long_double_less, KEFIR_OPT_OPCODE_LONG_DOUBLE_LESSER) _separator \
    _def(int_to_long_double, KEFIR_OPT_OPCODE_INT_TO_LONG_DOUBLE) _separator \
    _def(uint_to_long_double, KEFIR_OPT_OPCODE_UINT_TO_LONG_DOUBLE) _separator \
    _def(float32_to_long_double, KEFIR_OPT_OPCODE_FLOAT32_TO_LONG_DOUBLE) _separator \
    _def(float64_to_long_double, KEFIR_OPT_OPCODE_FLOAT64_TO_LONG_DOUBLE) _separator \
    _def(long_double_to_int, KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_INT) _separator \
    _def(long_double_to_uint, KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_UINT) _separator \
    _def(long_double_to_float32, KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_FLOAT32) _separator \
    _def(long_double_to_float64, KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_FLOAT64) _separator \
    _def(bits_extract_signed, KEFIR_OPT_OPCODE_BITS_EXTRACT_SIGNED) _separator \
    _def(bits_extract_unsigned, KEFIR_OPT_OPCODE_BITS_EXTRACT_UNSIGNED) _separator \
    _def(bits_insert, KEFIR_OPT_OPCODE_BITS_INSERT) _separator \
    _def(copy_memory, KEFIR_OPT_OPCODE_COPY_MEMORY) _separator \
    _def(zero_memory, KEFIR_OPT_OPCODE_ZERO_MEMORY) _separator \
    _def(stack_alloc, KEFIR_OPT_OPCODE_STACK_ALLOC) _separator \
    _def(push_scope, KEFIR_OPT_OPCODE_SCOPE_PUSH) _separator \
    _def(pop_scope, KEFIR_OPT_OPCODE_SCOPE_POP) _separator \
    _def(invoke, KEFIR_OPT_OPCODE_INVOKE) _separator \
    _def(invoke, KEFIR_OPT_OPCODE_INVOKE_VIRTUAL) _separator \
    _def(tail_invoke, KEFIR_OPT_OPCODE_TAIL_INVOKE) _separator \
    _def(tail_invoke, KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL) _separator \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT8_ADD) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT16_ADD) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT32_ADD) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT64_ADD) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT8_SUB) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT16_SUB) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT32_SUB) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT64_SUB) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT8_MUL) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT16_MUL) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT32_MUL) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT64_MUL) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_UINT8_MUL) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_UINT16_MUL) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_UINT32_MUL) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_UINT64_MUL) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT8_AND) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT16_AND) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT32_AND) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT64_AND) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT8_OR) _separator                                                     \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT16_OR) _separator                                                     \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT32_OR) _separator                                                     \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT64_OR) _separator                                                     \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT8_XOR) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT16_XOR) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT32_XOR) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT64_XOR) _separator                                                   \
    _def(int_div, KEFIR_OPT_OPCODE_INT8_DIV) _separator                                                   \
    _def(int_div, KEFIR_OPT_OPCODE_INT16_DIV) _separator                                                   \
    _def(int_div, KEFIR_OPT_OPCODE_INT32_DIV) _separator                                                   \
    _def(int_div, KEFIR_OPT_OPCODE_INT64_DIV) _separator                                                   \
    _def(int_mod, KEFIR_OPT_OPCODE_INT8_MOD) _separator                                                   \
    _def(int_mod, KEFIR_OPT_OPCODE_INT16_MOD) _separator                                                   \
    _def(int_mod, KEFIR_OPT_OPCODE_INT32_MOD) _separator                                                   \
    _def(int_mod, KEFIR_OPT_OPCODE_INT64_MOD) _separator                                                   \
    _def(uint_div, KEFIR_OPT_OPCODE_UINT8_DIV) _separator                                                   \
    _def(uint_div, KEFIR_OPT_OPCODE_UINT16_DIV) _separator                                                   \
    _def(uint_div, KEFIR_OPT_OPCODE_UINT32_DIV) _separator                                                   \
    _def(uint_div, KEFIR_OPT_OPCODE_UINT64_DIV) _separator                                                   \
    _def(uint_mod, KEFIR_OPT_OPCODE_UINT8_MOD) _separator                                                   \
    _def(uint_mod, KEFIR_OPT_OPCODE_UINT16_MOD) _separator                                                   \
    _def(uint_mod, KEFIR_OPT_OPCODE_UINT32_MOD) _separator                                                   \
    _def(uint_mod, KEFIR_OPT_OPCODE_UINT64_MOD) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT8_LSHIFT) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT16_LSHIFT) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT32_LSHIFT) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT64_LSHIFT) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT8_RSHIFT) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT16_RSHIFT) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT32_RSHIFT) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT64_RSHIFT) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT8_ARSHIFT) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT16_ARSHIFT) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT32_ARSHIFT) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT64_ARSHIFT) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT8_NOT) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT16_NOT) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT32_NOT) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT64_NOT) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT8_NEG) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT16_NEG) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT32_NEG) _separator                                                   \
    _def(int_arithmetics, KEFIR_OPT_OPCODE_INT64_NEG) _separator                                                   \
    _def(int_comparison, KEFIR_OPT_OPCODE_SCALAR_COMPARE) _separator                                                   \
    _def(int_bool_not, KEFIR_OPT_OPCODE_INT8_BOOL_NOT) _separator                                                   \
    _def(int_bool_not, KEFIR_OPT_OPCODE_INT16_BOOL_NOT) _separator                                                   \
    _def(int_bool_not, KEFIR_OPT_OPCODE_INT32_BOOL_NOT) _separator                                                   \
    _def(int_bool_not, KEFIR_OPT_OPCODE_INT64_BOOL_NOT) _separator                                                   \
    _def(int_bool_or, KEFIR_OPT_OPCODE_INT8_BOOL_OR) _separator                                                   \
    _def(int_bool_or, KEFIR_OPT_OPCODE_INT16_BOOL_OR) _separator                                                   \
    _def(int_bool_or, KEFIR_OPT_OPCODE_INT32_BOOL_OR) _separator                                                   \
    _def(int_bool_or, KEFIR_OPT_OPCODE_INT64_BOOL_OR) _separator                                                   \
    _def(int_bool_and, KEFIR_OPT_OPCODE_INT8_BOOL_AND) _separator                                                   \
    _def(int_bool_and, KEFIR_OPT_OPCODE_INT16_BOOL_AND) _separator                                                   \
    _def(int_bool_and, KEFIR_OPT_OPCODE_INT32_BOOL_AND) _separator                                                   \
    _def(int_bool_and, KEFIR_OPT_OPCODE_INT64_BOOL_AND) _separator                                                   \
    _def(int_to_bool, KEFIR_OPT_OPCODE_INT8_TO_BOOL) _separator                          \
    _def(int_to_bool, KEFIR_OPT_OPCODE_INT16_TO_BOOL) _separator                          \
    _def(int_to_bool, KEFIR_OPT_OPCODE_INT32_TO_BOOL) _separator                          \
    _def(int_to_bool, KEFIR_OPT_OPCODE_INT64_TO_BOOL) _separator                          \
    _def(int_zero_extend8, KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS) _separator                          \
    _def(int_zero_extend16, KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS) _separator                        \
    _def(int_zero_extend32, KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) _separator                        \
    _def(int_sign_extend8, KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS) _separator                          \
    _def(int_sign_extend16, KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS) _separator                        \
    _def(int_sign_extend32, KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS) _separator \
    _def(vararg_start, KEFIR_OPT_OPCODE_VARARG_START) _separator \
    _def(vararg_end, KEFIR_OPT_OPCODE_VARARG_END) _separator \
    _def(vararg_copy, KEFIR_OPT_OPCODE_VARARG_COPY) _separator \
    _def(vararg_get, KEFIR_OPT_OPCODE_VARARG_GET) _separator \
    _def(fenv_save, KEFIR_OPT_OPCODE_FENV_SAVE) _separator \
    _def(fenv_clear, KEFIR_OPT_OPCODE_FENV_CLEAR) _separator \
    _def(fenv_update, KEFIR_OPT_OPCODE_FENV_UPDATE) _separator \
    _def(complex_float32_from, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_FROM) _separator \
    _def(complex_float32_real, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_REAL) _separator \
    _def(complex_float32_imaginary, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_IMAGINARY) _separator \
    _def(complex_float64_from, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_FROM) _separator \
    _def(complex_float64_real, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_REAL) _separator \
    _def(complex_float64_imaginary, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_IMAGINARY) _separator \
    _def(complex_long_double_from, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_FROM) _separator \
    _def(complex_long_double_real, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_REAL) _separator \
    _def(complex_long_double_imaginary, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_IMAGINARY) _separator \
    _def(complex_float32_equals, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_EQUALS) _separator \
    _def(complex_float64_equals, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_EQUALS) _separator \
    _def(complex_long_double_equals, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_EQUALS) _separator \
    _def(complex_float32_truncate_1bit, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_TRUNCATE_1BIT) _separator \
    _def(complex_float64_truncate_1bit, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_TRUNCATE_1BIT) _separator \
    _def(complex_long_double_truncate_1bit, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_TRUNCATE_1BIT) _separator \
    _def(complex_float32_add_sub, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_ADD) _separator \
    _def(complex_float64_add_sub, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_ADD) _separator \
    _def(complex_long_double_add_sub, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_ADD) _separator \
    _def(complex_float32_add_sub, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_SUB) _separator \
    _def(complex_float64_add_sub, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_SUB) _separator \
    _def(complex_long_double_add_sub, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_SUB) _separator \
    _def(complex_float32_mul, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_MUL) _separator \
    _def(complex_float32_div, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_DIV) _separator \
    _def(complex_float64_mul, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_MUL) _separator \
    _def(complex_float64_div, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_DIV) _separator \
    _def(complex_long_double_mul, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_MUL) _separator \
    _def(complex_long_double_div, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_DIV) _separator \
    _def(complex_float32_neg, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_NEG) _separator \
    _def(complex_float64_neg, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_NEG) _separator \
    _def(complex_long_double_neg, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_NEG) _separator \
    _def(complex_float32_load, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_LOAD) _separator \
    _def(complex_float64_load, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_LOAD) _separator \
    _def(complex_long_double_load, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_LOAD) _separator \
    _def(complex_float32_store, KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE) _separator \
    _def(complex_float64_store, KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE) _separator \
    _def(complex_long_double_store, KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE) _separator \
    _def(atomic_load, KEFIR_OPT_OPCODE_ATOMIC_LOAD8) _separator \
    _def(atomic_load, KEFIR_OPT_OPCODE_ATOMIC_LOAD16) _separator \
    _def(atomic_load, KEFIR_OPT_OPCODE_ATOMIC_LOAD32) _separator \
    _def(atomic_load, KEFIR_OPT_OPCODE_ATOMIC_LOAD64) _separator \
    _def(atomic_load_long_double, KEFIR_OPT_OPCODE_ATOMIC_LOAD_LONG_DOUBLE) _separator \
    _def(atomic_load_complex, KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_FLOAT32) _separator \
    _def(atomic_load_complex, KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_FLOAT64) _separator \
    _def(atomic_load_complex, KEFIR_OPT_OPCODE_ATOMIC_LOAD_COMPLEX_LONG_DOUBLE) _separator \
    _def(atomic_store, KEFIR_OPT_OPCODE_ATOMIC_STORE8) _separator \
    _def(atomic_store, KEFIR_OPT_OPCODE_ATOMIC_STORE16) _separator \
    _def(atomic_store, KEFIR_OPT_OPCODE_ATOMIC_STORE32) _separator \
    _def(atomic_store, KEFIR_OPT_OPCODE_ATOMIC_STORE64) _separator \
    _def(atomic_store_long_double, KEFIR_OPT_OPCODE_ATOMIC_STORE_LONG_DOUBLE) _separator \
    _def(atomic_store_complex_float, KEFIR_OPT_OPCODE_ATOMIC_STORE_COMPLEX_FLOAT32) _separator \
    _def(atomic_store_complex_float, KEFIR_OPT_OPCODE_ATOMIC_STORE_COMPLEX_FLOAT64) _separator \
    _def(atomic_store_complex_long_double, KEFIR_OPT_OPCODE_ATOMIC_STORE_COMPLEX_LONG_DOUBLE) _separator \
    _def(atomic_copy_memory, KEFIR_OPT_OPCODE_ATOMIC_COPY_MEMORY_FROM) _separator \
    _def(atomic_copy_memory, KEFIR_OPT_OPCODE_ATOMIC_COPY_MEMORY_TO) _separator \
    _def(atomic_compare_exchange, KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG8) _separator \
    _def(atomic_compare_exchange, KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG16) _separator \
    _def(atomic_compare_exchange, KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG32) _separator \
    _def(atomic_compare_exchange, KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG64) _separator \
    _def(atomic_compare_exchange_long_double, KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_LONG_DOUBLE) _separator \
    _def(atomic_compare_exchange_complex_float, KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_COMPLEX_FLOAT32) _separator \
    _def(atomic_compare_exchange_complex_float, KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_COMPLEX_FLOAT64) _separator \
    _def(atomic_compare_exchange_complex_long_double, KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_COMPLEX_LONG_DOUBLE) _separator \
    _def(atomic_compare_exchange_memory, KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG_MEMORY) _separator \
    _def(add_overflow, KEFIR_OPT_OPCODE_ADD_OVERFLOW) _separator \
    _def(sub_overflow, KEFIR_OPT_OPCODE_SUB_OVERFLOW) _separator \
    _def(mul_overflow, KEFIR_OPT_OPCODE_MUL_OVERFLOW) _separator \
    _def(bitint_const, KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST) _separator \
    _def(bitint_const, KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_GET_SIGNED) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_GET_UNSIGNED) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_FROM_SIGNED) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_FROM_UNSIGNED) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_CAST_SIGNED) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_CAST_UNSIGNED) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_FLOAT) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_FLOAT) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_DOUBLE) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_DOUBLE) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_LONG_DOUBLE) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_LONG_DOUBLE) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_SIGNED) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_UNSIGNED) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_SIGNED) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_UNSIGNED) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_SIGNED) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_UNSIGNED) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_TO_BOOL) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_LOAD) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_STORE) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_ATOMIC_LOAD) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_ATOMIC_STORE) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_ATOMIC_COMPARE_EXCHANGE) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_NEGATE) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_INVERT) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_BOOL_NOT) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_ADD) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_SUB) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_IMUL) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_UMUL) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_IDIV) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_UDIV) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_IMOD) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_UMOD) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_LSHIFT) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_RSHIFT) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_ARSHIFT) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_AND) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_OR) _separator \
    _def(bitint_error_lowered, KEFIR_OPT_OPCODE_BITINT_XOR) _separator \
    _def(inline_assembly, KEFIR_OPT_OPCODE_INLINE_ASSEMBLY)
// clang-format on

#define KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(_id) kefir_codegen_amd64_translate_##_id

#define DECL_INSTR(_id, _opcode)                              \
    kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(_id)( \
        struct kefir_mem *, struct kefir_codegen_amd64_function *, const struct kefir_opt_instruction *)
KEFIR_CODEGEN_AMD64_INSTRUCTIONS(DECL_INSTR, ;);
#undef DECL_INSTR

kefir_result_t kefir_codegen_amd64_copy_memory(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                               kefir_asmcmp_virtual_register_index_t,
                                               kefir_asmcmp_virtual_register_index_t, kefir_size_t);

kefir_result_t kefir_codegen_amd64_translate_builtin(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                     const struct kefir_opt_instruction *, kefir_bool_t *,
                                                     kefir_asmcmp_virtual_register_index_t *);

kefir_result_t kefir_codegen_amd64_load_general_purpose_register(struct kefir_mem *,
                                                                 struct kefir_codegen_amd64_function *,
                                                                 kefir_asmcmp_instruction_index_t,
                                                                 kefir_asmcmp_instruction_index_t, kefir_size_t,
                                                                 kefir_int64_t);
kefir_result_t kefir_codegen_amd64_load_floating_point_register(struct kefir_mem *,
                                                                struct kefir_codegen_amd64_function *,
                                                                kefir_asmcmp_instruction_index_t,
                                                                kefir_asmcmp_instruction_index_t, kefir_size_t,
                                                                kefir_int64_t);
kefir_result_t kefir_codegen_amd64_store_general_purpose_register(struct kefir_mem *,
                                                                  struct kefir_codegen_amd64_function *,
                                                                  kefir_asmcmp_instruction_index_t,
                                                                  kefir_asmcmp_instruction_index_t, kefir_size_t,
                                                                  kefir_int64_t);
kefir_result_t kefir_codegen_amd64_store_floating_point_register(struct kefir_mem *,
                                                                 struct kefir_codegen_amd64_function *,
                                                                 kefir_asmcmp_instruction_index_t,
                                                                 kefir_asmcmp_instruction_index_t, kefir_size_t,
                                                                 kefir_int64_t);

kefir_result_t kefir_codegen_amd64_tail_call_possible(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                      kefir_opt_call_id_t, kefir_bool_t *);

kefir_result_t kefir_codegen_amd64_function_x87_ensure(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                       kefir_size_t);
kefir_result_t kefir_codegen_amd64_function_x87_push(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                     kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_x87_pop(struct kefir_mem *, struct kefir_codegen_amd64_function *);
kefir_result_t kefir_codegen_amd64_function_x87_load(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                     kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_x87_consume_by(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                           kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_x87_flush(struct kefir_mem *, struct kefir_codegen_amd64_function *);

typedef struct kefir_codegen_amd64_function_x87_locations_iterator {
    const struct kefir_hashtree *x87_slots;
    struct kefir_hashtree_node *node;
    kefir_opt_instruction_ref_t instr_ref;
} kefir_codegen_amd64_function_x87_locations_iterator_t;

kefir_result_t kefir_codegen_amd64_function_x87_locations_iter(
    const struct kefir_codegen_amd64_function *, kefir_opt_instruction_ref_t,
    struct kefir_codegen_amd64_function_x87_locations_iterator *, kefir_opt_instruction_ref_t *, kefir_size_t *);
kefir_result_t kefir_codegen_amd64_function_x87_locations_next(
    struct kefir_codegen_amd64_function_x87_locations_iterator *, kefir_opt_instruction_ref_t *, kefir_size_t *);

kefir_result_t kefir_codegen_amd64_function_int_to_float(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                         kefir_asmcmp_instruction_index_t, kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_uint_to_float(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                          kefir_asmcmp_instruction_index_t,
                                                          kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_int_to_double(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                          kefir_asmcmp_instruction_index_t,
                                                          kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_uint_to_double(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                           kefir_asmcmp_instruction_index_t,
                                                           kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_int_to_long_double(struct kefir_mem *,
                                                               struct kefir_codegen_amd64_function *,
                                                               kefir_asmcmp_instruction_index_t,
                                                               kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_uint_to_long_double(struct kefir_mem *,
                                                                struct kefir_codegen_amd64_function *,
                                                                kefir_asmcmp_instruction_index_t,
                                                                kefir_opt_instruction_ref_t);

kefir_result_t kefir_codegen_amd64_function_float_to_int(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                         kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_float_to_uint(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                          kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_double_to_int(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                          kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_double_to_uint(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                           kefir_opt_instruction_ref_t, kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_long_double_to_int(struct kefir_mem *,
                                                               struct kefir_codegen_amd64_function *,
                                                               kefir_opt_instruction_ref_t,
                                                               kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_amd64_function_long_double_to_uint(struct kefir_mem *,
                                                                struct kefir_codegen_amd64_function *,
                                                                kefir_opt_instruction_ref_t,
                                                                kefir_opt_instruction_ref_t);

kefir_result_t kefir_codegen_amd64_get_atomic_memorder(kefir_opt_memory_order_t, kefir_int64_t *);

kefir_result_t kefir_codegen_amd64_function_call_preserve_regs(struct kefir_mem *,
                                                               struct kefir_codegen_amd64_function *,
                                                               const struct kefir_opt_call_node *,
                                                               kefir_asmcmp_stash_index_t *,
                                                               kefir_asmcmp_virtual_register_index_t *);

#define BIGINT_GET_SET_SIGNED_INTEGER_FN "__kefir_bigint_set_signed_integer"
#define BIGINT_GET_SET_UNSIGNED_INTEGER_FN "__kefir_bigint_set_unsigned_integer"
#define BIGINT_CAST_SIGNED_FN "__kefir_bigint_cast_signed"
#define BIGINT_CAST_UNSIGNED_FN "__kefir_bigint_cast_unsigned"
#define BIGINT_SIGNED_TO_FLOAT_FN "__kefir_bigint_signed_to_float"
#define BIGINT_UNSIGNED_TO_FLOAT_FN "__kefir_bigint_unsigned_to_float"
#define BIGINT_SIGNED_TO_DOUBLE_FN "__kefir_bigint_signed_to_double"
#define BIGINT_UNSIGNED_TO_DOUBLE_FN "__kefir_bigint_unsigned_to_double"
#define BIGINT_SIGNED_TO_LONG_DOUBLE_FN "__kefir_bigint_signed_to_long_double"
#define BIGINT_UNSIGNED_TO_LONG_DOUBLE_FN "__kefir_bigint_unsigned_to_long_double"
#define BIGINT_SIGNED_FROM_FLOAT_FN "__kefir_bigint_signed_from_float"
#define BIGINT_SIGNED_FROM_DOUBLE_FN "__kefir_bigint_signed_from_double"
#define BIGINT_SIGNED_FROM_LONG_DOUBLE_FN "__kefir_bigint_signed_from_long_double"
#define BIGINT_UNSIGNED_FROM_FLOAT_FN "__kefir_bigint_unsigned_from_float"
#define BIGINT_UNSIGNED_FROM_DOUBLE_FN "__kefir_bigint_unsigned_from_double"
#define BIGINT_UNSIGNED_FROM_LONG_DOUBLE_FN "__kefir_bigint_unsigned_from_long_double"
#define BIGINT_IS_ZERO_FN "__kefir_bigint_is_zero"
#define BIGINT_NEGATE_FN "__kefir_bigint_negate"
#define BIGINT_INVERT_FN "__kefir_bigint_invert"
#define BIGINT_ADD_FN "__kefir_bigint_add"
#define BIGINT_SUBTRACT_FN "__kefir_bigint_subtract"
#define BIGINT_UNSIGNED_MULTIPLY_FN "__kefir_bigint_unsigned_multiply"
#define BIGINT_SIGNED_MULTIPLY_FN "__kefir_bigint_signed_multiply"
#define BIGINT_UNSIGNED_DIVIDE_FN "__kefir_bigint_unsigned_divide"
#define BIGINT_SIGNED_DIVIDE_FN "__kefir_bigint_signed_divide"
#define BIGINT_AND_FN "__kefir_bigint_and"
#define BIGINT_OR_FN "__kefir_bigint_or"
#define BIGINT_XOR_FN "__kefir_bigint_xor"
#define BIGINT_LEFT_SHIFT_FN "__kefir_bigint_left_shift"
#define BIGINT_RIGHT_SHIFT_FN "__kefir_bigint_right_shift"
#define BIGINT_ARITHMETIC_RIGHT_SHIFT_FN "__kefir_bigint_arithmetic_right_shift"

#define LIBATOMIC_SEQ_CST 5

#define LIBATOMIC_LOAD_N(_n) "__atomic_load_" #_n
#define LIBATOMIC_STORE_N(_n) "__atomic_store_" #_n
#define LIBATOMIC_LOAD "__atomic_load"
#define LIBATOMIC_STORE "__atomic_store"
#define LIBATOMIC_CMPXCHG_N(_n) "__atomic_compare_exchange_" #_n
#define LIBATOMIC_CMPXCHG "__atomic_compare_exchange"

#endif

#endif
