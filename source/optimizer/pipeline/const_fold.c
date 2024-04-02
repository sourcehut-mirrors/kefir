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
                                           struct kefir_opt_instruction *instr,
                                           kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;
    const kefir_opt_instruction_ref_t instr_id = instr->id;

    struct kefir_opt_instruction *arg1;
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
        case KEFIR_OPT_OPCODE_INT64_TRUNCATE_1BIT:
            result.uinteger = arg.uinteger != 0 ? 1 : 0;
            break;

        case KEFIR_OPT_OPCODE_INT_NEG:
            result.integer = -arg.integer;
            break;

        case KEFIR_OPT_OPCODE_INT_NOT:
            result.uinteger = ~arg.uinteger;
            break;

        case KEFIR_OPT_OPCODE_BOOL_NOT:
            result.uinteger = arg.uinteger == 0;
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
                                            struct kefir_opt_instruction *instr,
                                            kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;
    const kefir_opt_instruction_ref_t instr_id = instr->id;

    struct kefir_opt_instruction *arg1;
    struct kefir_opt_instruction *arg2;
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
        case KEFIR_OPT_OPCODE_INT16_ADD:
        case KEFIR_OPT_OPCODE_INT32_ADD:
        case KEFIR_OPT_OPCODE_INT64_ADD:
            result.uinteger = left.uinteger + right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT8_SUB:
        case KEFIR_OPT_OPCODE_INT16_SUB:
        case KEFIR_OPT_OPCODE_INT32_SUB:
        case KEFIR_OPT_OPCODE_INT64_SUB:
            result.uinteger = left.uinteger - right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT8_MUL:
        case KEFIR_OPT_OPCODE_INT16_MUL:
        case KEFIR_OPT_OPCODE_INT32_MUL:
        case KEFIR_OPT_OPCODE_INT64_MUL:
            result.uinteger = left.uinteger * right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT8_DIV:
        case KEFIR_OPT_OPCODE_INT16_DIV:
        case KEFIR_OPT_OPCODE_INT32_DIV:
        case KEFIR_OPT_OPCODE_INT64_DIV:
            REQUIRE(right.integer != 0, KEFIR_OK);
            result.integer = left.integer / right.integer;
            break;

        case KEFIR_OPT_OPCODE_INT8_MOD:
        case KEFIR_OPT_OPCODE_INT16_MOD:
        case KEFIR_OPT_OPCODE_INT32_MOD:
        case KEFIR_OPT_OPCODE_INT64_MOD:
            REQUIRE(right.integer != 0, KEFIR_OK);
            result.integer = left.integer % right.integer;
            break;

        case KEFIR_OPT_OPCODE_UINT8_DIV:
        case KEFIR_OPT_OPCODE_UINT16_DIV:
        case KEFIR_OPT_OPCODE_UINT32_DIV:
        case KEFIR_OPT_OPCODE_UINT64_DIV:
            REQUIRE(right.integer != 0, KEFIR_OK);
            result.uinteger = left.uinteger / right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_UINT8_MOD:
        case KEFIR_OPT_OPCODE_UINT16_MOD:
        case KEFIR_OPT_OPCODE_UINT32_MOD:
        case KEFIR_OPT_OPCODE_UINT64_MOD:
            REQUIRE(right.integer != 0, KEFIR_OK);
            result.uinteger = left.uinteger % right.uinteger;
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

        case KEFIR_OPT_OPCODE_INT_LSHIFT:
            result.uinteger = left.uinteger << right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT_RSHIFT:
            result.uinteger = left.uinteger >> right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT_ARSHIFT:
            result.integer = left.integer >> right.integer;
            break;

        case KEFIR_OPT_OPCODE_INT_EQUALS:
            result.integer = left.integer == right.integer;
            break;

        case KEFIR_OPT_OPCODE_INT_GREATER:
            result.integer = left.integer > right.integer;
            break;

        case KEFIR_OPT_OPCODE_INT_LESSER:
            result.integer = left.integer < right.integer;
            break;

        case KEFIR_OPT_OPCODE_INT_ABOVE:
            result.integer = left.uinteger > right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_INT_BELOW:
            result.integer = left.uinteger < right.uinteger;
            break;

        case KEFIR_OPT_OPCODE_BOOL_OR:
            result.integer = left.uinteger != 0 || right.uinteger != 0;
            break;

        case KEFIR_OPT_OPCODE_BOOL_AND:
            result.integer = left.uinteger != 0 && right.uinteger != 0;
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

        struct kefir_opt_instruction *instr = NULL;

        for (kefir_opt_code_block_instr_head(&func->code, block, &instr); instr != NULL;) {

            const kefir_opt_instruction_ref_t instr_id = instr->id;
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
                case KEFIR_OPT_OPCODE_INT_LSHIFT:
                case KEFIR_OPT_OPCODE_INT_RSHIFT:
                case KEFIR_OPT_OPCODE_INT_ARSHIFT:
                case KEFIR_OPT_OPCODE_INT_EQUALS:
                case KEFIR_OPT_OPCODE_INT_GREATER:
                case KEFIR_OPT_OPCODE_INT_LESSER:
                case KEFIR_OPT_OPCODE_INT_ABOVE:
                case KEFIR_OPT_OPCODE_INT_BELOW:
                    REQUIRE_OK(int_binary_const_fold(mem, func, instr, &replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_INT_NEG:
                case KEFIR_OPT_OPCODE_INT_NOT:
                case KEFIR_OPT_OPCODE_BOOL_NOT:
                case KEFIR_OPT_OPCODE_INT64_TRUNCATE_1BIT:
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
                REQUIRE_OK(kefir_opt_code_container_replace_references(&func->code, replacement_ref, instr->id));
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

const struct kefir_optimizer_pass KefirOptimizerPassConstFold = {
    .name = "constant-fold", .apply = const_fold_apply, .payload = NULL};
