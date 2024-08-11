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

union constant {
    kefir_int64_t integer;
    kefir_uint64_t uinteger;
};

static kefir_result_t int_unary_const_fold(struct kefir_mem *mem, struct kefir_opt_function *func,
                                           const struct kefir_opt_instruction *instr,
                                           kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;
    const kefir_opt_instruction_ref_t instr_id = instr->id;

    const struct kefir_opt_instruction *arg1;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));

    kefir_bool_t unsigned_arg = false;
    union constant arg, result;
    if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
        arg.integer = arg1->operation.parameters.imm.integer;
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
        arg.uinteger = arg1->operation.parameters.imm.uinteger;
        unsigned_arg = true;
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_PLACEHOLDER) {
        REQUIRE_OK(kefir_opt_code_builder_int_placeholder(mem, &func->code, block_id, replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
        return KEFIR_OK;
    } else {
        return KEFIR_OK;
    }

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_TO_BOOL:
            result.uinteger = ((kefir_uint8_t) arg.uinteger) != 0 ? 1 : 0;
            break;

        case KEFIR_OPT_OPCODE_INT16_TO_BOOL:
            result.uinteger = ((kefir_uint16_t) arg.uinteger) != 0 ? 1 : 0;
            break;

        case KEFIR_OPT_OPCODE_INT32_TO_BOOL:
            result.uinteger = ((kefir_uint32_t) arg.uinteger) != 0 ? 1 : 0;
            break;

        case KEFIR_OPT_OPCODE_INT64_TO_BOOL:
            result.uinteger = ((kefir_uint64_t) arg.uinteger) != 0 ? 1 : 0;
            break;

        case KEFIR_OPT_OPCODE_INT8_NEG:
            result.integer = -(kefir_int64_t) (kefir_uint8_t) arg.integer;
            break;

        case KEFIR_OPT_OPCODE_INT16_NEG:
            result.integer = -(kefir_int64_t) (kefir_uint16_t) arg.integer;
            break;

        case KEFIR_OPT_OPCODE_INT32_NEG:
            result.integer = -(kefir_int64_t) (kefir_uint32_t) arg.integer;
            break;

        case KEFIR_OPT_OPCODE_INT64_NEG:
            result.integer = -arg.integer;
            break;

        case KEFIR_OPT_OPCODE_INT8_NOT:
        case KEFIR_OPT_OPCODE_INT16_NOT:
        case KEFIR_OPT_OPCODE_INT32_NOT:
        case KEFIR_OPT_OPCODE_INT64_NOT:
            result.uinteger = ~arg.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT8_BOOL_NOT:
            result.uinteger = ((kefir_uint8_t) arg.uinteger) == 0;
            break;

        case KEFIR_OPT_OPCODE_INT16_BOOL_NOT:
            result.uinteger = ((kefir_uint16_t) arg.uinteger) == 0;
            break;

        case KEFIR_OPT_OPCODE_INT32_BOOL_NOT:
            result.uinteger = ((kefir_uint32_t) arg.uinteger) == 0;
            break;

        case KEFIR_OPT_OPCODE_INT64_BOOL_NOT:
            result.uinteger = ((kefir_uint64_t) arg.uinteger) == 0;
            break;

        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS:
            result.uinteger = (kefir_uint8_t) arg.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS:
            result.uinteger = (kefir_uint16_t) arg.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS:
            result.uinteger = (kefir_uint32_t) arg.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS:
            result.integer = (kefir_int8_t) arg.integer;
            break;

        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS:
            result.integer = (kefir_int16_t) arg.integer;
            break;

        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS:
            result.integer = (kefir_int32_t) arg.integer;
            break;

        default:
            return KEFIR_OK;
    }

    if (!unsigned_arg) {
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, result.integer, replacement_ref));
    } else {
        REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, result.uinteger, replacement_ref));
    }
    REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));

    return KEFIR_OK;
}

