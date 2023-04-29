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
    OPCODE(GET_ARGUMENT, "get_argument", ref1) SEPARATOR \
    OPCODE(PHI, "phi", ref1) SEPARATOR \
    /* Flow control */ \
    OPCODE(JUMP, "jump", jump) SEPARATOR \
    OPCODE(IJUMP, "indirect_jump", ref1) SEPARATOR \
    OPCODE(BRANCH, "branch", branch) SEPARATOR \
    OPCODE(RETURN, "return", ref1) SEPARATOR \
    /* Constants */ \
    OPCODE(INT_CONST, "int_const", imm_int) SEPARATOR \
    OPCODE(UINT_CONST, "uint_const", imm_uint) SEPARATOR \
    OPCODE(FLOAT32_CONST, "float32_const", imm_float32) SEPARATOR \
    OPCODE(FLOAT64_CONST, "float64_const", imm_float64) SEPARATOR \
    OPCODE(STRING_REF, "string_ref", ref1) SEPARATOR \
    OPCODE(BLOCK_LABEL, "block_label", ref1) SEPARATOR \
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
    OPCODE(INT_LESSER, "int_lesser", ref2) SEPARATOR \
    OPCODE(INT_ABOVE, "int_above", ref2) SEPARATOR \
    OPCODE(INT_BELOW, "int_below", ref2) SEPARATOR \
    OPCODE(BOOL_AND, "bool_and", ref2) SEPARATOR \
    OPCODE(BOOL_OR, "bool_or", ref2) SEPARATOR \
    OPCODE(BOOL_NOT, "bool_not", ref1) SEPARATOR \
    /* Type conversions */ \
    OPCODE(INT64_ZERO_EXTEND_1BIT, "int64_zero_extend_1bit", ref1) SEPARATOR \
    OPCODE(INT64_SIGN_EXTEND_8BITS, "int64_sign_extend_8bits", ref1) SEPARATOR \
    OPCODE(INT64_SIGN_EXTEND_16BITS, "int64_sign_extend_16bits", ref1) SEPARATOR \
    OPCODE(INT64_SIGN_EXTEND_32BITS, "int64_sign_extend_32bits", ref1) SEPARATOR \
    /* Data access */ \
    OPCODE(GET_GLOBAL, "get_global", ref1) SEPARATOR \
    OPCODE(GET_THREAD_LOCAL, "get_thread_local", ref1) SEPARATOR \
    OPCODE(GET_LOCAL, "get_local", ref1) SEPARATOR \
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
    OPCODE(ZERO_MEMORY, "zero_memory", mem_op) SEPARATOR \
    OPCODE(COPY_MEMORY, "copy_memory", mem_op) SEPARATOR \
    OPCODE(BITS_EXTRACT_SIGNED, "bits_extract_signed", bitfield) SEPARATOR \
    OPCODE(BITS_EXTRACT_UNSIGNED, "bits_extract_unsigned", bitfield) SEPARATOR \
    OPCODE(BITS_INSERT, "bits_insert", bitfield)
// clang-format on

#endif
