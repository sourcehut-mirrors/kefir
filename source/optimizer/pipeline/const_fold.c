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

    return KEFIR_OK;
}

static kefir_result_t int_extract_bits_const_fold(struct kefir_mem *mem, struct kefir_opt_function *func,
                                                  const struct kefir_opt_instruction *instr,
                                                  kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;

    const struct kefir_opt_instruction *arg1;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));

    if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST || arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
        kefir_uint64_t value = arg1->operation.parameters.imm.uinteger;
        value >>= instr->operation.parameters.bitfield.offset;

        kefir_bool_t sign_extend = false;
        if (instr->operation.opcode == KEFIR_OPT_OPCODE_BITS_EXTRACT_SIGNED &&
            instr->operation.parameters.bitfield.length > 0) {
            sign_extend = (value >> (instr->operation.parameters.bitfield.length - 1)) & 1;
        }

        if (instr->operation.parameters.bitfield.length < CHAR_BIT * sizeof(kefir_uint64_t)) {
            if (sign_extend) {
                value |= ~((1ull << instr->operation.parameters.bitfield.length) - 1);
            } else {
                value &= (1ull << instr->operation.parameters.bitfield.length) - 1;
            }
        }

        REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, value, replacement_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t int_binary_const_fold(struct kefir_mem *mem, struct kefir_opt_function *func,
                                            const struct kefir_opt_instruction *instr,
                                            kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;

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
        case KEFIR_OPT_OPCODE_UINT8_MUL:
            result.uinteger = ((kefir_uint8_t) left.uinteger) * ((kefir_uint8_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT16_MUL:
        case KEFIR_OPT_OPCODE_UINT16_MUL:
            result.uinteger = ((kefir_uint16_t) left.uinteger) * ((kefir_uint16_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT32_MUL:
        case KEFIR_OPT_OPCODE_UINT32_MUL:
            result.uinteger = ((kefir_uint32_t) left.uinteger) * ((kefir_uint32_t) right.uinteger);
            break;

        case KEFIR_OPT_OPCODE_INT64_MUL:
        case KEFIR_OPT_OPCODE_UINT64_MUL:
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

        case KEFIR_OPT_OPCODE_SCALAR_COMPARE:
            switch (instr->operation.parameters.comparison) {
                case KEFIR_OPT_COMPARISON_INT8_EQUALS:
                    result.integer = ((kefir_uint8_t) left.integer) == ((kefir_uint8_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT16_EQUALS:
                    result.integer = ((kefir_uint16_t) left.integer) == ((kefir_uint16_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT32_EQUALS:
                    result.integer = ((kefir_uint32_t) left.integer) == ((kefir_uint32_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT64_EQUALS:
                    result.integer = ((kefir_uint64_t) left.integer) == ((kefir_uint64_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT8_NOT_EQUALS:
                    result.integer = ((kefir_uint8_t) left.integer) != ((kefir_uint8_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT16_NOT_EQUALS:
                    result.integer = ((kefir_uint16_t) left.integer) != ((kefir_uint16_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT32_NOT_EQUALS:
                    result.integer = ((kefir_uint32_t) left.integer) != ((kefir_uint32_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT64_NOT_EQUALS:
                    result.integer = ((kefir_uint64_t) left.integer) != ((kefir_uint64_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT8_GREATER:
                    result.integer = ((kefir_int8_t) left.integer) > ((kefir_int8_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT16_GREATER:
                    result.integer = ((kefir_int16_t) left.integer) > ((kefir_int16_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT32_GREATER:
                    result.integer = ((kefir_int32_t) left.integer) > ((kefir_int32_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT64_GREATER:
                    result.integer = ((kefir_int64_t) left.integer) > ((kefir_int64_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT8_GREATER_OR_EQUALS:
                    result.integer = ((kefir_int8_t) left.integer) >= ((kefir_int8_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT16_GREATER_OR_EQUALS:
                    result.integer = ((kefir_int16_t) left.integer) >= ((kefir_int16_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT32_GREATER_OR_EQUALS:
                    result.integer = ((kefir_int32_t) left.integer) >= ((kefir_int32_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT64_GREATER_OR_EQUALS:
                    result.integer = ((kefir_int64_t) left.integer) >= ((kefir_int64_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT8_LESSER:
                    result.integer = ((kefir_int8_t) left.integer) < ((kefir_int8_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT16_LESSER:
                    result.integer = ((kefir_int16_t) left.integer) < ((kefir_int16_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT32_LESSER:
                    result.integer = ((kefir_int32_t) left.integer) < ((kefir_int32_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT64_LESSER:
                    result.integer = ((kefir_int64_t) left.integer) < ((kefir_int64_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT8_LESSER_OR_EQUALS:
                    result.integer = ((kefir_int8_t) left.integer) <= ((kefir_int8_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT16_LESSER_OR_EQUALS:
                    result.integer = ((kefir_int16_t) left.integer) <= ((kefir_int16_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT32_LESSER_OR_EQUALS:
                    result.integer = ((kefir_int32_t) left.integer) <= ((kefir_int32_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT64_LESSER_OR_EQUALS:
                    result.integer = ((kefir_int64_t) left.integer) <= ((kefir_int64_t) right.integer);
                    break;

                case KEFIR_OPT_COMPARISON_INT8_ABOVE:
                    result.integer = ((kefir_uint8_t) left.uinteger) > ((kefir_uint8_t) right.uinteger);
                    break;

                case KEFIR_OPT_COMPARISON_INT16_ABOVE:
                    result.integer = ((kefir_uint16_t) left.uinteger) > ((kefir_uint16_t) right.uinteger);
                    break;

                case KEFIR_OPT_COMPARISON_INT32_ABOVE:
                    result.integer = ((kefir_uint32_t) left.uinteger) > ((kefir_uint32_t) right.uinteger);
                    break;

                case KEFIR_OPT_COMPARISON_INT64_ABOVE:
                    result.integer = ((kefir_uint64_t) left.uinteger) > ((kefir_uint64_t) right.uinteger);
                    break;

                case KEFIR_OPT_COMPARISON_INT8_ABOVE_OR_EQUALS:
                    result.integer = ((kefir_uint8_t) left.uinteger) >= ((kefir_uint8_t) right.uinteger);
                    break;

                case KEFIR_OPT_COMPARISON_INT16_ABOVE_OR_EQUALS:
                    result.integer = ((kefir_uint16_t) left.uinteger) >= ((kefir_uint16_t) right.uinteger);
                    break;

                case KEFIR_OPT_COMPARISON_INT32_ABOVE_OR_EQUALS:
                    result.integer = ((kefir_uint32_t) left.uinteger) >= ((kefir_uint32_t) right.uinteger);
                    break;

                case KEFIR_OPT_COMPARISON_INT64_ABOVE_OR_EQUALS:
                    result.integer = ((kefir_uint64_t) left.uinteger) >= ((kefir_uint64_t) right.uinteger);
                    break;

                case KEFIR_OPT_COMPARISON_INT8_BELOW:
                    result.integer = ((kefir_uint8_t) left.uinteger) < ((kefir_uint8_t) right.uinteger);
                    break;

                case KEFIR_OPT_COMPARISON_INT16_BELOW:
                    result.integer = ((kefir_uint16_t) left.uinteger) < ((kefir_uint16_t) right.uinteger);
                    break;

                case KEFIR_OPT_COMPARISON_INT32_BELOW:
                    result.integer = ((kefir_uint32_t) left.uinteger) < ((kefir_uint32_t) right.uinteger);
                    break;

                case KEFIR_OPT_COMPARISON_INT64_BELOW:
                    result.integer = ((kefir_uint64_t) left.uinteger) < ((kefir_uint64_t) right.uinteger);
                    break;

                case KEFIR_OPT_COMPARISON_INT8_BELOW_OR_EQUALS:
                    result.integer = ((kefir_uint8_t) left.uinteger) <= ((kefir_uint8_t) right.uinteger);
                    break;

                case KEFIR_OPT_COMPARISON_INT16_BELOW_OR_EQUALS:
                    result.integer = ((kefir_uint16_t) left.uinteger) <= ((kefir_uint16_t) right.uinteger);
                    break;

                case KEFIR_OPT_COMPARISON_INT32_BELOW_OR_EQUALS:
                    result.integer = ((kefir_uint32_t) left.uinteger) <= ((kefir_uint32_t) right.uinteger);
                    break;

                case KEFIR_OPT_COMPARISON_INT64_BELOW_OR_EQUALS:
                    result.integer = ((kefir_uint64_t) left.uinteger) <= ((kefir_uint64_t) right.uinteger);
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer comparison operation");
            }
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

    return KEFIR_OK;
}

static kefir_result_t simplify_bitint_from(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                           struct kefir_opt_function *func,
                                           const struct kefir_opt_instruction *bitint_from_instr,
                                           kefir_opt_instruction_ref_t *replacement_ref) {
    const struct kefir_opt_instruction *source_instr;
    REQUIRE_OK(
        kefir_opt_code_container_instr(&func->code, bitint_from_instr->operation.parameters.refs[0], &source_instr));

    const kefir_bool_t signed_cast = bitint_from_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_FROM_SIGNED;

    kefir_bool_t ready = false;
    struct kefir_bigint bigint;
    kefir_result_t res = KEFIR_OK;
    if ((source_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
         source_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {
        REQUIRE_OK(kefir_bigint_init(&bigint));
        ready = true;
        res = kefir_bigint_resize_nocast(mem, &bigint, bitint_from_instr->operation.parameters.bitwidth);
        if (signed_cast) {
            REQUIRE_CHAIN(&res, kefir_bigint_set_signed_value(&bigint, source_instr->operation.parameters.imm.integer));
        } else {
            REQUIRE_CHAIN(&res,
                          kefir_bigint_set_unsigned_value(&bigint, source_instr->operation.parameters.imm.uinteger));
        }
    }

    if (ready) {
        kefir_id_t bigint_id;
        REQUIRE_CHAIN(&res, kefir_ir_module_new_bigint(mem, module->ir_module, &bigint, &bigint_id));
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_bigint_free(mem, &bigint);
            return res;
        });
        REQUIRE_OK(kefir_bigint_free(mem, &bigint));

        if (signed_cast) {
            REQUIRE_OK(kefir_opt_code_builder_bitint_signed_constant(mem, &func->code, bitint_from_instr->block_id,
                                                                     bigint_id, replacement_ref));
        } else {
            REQUIRE_OK(kefir_opt_code_builder_bitint_unsigned_constant(mem, &func->code, bitint_from_instr->block_id,
                                                                       bigint_id, replacement_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_bitint_from_float(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                                 struct kefir_opt_function *func,
                                                 const struct kefir_opt_instruction *bitint_from_instr,
                                                 kefir_opt_instruction_ref_t *replacement_ref) {
    const struct kefir_opt_instruction *source_instr;
    REQUIRE_OK(
        kefir_opt_code_container_instr(&func->code, bitint_from_instr->operation.parameters.refs[0], &source_instr));

    kefir_bool_t signed_bitint = false;

    kefir_bool_t ready = false;
    struct kefir_bigint bigint;
    kefir_result_t res = KEFIR_OK;
    if (source_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_CONST &&
        bitint_from_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_SIGNED) {
        REQUIRE_OK(kefir_bigint_init(&bigint));
        ready = true;
        res = kefir_bigint_resize_nocast(mem, &bigint, bitint_from_instr->operation.parameters.bitwidth);
        REQUIRE_CHAIN(&res, kefir_bigint_signed_from_float(&bigint, source_instr->operation.parameters.imm.float32));
        signed_bitint = true;
    } else if (source_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_CONST &&
               bitint_from_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_UNSIGNED) {
        REQUIRE_OK(kefir_bigint_init(&bigint));
        ready = true;
        res = kefir_bigint_resize_nocast(mem, &bigint, bitint_from_instr->operation.parameters.bitwidth);
        REQUIRE_CHAIN(&res, kefir_bigint_unsigned_from_float(&bigint, source_instr->operation.parameters.imm.float32));
        signed_bitint = false;
    } else if (source_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT64_CONST &&
               bitint_from_instr->operation.opcode == KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_SIGNED) {
        REQUIRE_OK(kefir_bigint_init(&bigint));
        ready = true;
        res = kefir_bigint_resize_nocast(mem, &bigint, bitint_from_instr->operation.parameters.bitwidth);
        REQUIRE_CHAIN(&res, kefir_bigint_signed_from_double(&bigint, source_instr->operation.parameters.imm.float64));
        signed_bitint = true;
    } else if (source_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT64_CONST &&
               bitint_from_instr->operation.opcode == KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_UNSIGNED) {
        REQUIRE_OK(kefir_bigint_init(&bigint));
        ready = true;
        res = kefir_bigint_resize_nocast(mem, &bigint, bitint_from_instr->operation.parameters.bitwidth);
        REQUIRE_CHAIN(&res, kefir_bigint_unsigned_from_double(&bigint, source_instr->operation.parameters.imm.float64));
        signed_bitint = false;
    } else if (source_instr->operation.opcode == KEFIR_OPT_OPCODE_LONG_DOUBLE_CONST &&
               bitint_from_instr->operation.opcode == KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_SIGNED) {
        REQUIRE_OK(kefir_bigint_init(&bigint));
        ready = true;
        res = kefir_bigint_resize_nocast(mem, &bigint, bitint_from_instr->operation.parameters.bitwidth);
        REQUIRE_CHAIN(
            &res, kefir_bigint_signed_from_long_double(&bigint, source_instr->operation.parameters.imm.long_double));
        signed_bitint = true;
    } else if (source_instr->operation.opcode == KEFIR_OPT_OPCODE_LONG_DOUBLE_CONST &&
               bitint_from_instr->operation.opcode == KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_UNSIGNED) {
        REQUIRE_OK(kefir_bigint_init(&bigint));
        ready = true;
        res = kefir_bigint_resize_nocast(mem, &bigint, bitint_from_instr->operation.parameters.bitwidth);
        REQUIRE_CHAIN(
            &res, kefir_bigint_unsigned_from_long_double(&bigint, source_instr->operation.parameters.imm.long_double));
        signed_bitint = false;
    }

    if (ready) {
        kefir_id_t bigint_id;
        REQUIRE_CHAIN(&res, kefir_ir_module_new_bigint(mem, module->ir_module, &bigint, &bigint_id));
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_bigint_free(mem, &bigint);
            return res;
        });
        REQUIRE_OK(kefir_bigint_free(mem, &bigint));

        if (signed_bitint) {
            REQUIRE_OK(kefir_opt_code_builder_bitint_signed_constant(mem, &func->code, bitint_from_instr->block_id,
                                                                     bigint_id, replacement_ref));
        } else {
            REQUIRE_OK(kefir_opt_code_builder_bitint_unsigned_constant(mem, &func->code, bitint_from_instr->block_id,
                                                                       bigint_id, replacement_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_bitint_to_int(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                             struct kefir_opt_function *func,
                                             const struct kefir_opt_instruction *bitint_to_instr,
                                             kefir_opt_instruction_ref_t *replacement_ref) {
    const struct kefir_opt_instruction *source_instr;
    REQUIRE_OK(
        kefir_opt_code_container_instr(&func->code, bitint_to_instr->operation.parameters.refs[0], &source_instr));

    if ((source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST ||
         source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST) &&
        bitint_to_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_GET_SIGNED) {
        const struct kefir_bigint *bigint;
        REQUIRE_OK(
            kefir_ir_module_get_bigint(module->ir_module, source_instr->operation.parameters.imm.bitint_ref, &bigint));

        kefir_int64_t value;
        REQUIRE_OK(kefir_bigint_get_signed(bigint, &value));
        REQUIRE_OK(
            kefir_opt_code_builder_int_constant(mem, &func->code, bitint_to_instr->block_id, value, replacement_ref));
    } else if ((source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST ||
                source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST) &&
               bitint_to_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_GET_UNSIGNED) {
        const struct kefir_bigint *bigint;
        REQUIRE_OK(
            kefir_ir_module_get_bigint(module->ir_module, source_instr->operation.parameters.imm.bitint_ref, &bigint));

        kefir_uint64_t value;
        REQUIRE_OK(kefir_bigint_get_unsigned(bigint, &value));
        REQUIRE_OK(
            kefir_opt_code_builder_uint_constant(mem, &func->code, bitint_to_instr->block_id, value, replacement_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_bitint_cast(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                           struct kefir_opt_function *func,
                                           const struct kefir_opt_instruction *bitint_cast_instr,
                                           kefir_opt_instruction_ref_t *replacement_ref) {
    const struct kefir_opt_instruction *source_instr;
    REQUIRE_OK(
        kefir_opt_code_container_instr(&func->code, bitint_cast_instr->operation.parameters.refs[0], &source_instr));

    const kefir_bool_t signed_cast = bitint_cast_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_CAST_SIGNED;

    kefir_bool_t ready = false;
    struct kefir_bigint bigint;
    kefir_result_t res = KEFIR_OK;
    if ((source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST ||
         source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST)) {
        const struct kefir_bigint *source_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, source_instr->operation.parameters.imm.bitint_ref,
                                              &source_bigint));

        REQUIRE_OK(kefir_bigint_init(&bigint));
        ready = true;
        res = kefir_bigint_copy_resize(mem, &bigint, source_bigint);
        REQUIRE_CHAIN(&res, kefir_bigint_resize_nocast(mem, &bigint,
                                                       MAX(bitint_cast_instr->operation.parameters.bitwidth,
                                                           bitint_cast_instr->operation.parameters.src_bitwidth)));
        if (signed_cast) {
            REQUIRE_CHAIN(&res, kefir_bigint_cast_signed(&bigint, bitint_cast_instr->operation.parameters.src_bitwidth,
                                                         bitint_cast_instr->operation.parameters.bitwidth));
        } else {
            REQUIRE_CHAIN(&res,
                          kefir_bigint_cast_unsigned(&bigint, bitint_cast_instr->operation.parameters.src_bitwidth,
                                                     bitint_cast_instr->operation.parameters.bitwidth));
        }
        REQUIRE_CHAIN(&res, kefir_bigint_resize_nocast(mem, &bigint, bitint_cast_instr->operation.parameters.bitwidth));
    }

    if (ready) {
        kefir_id_t bigint_id;
        REQUIRE_CHAIN(&res, kefir_ir_module_new_bigint(mem, module->ir_module, &bigint, &bigint_id));
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_bigint_free(mem, &bigint);
            return res;
        });
        REQUIRE_OK(kefir_bigint_free(mem, &bigint));

        if (signed_cast) {
            REQUIRE_OK(kefir_opt_code_builder_bitint_signed_constant(mem, &func->code, bitint_cast_instr->block_id,
                                                                     bigint_id, replacement_ref));
        } else {
            REQUIRE_OK(kefir_opt_code_builder_bitint_unsigned_constant(mem, &func->code, bitint_cast_instr->block_id,
                                                                       bigint_id, replacement_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_bitint_to_float_impl(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                                    struct kefir_opt_function *func,
                                                    const struct kefir_opt_instruction *bitint_to_instr,
                                                    kefir_opt_instruction_ref_t *replacement_ref,
                                                    struct kefir_bigint *tmp_bigint, struct kefir_bigint *tmp2_bigint) {
    const struct kefir_opt_instruction *source_instr;
    REQUIRE_OK(
        kefir_opt_code_container_instr(&func->code, bitint_to_instr->operation.parameters.refs[0], &source_instr));

    if ((source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST ||
         source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST) &&
        bitint_to_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_FLOAT) {
        const struct kefir_bigint *bigint;
        REQUIRE_OK(
            kefir_ir_module_get_bigint(module->ir_module, source_instr->operation.parameters.imm.bitint_ref, &bigint));

        kefir_float32_t value;
        REQUIRE_OK(
            kefir_bigint_resize_nocast(mem, tmp2_bigint, MAX(bigint->bitwidth, sizeof(kefir_float32_t) * CHAR_BIT)));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, tmp_bigint, bigint));
        REQUIRE_OK(kefir_bigint_resize_cast_signed(mem, tmp_bigint, tmp2_bigint->bitwidth));
        REQUIRE_OK(kefir_bigint_signed_to_float(tmp_bigint, tmp2_bigint, &value));
        REQUIRE_OK(kefir_opt_code_builder_float32_constant(mem, &func->code, bitint_to_instr->block_id, value,
                                                           replacement_ref));
    } else if ((source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST ||
                source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST) &&
               bitint_to_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_FLOAT) {
        const struct kefir_bigint *bigint;
        REQUIRE_OK(
            kefir_ir_module_get_bigint(module->ir_module, source_instr->operation.parameters.imm.bitint_ref, &bigint));

        kefir_float32_t value;
        REQUIRE_OK(
            kefir_bigint_resize_nocast(mem, tmp2_bigint, MAX(bigint->bitwidth, sizeof(kefir_float32_t) * CHAR_BIT)));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, tmp_bigint, bigint));
        REQUIRE_OK(kefir_bigint_resize_cast_unsigned(mem, tmp_bigint, tmp2_bigint->bitwidth));
        REQUIRE_OK(kefir_bigint_unsigned_to_float(tmp_bigint, tmp2_bigint, &value));
        REQUIRE_OK(kefir_opt_code_builder_float32_constant(mem, &func->code, bitint_to_instr->block_id, value,
                                                           replacement_ref));
    } else if ((source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST ||
                source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST) &&
               bitint_to_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_DOUBLE) {
        const struct kefir_bigint *bigint;
        REQUIRE_OK(
            kefir_ir_module_get_bigint(module->ir_module, source_instr->operation.parameters.imm.bitint_ref, &bigint));

        kefir_float64_t value;
        REQUIRE_OK(
            kefir_bigint_resize_nocast(mem, tmp2_bigint, MAX(bigint->bitwidth, sizeof(kefir_float64_t) * CHAR_BIT)));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, tmp_bigint, bigint));
        REQUIRE_OK(kefir_bigint_resize_cast_signed(mem, tmp_bigint, tmp2_bigint->bitwidth));
        REQUIRE_OK(kefir_bigint_signed_to_double(tmp_bigint, tmp2_bigint, &value));
        REQUIRE_OK(kefir_opt_code_builder_float64_constant(mem, &func->code, bitint_to_instr->block_id, value,
                                                           replacement_ref));
    } else if ((source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST ||
                source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST) &&
               bitint_to_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_DOUBLE) {
        const struct kefir_bigint *bigint;
        REQUIRE_OK(
            kefir_ir_module_get_bigint(module->ir_module, source_instr->operation.parameters.imm.bitint_ref, &bigint));

        kefir_float64_t value;
        REQUIRE_OK(
            kefir_bigint_resize_nocast(mem, tmp2_bigint, MAX(bigint->bitwidth, sizeof(kefir_float64_t) * CHAR_BIT)));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, tmp_bigint, bigint));
        REQUIRE_OK(kefir_bigint_resize_cast_unsigned(mem, tmp_bigint, tmp2_bigint->bitwidth));
        REQUIRE_OK(kefir_bigint_unsigned_to_double(tmp_bigint, tmp2_bigint, &value));
        REQUIRE_OK(kefir_opt_code_builder_float64_constant(mem, &func->code, bitint_to_instr->block_id, value,
                                                           replacement_ref));
    } else if ((source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST ||
                source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST) &&
               bitint_to_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_LONG_DOUBLE) {
        const struct kefir_bigint *bigint;
        REQUIRE_OK(
            kefir_ir_module_get_bigint(module->ir_module, source_instr->operation.parameters.imm.bitint_ref, &bigint));

        kefir_long_double_t value;
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, tmp2_bigint,
                                              MAX(bigint->bitwidth, sizeof(kefir_long_double_t) * CHAR_BIT)));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, tmp_bigint, bigint));
        REQUIRE_OK(kefir_bigint_resize_cast_signed(mem, tmp_bigint, tmp2_bigint->bitwidth));
        REQUIRE_OK(kefir_bigint_signed_to_long_double(tmp_bigint, tmp2_bigint, &value));
        REQUIRE_OK(kefir_opt_code_builder_long_double_constant(mem, &func->code, bitint_to_instr->block_id, value,
                                                               replacement_ref));
    } else if ((source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST ||
                source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST) &&
               bitint_to_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_LONG_DOUBLE) {
        const struct kefir_bigint *bigint;
        REQUIRE_OK(
            kefir_ir_module_get_bigint(module->ir_module, source_instr->operation.parameters.imm.bitint_ref, &bigint));

        kefir_long_double_t value;
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, tmp2_bigint,
                                              MAX(bigint->bitwidth, sizeof(kefir_long_double_t) * CHAR_BIT)));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, tmp_bigint, bigint));
        REQUIRE_OK(kefir_bigint_resize_cast_unsigned(mem, tmp_bigint, tmp2_bigint->bitwidth));
        REQUIRE_OK(kefir_bigint_unsigned_to_long_double(tmp_bigint, tmp2_bigint, &value));
        REQUIRE_OK(kefir_opt_code_builder_long_double_constant(mem, &func->code, bitint_to_instr->block_id, value,
                                                               replacement_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_bitint_to_float(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                               struct kefir_opt_function *func,
                                               const struct kefir_opt_instruction *bitint_to_instr,
                                               kefir_opt_instruction_ref_t *replacement_ref) {
    struct kefir_bigint tmp_bigint, tmp2_bigint;
    REQUIRE_OK(kefir_bigint_init(&tmp_bigint));
    REQUIRE_OK(kefir_bigint_init(&tmp2_bigint));

    kefir_result_t res =
        simplify_bitint_to_float_impl(mem, module, func, bitint_to_instr, replacement_ref, &tmp_bigint, &tmp2_bigint);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_bigint_free(mem, &tmp2_bigint);
        kefir_bigint_free(mem, &tmp_bigint);
        return res;
    });
    res = kefir_bigint_free(mem, &tmp2_bigint);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_bigint_free(mem, &tmp_bigint);
        return res;
    });
    REQUIRE_OK(kefir_bigint_free(mem, &tmp_bigint));
    return KEFIR_OK;
}

static kefir_result_t simplify_bitint_to_bool(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                              struct kefir_opt_function *func,
                                              const struct kefir_opt_instruction *bitint_to_instr,
                                              kefir_opt_instruction_ref_t *replacement_ref) {
    const struct kefir_opt_instruction *source_instr;
    REQUIRE_OK(
        kefir_opt_code_container_instr(&func->code, bitint_to_instr->operation.parameters.refs[0], &source_instr));

    if ((source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST ||
         source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST) &&
        bitint_to_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_TO_BOOL) {
        const struct kefir_bigint *bigint;
        REQUIRE_OK(
            kefir_ir_module_get_bigint(module->ir_module, source_instr->operation.parameters.imm.bitint_ref, &bigint));

        kefir_bool_t is_zero;
        REQUIRE_OK(kefir_bigint_is_zero(bigint, &is_zero));
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, bitint_to_instr->block_id, !is_zero,
                                                       replacement_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_bitint_unary_impl(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                                 struct kefir_opt_function *func,
                                                 const struct kefir_opt_instruction *bitint_unary_instr,
                                                 kefir_opt_instruction_ref_t *replacement_ref,
                                                 struct kefir_bigint *tmp_bigint) {
    const struct kefir_opt_instruction *source_instr;
    REQUIRE_OK(
        kefir_opt_code_container_instr(&func->code, bitint_unary_instr->operation.parameters.refs[0], &source_instr));

    kefir_bool_t ready = false;
    if ((source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST ||
         source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST) &&
        bitint_unary_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_INVERT) {
        const struct kefir_bigint *bigint;
        REQUIRE_OK(
            kefir_ir_module_get_bigint(module->ir_module, source_instr->operation.parameters.imm.bitint_ref, &bigint));

        REQUIRE_OK(kefir_bigint_copy_resize(mem, tmp_bigint, bigint));
        REQUIRE_OK(kefir_bigint_invert(tmp_bigint));
        ready = true;
    } else if ((source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST ||
                source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST) &&
               bitint_unary_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_NEGATE) {
        const struct kefir_bigint *bigint;
        REQUIRE_OK(
            kefir_ir_module_get_bigint(module->ir_module, source_instr->operation.parameters.imm.bitint_ref, &bigint));

        REQUIRE_OK(kefir_bigint_copy_resize(mem, tmp_bigint, bigint));
        REQUIRE_OK(kefir_bigint_negate(tmp_bigint));
        ready = true;
    }

    if (ready) {
        kefir_id_t bigint_id;
        REQUIRE_OK(kefir_ir_module_new_bigint(mem, module->ir_module, tmp_bigint, &bigint_id));

        if (source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST) {
            REQUIRE_OK(kefir_opt_code_builder_bitint_signed_constant(mem, &func->code, bitint_unary_instr->block_id,
                                                                     bigint_id, replacement_ref));
        } else {
            REQUIRE_OK(kefir_opt_code_builder_bitint_unsigned_constant(mem, &func->code, bitint_unary_instr->block_id,
                                                                       bigint_id, replacement_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_bitint_unary(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                            struct kefir_opt_function *func,
                                            const struct kefir_opt_instruction *bitint_unary_instr,
                                            kefir_opt_instruction_ref_t *replacement_ref) {
    struct kefir_bigint tmp_bigint;
    REQUIRE_OK(kefir_bigint_init(&tmp_bigint));

    kefir_result_t res =
        simplify_bitint_unary_impl(mem, module, func, bitint_unary_instr, replacement_ref, &tmp_bigint);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_bigint_free(mem, &tmp_bigint);
        return res;
    });
    REQUIRE_OK(kefir_bigint_free(mem, &tmp_bigint));
    return KEFIR_OK;
}

static kefir_result_t simplify_bitint_bool_not(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                               struct kefir_opt_function *func,
                                               const struct kefir_opt_instruction *bitint_to_instr,
                                               kefir_opt_instruction_ref_t *replacement_ref) {
    const struct kefir_opt_instruction *source_instr;
    REQUIRE_OK(
        kefir_opt_code_container_instr(&func->code, bitint_to_instr->operation.parameters.refs[0], &source_instr));

    if ((source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST ||
         source_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST) &&
        bitint_to_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_BOOL_NOT) {
        const struct kefir_bigint *bigint;
        REQUIRE_OK(
            kefir_ir_module_get_bigint(module->ir_module, source_instr->operation.parameters.imm.bitint_ref, &bigint));

        kefir_bool_t is_zero;
        REQUIRE_OK(kefir_bigint_is_zero(bigint, &is_zero));
        REQUIRE_OK(
            kefir_opt_code_builder_int_constant(mem, &func->code, bitint_to_instr->block_id, is_zero, replacement_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_bitint_binary_impl(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                                  struct kefir_opt_function *func,
                                                  const struct kefir_opt_instruction *bitint_binary_instr,
                                                  kefir_opt_instruction_ref_t *replacement_ref,
                                                  struct kefir_bigint_pool *bigints) {
    const struct kefir_opt_instruction *arg1_instr, *arg2_instr;
    REQUIRE_OK(
        kefir_opt_code_container_instr(&func->code, bitint_binary_instr->operation.parameters.refs[0], &arg1_instr));
    REQUIRE_OK(
        kefir_opt_code_container_instr(&func->code, bitint_binary_instr->operation.parameters.refs[1], &arg2_instr));

    REQUIRE((arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST ||
             arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST) &&
                (arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST ||
                 arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST),
            KEFIR_OK);

    struct kefir_bigint *result_bigint = NULL;
    if (bitint_binary_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_ADD) {
        const struct kefir_bigint *arg1_bigint, *arg2_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg2_instr->operation.parameters.imm.bitint_ref,
                                              &arg2_bigint));

        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &result_bigint));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, result_bigint, arg1_bigint));
        REQUIRE_OK(kefir_bigint_add(result_bigint, arg2_bigint));
    } else if (bitint_binary_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SUB) {
        const struct kefir_bigint *arg1_bigint, *arg2_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg2_instr->operation.parameters.imm.bitint_ref,
                                              &arg2_bigint));

        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &result_bigint));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, result_bigint, arg1_bigint));
        REQUIRE_OK(kefir_bigint_subtract(result_bigint, arg2_bigint));
    } else if (bitint_binary_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_AND) {
        const struct kefir_bigint *arg1_bigint, *arg2_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg2_instr->operation.parameters.imm.bitint_ref,
                                              &arg2_bigint));

        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &result_bigint));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, result_bigint, arg1_bigint));
        REQUIRE_OK(kefir_bigint_and(result_bigint, arg2_bigint));
    } else if (bitint_binary_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_OR) {
        const struct kefir_bigint *arg1_bigint, *arg2_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg2_instr->operation.parameters.imm.bitint_ref,
                                              &arg2_bigint));

        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &result_bigint));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, result_bigint, arg1_bigint));
        REQUIRE_OK(kefir_bigint_or(result_bigint, arg2_bigint));
    } else if (bitint_binary_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_XOR) {
        const struct kefir_bigint *arg1_bigint, *arg2_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg2_instr->operation.parameters.imm.bitint_ref,
                                              &arg2_bigint));

        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &result_bigint));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, result_bigint, arg1_bigint));
        REQUIRE_OK(kefir_bigint_xor(result_bigint, arg2_bigint));
    } else if (bitint_binary_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_IMUL) {
        const struct kefir_bigint *arg1_bigint, *arg2_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg2_instr->operation.parameters.imm.bitint_ref,
                                              &arg2_bigint));

        struct kefir_bigint *lhs_bigint, *acc_bigint;
        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &result_bigint));
        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &lhs_bigint));
        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &acc_bigint));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, lhs_bigint, arg1_bigint));
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, result_bigint, lhs_bigint->bitwidth));
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, acc_bigint, result_bigint->bitwidth * 2 + 1));
        REQUIRE_OK(kefir_bigint_signed_multiply(result_bigint, lhs_bigint, arg2_bigint, acc_bigint));
    } else if (bitint_binary_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UMUL) {
        const struct kefir_bigint *arg1_bigint, *arg2_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg2_instr->operation.parameters.imm.bitint_ref,
                                              &arg2_bigint));

        struct kefir_bigint *acc_bigint;
        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &result_bigint));
        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &acc_bigint));
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, result_bigint, arg1_bigint->bitwidth));
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, acc_bigint, result_bigint->bitwidth * 2 + 1));
        REQUIRE_OK(kefir_bigint_unsigned_multiply(result_bigint, arg1_bigint, arg2_bigint));
    } else if (bitint_binary_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_IDIV) {
        const struct kefir_bigint *arg1_bigint, *arg2_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg2_instr->operation.parameters.imm.bitint_ref,
                                              &arg2_bigint));

        struct kefir_bigint *remainder_bigint, *rhs_bigint;
        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &result_bigint));
        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &remainder_bigint));
        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &rhs_bigint));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, result_bigint, arg1_bigint));
        REQUIRE_OK(kefir_bigint_resize_cast_signed(mem, result_bigint, result_bigint->bitwidth * 2 + 1));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, rhs_bigint, arg2_bigint));
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, remainder_bigint, result_bigint->bitwidth));
        REQUIRE_OK(kefir_bigint_signed_divide(result_bigint, remainder_bigint, rhs_bigint));
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, result_bigint, arg1_bigint->bitwidth));
    } else if (bitint_binary_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UDIV) {
        const struct kefir_bigint *arg1_bigint, *arg2_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg2_instr->operation.parameters.imm.bitint_ref,
                                              &arg2_bigint));

        struct kefir_bigint *remainder_bigint;
        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &result_bigint));
        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &remainder_bigint));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, result_bigint, arg1_bigint));
        REQUIRE_OK(kefir_bigint_resize_cast_unsigned(mem, result_bigint, result_bigint->bitwidth * 2 + 1));
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, remainder_bigint, result_bigint->bitwidth));
        REQUIRE_OK(kefir_bigint_unsigned_divide(result_bigint, remainder_bigint, arg2_bigint));
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, result_bigint, arg1_bigint->bitwidth));
    } else if (bitint_binary_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_IMOD) {
        const struct kefir_bigint *arg1_bigint, *arg2_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg2_instr->operation.parameters.imm.bitint_ref,
                                              &arg2_bigint));

        struct kefir_bigint *lhs_bigint, *rhs_bigint;
        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &result_bigint));
        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &lhs_bigint));
        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &rhs_bigint));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, lhs_bigint, arg1_bigint));
        REQUIRE_OK(kefir_bigint_resize_cast_signed(mem, lhs_bigint, lhs_bigint->bitwidth * 2 + 1));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, rhs_bigint, arg2_bigint));
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, result_bigint, lhs_bigint->bitwidth));
        REQUIRE_OK(kefir_bigint_signed_divide(lhs_bigint, result_bigint, rhs_bigint));
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, result_bigint, arg1_bigint->bitwidth));
    } else if (bitint_binary_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UMOD) {
        const struct kefir_bigint *arg1_bigint, *arg2_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg2_instr->operation.parameters.imm.bitint_ref,
                                              &arg2_bigint));

        struct kefir_bigint *lhs_bigint;
        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &result_bigint));
        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &lhs_bigint));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, lhs_bigint, arg1_bigint));
        REQUIRE_OK(kefir_bigint_resize_cast_unsigned(mem, lhs_bigint, lhs_bigint->bitwidth * 2 + 1));
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, result_bigint, lhs_bigint->bitwidth));
        REQUIRE_OK(kefir_bigint_unsigned_divide(lhs_bigint, result_bigint, arg2_bigint));
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, result_bigint, arg1_bigint->bitwidth));
    }

    if (result_bigint != NULL) {
        kefir_id_t bigint_id;
        REQUIRE_OK(kefir_ir_module_new_bigint(mem, module->ir_module, result_bigint, &bigint_id));
        REQUIRE_OK(kefir_opt_code_builder_bitint_signed_constant(mem, &func->code, bitint_binary_instr->block_id,
                                                                 bigint_id, replacement_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_bitint_binary(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                             struct kefir_opt_function *func,
                                             const struct kefir_opt_instruction *bitint_binary_instr,
                                             kefir_opt_instruction_ref_t *replacement_ref) {
    struct kefir_bigint_pool bigints;
    REQUIRE_OK(kefir_bigint_pool_init(&bigints));

    kefir_result_t res = simplify_bitint_binary_impl(mem, module, func, bitint_binary_instr, replacement_ref, &bigints);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_bigint_pool_free(mem, &bigints);
        return res;
    });
    REQUIRE_OK(kefir_bigint_pool_free(mem, &bigints));
    return KEFIR_OK;
}

static kefir_result_t simplify_bitint_shift_impl(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                                 struct kefir_opt_function *func,
                                                 const struct kefir_opt_instruction *bitint_binary_instr,
                                                 kefir_opt_instruction_ref_t *replacement_ref,
                                                 struct kefir_bigint_pool *bigints) {
    const struct kefir_opt_instruction *arg1_instr, *arg2_instr;
    REQUIRE_OK(
        kefir_opt_code_container_instr(&func->code, bitint_binary_instr->operation.parameters.refs[0], &arg1_instr));
    REQUIRE_OK(
        kefir_opt_code_container_instr(&func->code, bitint_binary_instr->operation.parameters.refs[1], &arg2_instr));

    REQUIRE((arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST ||
             arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST) &&
                (arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                 arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST),
            KEFIR_OK);

    struct kefir_bigint *result_bigint = NULL;
    if (bitint_binary_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_LSHIFT) {
        const struct kefir_bigint *arg1_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));

        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &result_bigint));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, result_bigint, arg1_bigint));
        REQUIRE_OK(kefir_bigint_left_shift(result_bigint, arg2_instr->operation.parameters.imm.integer));
    } else if (bitint_binary_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_RSHIFT) {
        const struct kefir_bigint *arg1_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));

        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &result_bigint));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, result_bigint, arg1_bigint));
        REQUIRE_OK(kefir_bigint_right_shift(result_bigint, arg2_instr->operation.parameters.imm.integer));
    } else if (bitint_binary_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_ARSHIFT) {
        const struct kefir_bigint *arg1_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));

        REQUIRE_OK(kefir_bigint_pool_alloc(mem, bigints, &result_bigint));
        REQUIRE_OK(kefir_bigint_copy_resize(mem, result_bigint, arg1_bigint));
        REQUIRE_OK(kefir_bigint_arithmetic_right_shift(result_bigint, arg2_instr->operation.parameters.imm.integer));
    }

    if (result_bigint != NULL) {
        kefir_id_t bigint_id;
        REQUIRE_OK(kefir_ir_module_new_bigint(mem, module->ir_module, result_bigint, &bigint_id));
        REQUIRE_OK(kefir_opt_code_builder_bitint_signed_constant(mem, &func->code, bitint_binary_instr->block_id,
                                                                 bigint_id, replacement_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_bitint_shift(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                            struct kefir_opt_function *func,
                                            const struct kefir_opt_instruction *bitint_binary_instr,
                                            kefir_opt_instruction_ref_t *replacement_ref) {
    struct kefir_bigint_pool bigints;
    REQUIRE_OK(kefir_bigint_pool_init(&bigints));

    kefir_result_t res = simplify_bitint_shift_impl(mem, module, func, bitint_binary_instr, replacement_ref, &bigints);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_bigint_pool_free(mem, &bigints);
        return res;
    });
    REQUIRE_OK(kefir_bigint_pool_free(mem, &bigints));
    return KEFIR_OK;
}

static kefir_result_t simplify_bitint_relational(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                                 struct kefir_opt_function *func,
                                                 const struct kefir_opt_instruction *bitint_relational_instr,
                                                 kefir_opt_instruction_ref_t *replacement_ref) {
    const struct kefir_opt_instruction *arg1_instr, *arg2_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, bitint_relational_instr->operation.parameters.refs[0],
                                              &arg1_instr));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, bitint_relational_instr->operation.parameters.refs[1],
                                              &arg2_instr));

    REQUIRE((arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST ||
             arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST) &&
                (arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST ||
                 arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_UNSIGNED_CONST),
            KEFIR_OK);

    if (bitint_relational_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_EQUAL) {
        const struct kefir_bigint *arg1_bigint, *arg2_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg2_instr->operation.parameters.imm.bitint_ref,
                                              &arg2_bigint));

        kefir_int_t comparison;
        REQUIRE_OK(kefir_bigint_signed_compare(arg1_bigint, arg2_bigint, &comparison));
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, bitint_relational_instr->block_id,
                                                       comparison == 0, replacement_ref));
    } else if (bitint_relational_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_GREATER) {
        const struct kefir_bigint *arg1_bigint, *arg2_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg2_instr->operation.parameters.imm.bitint_ref,
                                              &arg2_bigint));

        kefir_int_t comparison;
        REQUIRE_OK(kefir_bigint_signed_compare(arg1_bigint, arg2_bigint, &comparison));
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, bitint_relational_instr->block_id,
                                                       comparison > 0, replacement_ref));
    } else if (bitint_relational_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_LESS) {
        const struct kefir_bigint *arg1_bigint, *arg2_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg2_instr->operation.parameters.imm.bitint_ref,
                                              &arg2_bigint));

        kefir_int_t comparison;
        REQUIRE_OK(kefir_bigint_signed_compare(arg1_bigint, arg2_bigint, &comparison));
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, bitint_relational_instr->block_id,
                                                       comparison < 0, replacement_ref));
    } else if (bitint_relational_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_ABOVE) {
        const struct kefir_bigint *arg1_bigint, *arg2_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg2_instr->operation.parameters.imm.bitint_ref,
                                              &arg2_bigint));

        kefir_int_t comparison;
        REQUIRE_OK(kefir_bigint_unsigned_compare(arg1_bigint, arg2_bigint, &comparison));
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, bitint_relational_instr->block_id,
                                                       comparison > 0, replacement_ref));
    } else if (bitint_relational_instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_BELOW) {
        const struct kefir_bigint *arg1_bigint, *arg2_bigint;
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg1_instr->operation.parameters.imm.bitint_ref,
                                              &arg1_bigint));
        REQUIRE_OK(kefir_ir_module_get_bigint(module->ir_module, arg2_instr->operation.parameters.imm.bitint_ref,
                                              &arg2_bigint));

        kefir_int_t comparison;
        REQUIRE_OK(kefir_bigint_unsigned_compare(arg1_bigint, arg2_bigint, &comparison));
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, bitint_relational_instr->block_id,
                                                       comparison < 0, replacement_ref));
    }

    return KEFIR_OK;
}

