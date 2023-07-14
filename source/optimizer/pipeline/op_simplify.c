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

#include "kefir/optimizer/pipeline.h"
#include "kefir/optimizer/builder.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t simplify_boot_not(struct kefir_mem *mem, struct kefir_opt_function *func,
                                        struct kefir_opt_instruction *instr,
                                        kefir_opt_instruction_ref_t *replacement_ref) {
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
        case KEFIR_OPT_OPCODE_INT_GREATER_OR_EQUALS:
        case KEFIR_OPT_OPCODE_INT_LESSER:
        case KEFIR_OPT_OPCODE_INT_LESSER_OR_EQUALS:
        case KEFIR_OPT_OPCODE_INT_ABOVE:
        case KEFIR_OPT_OPCODE_INT_ABOVE_OR_EQUALS:
        case KEFIR_OPT_OPCODE_INT_BELOW:
        case KEFIR_OPT_OPCODE_INT_BELOW_OR_EQUALS:
        case KEFIR_OPT_OPCODE_FLOAT32_EQUALS:
        case KEFIR_OPT_OPCODE_FLOAT32_GREATER:
        case KEFIR_OPT_OPCODE_FLOAT32_GREATER_OR_EQUALS:
        case KEFIR_OPT_OPCODE_FLOAT32_LESSER:
        case KEFIR_OPT_OPCODE_FLOAT32_LESSER_OR_EQUALS:
        case KEFIR_OPT_OPCODE_FLOAT64_EQUALS:
        case KEFIR_OPT_OPCODE_FLOAT64_GREATER:
        case KEFIR_OPT_OPCODE_FLOAT64_LESSER:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_EQUALS:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_GREATER:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_LESSER:
        case KEFIR_OPT_OPCODE_INT64_TRUNCATE_1BIT:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_TRUNCATE_1BIT:
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
        case KEFIR_OPT_OPCODE_FLOAT64_TO_LONG_DOUBLE:
            REQUIRE_OK(kefir_opt_code_builder_long_double_truncate_1bit(mem, &func->code, instr->block_id, arg->id,
                                                                        replacement_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr->id, *replacement_ref));
            break;

        default:
            REQUIRE_OK(kefir_opt_code_builder_int64_truncate_1bit(mem, &func->code, instr->block_id, arg->id,
                                                                  replacement_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr->id, *replacement_ref));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_boot_or(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    struct kefir_opt_instruction *arg1;
    struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));
    if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_GREATER) {
        REQUIRE(arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_EQUALS, KEFIR_OK);
        REQUIRE((arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[0] &&
                 arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[1]) ||
                    (arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[1] &&
                     arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[0]),
                KEFIR_OK);

        REQUIRE_OK(kefir_opt_code_builder_int_greater_or_equals(mem, &func->code, instr->block_id,
                                                                arg1->operation.parameters.refs[0],
                                                                arg1->operation.parameters.refs[1], replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr->id, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_LESSER) {
        REQUIRE(arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_EQUALS, KEFIR_OK);
        REQUIRE((arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[0] &&
                 arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[1]) ||
                    (arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[1] &&
                     arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[0]),
                KEFIR_OK);

        REQUIRE_OK(kefir_opt_code_builder_int_lesser_or_equals(mem, &func->code, instr->block_id,
                                                               arg1->operation.parameters.refs[0],
                                                               arg1->operation.parameters.refs[1], replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr->id, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_ABOVE) {
        REQUIRE(arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_EQUALS, KEFIR_OK);
        REQUIRE((arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[0] &&
                 arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[1]) ||
                    (arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[1] &&
                     arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[0]),
                KEFIR_OK);

        REQUIRE_OK(kefir_opt_code_builder_int_above_or_equals(mem, &func->code, instr->block_id,
                                                              arg1->operation.parameters.refs[0],
                                                              arg1->operation.parameters.refs[1], replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr->id, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_BELOW) {
        REQUIRE(arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_EQUALS, KEFIR_OK);
        REQUIRE((arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[0] &&
                 arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[1]) ||
                    (arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[1] &&
                     arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[0]),
                KEFIR_OK);

        REQUIRE_OK(kefir_opt_code_builder_int_below_or_equals(mem, &func->code, instr->block_id,
                                                              arg1->operation.parameters.refs[0],
                                                              arg1->operation.parameters.refs[1], replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr->id, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_GREATER) {
        REQUIRE(arg2->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_EQUALS, KEFIR_OK);
        REQUIRE((arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[0] &&
                 arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[1]) ||
                    (arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[1] &&
                     arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[0]),
                KEFIR_OK);

        REQUIRE_OK(kefir_opt_code_builder_float32_greater_or_equals(
            mem, &func->code, instr->block_id, arg1->operation.parameters.refs[0], arg1->operation.parameters.refs[1],
            replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr->id, *replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_LESSER) {
        REQUIRE(arg2->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_EQUALS, KEFIR_OK);
        REQUIRE((arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[0] &&
                 arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[1]) ||
                    (arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[1] &&
                     arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[0]),
                KEFIR_OK);

        REQUIRE_OK(kefir_opt_code_builder_float32_lesser_or_equals(
            mem, &func->code, instr->block_id, arg1->operation.parameters.refs[0], arg1->operation.parameters.refs[1],
            replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr->id, *replacement_ref));
    }

    return KEFIR_OK;
}

static kefir_result_t simplify_int_and(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    struct kefir_opt_instruction *arg1;
    struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));
    REQUIRE(arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST, KEFIR_OK);

    if (arg2->operation.parameters.imm.uinteger == ((1ull << 32) - 1)) {
        REQUIRE_OK(kefir_opt_code_builder_int64_zero_extend_32bits(mem, &func->code, instr->block_id, arg1->id,
                                                                   replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr->id, *replacement_ref));
    } else if (arg2->operation.parameters.imm.uinteger == ((1ull << 16) - 1)) {
        REQUIRE_OK(kefir_opt_code_builder_int64_zero_extend_16bits(mem, &func->code, instr->block_id, arg1->id,
                                                                   replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr->id, *replacement_ref));
    } else if (arg2->operation.parameters.imm.uinteger == ((1ull << 8) - 1)) {
        REQUIRE_OK(kefir_opt_code_builder_int64_zero_extend_8bits(mem, &func->code, instr->block_id, arg1->id,
                                                                  replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_instruction_move_after(&func->code, instr->id, *replacement_ref));
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
        for (kefir_opt_code_block_instr_head(&func->code, block, &instr); instr != NULL;
             kefir_opt_instruction_next_sibling(&func->code, instr, &instr)) {

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

                default:
                    // Intentionally left blank
                    break;
            }

            if (replacement_ref != KEFIR_ID_NONE) {
                REQUIRE_OK(kefir_opt_code_container_replace_references(&func->code, replacement_ref, instr->id));
                if (instr->control_flow.prev != KEFIR_ID_NONE || instr->control_flow.next != KEFIR_ID_NONE) {
                    struct kefir_opt_instruction *replacement_instr = NULL;
                    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, replacement_ref, &replacement_instr));
                    if (replacement_instr->control_flow.prev == KEFIR_ID_NONE &&
                        replacement_instr->control_flow.next == KEFIR_ID_NONE) {
                        REQUIRE_OK(
                            kefir_opt_code_container_insert_control(&func->code, block, instr->id, replacement_ref));
                    }
                    REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr->id));
                }
            }
        }
    }
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassOpSimplify = {
    .name = "op-simplify", .apply = op_simplify_apply, .payload = NULL};
