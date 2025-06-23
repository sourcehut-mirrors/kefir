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

#ifndef KEFIR_OPTIMIZER_OPCODE_DEFS_H_
#define KEFIR_OPTIMIZER_OPCODE_DEFS_H_

#include "kefir/optimizer/base.h"

// clang-format off
#define KEFIR_OPTIMIZER_OPCODE_DEFS(OPCODE, SEPARATOR) \
    /* Special */ \
    OPCODE(GET_ARGUMENT, "get_argument", index) SEPARATOR \
    OPCODE(PHI, "phi", phi_ref) SEPARATOR \
    OPCODE(INLINE_ASSEMBLY, "inline_assembly", inline_asm) SEPARATOR \
    OPCODE(SELECT, "select", ref3_cond) SEPARATOR \
    OPCODE(SELECT_COMPARE, "select_compare", ref4_compare) SEPARATOR \
    OPCODE(LOCAL_LIFETIME_MARK, "local_lifetime_mark", ref1) SEPARATOR \
    OPCODE(TEMPORARY_OBJECT, "temporary_object", tmpobj) SEPARATOR \
    OPCODE(PAIR, "pair", ref2) SEPARATOR \
    /* Flow control */ \
    OPCODE(JUMP, "jump", branch) SEPARATOR \
    OPCODE(IJUMP, "indirect_jump", ref1) SEPARATOR \
    OPCODE(BRANCH, "branch", branch) SEPARATOR \
    OPCODE(BRANCH_COMPARE, "branch_compare", branch_compare) SEPARATOR \
    OPCODE(RETURN, "return", ref1) SEPARATOR \
    OPCODE(INVOKE, "invoke", call_ref) SEPARATOR \
    OPCODE(INVOKE_VIRTUAL, "invoke_virtual", call_ref) SEPARATOR \
    OPCODE(TAIL_INVOKE, "tail_invoke", call_ref) SEPARATOR \
    OPCODE(TAIL_INVOKE_VIRTUAL, "tail_invoke_virtual", call_ref) SEPARATOR \
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
    OPCODE(INT8_ADD, "int8_add", ref2) SEPARATOR \
    OPCODE(INT16_ADD, "int16_add", ref2) SEPARATOR \
    OPCODE(INT32_ADD, "int32_add", ref2) SEPARATOR \
    OPCODE(INT64_ADD, "int64_add", ref2) SEPARATOR \
    OPCODE(INT8_SUB, "int8_sub", ref2) SEPARATOR \
    OPCODE(INT16_SUB, "int16_sub", ref2) SEPARATOR \
    OPCODE(INT32_SUB, "int32_sub", ref2) SEPARATOR \
    OPCODE(INT64_SUB, "int64_sub", ref2) SEPARATOR \
    OPCODE(INT8_MUL, "int8_mul", ref2) SEPARATOR \
    OPCODE(INT16_MUL, "int16_mul", ref2) SEPARATOR \
    OPCODE(INT32_MUL, "int32_mul", ref2) SEPARATOR \
    OPCODE(INT64_MUL, "int64_mul", ref2) SEPARATOR \
    OPCODE(UINT8_MUL, "uint8_mul", ref2) SEPARATOR \
    OPCODE(UINT16_MUL, "uint16_mul", ref2) SEPARATOR \
    OPCODE(UINT32_MUL, "uint32_mul", ref2) SEPARATOR \
    OPCODE(UINT64_MUL, "uint64_mul", ref2) SEPARATOR \
    OPCODE(INT8_DIV, "int8_div", ref2) SEPARATOR \
    OPCODE(INT16_DIV, "int16_div", ref2) SEPARATOR \
    OPCODE(INT32_DIV, "int32_div", ref2) SEPARATOR \
    OPCODE(INT64_DIV, "int64_div", ref2) SEPARATOR \
    OPCODE(INT8_MOD, "int8_mod", ref2) SEPARATOR \
    OPCODE(INT16_MOD, "int16_mod", ref2) SEPARATOR \
    OPCODE(INT32_MOD, "int32_mod", ref2) SEPARATOR \
    OPCODE(INT64_MOD, "int64_mod", ref2) SEPARATOR \
    OPCODE(UINT8_DIV, "uint8_div", ref2) SEPARATOR \
    OPCODE(UINT16_DIV, "uint16_div", ref2) SEPARATOR \
    OPCODE(UINT32_DIV, "uint32_div", ref2) SEPARATOR \
    OPCODE(UINT64_DIV, "uint64_div", ref2) SEPARATOR \
    OPCODE(UINT8_MOD, "uint8_mod", ref2) SEPARATOR \
    OPCODE(UINT16_MOD, "uint16_mod", ref2) SEPARATOR \
    OPCODE(UINT32_MOD, "uint32_mod", ref2) SEPARATOR \
    OPCODE(UINT64_MOD, "uint64_mod", ref2) SEPARATOR \
    OPCODE(INT8_AND, "int8_and", ref2) SEPARATOR \
    OPCODE(INT16_AND, "int16_and", ref2) SEPARATOR \
    OPCODE(INT32_AND, "int32_and", ref2) SEPARATOR \
    OPCODE(INT64_AND, "int64_and", ref2) SEPARATOR \
    OPCODE(INT8_OR, "int8_or", ref2) SEPARATOR \
    OPCODE(INT16_OR, "int16_or", ref2) SEPARATOR \
    OPCODE(INT32_OR, "int32_or", ref2) SEPARATOR \
    OPCODE(INT64_OR, "int64_or", ref2) SEPARATOR \
    OPCODE(INT8_XOR, "int8_xor", ref2) SEPARATOR \
    OPCODE(INT16_XOR, "int16_xor", ref2) SEPARATOR \
    OPCODE(INT32_XOR, "int32_xor", ref2) SEPARATOR \
    OPCODE(INT64_XOR, "int64_xor", ref2) SEPARATOR \
    OPCODE(INT8_LSHIFT, "int8_lshift", ref2) SEPARATOR \
    OPCODE(INT16_LSHIFT, "int16_lshift", ref2) SEPARATOR \
    OPCODE(INT32_LSHIFT, "int32_lshift", ref2) SEPARATOR \
    OPCODE(INT64_LSHIFT, "int64_lshift", ref2) SEPARATOR \
    OPCODE(INT8_RSHIFT, "int8_rshift", ref2) SEPARATOR \
    OPCODE(INT16_RSHIFT, "int16_rshift", ref2) SEPARATOR \
    OPCODE(INT32_RSHIFT, "int32_rshift", ref2) SEPARATOR \
    OPCODE(INT64_RSHIFT, "int64_rshift", ref2) SEPARATOR \
    OPCODE(INT8_ARSHIFT, "int8_arshift", ref2) SEPARATOR \
    OPCODE(INT16_ARSHIFT, "int16_arshift", ref2) SEPARATOR \
    OPCODE(INT32_ARSHIFT, "int32_arshift", ref2) SEPARATOR \
    OPCODE(INT64_ARSHIFT, "int64_arshift", ref2) SEPARATOR \
    OPCODE(INT8_NOT, "int8_not", ref1) SEPARATOR \
    OPCODE(INT16_NOT, "int16_not", ref1) SEPARATOR \
    OPCODE(INT32_NOT, "int32_not", ref1) SEPARATOR \
    OPCODE(INT64_NOT, "int64_not", ref1) SEPARATOR \
    OPCODE(INT8_NEG, "int8_neg", ref1) SEPARATOR \
    OPCODE(INT16_NEG, "int16_neg", ref1) SEPARATOR \
    OPCODE(INT32_NEG, "int32_neg", ref1) SEPARATOR \
    OPCODE(INT64_NEG, "int64_neg", ref1) SEPARATOR \
    /* Logical & comparison operations */ \
    OPCODE(SCALAR_COMPARE, "scalar_compare", compare_ref2) SEPARATOR \
    OPCODE(INT8_BOOL_NOT, "int8_bool_not", ref1) SEPARATOR \
    OPCODE(INT16_BOOL_NOT, "int16_bool_not", ref1) SEPARATOR \
    OPCODE(INT32_BOOL_NOT, "int32_bool_not", ref1) SEPARATOR \
    OPCODE(INT64_BOOL_NOT, "int64_bool_not", ref1) SEPARATOR \
    OPCODE(INT8_BOOL_OR, "int8_bool_or", ref2) SEPARATOR \
    OPCODE(INT16_BOOL_OR, "int16_bool_or", ref2) SEPARATOR \
    OPCODE(INT32_BOOL_OR, "int32_bool_or", ref2) SEPARATOR \
    OPCODE(INT64_BOOL_OR, "int64_bool_or", ref2) SEPARATOR \
    OPCODE(INT8_BOOL_AND, "int8_bool_and", ref2) SEPARATOR \
    OPCODE(INT16_BOOL_AND, "int16_bool_and", ref2) SEPARATOR \
    OPCODE(INT32_BOOL_AND, "int32_bool_and", ref2) SEPARATOR \
    OPCODE(INT64_BOOL_AND, "int64_bool_and", ref2) SEPARATOR \
    /* Type conversions */ \
    OPCODE(INT8_TO_BOOL, "int8_to_bool", ref1) SEPARATOR \
    OPCODE(INT16_TO_BOOL, "int16_to_bool", ref1) SEPARATOR \
    OPCODE(INT32_TO_BOOL, "int32_to_bool", ref1) SEPARATOR \
    OPCODE(INT64_TO_BOOL, "int64_to_bool", ref1) SEPARATOR \
    OPCODE(INT64_ZERO_EXTEND_8BITS, "int64_zero_extend_8bits", ref1) SEPARATOR \
    OPCODE(INT64_ZERO_EXTEND_16BITS, "int64_zero_extend_16bits", ref1) SEPARATOR \
    OPCODE(INT64_ZERO_EXTEND_32BITS, "int64_zero_extend_32bits", ref1) SEPARATOR \
    OPCODE(INT64_SIGN_EXTEND_8BITS, "int64_sign_extend_8bits", ref1) SEPARATOR \
    OPCODE(INT64_SIGN_EXTEND_16BITS, "int64_sign_extend_16bits", ref1) SEPARATOR \
    OPCODE(INT64_SIGN_EXTEND_32BITS, "int64_sign_extend_32bits", ref1) SEPARATOR \
    /* Data access */ \
    OPCODE(GET_GLOBAL, "get_global", variable) SEPARATOR \
    OPCODE(GET_THREAD_LOCAL, "get_thread_local", variable) SEPARATOR \
    OPCODE(ALLOC_LOCAL, "alloc_local", type) SEPARATOR \
    OPCODE(REF_LOCAL, "ref_local", ref_offset) SEPARATOR \
    OPCODE(INT8_LOAD, "int8_load", load_mem) SEPARATOR \
    OPCODE(INT16_LOAD, "int16_load", load_mem) SEPARATOR \
    OPCODE(INT32_LOAD, "int32_load", load_mem) SEPARATOR \
    OPCODE(INT64_LOAD, "int64_load", load_mem) SEPARATOR \
    OPCODE(LONG_DOUBLE_LOAD, "long_double_load", load_mem) SEPARATOR \
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
    OPCODE(VARARG_GET, "vararg_get", typed_ref2) SEPARATOR \
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
    OPCODE(LONG_DOUBLE_ADD, "long_double_add", ref2) SEPARATOR \
    OPCODE(LONG_DOUBLE_SUB, "long_double_sub", ref2) SEPARATOR \
    OPCODE(LONG_DOUBLE_MUL, "long_double_mul", ref2) SEPARATOR \
    OPCODE(LONG_DOUBLE_DIV, "long_double_div", ref2) SEPARATOR \
    OPCODE(LONG_DOUBLE_NEG, "long_double_neg", ref1) SEPARATOR \
    /* Floating-point comparison */ \
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
    OPCODE(LONG_DOUBLE_TO_INT, "long_double_to_int", ref1) SEPARATOR \
    OPCODE(LONG_DOUBLE_TO_UINT, "long_double_to_uint", ref1) SEPARATOR \
    OPCODE(LONG_DOUBLE_TO_FLOAT32, "long_double_to_float32", ref1) SEPARATOR \
    OPCODE(LONG_DOUBLE_TO_FLOAT64, "long_double_to_float64", ref1) SEPARATOR \
    OPCODE(INT_TO_LONG_DOUBLE, "int_to_long_double", ref1) SEPARATOR \
    OPCODE(UINT_TO_LONG_DOUBLE, "uint_to_long_double", ref1) SEPARATOR \
    OPCODE(FLOAT32_TO_LONG_DOUBLE, "float32_to_long_double", ref1) SEPARATOR \
    OPCODE(FLOAT64_TO_LONG_DOUBLE, "float64_to_long_double", ref1) SEPARATOR \
    /* Floating-point environment */ \
    OPCODE(FENV_SAVE, "fenv_save", none) SEPARATOR \
    OPCODE(FENV_CLEAR, "fenv_clear", none) SEPARATOR \
    OPCODE(FENV_UPDATE, "fenv_update", ref1) SEPARATOR \
    /* Complex numbers */ \
    OPCODE(COMPLEX_FLOAT32_FROM, "complex_float32_from", ref2) SEPARATOR \
    OPCODE(COMPLEX_FLOAT32_REAL, "complex_float32_real", ref1) SEPARATOR \
    OPCODE(COMPLEX_FLOAT32_IMAGINARY, "complex_float32_imaginary", ref1) SEPARATOR \
    OPCODE(COMPLEX_FLOAT64_FROM, "complex_float64_from", ref2) SEPARATOR \
    OPCODE(COMPLEX_FLOAT64_REAL, "complex_float64_real", ref1) SEPARATOR \
    OPCODE(COMPLEX_FLOAT64_IMAGINARY, "complex_float64_imaginary", ref1) SEPARATOR \
    OPCODE(COMPLEX_LONG_DOUBLE_FROM, "complex_long_double_from", ref2) SEPARATOR \
    OPCODE(COMPLEX_LONG_DOUBLE_REAL, "complex_long_double_real", ref1) SEPARATOR \
    OPCODE(COMPLEX_LONG_DOUBLE_IMAGINARY, "complex_long_double_imaginary", ref1) SEPARATOR \
    OPCODE(COMPLEX_FLOAT32_EQUALS, "complex_float32_equals", ref2) SEPARATOR \
    OPCODE(COMPLEX_FLOAT64_EQUALS, "complex_float64_equals", ref2) SEPARATOR \
    OPCODE(COMPLEX_LONG_DOUBLE_EQUALS, "complex_long_double_equals", ref2) SEPARATOR \
    OPCODE(COMPLEX_FLOAT32_TRUNCATE_1BIT, "complex_float32_truncate_1bit", ref1) SEPARATOR \
    OPCODE(COMPLEX_FLOAT64_TRUNCATE_1BIT, "complex_float64_truncate_1bit", ref1) SEPARATOR \
    OPCODE(COMPLEX_LONG_DOUBLE_TRUNCATE_1BIT, "complex_long_double_truncate_1bit", ref1) SEPARATOR \
    OPCODE(COMPLEX_FLOAT32_ADD, "complex_float32_add", ref2) SEPARATOR \
    OPCODE(COMPLEX_FLOAT64_ADD, "complex_float64_add", ref2) SEPARATOR \
    OPCODE(COMPLEX_LONG_DOUBLE_ADD, "complex_long_double_add", ref2) SEPARATOR \
    OPCODE(COMPLEX_FLOAT32_SUB, "complex_float32_sub", ref2) SEPARATOR \
    OPCODE(COMPLEX_FLOAT64_SUB, "complex_float64_sub", ref2) SEPARATOR \
    OPCODE(COMPLEX_LONG_DOUBLE_SUB, "complex_long_double_sub", ref2) SEPARATOR \
    OPCODE(COMPLEX_FLOAT32_MUL, "complex_float32_mul", ref2) SEPARATOR \
    OPCODE(COMPLEX_FLOAT64_MUL, "complex_float64_mul", ref2) SEPARATOR \
    OPCODE(COMPLEX_LONG_DOUBLE_MUL, "complex_long_double_mul", ref2) SEPARATOR \
    OPCODE(COMPLEX_FLOAT32_DIV, "complex_float32_div", ref2) SEPARATOR \
    OPCODE(COMPLEX_FLOAT64_DIV, "complex_float64_div", ref2) SEPARATOR \
    OPCODE(COMPLEX_LONG_DOUBLE_DIV, "complex_long_double_div", ref2) SEPARATOR \
    OPCODE(COMPLEX_FLOAT32_NEG, "complex_float32_neg", ref1) SEPARATOR \
    OPCODE(COMPLEX_FLOAT64_NEG, "complex_float64_neg", ref1) SEPARATOR \
    OPCODE(COMPLEX_LONG_DOUBLE_NEG, "complex_long_double_neg", ref1) SEPARATOR \
    OPCODE(COMPLEX_FLOAT32_LOAD, "complex_float32_load", load_mem) SEPARATOR \
    OPCODE(COMPLEX_FLOAT64_LOAD, "complex_float64_load", load_mem) SEPARATOR \
    OPCODE(COMPLEX_LONG_DOUBLE_LOAD, "complex_long_double_load", load_mem) SEPARATOR \
    OPCODE(COMPLEX_FLOAT32_STORE, "complex_float32_store", store_mem) SEPARATOR \
    OPCODE(COMPLEX_FLOAT64_STORE, "complex_float64_store", store_mem) SEPARATOR \
    OPCODE(COMPLEX_LONG_DOUBLE_STORE, "complex_long_double_store", store_mem) SEPARATOR \
    /* Atomics */ \
    OPCODE(ATOMIC_LOAD8, "atomic_load8", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_LOAD16, "atomic_load16", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_LOAD32, "atomic_load32", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_LOAD64, "atomic_load64", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_LOAD_LONG_DOUBLE, "atomic_load_long_double", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_LOAD_COMPLEX_FLOAT32, "atomic_load_complex_float32", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_LOAD_COMPLEX_FLOAT64, "atomic_load_complex_float64", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_LOAD_COMPLEX_LONG_DOUBLE, "atomic_load_complex_long_double", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_STORE8, "atomic_store8", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_STORE16, "atomic_store16", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_STORE32, "atomic_store32", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_STORE64, "atomic_store64", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_STORE_LONG_DOUBLE, "atomic_store_long_double", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_STORE_COMPLEX_FLOAT32, "atomic_store_complex_float32", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_STORE_COMPLEX_FLOAT64, "atomic_store_complex_float64", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_STORE_COMPLEX_LONG_DOUBLE, "atomic_store_complex_long_double", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_COPY_MEMORY_FROM, "atomic_copy_memory_from", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_COPY_MEMORY_TO, "atomic_copy_memory_to", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_CMPXCHG8, "atomic_cmpxchg8", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_CMPXCHG16, "atomic_cmpxchg16", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_CMPXCHG32, "atomic_cmpxchg32", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_CMPXCHG64, "atomic_cmpxchg64", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_CMPXCHG_LONG_DOUBLE, "atomic_cmpxchg_long_double", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_CMPXCHG_COMPLEX_FLOAT32, "atomic_cmpxchg_complex_float32", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_CMPXCHG_COMPLEX_FLOAT64, "atomic_cmpxchg_complex_float64", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_CMPXCHG_COMPLEX_LONG_DOUBLE, "atomic_cmpxchg_complex_long_double", atomic_op) SEPARATOR \
    OPCODE(ATOMIC_CMPXCHG_MEMORY, "atomic_cmpxchg_memory", atomic_op) SEPARATOR \
    /* Overflow arithmetics */ \
    OPCODE(ADD_OVERFLOW, "add_overflow", overflow_arith) SEPARATOR \
    OPCODE(SUB_OVERFLOW, "sub_overflow", overflow_arith) SEPARATOR \
    OPCODE(MUL_OVERFLOW, "mul_overflow", overflow_arith) SEPARATOR \
    /* Bit-precise */ \
    OPCODE(BITINT_SIGNED_CONST, "bitint_signed_const", immediate) SEPARATOR \
    OPCODE(BITINT_UNSIGNED_CONST, "bitint_unsigned_const", immediate) SEPARATOR \
    OPCODE(BITINT_GET_SIGNED, "bitint_get_signed", bitint_ref1) SEPARATOR \
    OPCODE(BITINT_GET_UNSIGNED, "bitint_get_unsigned", bitint_ref1) SEPARATOR \
    OPCODE(BITINT_FROM_SIGNED, "bitint_from_signed", bitint_ref1) SEPARATOR \
    OPCODE(BITINT_FROM_UNSIGNED, "bitint_from_unsigned", bitint_ref1) SEPARATOR \
    OPCODE(BITINT_CAST_SIGNED, "bitint_cast_signed", bitint2_ref1) SEPARATOR \
    OPCODE(BITINT_CAST_UNSIGNED, "bitint_cast_unsigned", bitint2_ref1) SEPARATOR \
    OPCODE(BITINT_SIGNED_TO_FLOAT, "bitint_signed_to_float", bitint_ref1) SEPARATOR \
    OPCODE(BITINT_UNSIGNED_TO_FLOAT, "bitint_unsigned_to_float", bitint_ref1) SEPARATOR \
    OPCODE(BITINT_SIGNED_TO_DOUBLE, "bitint_signed_to_double", bitint_ref1) SEPARATOR \
    OPCODE(BITINT_UNSIGNED_TO_DOUBLE, "bitint_unsigned_to_double", bitint_ref1) SEPARATOR \
    OPCODE(BITINT_SIGNED_TO_LONG_DOUBLE, "bitint_signed_to_long_double", bitint_ref1) SEPARATOR \
    OPCODE(BITINT_UNSIGNED_TO_LONG_DOUBLE, "bitint_unsigned_to_long_double", bitint_ref1) SEPARATOR \
    OPCODE(FLOAT_TO_BITINT_SIGNED, "float_to_bitint_signed", bitint_ref1) SEPARATOR \
    OPCODE(FLOAT_TO_BITINT_UNSIGNED, "float_to_bitint_unsigned", bitint_ref1) SEPARATOR \
    OPCODE(DOUBLE_TO_BITINT_SIGNED, "double_to_bitint_signed", bitint_ref1) SEPARATOR \
    OPCODE(DOUBLE_TO_BITINT_UNSIGNED, "double_to_bitint_unsigned", bitint_ref1) SEPARATOR \
    OPCODE(LONG_DOUBLE_TO_BITINT_SIGNED, "long_double_to_bitint_signed", bitint_ref1) SEPARATOR \
    OPCODE(LONG_DOUBLE_TO_BITINT_UNSIGNED, "long_double_to_bitint_unsigned", bitint_ref1) SEPARATOR \
    OPCODE(BITINT_TO_BOOL, "bitint_to_bool", bitint_ref1) SEPARATOR \
    OPCODE(BITINT_LOAD, "bitint_load", bitint_load) SEPARATOR \
    OPCODE(BITINT_STORE, "bitint_store", bitint_store) SEPARATOR \
    OPCODE(BITINT_ATOMIC_LOAD, "bitint_atomic_load", bitint_atomic) SEPARATOR \
    OPCODE(BITINT_ATOMIC_STORE, "bitint_atomic_store", bitint_atomic) SEPARATOR \
    OPCODE(BITINT_ATOMIC_COMPARE_EXCHANGE, "bitint_atomic_compare_exchange", bitint_atomic) SEPARATOR \
    OPCODE(BITINT_NEGATE, "bitint_negate", bitint_ref1) SEPARATOR \
    OPCODE(BITINT_INVERT, "bitint_invert", bitint_ref1) SEPARATOR \
    OPCODE(BITINT_BOOL_NOT, "bitint_bool_not", bitint_ref1) SEPARATOR \
    OPCODE(BITINT_ADD, "bitint_add", bitint_ref2) SEPARATOR \
    OPCODE(BITINT_SUB, "bitint_sub", bitint_ref2)

// clang-format on

#endif