static kefir_result_t const_fold_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                       struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                       const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct kefir_opt_code_container_iterator iter;
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(&func->code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {

        kefir_opt_instruction_ref_t instr_id;
        const struct kefir_opt_instruction *instr = NULL;

        for (kefir_opt_code_block_instr_head(&func->code, block, &instr_id); instr_id != KEFIR_ID_NONE;) {

            REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor_of(&func->debug_info, instr_id));

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
                case KEFIR_OPT_OPCODE_UINT8_MUL:
                case KEFIR_OPT_OPCODE_UINT16_MUL:
                case KEFIR_OPT_OPCODE_UINT32_MUL:
                case KEFIR_OPT_OPCODE_UINT64_MUL:
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
                case KEFIR_OPT_OPCODE_SCALAR_COMPARE:
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

                case KEFIR_OPT_OPCODE_BITS_EXTRACT_SIGNED:
                case KEFIR_OPT_OPCODE_BITS_EXTRACT_UNSIGNED:
                    REQUIRE_OK(int_extract_bits_const_fold(mem, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_BITINT_FROM_SIGNED:
                case KEFIR_OPT_OPCODE_BITINT_FROM_UNSIGNED:
                    REQUIRE_OK(simplify_bitint_from(mem, module, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_BITINT_GET_SIGNED:
                case KEFIR_OPT_OPCODE_BITINT_GET_UNSIGNED:
                    REQUIRE_OK(simplify_bitint_to_int(mem, module, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_BITINT_CAST_SIGNED:
                case KEFIR_OPT_OPCODE_BITINT_CAST_UNSIGNED:
                    REQUIRE_OK(simplify_bitint_cast(mem, module, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_SIGNED:
                case KEFIR_OPT_OPCODE_FLOAT_TO_BITINT_UNSIGNED:
                case KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_SIGNED:
                case KEFIR_OPT_OPCODE_DOUBLE_TO_BITINT_UNSIGNED:
                case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_SIGNED:
                case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_BITINT_UNSIGNED:
                    REQUIRE_OK(simplify_bitint_from_float(mem, module, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_FLOAT:
                case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_FLOAT:
                case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_DOUBLE:
                case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_DOUBLE:
                case KEFIR_OPT_OPCODE_BITINT_SIGNED_TO_LONG_DOUBLE:
                case KEFIR_OPT_OPCODE_BITINT_UNSIGNED_TO_LONG_DOUBLE:
                    REQUIRE_OK(simplify_bitint_to_float(mem, module, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_BITINT_TO_BOOL:
                    REQUIRE_OK(simplify_bitint_to_bool(mem, module, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_BITINT_INVERT:
                case KEFIR_OPT_OPCODE_BITINT_NEGATE:
                    REQUIRE_OK(simplify_bitint_unary(mem, module, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_BITINT_BOOL_NOT:
                    REQUIRE_OK(simplify_bitint_bool_not(mem, module, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_BITINT_ADD:
                case KEFIR_OPT_OPCODE_BITINT_SUB:
                case KEFIR_OPT_OPCODE_BITINT_IMUL:
                case KEFIR_OPT_OPCODE_BITINT_UMUL:
                case KEFIR_OPT_OPCODE_BITINT_IDIV:
                case KEFIR_OPT_OPCODE_BITINT_UDIV:
                case KEFIR_OPT_OPCODE_BITINT_IMOD:
                case KEFIR_OPT_OPCODE_BITINT_UMOD:
                case KEFIR_OPT_OPCODE_BITINT_AND:
                case KEFIR_OPT_OPCODE_BITINT_OR:
                case KEFIR_OPT_OPCODE_BITINT_XOR:
                    REQUIRE_OK(simplify_bitint_binary(mem, module, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_BITINT_LSHIFT:
                case KEFIR_OPT_OPCODE_BITINT_RSHIFT:
                case KEFIR_OPT_OPCODE_BITINT_ARSHIFT:
                    REQUIRE_OK(simplify_bitint_shift(mem, module, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_BITINT_EQUAL:
                case KEFIR_OPT_OPCODE_BITINT_GREATER:
                case KEFIR_OPT_OPCODE_BITINT_LESS:
                case KEFIR_OPT_OPCODE_BITINT_ABOVE:
                case KEFIR_OPT_OPCODE_BITINT_BELOW:
                    REQUIRE_OK(simplify_bitint_relational(mem, module, func, instr, &replacement_ref));
                    break;

                default:
                    // Intentionally left blank
                    break;
            }

            if (replacement_ref != KEFIR_ID_NONE) {
                REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_id, &instr));
                REQUIRE_OK(kefir_opt_code_container_replace_references(mem, &func->code, replacement_ref, instr->id));
                REQUIRE_OK(kefir_opt_code_debug_info_replace_local_variable(mem, &func->debug_info, instr->id,
                                                                            replacement_ref));
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
                REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, prev_instr_id));
            } else {
                REQUIRE_OK(kefir_opt_instruction_next_sibling(&func->code, instr_id, &instr_id));
            }

            REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor(
                &func->debug_info, KEFIR_OPT_CODE_DEBUG_INSTRUCTION_LOCATION_NONE));
        }
    }
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassConstFold = {
    .name = "constant-fold", .apply = const_fold_apply, .payload = NULL};
