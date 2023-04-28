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
    OPCODE(INT_NEG, "int_neg", ref1)
// clang-format on

#endif
