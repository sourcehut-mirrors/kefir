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

#ifndef KEFIR_OPTIMIZER_OPCODE_DEFS_H_
#define KEFIR_OPTIMIZER_OPCODE_DEFS_H_

#include "kefir/optimizer/base.h"

// clang-format off
#define KEFIR_OPTIMIZER_OPCODE_DEFS(OPCODE, SEPARATOR) \
    /* Special */ \
    OPCODE(GET_ARGUMENT, "get_argument", index) SEPARATOR \
    OPCODE(PHI, "phi", phi_ref) SEPARATOR \
    OPCODE(INLINE_ASSEMBLY, "inline_assembly", inline_asm) SEPARATOR \
    /* Flow control */ \
    OPCODE(JUMP, "jump", branch) SEPARATOR \
    OPCODE(IJUMP, "indirect_jump", ref1) SEPARATOR \
    OPCODE(BRANCH, "branch", branch) SEPARATOR \
    OPCODE(RETURN, "return", ref1) SEPARATOR \
    OPCODE(INVOKE, "invoke", call_ref) SEPARATOR \
    OPCODE(INVOKE_VIRTUAL, "invoke_virtual", call_ref) SEPARATOR \
    OPCODE(COMPARE_BRANCH, "compare_branch", cmp_branch) SEPARATOR \
    /* Constants */ \
    OPCODE(INT_CONST, "int_const", immediate) SEPARATOR \
    OPCODE(UINT_CONST, "uint_const", immediate) SEPARATOR \
    OPCODE(FLOAT32_CONST, "float32_const", immediate) SEPARATOR \
    OPCODE(FLOAT64_CONST, "float64_const", immediate) SEPARATOR \
    OPCODE(LONG_DOUBLE_CONST, "long_double_const", immediate) SEPARATOR \
    OPCODE(STRING_REF, "string_ref", immediate) SEPARATOR \
    OPCODE(BLOCK_LABEL, "block_label", immediate) SEPARATOR \
    /* Placeholders */ \
    OPCODE(INT_PLACEHOLDER, "int_placeholder", none) SEPARATOR \
    OPCODE(FLOAT32_PLACEHOLDER, "float32_placeholder", none) SEPARATOR \
    OPCODE(FLOAT64_PLACEHOLDER, "float64_placeholder", none) SEPARATOR \
    /* Integral arithmetics & binary operations */ \
    OPCODE(INT_ADD, "int_add", ref2) SEPARATOR \
    OPCODE(INT_SUB, "int_sub", ref2) SEPARATOR \
    OPCODE(INT_MUL, "int_mul", ref2) SEPARATOR \
    OPCODE(INT_DIV, "int_div", ref2) SEPARATOR \
    OPCODE(INT_MOD, "int_mod", ref2) SEPARATOR \
    OPCODE(UINT_DIV, "uint_div", ref2) SEPARATOR \
    OPCODE(UINT_MOD, "uint_mod", ref2) SEPARATOR \
    OPCODE(INT_AND, "int_and", ref2) SEPARATOR \
    OPCODE(INT_OR, "int_or", ref2) SEPARATOR \
    OPCODE(INT_XOR, "int_xor", ref2) SEPARATOR \
    OPCODE(INT_LSHIFT, "int_lshift", ref2) SEPARATOR \
    OPCODE(INT_RSHIFT, "int_rshift", ref2) SEPARATOR \
    OPCODE(INT_ARSHIFT, "int_arshift", ref2) SEPARATOR \
    OPCODE(INT_NOT, "int_not", ref1) SEPARATOR \
    OPCODE(INT_NEG, "int_neg", ref1) SEPARATOR \
    /* Logical & comparison operations */ \
    OPCODE(INT_EQUALS, "int_equals", ref2) SEPARATOR \
    OPCODE(INT_GREATER, "int_greater", ref2) SEPARATOR \
    OPCODE(INT_GREATER_OR_EQUALS, "int_greater_or_equals", ref2) SEPARATOR \
    OPCODE(INT_LESSER, "int_lesser", ref2) SEPARATOR \
    OPCODE(INT_LESSER_OR_EQUALS, "int_lesser_or_equals", ref2) SEPARATOR \
    OPCODE(INT_ABOVE, "int_above", ref2) SEPARATOR \
    OPCODE(INT_ABOVE_OR_EQUALS, "int_above_or_equals", ref2) SEPARATOR \
    OPCODE(INT_BELOW, "int_below", ref2) SEPARATOR \
    OPCODE(INT_BELOW_OR_EQUALS, "int_below_or_equals", ref2) SEPARATOR \
    OPCODE(BOOL_AND, "bool_and", ref2) SEPARATOR \
    OPCODE(BOOL_OR, "bool_or", ref2) SEPARATOR \
    OPCODE(BOOL_NOT, "bool_not", ref1) SEPARATOR \
    /* Type conversions */ \
    OPCODE(INT64_TRUNCATE_1BIT, "int64_truncate_1bit", ref1) SEPARATOR \
    OPCODE(INT64_ZERO_EXTEND_8BITS, "int64_zero_extend_8bits", ref1) SEPARATOR \
    OPCODE(INT64_ZERO_EXTEND_16BITS, "int64_zero_extend_16bits", ref1) SEPARATOR \
    OPCODE(INT64_ZERO_EXTEND_32BITS, "int64_zero_extend_32bits", ref1) SEPARATOR \
    OPCODE(INT64_SIGN_EXTEND_8BITS, "int64_sign_extend_8bits", ref1) SEPARATOR \
    OPCODE(INT64_SIGN_EXTEND_16BITS, "int64_sign_extend_16bits", ref1) SEPARATOR \
    OPCODE(INT64_SIGN_EXTEND_32BITS, "int64_sign_extend_32bits", ref1) SEPARATOR \
    /* Data access */ \
    OPCODE(GET_GLOBAL, "get_global", ir_ref) SEPARATOR \
    OPCODE(GET_THREAD_LOCAL, "get_thread_local", ir_ref) SEPARATOR \
    OPCODE(GET_LOCAL, "get_local", index) SEPARATOR \
    OPCODE(INT8_LOAD_SIGNED, "int8_load_signed", load_mem) SEPARATOR \
    OPCODE(INT8_LOAD_UNSIGNED, "int8_load_unsigned", load_mem) SEPARATOR \
    OPCODE(INT16_LOAD_SIGNED, "int16_load_signed", load_mem) SEPARATOR \
    OPCODE(INT16_LOAD_UNSIGNED, "int16_load_unsigned", load_mem) SEPARATOR \
    OPCODE(INT32_LOAD_SIGNED, "int32_load_signed", load_mem) SEPARATOR \
    OPCODE(INT32_LOAD_UNSIGNED, "int32_load_unsigned", load_mem) SEPARATOR \
    OPCODE(INT64_LOAD, "int64_load", load_mem) SEPARATOR \
    OPCODE(INT8_STORE, "int8_store", store_mem) SEPARATOR \
    OPCODE(INT16_STORE, "int16_store", store_mem) SEPARATOR \
    OPCODE(INT32_STORE, "int32_store", store_mem) SEPARATOR \
    OPCODE(INT64_STORE, "int64_store", store_mem) SEPARATOR \
    OPCODE(LONG_DOUBLE_STORE, "long_double_store", store_mem) SEPARATOR \
    OPCODE(ZERO_MEMORY, "zero_memory", typed_ref2) SEPARATOR \
    OPCODE(COPY_MEMORY, "copy_memory", typed_ref2) SEPARATOR \
    OPCODE(BITS_EXTRACT_SIGNED, "bits_extract_signed", bitfield) SEPARATOR \
    OPCODE(BITS_EXTRACT_UNSIGNED, "bits_extract_unsigned", bitfield) SEPARATOR \
    OPCODE(BITS_INSERT, "bits_insert", bitfield) SEPARATOR \
    /* Built-ins */ \
    OPCODE(VARARG_START, "vararg_start", ref1) SEPARATOR \
    OPCODE(VARARG_COPY, "vararg_copy", ref2) SEPARATOR \
    OPCODE(VARARG_GET, "vararg_get", typed_ref1) SEPARATOR \
    OPCODE(VARARG_END, "vararg_end", ref1) SEPARATOR \
    OPCODE(STACK_ALLOC, "stack_alloc", stack_alloc) SEPARATOR \
    OPCODE(SCOPE_PUSH, "scope_push", none) SEPARATOR \
    OPCODE(SCOPE_POP, "scope_pop", ref1) SEPARATOR \
    /* Floating-point arithmetics */ \
    OPCODE(FLOAT32_ADD, "float32_add", ref2) SEPARATOR \
    OPCODE(FLOAT32_SUB, "float32_sub", ref2) SEPARATOR \
    OPCODE(FLOAT32_MUL, "float32_mul", ref2) SEPARATOR \
    OPCODE(FLOAT32_DIV, "float32_div", ref2) SEPARATOR \
    OPCODE(FLOAT32_NEG, "float32_neg", ref1) SEPARATOR \
    OPCODE(FLOAT64_ADD, "float64_add", ref2) SEPARATOR \
    OPCODE(FLOAT64_SUB, "float64_sub", ref2) SEPARATOR \
    OPCODE(FLOAT64_MUL, "float64_mul", ref2) SEPARATOR \
    OPCODE(FLOAT64_DIV, "float64_div", ref2) SEPARATOR \
    OPCODE(FLOAT64_NEG, "float64_neg", ref1) SEPARATOR \
    OPCODE(LONG_DOUBLE_ADD, "long_double_add", ref3) SEPARATOR \
    OPCODE(LONG_DOUBLE_SUB, "long_double_sub", ref3) SEPARATOR \
    OPCODE(LONG_DOUBLE_MUL, "long_double_mul", ref3) SEPARATOR \
    OPCODE(LONG_DOUBLE_DIV, "long_double_div", ref3) SEPARATOR \
    OPCODE(LONG_DOUBLE_NEG, "long_double_neg", ref2) SEPARATOR \
    /* Floating-point comparison */ \
    OPCODE(FLOAT32_EQUALS, "float32_equals", ref2) SEPARATOR \
    OPCODE(FLOAT32_GREATER, "float32_greater", ref2) SEPARATOR \
    OPCODE(FLOAT32_GREATER_OR_EQUALS, "float32_greater_or_equals", ref2) SEPARATOR \
    OPCODE(FLOAT32_LESSER, "float32_lesser", ref2) SEPARATOR \
    OPCODE(FLOAT32_LESSER_OR_EQUALS, "float32_lesser_or_equals", ref2) SEPARATOR \
    OPCODE(FLOAT64_EQUALS, "float64_equals", ref2) SEPARATOR \
    OPCODE(FLOAT64_GREATER, "float64_greater", ref2) SEPARATOR \
    OPCODE(FLOAT64_GREATER_OR_EQUALS, "float64_greater_or_equals", ref2) SEPARATOR \
    OPCODE(FLOAT64_LESSER, "float64_lesser", ref2) SEPARATOR \
    OPCODE(FLOAT64_LESSER_OR_EQUALS, "float64_lesser_or_equals", ref2) SEPARATOR \
    OPCODE(LONG_DOUBLE_EQUALS, "long_double_equals", ref2) SEPARATOR \
    OPCODE(LONG_DOUBLE_GREATER, "long_double_greater", ref2) SEPARATOR \
    OPCODE(LONG_DOUBLE_LESSER, "long_double_lesser", ref2) SEPARATOR \
    /* Floating-point conversions */ \
    OPCODE(FLOAT32_TO_INT, "float32_to_int", ref1) SEPARATOR \
    OPCODE(FLOAT64_TO_INT, "float64_to_int", ref1) SEPARATOR \
    OPCODE(FLOAT32_TO_UINT, "float32_to_uint", ref1) SEPARATOR \
    OPCODE(FLOAT64_TO_UINT, "float64_to_uint", ref1) SEPARATOR \
    OPCODE(INT_TO_FLOAT32, "int_to_float32", ref1) SEPARATOR \
    OPCODE(INT_TO_FLOAT64, "int_to_float64", ref1) SEPARATOR \
    OPCODE(UINT_TO_FLOAT32, "uint_to_float32", ref1) SEPARATOR \
    OPCODE(UINT_TO_FLOAT64, "uint_to_float64", ref1) SEPARATOR \
    OPCODE(FLOAT32_TO_FLOAT64, "float32_to_float64", ref1) SEPARATOR \
    OPCODE(FLOAT64_TO_FLOAT32, "float64_to_float32", ref1) SEPARATOR \
    OPCODE(LONG_DOUBLE_TRUNCATE_1BIT, "long_double_truncate_1bit", ref1) SEPARATOR \
    OPCODE(LONG_DOUBLE_TO_INT, "long_double_to_int", ref1) SEPARATOR \
    OPCODE(LONG_DOUBLE_TO_UINT, "long_double_to_uint", ref1) SEPARATOR \
    OPCODE(LONG_DOUBLE_TO_FLOAT32, "long_double_to_float32", ref1) SEPARATOR \
    OPCODE(LONG_DOUBLE_TO_FLOAT64, "long_double_to_float64", ref1) SEPARATOR \
    OPCODE(INT_TO_LONG_DOUBLE, "int_to_long_double", ref2) SEPARATOR \
    OPCODE(UINT_TO_LONG_DOUBLE, "uint_to_long_double", ref2) SEPARATOR \
    OPCODE(FLOAT32_TO_LONG_DOUBLE, "float32_to_long_double", ref2) SEPARATOR \
    OPCODE(FLOAT64_TO_LONG_DOUBLE, "float64_to_long_double", ref2)
// clang-format on

#endif
