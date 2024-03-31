/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#include "kefir/optimizer/pipeline.h"
#include "kefir/optimizer/builder.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t simplify_boot_not(struct kefir_mem *mem, struct kefir_opt_function *func,
                                        struct kefir_opt_instruction *instr,
                                        kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;
    const kefir_opt_instruction_ref_t instr_id = instr->id;

    struct kefir_opt_instruction *arg;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg));
    REQUIRE(arg->operation.opcode == KEFIR_OPT_OPCODE_BOOL_NOT, KEFIR_OK);

    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg->operation.parameters.refs[0], &arg));
    switch (arg->operation.opcode) {
        case KEFIR_OPT_OPCODE_BOOL_NOT:
        case KEFIR_OPT_OPCODE_BOOL_AND:
        case KEFIR_OPT_OPCODE_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT_EQUALS:
        case KEFIR_OPT_OPCODE_INT_GREATER:
        case KEFIR_OPT_OPCODE_INT_LESSER:
        case KEFIR_OPT_OPCODE_INT_ABOVE:
        case KEFIR_OPT_OPCODE_INT_BELOW:
        case KEFIR_OPT_OPCODE_FLOAT32_EQUALS:
        case KEFIR_OPT_OPCODE_FLOAT32_GREATER:
        case KEFIR_OPT_OPCODE_FLOAT32_LESSER:
        case KEFIR_OPT_OPCODE_FLOAT64_EQUALS:
        case KEFIR_OPT_OPCODE_FLOAT64_GREATER:
        case KEFIR_OPT_OPCODE_FLOAT64_LESSER:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_EQUALS:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_GREATER:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_LESSER:
        case KEFIR_OPT_OPCODE_INT64_TRUNCATE_1BIT:
            *replacement_ref = arg->id;
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_CONST:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_ADD:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_SUB:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_MUL:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_DIV:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_NEG:
        case KEFIR_OPT_OPCODE_INT_TO_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_UINT_TO_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_FLOAT32_TO_LONG_DOUBLE:
        case KEFIR_OPT_OPCODE_FLOAT64_TO_LONG_DOUBLE: {
            kefir_opt_instruction_ref_t zero_ref, equality_ref;
            REQUIRE_OK(kefir_opt_code_builder_long_double_constant(mem, &func->code, block_id, 0.0L, &zero_ref));
            REQUIRE_OK(kefir_opt_code_builder_long_double_equals(mem, &func->code, block_id, arg->id, zero_ref,
                                                                 &equality_ref));
            REQUIRE_OK(kefir_opt_code_builder_bool_not(mem, &func->code, block_id, equality_ref, replacement_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
        } break;

        default:
            REQUIRE_OK(
                kefir_opt_code_builder_int64_truncate_1bit(mem, &func->code, block_id, arg->id, replacement_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_boot_or(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;
    const kefir_opt_instruction_ref_t instr_id = instr->id;

    struct kefir_opt_instruction *arg1;
    struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));
    if (arg1->operation.opcode == KEFIR_OPT_OPCODE_LONG_DOUBLE_GREATER) {
        REQUIRE(arg2->operation.opcode == KEFIR_OPT_OPCODE_LONG_DOUBLE_EQUALS, KEFIR_OK);
        REQUIRE((arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[0] &&
                 arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[1]) ||
                    (arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[1] &&
                     arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[0]),
                KEFIR_OK);

        kefir_opt_instruction_ref_t cmp_ref;
        REQUIRE_OK(kefir_opt_code_builder_long_double_lesser(mem, &func->code, block_id,
                                                             arg1->operation.parameters.refs[0],
                                                             arg1->operation.parameters.refs[1], &cmp_ref));
        REQUIRE_OK(kefir_opt_code_builder_bool_not(mem, &func->code, block_id, cmp_ref, replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, cmp_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, cmp_ref, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_LONG_DOUBLE_LESSER) {
        REQUIRE(arg2->operation.opcode == KEFIR_OPT_OPCODE_LONG_DOUBLE_EQUALS, KEFIR_OK);
        REQUIRE((arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[0] &&
                 arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[1]) ||
                    (arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[1] &&
                     arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[0]),
                KEFIR_OK);

        kefir_opt_instruction_ref_t cmp_ref;
        REQUIRE_OK(kefir_opt_code_builder_long_double_greater(mem, &func->code, block_id,
                                                              arg1->operation.parameters.refs[0],
                                                              arg1->operation.parameters.refs[1], &cmp_ref));
        REQUIRE_OK(kefir_opt_code_builder_bool_not(mem, &func->code, block_id, cmp_ref, replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, cmp_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, cmp_ref, *replacement_ref));
    }

    return KEFIR_OK;
}

static kefir_result_t simplify_int_and(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;
    const kefir_opt_instruction_ref_t instr_id = instr->id;

    struct kefir_opt_instruction *arg1;
    struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    if (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
        arg2->operation.parameters.imm.uinteger == ((1ull << 32) - 1)) {
        REQUIRE_OK(
            kefir_opt_code_builder_int64_zero_extend_32bits(mem, &func->code, block_id, arg1->id, replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
    } else if (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
               arg2->operation.parameters.imm.uinteger == ((1ull << 16) - 1)) {
        REQUIRE_OK(
            kefir_opt_code_builder_int64_zero_extend_16bits(mem, &func->code, block_id, arg1->id, replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
    } else if (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
               arg2->operation.parameters.imm.uinteger == ((1ull << 8) - 1)) {
        REQUIRE_OK(
            kefir_opt_code_builder_int64_zero_extend_8bits(mem, &func->code, block_id, arg1->id, replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_AND &&
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *and_arg1;
        struct kefir_opt_instruction *and_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &and_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &and_arg2));

        kefir_uint64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? (kefir_uint64_t) arg2->operation.parameters.imm.integer
                                        : arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (and_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand &= (kefir_uint64_t) and_arg1->operation.parameters.imm.integer;
            operand_ref = and_arg2->id;
        } else if (and_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand &= and_arg1->operation.parameters.imm.uinteger;
            operand_ref = and_arg2->id;
        } else if (and_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand &= (kefir_uint64_t) and_arg2->operation.parameters.imm.integer;
            operand_ref = and_arg1->id;
        } else if (and_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand &= and_arg2->operation.parameters.imm.uinteger;
            operand_ref = and_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int_and(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref));

            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    } else if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_AND &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *and_arg1;
        struct kefir_opt_instruction *and_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[0], &and_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[1], &and_arg2));

        kefir_uint64_t imm_operand = arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? (kefir_uint64_t) arg1->operation.parameters.imm.integer
                                        : arg1->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (and_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand &= (kefir_uint64_t) and_arg1->operation.parameters.imm.integer;
            operand_ref = and_arg2->id;
        } else if (and_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand &= and_arg1->operation.parameters.imm.uinteger;
            operand_ref = and_arg2->id;
        } else if (and_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand &= (kefir_uint64_t) and_arg2->operation.parameters.imm.integer;
            operand_ref = and_arg1->id;
        } else if (and_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand &= and_arg2->operation.parameters.imm.uinteger;
            operand_ref = and_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int_and(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref));

            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_or(struct kefir_mem *mem, struct kefir_opt_function *func,
                                      struct kefir_opt_instruction *instr,
                                      kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;
    const kefir_opt_instruction_ref_t instr_id = instr->id;

    struct kefir_opt_instruction *arg1;
    struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_OR &&
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *or_arg1;
        struct kefir_opt_instruction *or_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &or_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &or_arg2));

        kefir_uint64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? (kefir_uint64_t) arg2->operation.parameters.imm.integer
                                        : arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (or_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand |= (kefir_uint64_t) or_arg1->operation.parameters.imm.integer;
            operand_ref = or_arg2->id;
        } else if (or_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand |= or_arg1->operation.parameters.imm.uinteger;
            operand_ref = or_arg2->id;
        } else if (or_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand |= (kefir_uint64_t) or_arg2->operation.parameters.imm.integer;
            operand_ref = or_arg1->id;
        } else if (or_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand |= or_arg2->operation.parameters.imm.uinteger;
            operand_ref = or_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int_or(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref));

            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    } else if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_OR &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *or_arg1;
        struct kefir_opt_instruction *or_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[0], &or_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[1], &or_arg2));

        kefir_uint64_t imm_operand = arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? (kefir_uint64_t) arg1->operation.parameters.imm.integer
                                        : arg1->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (or_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand |= (kefir_uint64_t) or_arg1->operation.parameters.imm.integer;
            operand_ref = or_arg2->id;
        } else if (or_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand |= or_arg1->operation.parameters.imm.uinteger;
            operand_ref = or_arg2->id;
        } else if (or_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand |= (kefir_uint64_t) or_arg2->operation.parameters.imm.integer;
            operand_ref = or_arg1->id;
        } else if (or_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand |= or_arg2->operation.parameters.imm.uinteger;
            operand_ref = or_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int_or(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref));

            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_xor(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;
    const kefir_opt_instruction_ref_t instr_id = instr->id;

    struct kefir_opt_instruction *arg1;
    struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_XOR &&
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *xor_arg1;
        struct kefir_opt_instruction *xor_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &xor_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &xor_arg2));

        kefir_uint64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? (kefir_uint64_t) arg2->operation.parameters.imm.integer
                                        : arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (xor_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand ^= (kefir_uint64_t) xor_arg1->operation.parameters.imm.integer;
            operand_ref = xor_arg2->id;
        } else if (xor_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand ^= xor_arg1->operation.parameters.imm.uinteger;
            operand_ref = xor_arg2->id;
        } else if (xor_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand ^= (kefir_uint64_t) xor_arg2->operation.parameters.imm.integer;
            operand_ref = xor_arg1->id;
        } else if (xor_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand ^= xor_arg2->operation.parameters.imm.uinteger;
            operand_ref = xor_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int_xor(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref));

            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    } else if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_XOR &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *xor_arg1;
        struct kefir_opt_instruction *xor_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[0], &xor_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[1], &xor_arg2));

        kefir_uint64_t imm_operand = arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? (kefir_uint64_t) arg1->operation.parameters.imm.integer
                                        : arg1->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (xor_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand ^= (kefir_uint64_t) xor_arg1->operation.parameters.imm.integer;
            operand_ref = xor_arg2->id;
        } else if (xor_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand ^= xor_arg1->operation.parameters.imm.uinteger;
            operand_ref = xor_arg2->id;
        } else if (xor_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand ^= (kefir_uint64_t) xor_arg2->operation.parameters.imm.integer;
            operand_ref = xor_arg1->id;
        } else if (xor_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand ^= xor_arg2->operation.parameters.imm.uinteger;
            operand_ref = xor_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int_xor(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref));

            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_add(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;
    const kefir_opt_instruction_ref_t instr_id = instr->id;

    struct kefir_opt_instruction *arg1;
    struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    if (arg1->operation.opcode == KEFIR_OPT_OPCODE_GET_LOCAL && arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
        REQUIRE_OK(kefir_opt_code_builder_get_local(
            mem, &func->code, block_id, arg1->operation.parameters.variable.local_index,
            arg1->operation.parameters.variable.offset + arg2->operation.parameters.imm.integer, replacement_ref));

        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_GET_LOCAL &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
        REQUIRE_OK(kefir_opt_code_builder_get_local(
            mem, &func->code, block_id, arg1->operation.parameters.variable.local_index,
            arg1->operation.parameters.variable.offset + (kefir_int64_t) arg2->operation.parameters.imm.uinteger,
            replacement_ref));

        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_GET_LOCAL) {
        REQUIRE_OK(kefir_opt_code_builder_get_local(
            mem, &func->code, block_id, arg2->operation.parameters.variable.local_index,
            arg2->operation.parameters.variable.offset + arg1->operation.parameters.imm.integer, replacement_ref));

        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_GET_LOCAL) {
        REQUIRE_OK(kefir_opt_code_builder_get_local(
            mem, &func->code, block_id, arg2->operation.parameters.variable.local_index,
            arg2->operation.parameters.variable.offset + (kefir_int64_t) arg1->operation.parameters.imm.uinteger,
            replacement_ref));

        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
        REQUIRE_OK(kefir_opt_code_builder_get_global(
            mem, &func->code, block_id, arg1->operation.parameters.variable.global_ref,
            arg1->operation.parameters.variable.offset + arg2->operation.parameters.imm.integer, replacement_ref));

        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
        REQUIRE_OK(kefir_opt_code_builder_get_global(
            mem, &func->code, block_id, arg1->operation.parameters.variable.global_ref,
            arg1->operation.parameters.variable.offset + (kefir_int64_t) arg2->operation.parameters.imm.uinteger,
            replacement_ref));

        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL) {
        REQUIRE_OK(kefir_opt_code_builder_get_global(
            mem, &func->code, block_id, arg2->operation.parameters.variable.global_ref,
            arg2->operation.parameters.variable.offset + arg1->operation.parameters.imm.integer, replacement_ref));

        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL) {
        REQUIRE_OK(kefir_opt_code_builder_get_global(
            mem, &func->code, block_id, arg2->operation.parameters.variable.global_ref,
            arg2->operation.parameters.variable.offset + (kefir_int64_t) arg1->operation.parameters.imm.uinteger,
            replacement_ref));

        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
        REQUIRE_OK(kefir_opt_code_builder_get_thread_local(
            mem, &func->code, block_id, arg1->operation.parameters.variable.global_ref,
            arg1->operation.parameters.variable.offset + arg2->operation.parameters.imm.integer, replacement_ref));

        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
        REQUIRE_OK(kefir_opt_code_builder_get_thread_local(
            mem, &func->code, block_id, arg1->operation.parameters.variable.global_ref,
            arg1->operation.parameters.variable.offset + (kefir_int64_t) arg2->operation.parameters.imm.uinteger,
            replacement_ref));

        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL) {
        REQUIRE_OK(kefir_opt_code_builder_get_thread_local(
            mem, &func->code, block_id, arg2->operation.parameters.variable.global_ref,
            arg2->operation.parameters.variable.offset + arg1->operation.parameters.imm.integer, replacement_ref));

        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL) {
        REQUIRE_OK(kefir_opt_code_builder_get_thread_local(
            mem, &func->code, block_id, arg2->operation.parameters.variable.global_ref,
            arg2->operation.parameters.variable.offset + (kefir_int64_t) arg1->operation.parameters.imm.uinteger,
            replacement_ref));

        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_ADD &&
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *add_arg1;
        struct kefir_opt_instruction *add_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &add_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &add_arg2));

        kefir_int64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg2->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (add_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += add_arg1->operation.parameters.imm.integer;
            operand_ref = add_arg2->id;
        } else if (add_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) add_arg1->operation.parameters.imm.uinteger;
            operand_ref = add_arg2->id;
        } else if (add_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += add_arg2->operation.parameters.imm.integer;
            operand_ref = add_arg1->id;
        } else if (add_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) add_arg2->operation.parameters.imm.uinteger;
            operand_ref = add_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int_add(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref));

            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    } else if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_ADD &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *add_arg1;
        struct kefir_opt_instruction *add_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[0], &add_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[1], &add_arg2));

        kefir_int64_t imm_operand = arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg1->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg1->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (add_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += add_arg1->operation.parameters.imm.integer;
            operand_ref = add_arg2->id;
        } else if (add_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) add_arg1->operation.parameters.imm.uinteger;
            operand_ref = add_arg2->id;
        } else if (add_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += add_arg2->operation.parameters.imm.integer;
            operand_ref = add_arg1->id;
        } else if (add_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) add_arg2->operation.parameters.imm.uinteger;
            operand_ref = add_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int_add(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref));

            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_SUB &&
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *sub_arg1;
        struct kefir_opt_instruction *sub_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &sub_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &sub_arg2));

        kefir_int64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg2->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (sub_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += sub_arg1->operation.parameters.imm.integer;
            operand_ref = sub_arg2->id;
        } else if (sub_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) sub_arg1->operation.parameters.imm.uinteger;
            operand_ref = sub_arg2->id;
        } else if (sub_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand = sub_arg2->operation.parameters.imm.integer - imm_operand;
            operand_ref = sub_arg1->id;
        } else if (sub_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand = (kefir_int64_t) sub_arg2->operation.parameters.imm.uinteger - imm_operand;
            operand_ref = sub_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            if (operand_ref == sub_arg1->id) {
                REQUIRE_OK(kefir_opt_code_builder_int_sub(mem, &func->code, block_id, operand_ref, imm_op_ref,
                                                          replacement_ref));
            } else {
                REQUIRE_OK(kefir_opt_code_builder_int_sub(mem, &func->code, block_id, imm_op_ref, operand_ref,
                                                          replacement_ref));
            }

            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    } else if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_SUB &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *sub_arg1;
        struct kefir_opt_instruction *sub_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[0], &sub_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[1], &sub_arg2));

        kefir_int64_t imm_operand = arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg1->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg1->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (sub_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += sub_arg1->operation.parameters.imm.integer;
            operand_ref = sub_arg2->id;
        } else if (sub_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) sub_arg1->operation.parameters.imm.uinteger;
            operand_ref = sub_arg2->id;
        } else if (sub_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand = sub_arg2->operation.parameters.imm.integer - imm_operand;
            operand_ref = sub_arg1->id;
        } else if (sub_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand = (kefir_int64_t) sub_arg2->operation.parameters.imm.uinteger - imm_operand;
            operand_ref = sub_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            if (operand_ref == sub_arg1->id) {
                REQUIRE_OK(kefir_opt_code_builder_int_sub(mem, &func->code, block_id, operand_ref, imm_op_ref,
                                                          replacement_ref));
            } else {
                REQUIRE_OK(kefir_opt_code_builder_int_sub(mem, &func->code, block_id, imm_op_ref, operand_ref,
                                                          replacement_ref));
            }

            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_sub(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;
    const kefir_opt_instruction_ref_t instr_id = instr->id;

    struct kefir_opt_instruction *arg1;
    struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_ADD && (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                                                               arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *add_arg1;
        struct kefir_opt_instruction *add_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &add_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &add_arg2));

        kefir_int64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg2->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (add_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand -= add_arg1->operation.parameters.imm.integer;
            operand_ref = add_arg2->id;
        } else if (add_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand -= (kefir_int64_t) add_arg1->operation.parameters.imm.uinteger;
            operand_ref = add_arg2->id;
        } else if (add_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand -= add_arg2->operation.parameters.imm.integer;
            operand_ref = add_arg1->id;
        } else if (add_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand -= (kefir_int64_t) add_arg2->operation.parameters.imm.uinteger;
            operand_ref = add_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int_sub(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref));

            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    } else if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_ADD &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *add_arg1;
        struct kefir_opt_instruction *add_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[0], &add_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[1], &add_arg2));

        kefir_int64_t imm_operand = arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg1->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg1->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (add_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand -= add_arg1->operation.parameters.imm.integer;
            operand_ref = add_arg2->id;
        } else if (add_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand -= (kefir_int64_t) add_arg1->operation.parameters.imm.uinteger;
            operand_ref = add_arg2->id;
        } else if (add_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand -= add_arg2->operation.parameters.imm.integer;
            operand_ref = add_arg1->id;
        } else if (add_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand -= (kefir_int64_t) add_arg2->operation.parameters.imm.uinteger;
            operand_ref = add_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int_sub(mem, &func->code, block_id, imm_op_ref, operand_ref, replacement_ref));

            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_SUB &&
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *sub_arg1;
        struct kefir_opt_instruction *sub_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &sub_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &sub_arg2));

        kefir_int64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg2->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (sub_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand = sub_arg1->operation.parameters.imm.integer - imm_operand;
            operand_ref = sub_arg2->id;
        } else if (sub_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand = (kefir_int64_t) sub_arg1->operation.parameters.imm.uinteger - imm_operand;
            operand_ref = sub_arg2->id;
        } else if (sub_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += sub_arg2->operation.parameters.imm.integer;
            operand_ref = sub_arg1->id;
        } else if (sub_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) sub_arg2->operation.parameters.imm.uinteger;
            operand_ref = sub_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            if (operand_ref == sub_arg1->id) {
                REQUIRE_OK(kefir_opt_code_builder_int_sub(mem, &func->code, block_id, operand_ref, imm_op_ref,
                                                          replacement_ref));
            } else {
                REQUIRE_OK(kefir_opt_code_builder_int_sub(mem, &func->code, block_id, imm_op_ref, operand_ref,
                                                          replacement_ref));
            }

            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    } else if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_SUB &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *sub_arg1;
        struct kefir_opt_instruction *sub_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[0], &sub_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[1], &sub_arg2));

        kefir_int64_t imm_operand = arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg1->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg1->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (sub_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand -= sub_arg1->operation.parameters.imm.integer;
            operand_ref = sub_arg2->id;
        } else if (sub_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand -= (kefir_int64_t) sub_arg1->operation.parameters.imm.uinteger;
            operand_ref = sub_arg2->id;
        } else if (sub_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += sub_arg2->operation.parameters.imm.integer;
            operand_ref = sub_arg1->id;
        } else if (sub_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) sub_arg2->operation.parameters.imm.uinteger;
            operand_ref = sub_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            if (operand_ref == sub_arg1->id) {
                REQUIRE_OK(kefir_opt_code_builder_int_sub(mem, &func->code, block_id, imm_op_ref, operand_ref,
                                                          replacement_ref));
            } else {
                REQUIRE_OK(kefir_opt_code_builder_int_add(mem, &func->code, block_id, imm_op_ref, operand_ref,
                                                          replacement_ref));
            }
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_mul(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;
    const kefir_opt_instruction_ref_t instr_id = instr->id;

    struct kefir_opt_instruction *arg1;
    struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_MUL && (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                                                               arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *mul_arg1;
        struct kefir_opt_instruction *mul_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &mul_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &mul_arg2));

        kefir_int64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg2->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (mul_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand *= mul_arg1->operation.parameters.imm.integer;
            operand_ref = mul_arg2->id;
        } else if (mul_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand *= (kefir_int64_t) mul_arg1->operation.parameters.imm.uinteger;
            operand_ref = mul_arg2->id;
        } else if (mul_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand *= mul_arg2->operation.parameters.imm.integer;
            operand_ref = mul_arg1->id;
        } else if (mul_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand *= (kefir_int64_t) mul_arg2->operation.parameters.imm.uinteger;
            operand_ref = mul_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int_mul(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref));

            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    } else if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_MUL &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *mul_arg1;
        struct kefir_opt_instruction *mul_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[0], &mul_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[1], &mul_arg2));

        kefir_int64_t imm_operand = arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg1->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg1->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (mul_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand *= mul_arg1->operation.parameters.imm.integer;
            operand_ref = mul_arg2->id;
        } else if (mul_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand *= (kefir_int64_t) mul_arg1->operation.parameters.imm.uinteger;
            operand_ref = mul_arg2->id;
        } else if (mul_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand *= mul_arg2->operation.parameters.imm.integer;
            operand_ref = mul_arg1->id;
        } else if (mul_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand *= (kefir_int64_t) mul_arg2->operation.parameters.imm.uinteger;
            operand_ref = mul_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int_mul(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref));

            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_shl(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;
    const kefir_opt_instruction_ref_t instr_id = instr->id;

    struct kefir_opt_instruction *arg1;
    struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));
    
    if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_LSHIFT && (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                                                               arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *shl_arg1;
        struct kefir_opt_instruction *shl_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &shl_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &shl_arg2));

        kefir_int64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg2->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (shl_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += shl_arg2->operation.parameters.imm.integer;
            operand_ref = shl_arg1->id;
        } else if (shl_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) shl_arg2->operation.parameters.imm.uinteger;
            operand_ref = shl_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int_lshift(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref));

            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_shr(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;
    const kefir_opt_instruction_ref_t instr_id = instr->id;

    struct kefir_opt_instruction *arg1;
    struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_RSHIFT && (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                                                               arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *shr_arg1;
        struct kefir_opt_instruction *shr_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &shr_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &shr_arg2));

        kefir_int64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg2->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (shr_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += shr_arg2->operation.parameters.imm.integer;
            operand_ref = shr_arg1->id;
        } else if (shr_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) shr_arg2->operation.parameters.imm.uinteger;
            operand_ref = shr_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int_rshift(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref));

            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_sar(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;
    const kefir_opt_instruction_ref_t instr_id = instr->id;

    struct kefir_opt_instruction *arg1;
    struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_ARSHIFT && (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                                                               arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        struct kefir_opt_instruction *sar_arg1;
        struct kefir_opt_instruction *sar_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &sar_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &sar_arg2));

        kefir_int64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg2->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (sar_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += sar_arg2->operation.parameters.imm.integer;
            operand_ref = sar_arg1->id;
        } else if (sar_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) sar_arg2->operation.parameters.imm.uinteger;
            operand_ref = sar_arg1->id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_int_arshift(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref));

            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, imm_op_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, imm_op_ref, *replacement_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t op_simplify_apply(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                        struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass) {
    UNUSED(pass);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct kefir_opt_code_container_iterator iter;
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(&func->code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {

        struct kefir_opt_instruction *instr = NULL;
        for (kefir_opt_code_block_instr_head(&func->code, block, &instr); instr != NULL;) {

            const kefir_opt_instruction_ref_t instr_id = instr->id;
            kefir_opt_instruction_ref_t replacement_ref = KEFIR_ID_NONE;
            switch (instr->operation.opcode) {
                case KEFIR_OPT_OPCODE_BOOL_NOT:
                    REQUIRE_OK(simplify_boot_not(mem, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_BOOL_OR:
                    REQUIRE_OK(simplify_boot_or(mem, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_INT_AND:
                    REQUIRE_OK(simplify_int_and(mem, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_INT_OR:
                    REQUIRE_OK(simplify_int_or(mem, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_INT_XOR:
                    REQUIRE_OK(simplify_int_xor(mem, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_INT_ADD:
                    REQUIRE_OK(simplify_int_add(mem, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_INT_SUB:
                    REQUIRE_OK(simplify_int_sub(mem, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_INT_MUL:
                    REQUIRE_OK(simplify_int_mul(mem, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_INT_LSHIFT:
                    REQUIRE_OK(simplify_int_shl(mem, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_INT_RSHIFT:
                    REQUIRE_OK(simplify_int_shr(mem, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_INT_ARSHIFT:
                    REQUIRE_OK(simplify_int_sar(mem, func, instr, &replacement_ref));
                    break;

                default:
                    // Intentionally left blank
                    break;
            }

            if (replacement_ref != KEFIR_ID_NONE) {
                REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_id, &instr));
                REQUIRE_OK(kefir_opt_code_container_replace_references(&func->code, replacement_ref, instr_id));
                if (instr->control_flow.prev != KEFIR_ID_NONE || instr->control_flow.next != KEFIR_ID_NONE) {
                    struct kefir_opt_instruction *replacement_instr = NULL;
                    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, replacement_ref, &replacement_instr));
                    if (replacement_instr->control_flow.prev == KEFIR_ID_NONE &&
                        replacement_instr->control_flow.next == KEFIR_ID_NONE) {
                        REQUIRE_OK(
                            kefir_opt_code_container_insert_control(&func->code, block, instr_id, replacement_ref));
                    }
                    REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));
                }
                REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_id, &instr));
                REQUIRE_OK(kefir_opt_instruction_next_sibling(&func->code, instr, &instr));
                REQUIRE_OK(kefir_opt_code_container_drop_instr(&func->code, instr_id));
            } else {
                REQUIRE_OK(kefir_opt_instruction_next_sibling(&func->code, instr, &instr));
            }
        }
    }
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassOpSimplify = {
    .name = "op-simplify", .apply = op_simplify_apply, .payload = NULL};