static kefir_result_t int_binary_const_fold(struct kefir_mem *mem, struct kefir_opt_function *func,
                                            const struct kefir_opt_instruction *instr,
                                            kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;
    const kefir_opt_instruction_ref_t instr_id = instr->id;

    const struct kefir_opt_instruction *arg1;
    const struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    kefir_bool_t unsigned_arg = false;
    union constant left, right, result;
    if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
        left.integer = arg1->operation.parameters.imm.integer;
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
        left.uinteger = arg1->operation.parameters.imm.uinteger;
        unsigned_arg = true;
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_PLACEHOLDER) {
        REQUIRE_OK(kefir_opt_code_builder_int_placeholder(mem, &func->code, block_id, replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));
        return KEFIR_OK;
    } else {
        return KEFIR_OK;
    }
    if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
        right.integer = arg2->operation.parameters.imm.integer;
    } else if (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
        right.uinteger = arg2->operation.parameters.imm.uinteger;
        unsigned_arg = true;
    } else if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_PLACEHOLDER) {
        right.integer = 0;
    } else {
        return KEFIR_OK;
    }

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_ADD:
            result.uinteger = ((kefir_uint8_t) left.uinteger) + ((kefir_uint8_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT16_ADD:
            result.uinteger = ((kefir_uint16_t) left.uinteger) + ((kefir_uint16_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT32_ADD:
            result.uinteger = ((kefir_uint32_t) left.uinteger) + ((kefir_uint32_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT64_ADD:
            result.uinteger = ((kefir_uint64_t) left.uinteger) + ((kefir_uint64_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT8_SUB:
            result.uinteger = ((kefir_uint8_t) left.uinteger) - ((kefir_uint8_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT16_SUB:
            result.uinteger = ((kefir_uint16_t) left.uinteger) - ((kefir_uint16_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT32_SUB:
            result.uinteger = ((kefir_uint32_t) left.uinteger) - ((kefir_uint32_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT64_SUB:
            result.uinteger = ((kefir_uint64_t) left.uinteger) - ((kefir_uint64_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT8_MUL:
            result.uinteger = ((kefir_uint8_t) left.uinteger) * ((kefir_uint8_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT16_MUL:
            result.uinteger = ((kefir_uint16_t) left.uinteger) * ((kefir_uint16_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT32_MUL:
            result.uinteger = ((kefir_uint32_t) left.uinteger) * ((kefir_uint32_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT64_MUL:
            result.uinteger = ((kefir_uint64_t) left.uinteger) * ((kefir_uint64_t) right.uinteger);
            break;

#define DIV_MOD_PRECONDITIONS(_type, _min) \
    ((((_type) right.integer) != 0) && (((_type) left.integer) != (_min) || ((_type) right.integer) != -1))
        case KEFIR_OPT_OPCODE_INT8_DIV:
            REQUIRE(DIV_MOD_PRECONDITIONS(kefir_int8_t, KEFIR_INT8_MIN), KEFIR_OK);
            result.integer = ((kefir_int8_t) left.integer) / ((kefir_int8_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT16_DIV:
            REQUIRE(DIV_MOD_PRECONDITIONS(kefir_int16_t, KEFIR_INT16_MIN), KEFIR_OK);
            result.integer = ((kefir_int16_t) left.integer) / ((kefir_int16_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT32_DIV:
            REQUIRE(DIV_MOD_PRECONDITIONS(kefir_int32_t, KEFIR_INT32_MIN), KEFIR_OK);
            result.integer = ((kefir_int32_t) left.integer) / ((kefir_int32_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT64_DIV:
            REQUIRE(DIV_MOD_PRECONDITIONS(kefir_int64_t, KEFIR_INT64_MIN), KEFIR_OK);
            result.integer = ((kefir_int64_t) left.integer) / ((kefir_int64_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT8_MOD:
            REQUIRE(DIV_MOD_PRECONDITIONS(kefir_int8_t, KEFIR_INT8_MIN), KEFIR_OK);
            result.integer = ((kefir_int8_t) left.integer) % ((kefir_int8_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT16_MOD:
            REQUIRE(DIV_MOD_PRECONDITIONS(kefir_int16_t, KEFIR_INT16_MIN), KEFIR_OK);
            result.integer = ((kefir_int16_t) left.integer) % ((kefir_int16_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT32_MOD:
            REQUIRE(DIV_MOD_PRECONDITIONS(kefir_int32_t, KEFIR_INT32_MIN), KEFIR_OK);
            result.integer = ((kefir_int32_t) left.integer) % ((kefir_int32_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT64_MOD:
            REQUIRE(DIV_MOD_PRECONDITIONS(kefir_int64_t, KEFIR_INT64_MIN), KEFIR_OK);
            result.integer = ((kefir_int64_t) left.integer) % ((kefir_int64_t) right.integer);
            break;
#undef DIV_MOD_PRECONDITIONS

        case KEFIR_OPT_OPCODE_UINT8_DIV:
            REQUIRE(((kefir_uint8_t) right.uinteger) != 0, KEFIR_OK);
            result.uinteger = ((kefir_uint8_t) left.uinteger) / ((kefir_uint8_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_UINT16_DIV:
            REQUIRE(((kefir_uint16_t) right.uinteger) != 0, KEFIR_OK);
            result.uinteger = ((kefir_uint16_t) left.uinteger) / ((kefir_uint16_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_UINT32_DIV:
            REQUIRE(((kefir_uint32_t) right.uinteger) != 0, KEFIR_OK);
            result.uinteger = ((kefir_uint32_t) left.uinteger) / ((kefir_uint32_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_UINT64_DIV:
            REQUIRE(((kefir_uint64_t) right.uinteger) != 0, KEFIR_OK);
            result.uinteger = ((kefir_uint64_t) left.uinteger) / ((kefir_uint64_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_UINT8_MOD:
            REQUIRE((kefir_uint8_t) right.integer != 0, KEFIR_OK);
            result.uinteger = ((kefir_uint8_t) left.uinteger) % ((kefir_uint8_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_UINT16_MOD:
            REQUIRE((kefir_uint16_t) right.integer != 0, KEFIR_OK);
            result.uinteger = ((kefir_uint16_t) left.uinteger) % ((kefir_uint16_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_UINT32_MOD:
            REQUIRE((kefir_uint32_t) right.integer != 0, KEFIR_OK);
            result.uinteger = ((kefir_uint32_t) left.uinteger) % ((kefir_uint32_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_UINT64_MOD:
            REQUIRE((kefir_uint64_t) right.integer != 0, KEFIR_OK);
            result.uinteger = ((kefir_uint64_t) left.uinteger) % ((kefir_uint64_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT8_AND:
        case KEFIR_OPT_OPCODE_INT16_AND:
        case KEFIR_OPT_OPCODE_INT32_AND:
        case KEFIR_OPT_OPCODE_INT64_AND:
            result.uinteger = left.uinteger & right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT8_OR:
        case KEFIR_OPT_OPCODE_INT16_OR:
        case KEFIR_OPT_OPCODE_INT32_OR:
        case KEFIR_OPT_OPCODE_INT64_OR:
            result.uinteger = left.uinteger | right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT8_XOR:
        case KEFIR_OPT_OPCODE_INT16_XOR:
        case KEFIR_OPT_OPCODE_INT32_XOR:
        case KEFIR_OPT_OPCODE_INT64_XOR:
            result.uinteger = left.uinteger ^ right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT8_LSHIFT:
            result.uinteger = ((kefir_uint8_t) left.uinteger) << right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT16_LSHIFT:
            result.uinteger = ((kefir_uint16_t) left.uinteger) << right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT32_LSHIFT:
            result.uinteger = ((kefir_uint32_t) left.uinteger) << right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT64_LSHIFT:
            result.uinteger = ((kefir_uint64_t) left.uinteger) << right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT8_RSHIFT:
            result.uinteger = ((kefir_uint8_t) left.uinteger) >> right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT16_RSHIFT:
            result.uinteger = ((kefir_uint16_t) left.uinteger) >> right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT32_RSHIFT:
            result.uinteger = ((kefir_uint32_t) left.uinteger) >> right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT64_RSHIFT:
            result.uinteger = ((kefir_uint64_t) left.uinteger) >> right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT8_ARSHIFT:
            result.integer = ((kefir_int8_t) left.integer) >> right.integer;
            break;

        case KEFIR_OPT_OPCODE_INT16_ARSHIFT:
            result.integer = ((kefir_int16_t) left.integer) >> right.integer;
            break;

        case KEFIR_OPT_OPCODE_INT32_ARSHIFT:
            result.integer = ((kefir_int32_t) left.integer) >> right.integer;
            break;

        case KEFIR_OPT_OPCODE_INT64_ARSHIFT:
            result.integer = ((kefir_int64_t) left.integer) >> right.integer;
            break;

        case KEFIR_OPT_OPCODE_INT8_EQUALS:
            result.integer = ((kefir_uint8_t) left.integer) == ((kefir_uint8_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT16_EQUALS:
            result.integer = ((kefir_uint16_t) left.integer) == ((kefir_uint16_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT32_EQUALS:
            result.integer = ((kefir_uint32_t) left.integer) == ((kefir_uint32_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT64_EQUALS:
            result.integer = ((kefir_uint64_t) left.integer) == ((kefir_uint64_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT8_GREATER:
            result.integer = ((kefir_int8_t) left.integer) > ((kefir_int8_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT16_GREATER:
            result.integer = ((kefir_int16_t) left.integer) > ((kefir_int16_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT32_GREATER:
            result.integer = ((kefir_int32_t) left.integer) > ((kefir_int32_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT64_GREATER:
            result.integer = ((kefir_int64_t) left.integer) > ((kefir_int64_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT8_LESSER:
            result.integer = ((kefir_int8_t) left.integer) < ((kefir_int8_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT16_LESSER:
            result.integer = ((kefir_int16_t) left.integer) < ((kefir_int16_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT32_LESSER:
            result.integer = ((kefir_int32_t) left.integer) < ((kefir_int32_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT64_LESSER:
            result.integer = ((kefir_int64_t) left.integer) < ((kefir_int64_t) right.integer);
            break;

        case KEFIR_OPT_OPCODE_INT8_ABOVE:
            result.integer = ((kefir_uint8_t) left.uinteger) > ((kefir_uint8_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT16_ABOVE:
            result.integer = ((kefir_uint16_t) left.uinteger) > ((kefir_uint16_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT32_ABOVE:
            result.integer = ((kefir_uint32_t) left.uinteger) > ((kefir_uint32_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT64_ABOVE:
            result.integer = ((kefir_uint64_t) left.uinteger) > ((kefir_uint64_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT8_BELOW:
            result.integer = ((kefir_uint8_t) left.uinteger) < ((kefir_uint8_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT16_BELOW:
            result.integer = ((kefir_uint16_t) left.uinteger) < ((kefir_uint16_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT32_BELOW:
            result.integer = ((kefir_uint32_t) left.uinteger) < ((kefir_uint32_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT64_BELOW:
            result.integer = ((kefir_uint64_t) left.uinteger) < ((kefir_uint64_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT8_BOOL_OR:
            result.integer = ((kefir_uint8_t) left.uinteger) != 0 || ((kefir_uint8_t) right.uinteger != 0);
            break;

        case KEFIR_OPT_OPCODE_INT16_BOOL_OR:
            result.integer = ((kefir_uint16_t) left.uinteger) != 0 || ((kefir_uint16_t) right.uinteger != 0);
            break;

        case KEFIR_OPT_OPCODE_INT32_BOOL_OR:
            result.integer = ((kefir_uint32_t) left.uinteger) != 0 || ((kefir_uint32_t) right.uinteger != 0);
            break;

        case KEFIR_OPT_OPCODE_INT64_BOOL_OR:
            result.integer = ((kefir_uint64_t) left.uinteger) != 0 || ((kefir_uint64_t) right.uinteger != 0);
            break;

        case KEFIR_OPT_OPCODE_INT8_BOOL_AND:
            result.integer = ((kefir_uint8_t) left.uinteger) != 0 && ((kefir_uint8_t) right.uinteger != 0);
            break;

        case KEFIR_OPT_OPCODE_INT16_BOOL_AND:
            result.integer = ((kefir_uint16_t) left.uinteger) != 0 && ((kefir_uint16_t) right.uinteger != 0);
            break;

        case KEFIR_OPT_OPCODE_INT32_BOOL_AND:
            result.integer = ((kefir_uint32_t) left.uinteger) != 0 && ((kefir_uint32_t) right.uinteger != 0);
            break;

        case KEFIR_OPT_OPCODE_INT64_BOOL_AND:
            result.integer = ((kefir_uint64_t) left.uinteger) != 0 && ((kefir_uint64_t) right.uinteger != 0);
            break;

        default:
            return KEFIR_OK;
    }

    if (!unsigned_arg) {
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, result.integer, replacement_ref));
    } else {
        REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, result.uinteger, replacement_ref));
    }
    REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr_id, *replacement_ref));

    return KEFIR_OK;
}

static kefir_result_t const_fold_apply(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                       struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass) {
    UNUSED(pass);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct kefir_opt_code_container_iterator iter;
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(&func->code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {

        kefir_opt_instruction_ref_t instr_id;
        const struct kefir_opt_instruction *instr = NULL;

        for (kefir_opt_code_block_instr_head(&func->code, block, &instr_id); instr_id != KEFIR_ID_NONE;) {

            REQUIRE_OK(kefir_opt_code_container_set_ir_instruction_index_of(&func->code, instr_id));
            REQUIRE_OK(kefir_opt_code_container_set_source_location_cursor_of(&func->code, instr_id));

            REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_id, &instr));
            kefir_opt_instruction_ref_t replacement_ref = KEFIR_ID_NONE;
            switch (instr->operation.opcode) {
                case KEFIR_OPT_OPCODE_INT8_ADD:
                case KEFIR_OPT_OPCODE_INT16_ADD:
                case KEFIR_OPT_OPCODE_INT32_ADD:
                case KEFIR_OPT_OPCODE_INT64_ADD:
                case KEFIR_OPT_OPCODE_INT8_SUB:
                case KEFIR_OPT_OPCODE_INT16_SUB:
                case KEFIR_OPT_OPCODE_INT32_SUB:
                case KEFIR_OPT_OPCODE_INT64_SUB:
                case KEFIR_OPT_OPCODE_INT8_MUL:
                case KEFIR_OPT_OPCODE_INT16_MUL:
                case KEFIR_OPT_OPCODE_INT32_MUL:
                case KEFIR_OPT_OPCODE_INT64_MUL:
                case KEFIR_OPT_OPCODE_INT8_DIV:
                case KEFIR_OPT_OPCODE_INT16_DIV:
                case KEFIR_OPT_OPCODE_INT32_DIV:
                case KEFIR_OPT_OPCODE_INT64_DIV:
                case KEFIR_OPT_OPCODE_INT8_MOD:
                case KEFIR_OPT_OPCODE_INT16_MOD:
                case KEFIR_OPT_OPCODE_INT32_MOD:
                case KEFIR_OPT_OPCODE_INT64_MOD:
                case KEFIR_OPT_OPCODE_UINT8_DIV:
                case KEFIR_OPT_OPCODE_UINT16_DIV:
                case KEFIR_OPT_OPCODE_UINT32_DIV:
                case KEFIR_OPT_OPCODE_UINT64_DIV:
                case KEFIR_OPT_OPCODE_UINT8_MOD:
                case KEFIR_OPT_OPCODE_UINT16_MOD:
                case KEFIR_OPT_OPCODE_UINT32_MOD:
                case KEFIR_OPT_OPCODE_UINT64_MOD:
                case KEFIR_OPT_OPCODE_INT8_AND:
                case KEFIR_OPT_OPCODE_INT16_AND:
                case KEFIR_OPT_OPCODE_INT32_AND:
                case KEFIR_OPT_OPCODE_INT64_AND:
                case KEFIR_OPT_OPCODE_INT8_OR:
                case KEFIR_OPT_OPCODE_INT16_OR:
                case KEFIR_OPT_OPCODE_INT32_OR:
                case KEFIR_OPT_OPCODE_INT64_OR:
                case KEFIR_OPT_OPCODE_INT8_XOR:
                case KEFIR_OPT_OPCODE_INT16_XOR:
                case KEFIR_OPT_OPCODE_INT32_XOR:
                case KEFIR_OPT_OPCODE_INT64_XOR:
                case KEFIR_OPT_OPCODE_INT8_LSHIFT:
                case KEFIR_OPT_OPCODE_INT16_LSHIFT:
                case KEFIR_OPT_OPCODE_INT32_LSHIFT:
                case KEFIR_OPT_OPCODE_INT64_LSHIFT:
                case KEFIR_OPT_OPCODE_INT8_RSHIFT:
                case KEFIR_OPT_OPCODE_INT16_RSHIFT:
                case KEFIR_OPT_OPCODE_INT32_RSHIFT:
                case KEFIR_OPT_OPCODE_INT64_RSHIFT:
                case KEFIR_OPT_OPCODE_INT8_ARSHIFT:
                case KEFIR_OPT_OPCODE_INT16_ARSHIFT:
                case KEFIR_OPT_OPCODE_INT32_ARSHIFT:
                case KEFIR_OPT_OPCODE_INT64_ARSHIFT:
                case KEFIR_OPT_OPCODE_INT8_EQUALS:
                case KEFIR_OPT_OPCODE_INT16_EQUALS:
                case KEFIR_OPT_OPCODE_INT32_EQUALS:
                case KEFIR_OPT_OPCODE_INT64_EQUALS:
                case KEFIR_OPT_OPCODE_INT8_GREATER:
                case KEFIR_OPT_OPCODE_INT16_GREATER:
                case KEFIR_OPT_OPCODE_INT32_GREATER:
                case KEFIR_OPT_OPCODE_INT64_GREATER:
                case KEFIR_OPT_OPCODE_INT8_LESSER:
                case KEFIR_OPT_OPCODE_INT16_LESSER:
                case KEFIR_OPT_OPCODE_INT32_LESSER:
                case KEFIR_OPT_OPCODE_INT64_LESSER:
                case KEFIR_OPT_OPCODE_INT8_ABOVE:
                case KEFIR_OPT_OPCODE_INT16_ABOVE:
                case KEFIR_OPT_OPCODE_INT32_ABOVE:
                case KEFIR_OPT_OPCODE_INT64_ABOVE:
                case KEFIR_OPT_OPCODE_INT8_BELOW:
                case KEFIR_OPT_OPCODE_INT16_BELOW:
                case KEFIR_OPT_OPCODE_INT32_BELOW:
                case KEFIR_OPT_OPCODE_INT64_BELOW:
                    REQUIRE_OK(int_binary_const_fold(mem, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_INT8_NEG:
                case KEFIR_OPT_OPCODE_INT16_NEG:
                case KEFIR_OPT_OPCODE_INT32_NEG:
                case KEFIR_OPT_OPCODE_INT64_NEG:
                case KEFIR_OPT_OPCODE_INT8_NOT:
                case KEFIR_OPT_OPCODE_INT16_NOT:
                case KEFIR_OPT_OPCODE_INT32_NOT:
                case KEFIR_OPT_OPCODE_INT64_NOT:
                case KEFIR_OPT_OPCODE_INT8_BOOL_NOT:
                case KEFIR_OPT_OPCODE_INT16_BOOL_NOT:
                case KEFIR_OPT_OPCODE_INT32_BOOL_NOT:
                case KEFIR_OPT_OPCODE_INT64_BOOL_NOT:
                case KEFIR_OPT_OPCODE_INT8_TO_BOOL:
                case KEFIR_OPT_OPCODE_INT16_TO_BOOL:
                case KEFIR_OPT_OPCODE_INT32_TO_BOOL:
                case KEFIR_OPT_OPCODE_INT64_TO_BOOL:
                case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS:
                case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS:
                case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS:
                case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS:
                case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS:
                case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS:
                    REQUIRE_OK(int_unary_const_fold(mem, func, instr, &replacement_ref));
                    break;

                default:
                    // Intentionally left blank
                    break;
            }

            if (replacement_ref != KEFIR_ID_NONE) {
                REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_id, &instr));
                REQUIRE_OK(kefir_opt_code_container_replace_references(mem, &func->code, replacement_ref, instr->id));
                if (instr->control_flow.prev != KEFIR_ID_NONE || instr->control_flow.next != KEFIR_ID_NONE) {
                    const struct kefir_opt_instruction *replacement_instr = NULL;
                    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, replacement_ref, &replacement_instr));
                    if (replacement_instr->control_flow.prev == KEFIR_ID_NONE &&
                        replacement_instr->control_flow.next == KEFIR_ID_NONE) {
                        REQUIRE_OK(
                            kefir_opt_code_container_insert_control(&func->code, block->id, instr_id, replacement_ref));
                    }
                    REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));
                }
                kefir_opt_instruction_ref_t prev_instr_id = instr_id;
                REQUIRE_OK(kefir_opt_instruction_next_sibling(&func->code, instr_id, &instr_id));
                REQUIRE_OK(kefir_opt_code_container_drop_instr(&func->code, prev_instr_id));
            } else {
                REQUIRE_OK(kefir_opt_instruction_next_sibling(&func->code, instr_id, &instr_id));
            }

            REQUIRE_OK(
                kefir_opt_code_container_set_ir_instruction_index(&func->code, KEFIR_OPT_IR_INSTRUCTION_INDEX_NONE));
            REQUIRE_OK(kefir_opt_code_container_set_source_location_cursor(mem, &func->code, NULL));
        }
    }
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassConstFold = {
    .name = "constant-fold", .apply = const_fold_apply, .payload = NULL};
