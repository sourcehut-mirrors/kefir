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
#include "kefir/optimizer/structure.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t simplify_bool_not(struct kefir_mem *mem, struct kefir_opt_function *func,
                                        const struct kefir_opt_instruction *instr,
                                        kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;

    const struct kefir_opt_instruction *arg0, *arg;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg0));

#define IS_BOOL_INSTR(_instr)                                         \
    ((_instr)->operation.opcode == KEFIR_OPT_OPCODE_SCALAR_COMPARE || \
     (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_OR ||   \
     (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT16_BOOL_OR ||  \
     (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT32_BOOL_OR ||  \
     (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT64_BOOL_OR ||  \
     (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_AND ||  \
     (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT16_BOOL_AND || \
     (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT32_BOOL_AND || \
     (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT64_BOOL_AND || \
     (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_NOT ||  \
     (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT16_BOOL_NOT || \
     (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT32_BOOL_NOT || \
     (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT64_BOOL_NOT || \
     (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT8_TO_BOOL ||   \
     (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT16_TO_BOOL ||  \
     (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT32_TO_BOOL ||  \
     (_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT64_TO_BOOL ||  \
     (((_instr)->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||    \
       (_instr)->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) &&  \
      ((_instr)->operation.parameters.imm.integer == 0 || (_instr)->operation.parameters.imm.integer == 1)))
    if (instr->operation.opcode != KEFIR_OPT_OPCODE_INT8_BOOL_NOT && IS_BOOL_INSTR(arg0)) {
        REQUIRE_OK(kefir_opt_code_builder_int8_bool_not(mem, &func->code, instr->block_id, arg0->id, replacement_ref));
        return KEFIR_OK;
    }

    kefir_opt_comparison_operation_t inverse_comparison;
    if (arg0->operation.opcode == KEFIR_OPT_OPCODE_SCALAR_COMPARE) {
        switch (arg0->operation.parameters.comparison) {
#define FLIP(_opcode)                                                                                           \
    case (_opcode):                                                                                             \
        REQUIRE_OK(kefir_opt_comparison_operation_inverse((_opcode), &inverse_comparison));                     \
        REQUIRE_OK(kefir_opt_code_builder_scalar_compare(mem, &func->code, block_id, inverse_comparison,        \
                                                         arg0->operation.parameters.refs[0],                    \
                                                         arg0->operation.parameters.refs[1], replacement_ref)); \
        break
            FLIP(KEFIR_OPT_COMPARISON_INT8_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT16_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT32_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT64_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT8_NOT_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT16_NOT_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT32_NOT_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT64_NOT_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT8_GREATER);
            FLIP(KEFIR_OPT_COMPARISON_INT16_GREATER);
            FLIP(KEFIR_OPT_COMPARISON_INT32_GREATER);
            FLIP(KEFIR_OPT_COMPARISON_INT64_GREATER);
            FLIP(KEFIR_OPT_COMPARISON_INT8_GREATER_OR_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT16_GREATER_OR_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT32_GREATER_OR_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT64_GREATER_OR_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT8_LESSER);
            FLIP(KEFIR_OPT_COMPARISON_INT16_LESSER);
            FLIP(KEFIR_OPT_COMPARISON_INT32_LESSER);
            FLIP(KEFIR_OPT_COMPARISON_INT64_LESSER);
            FLIP(KEFIR_OPT_COMPARISON_INT8_LESSER_OR_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT16_LESSER_OR_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT32_LESSER_OR_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT64_LESSER_OR_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT8_ABOVE);
            FLIP(KEFIR_OPT_COMPARISON_INT16_ABOVE);
            FLIP(KEFIR_OPT_COMPARISON_INT32_ABOVE);
            FLIP(KEFIR_OPT_COMPARISON_INT64_ABOVE);
            FLIP(KEFIR_OPT_COMPARISON_INT8_ABOVE_OR_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT16_ABOVE_OR_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT32_ABOVE_OR_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT64_ABOVE_OR_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT8_BELOW);
            FLIP(KEFIR_OPT_COMPARISON_INT16_BELOW);
            FLIP(KEFIR_OPT_COMPARISON_INT32_BELOW);
            FLIP(KEFIR_OPT_COMPARISON_INT64_BELOW);
            FLIP(KEFIR_OPT_COMPARISON_INT8_BELOW_OR_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT16_BELOW_OR_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT32_BELOW_OR_EQUALS);
            FLIP(KEFIR_OPT_COMPARISON_INT64_BELOW_OR_EQUALS);

            FLIP(KEFIR_OPT_COMPARISON_FLOAT32_EQUAL);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT32_NOT_EQUAL);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT32_GREATER);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT32_GREATER_OR_EQUAL);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER_OR_EQUAL);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT32_LESSER);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT32_LESSER_OR_EQUAL);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER_OR_EQUAL);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT64_EQUAL);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT64_NOT_EQUAL);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT64_GREATER);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT64_GREATER_OR_EQUAL);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER_OR_EQUAL);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT64_LESSER);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT64_LESSER_OR_EQUAL);
            FLIP(KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER_OR_EQUAL);
#undef FLIP

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer comparison operation");
        }

        return KEFIR_OK;
    }

    REQUIRE(arg0->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_NOT ||
                arg0->operation.opcode == KEFIR_OPT_OPCODE_INT16_BOOL_NOT ||
                arg0->operation.opcode == KEFIR_OPT_OPCODE_INT32_BOOL_NOT ||
                arg0->operation.opcode == KEFIR_OPT_OPCODE_INT64_BOOL_NOT,
            KEFIR_OK);

    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg0->operation.parameters.refs[0], &arg));
    switch (arg->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT16_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT32_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT64_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT8_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT16_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT32_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT64_BOOL_AND:
        case KEFIR_OPT_OPCODE_INT8_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT16_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT32_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT64_BOOL_OR:
        case KEFIR_OPT_OPCODE_SCALAR_COMPARE:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_EQUALS:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_GREATER:
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_LESSER:
        case KEFIR_OPT_OPCODE_INT8_TO_BOOL:
        case KEFIR_OPT_OPCODE_INT16_TO_BOOL:
        case KEFIR_OPT_OPCODE_INT32_TO_BOOL:
        case KEFIR_OPT_OPCODE_INT64_TO_BOOL:
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
            REQUIRE_OK(
                kefir_opt_code_builder_int64_bool_not(mem, &func->code, block_id, equality_ref, replacement_ref));
        } break;

        default:
            switch (arg0->operation.opcode) {
                case KEFIR_OPT_OPCODE_INT8_BOOL_NOT:
                    REQUIRE_OK(
                        kefir_opt_code_builder_int8_to_bool(mem, &func->code, block_id, arg->id, replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_INT16_BOOL_NOT:
                    REQUIRE_OK(
                        kefir_opt_code_builder_int16_to_bool(mem, &func->code, block_id, arg->id, replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_INT32_BOOL_NOT:
                    REQUIRE_OK(
                        kefir_opt_code_builder_int32_to_bool(mem, &func->code, block_id, arg->id, replacement_ref));
                    break;

                case KEFIR_OPT_OPCODE_INT64_BOOL_NOT:
                    REQUIRE_OK(
                        kefir_opt_code_builder_int64_to_bool(mem, &func->code, block_id, arg->id, replacement_ref));
                    break;

                default:
                    // Intentionally left blank
                    break;
            }
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_or_candidate(struct kefir_mem *mem, struct kefir_opt_function *func,
                                            struct kefir_opt_code_structure *structure,
                                            kefir_opt_instruction_ref_t instr_ref,
                                            kefir_opt_instruction_ref_t *replacement_ref) {
    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_ref, &instr));
    REQUIRE(instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_OR ||
                instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_BOOL_OR ||
                instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_BOOL_OR ||
                instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_BOOL_OR,
            KEFIR_OK);

    const struct kefir_opt_instruction *arg1, *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    kefir_bool_t only_predecessor;
    REQUIRE_OK(kefir_opt_code_structure_block_exclusive_direct_predecessor(structure, arg1->block_id, instr->block_id,
                                                                           &only_predecessor));
    if (only_predecessor) {
        const struct kefir_opt_code_block *arg1_block;
        REQUIRE_OK(kefir_opt_code_container_block(&func->code, arg1->block_id, &arg1_block));

        kefir_opt_instruction_ref_t arg1_block_tail_ref;
        REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&func->code, arg1_block, &arg1_block_tail_ref));
        REQUIRE(arg1_block_tail_ref != KEFIR_ID_NONE, KEFIR_OK);

        const struct kefir_opt_instruction *arg1_block_tail;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1_block_tail_ref, &arg1_block_tail));
        REQUIRE(arg1_block_tail->operation.opcode == KEFIR_OPT_OPCODE_BRANCH, KEFIR_OK);
        REQUIRE(arg1_block_tail->operation.parameters.branch.alternative_block == instr->block_id, KEFIR_OK);
        REQUIRE(arg1_block_tail->operation.parameters.branch.condition_ref == arg1->id, KEFIR_OK);

    } else {
        kefir_opt_instruction_ref_t sole_use_ref;
        REQUIRE_OK(kefir_opt_instruction_get_sole_use(&func->code, instr_ref, &sole_use_ref));
        REQUIRE(sole_use_ref != KEFIR_ID_NONE, KEFIR_OK);

        const struct kefir_opt_instruction *sole_use_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, sole_use_ref, &sole_use_instr));
        REQUIRE(sole_use_instr->operation.opcode == KEFIR_OPT_OPCODE_SELECT, KEFIR_OK);
        REQUIRE(sole_use_instr->operation.parameters.refs[0] == instr->operation.parameters.refs[0], KEFIR_OK);
        REQUIRE(sole_use_instr->operation.parameters.refs[1] == instr->operation.parameters.refs[0], KEFIR_OK);
        REQUIRE(sole_use_instr->operation.parameters.refs[2] == instr->id, KEFIR_OK);
    }

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_BOOL_OR:
            REQUIRE_OK(
                kefir_opt_code_builder_int8_to_bool(mem, &func->code, instr->block_id, arg2->id, replacement_ref));
            break;

        case KEFIR_OPT_OPCODE_INT16_BOOL_OR:
            REQUIRE_OK(
                kefir_opt_code_builder_int16_to_bool(mem, &func->code, instr->block_id, arg2->id, replacement_ref));
            break;

        case KEFIR_OPT_OPCODE_INT32_BOOL_OR:
            REQUIRE_OK(
                kefir_opt_code_builder_int32_to_bool(mem, &func->code, instr->block_id, arg2->id, replacement_ref));
            break;

        case KEFIR_OPT_OPCODE_INT64_BOOL_OR:
            REQUIRE_OK(
                kefir_opt_code_builder_int64_to_bool(mem, &func->code, instr->block_id, arg2->id, replacement_ref));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_bool_or(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       struct kefir_opt_code_structure *structure,
                                       const struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id;

    REQUIRE_OK(simplify_or_candidate(mem, func, structure, instr->id, replacement_ref));
    REQUIRE(*replacement_ref == KEFIR_ID_NONE, KEFIR_OK);

    const struct kefir_opt_instruction *arg1, *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

#define SIMPLIFY_CONST_BOOL_OR(_width)                                                                              \
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT##_width##_BOOL_OR &&                                        \
        (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||                                                    \
         arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {                                                  \
        if (((kefir_uint##_width##_t) arg1->operation.parameters.imm.integer) == 0) {                               \
            REQUIRE_OK(kefir_opt_code_builder_int##_width##_to_bool(mem, &func->code, instr->block_id, arg2->id,    \
                                                                    replacement_ref));                              \
            return KEFIR_OK;                                                                                        \
        } else {                                                                                                    \
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, instr->block_id, 1, replacement_ref)); \
            return KEFIR_OK;                                                                                        \
        }                                                                                                           \
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT##_width##_BOOL_OR &&                                 \
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||                                             \
                arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {                                           \
        if (((kefir_uint##_width##_t) arg2->operation.parameters.imm.integer) == 0) {                               \
            REQUIRE_OK(kefir_opt_code_builder_int##_width##_to_bool(mem, &func->code, instr->block_id, arg1->id,    \
                                                                    replacement_ref));                              \
            return KEFIR_OK;                                                                                        \
        } else {                                                                                                    \
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, instr->block_id, 1, replacement_ref)); \
            return KEFIR_OK;                                                                                        \
        }                                                                                                           \
    }
    SIMPLIFY_CONST_BOOL_OR(8)
    else SIMPLIFY_CONST_BOOL_OR(16) else SIMPLIFY_CONST_BOOL_OR(32) else SIMPLIFY_CONST_BOOL_OR(64)
#undef SIMPLIFY_CONST_BOOL_OR
        else if (instr->operation.opcode != KEFIR_OPT_OPCODE_INT8_BOOL_OR && IS_BOOL_INSTR(arg1) &&
                 IS_BOOL_INSTR(arg2)) {
        REQUIRE_OK(kefir_opt_code_builder_int8_bool_or(mem, &func->code, instr->block_id, arg1->id, arg2->id,
                                                       replacement_ref));
        return KEFIR_OK;
    }
#define SIMPLIFY_HALF_BOOL_OR(_width)                                                                           \
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_OR &&                                             \
        arg1->operation.opcode == KEFIR_OPT_OPCODE_INT##_width##_TO_BOOL && IS_BOOL_INSTR(arg2)) {              \
        REQUIRE_OK(kefir_opt_code_builder_int##_width##_bool_or(                                                \
            mem, &func->code, instr->block_id, arg1->operation.parameters.refs[0], arg2->id, replacement_ref)); \
        return KEFIR_OK;                                                                                        \
    }
    else SIMPLIFY_HALF_BOOL_OR(8) else SIMPLIFY_HALF_BOOL_OR(16) else SIMPLIFY_HALF_BOOL_OR(
        32) else SIMPLIFY_HALF_BOOL_OR(64)
#undef SIMPLIFY_HALF_BOOL_OR
#define SIMPLIFY_HALF_BOOL_OR(_width)                                                                                  \
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_OR && IS_BOOL_INSTR(arg1) &&                             \
        arg2->operation.opcode == KEFIR_OPT_OPCODE_INT##_width##_TO_BOOL) {                                            \
        REQUIRE_OK(kefir_opt_code_builder_int##_width##_bool_or(mem, &func->code, instr->block_id, arg1->id,           \
                                                                arg2->operation.parameters.refs[0], replacement_ref)); \
        return KEFIR_OK;                                                                                               \
    }
        else SIMPLIFY_HALF_BOOL_OR(8) else SIMPLIFY_HALF_BOOL_OR(16) else SIMPLIFY_HALF_BOOL_OR(
            32) else SIMPLIFY_HALF_BOOL_OR(64)
#undef SIMPLIFY_HALF_BOOL_OR
#define SIMPLIFY_HALF_BOOL_OR2(_width)                                                                          \
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT##_width##_BOOL_OR &&                                    \
        arg1->operation.opcode == KEFIR_OPT_OPCODE_INT##_width##_TO_BOOL) {                                     \
        REQUIRE_OK(kefir_opt_code_builder_int##_width##_bool_or(                                                \
            mem, &func->code, instr->block_id, arg1->operation.parameters.refs[0], arg2->id, replacement_ref)); \
        return KEFIR_OK;                                                                                        \
    }
            else SIMPLIFY_HALF_BOOL_OR2(8) else SIMPLIFY_HALF_BOOL_OR2(16) else SIMPLIFY_HALF_BOOL_OR2(
                32) else SIMPLIFY_HALF_BOOL_OR2(64)
#undef SIMPLIFY_HALF_BOOL_OR2
#define SIMPLIFY_HALF_BOOL_OR2(_width)                                                                                 \
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT##_width##_BOOL_OR &&                                           \
        arg2->operation.opcode == KEFIR_OPT_OPCODE_INT##_width##_TO_BOOL) {                                            \
        REQUIRE_OK(kefir_opt_code_builder_int##_width##_bool_or(mem, &func->code, instr->block_id, arg1->id,           \
                                                                arg2->operation.parameters.refs[0], replacement_ref)); \
        return KEFIR_OK;                                                                                               \
    }
                else SIMPLIFY_HALF_BOOL_OR2(8) else SIMPLIFY_HALF_BOOL_OR2(16) else SIMPLIFY_HALF_BOOL_OR2(
                    32) else SIMPLIFY_HALF_BOOL_OR2(64)
#undef SIMPLIFY_HALF_BOOL_OR2
                    else if ((instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_OR ||
                              instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_BOOL_OR ||
                              instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_BOOL_OR ||
                              instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_BOOL_OR) &&
                             arg1->operation.opcode == instr->operation.opcode &&
                             (arg1->operation.parameters.refs[0] == instr->operation.parameters.refs[0] ||
                              arg1->operation.parameters.refs[1] == instr->operation.parameters.refs[0])) {
        *replacement_ref = arg1->id;
        return KEFIR_OK;
    }
    else if ((instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_OR ||
              instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_BOOL_OR ||
              instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_BOOL_OR ||
              instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_BOOL_OR) &&
             arg2->operation.opcode == instr->operation.opcode &&
             (arg2->operation.parameters.refs[0] == instr->operation.parameters.refs[0] ||
              arg2->operation.parameters.refs[1] == instr->operation.parameters.refs[0])) {
        *replacement_ref = arg2->id;
        return KEFIR_OK;
    }
    else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_OR && arg1->id == arg2->id) {
        REQUIRE_OK(kefir_opt_code_builder_int8_to_bool(mem, &func->code, instr->block_id, arg1->id, replacement_ref));
        return KEFIR_OK;
    }
    else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_BOOL_OR && arg1->id == arg2->id) {
        REQUIRE_OK(kefir_opt_code_builder_int16_to_bool(mem, &func->code, instr->block_id, arg1->id, replacement_ref));
        return KEFIR_OK;
    }
    else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_BOOL_OR && arg1->id == arg2->id) {
        REQUIRE_OK(kefir_opt_code_builder_int32_to_bool(mem, &func->code, instr->block_id, arg1->id, replacement_ref));
        return KEFIR_OK;
    }
    else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_BOOL_OR && arg1->id == arg2->id) {
        REQUIRE_OK(kefir_opt_code_builder_int64_to_bool(mem, &func->code, instr->block_id, arg1->id, replacement_ref));
        return KEFIR_OK;
    }

    REQUIRE(arg1->operation.opcode == KEFIR_OPT_OPCODE_SCALAR_COMPARE &&
                arg2->operation.opcode == KEFIR_OPT_OPCODE_SCALAR_COMPARE,
            KEFIR_OK);

#define FUSE(_opcode1, _opcode2, _opcode3, _asymetric)                                                                \
    if (arg1->operation.parameters.comparison == (_opcode1) && arg2->operation.parameters.comparison == (_opcode2) && \
        ((arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[0] &&                                 \
          arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[1]) ||                                \
         ((_asymetric) && arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[1] &&                 \
          arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[0]))) {                               \
        REQUIRE_OK(kefir_opt_code_builder_scalar_compare(mem, &func->code, block_id, (_opcode3),                      \
                                                         arg1->operation.parameters.refs[0],                          \
                                                         arg1->operation.parameters.refs[1], replacement_ref));       \
        return KEFIR_OK;                                                                                              \
    }                                                                                                                 \
    if (arg1->operation.parameters.comparison == (_opcode2) && arg2->operation.parameters.comparison == (_opcode1) && \
        ((arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[0] &&                                 \
          arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[1]) ||                                \
         ((_asymetric) && arg1->operation.parameters.refs[0] == arg2->operation.parameters.refs[1] &&                 \
          arg1->operation.parameters.refs[1] == arg2->operation.parameters.refs[0]))) {                               \
        REQUIRE_OK(kefir_opt_code_builder_scalar_compare(mem, &func->code, block_id, (_opcode3),                      \
                                                         arg2->operation.parameters.refs[0],                          \
                                                         arg2->operation.parameters.refs[1], replacement_ref));       \
        return KEFIR_OK;                                                                                              \
    }

    FUSE(KEFIR_OPT_COMPARISON_INT8_GREATER, KEFIR_OPT_COMPARISON_INT8_EQUALS,
         KEFIR_OPT_COMPARISON_INT8_GREATER_OR_EQUALS, true)
    FUSE(KEFIR_OPT_COMPARISON_INT16_GREATER, KEFIR_OPT_COMPARISON_INT16_EQUALS,
         KEFIR_OPT_COMPARISON_INT16_GREATER_OR_EQUALS, true)
    FUSE(KEFIR_OPT_COMPARISON_INT32_GREATER, KEFIR_OPT_COMPARISON_INT32_EQUALS,
         KEFIR_OPT_COMPARISON_INT32_GREATER_OR_EQUALS, true)
    FUSE(KEFIR_OPT_COMPARISON_INT64_GREATER, KEFIR_OPT_COMPARISON_INT64_EQUALS,
         KEFIR_OPT_COMPARISON_INT64_GREATER_OR_EQUALS, true)
    FUSE(KEFIR_OPT_COMPARISON_INT8_LESSER, KEFIR_OPT_COMPARISON_INT8_EQUALS, KEFIR_OPT_COMPARISON_INT8_LESSER_OR_EQUALS,
         true)
    FUSE(KEFIR_OPT_COMPARISON_INT16_LESSER, KEFIR_OPT_COMPARISON_INT16_EQUALS,
         KEFIR_OPT_COMPARISON_INT16_LESSER_OR_EQUALS, true)
    FUSE(KEFIR_OPT_COMPARISON_INT32_LESSER, KEFIR_OPT_COMPARISON_INT32_EQUALS,
         KEFIR_OPT_COMPARISON_INT32_LESSER_OR_EQUALS, true)
    FUSE(KEFIR_OPT_COMPARISON_INT64_LESSER, KEFIR_OPT_COMPARISON_INT64_EQUALS,
         KEFIR_OPT_COMPARISON_INT64_LESSER_OR_EQUALS, true)
    FUSE(KEFIR_OPT_COMPARISON_INT8_ABOVE, KEFIR_OPT_COMPARISON_INT8_EQUALS, KEFIR_OPT_COMPARISON_INT8_ABOVE_OR_EQUALS,
         true)
    FUSE(KEFIR_OPT_COMPARISON_INT16_ABOVE, KEFIR_OPT_COMPARISON_INT16_EQUALS,
         KEFIR_OPT_COMPARISON_INT16_ABOVE_OR_EQUALS, true)
    FUSE(KEFIR_OPT_COMPARISON_INT32_ABOVE, KEFIR_OPT_COMPARISON_INT32_EQUALS,
         KEFIR_OPT_COMPARISON_INT32_ABOVE_OR_EQUALS, true)
    FUSE(KEFIR_OPT_COMPARISON_INT64_ABOVE, KEFIR_OPT_COMPARISON_INT64_EQUALS,
         KEFIR_OPT_COMPARISON_INT64_ABOVE_OR_EQUALS, true)
    FUSE(KEFIR_OPT_COMPARISON_INT8_BELOW, KEFIR_OPT_COMPARISON_INT8_EQUALS, KEFIR_OPT_COMPARISON_INT8_BELOW_OR_EQUALS,
         true)
    FUSE(KEFIR_OPT_COMPARISON_INT16_BELOW, KEFIR_OPT_COMPARISON_INT16_EQUALS,
         KEFIR_OPT_COMPARISON_INT16_BELOW_OR_EQUALS, true)
    FUSE(KEFIR_OPT_COMPARISON_INT32_BELOW, KEFIR_OPT_COMPARISON_INT32_EQUALS,
         KEFIR_OPT_COMPARISON_INT32_BELOW_OR_EQUALS, true)
    FUSE(KEFIR_OPT_COMPARISON_INT64_BELOW, KEFIR_OPT_COMPARISON_INT64_EQUALS,
         KEFIR_OPT_COMPARISON_INT64_BELOW_OR_EQUALS, true)

    FUSE(KEFIR_OPT_COMPARISON_FLOAT32_GREATER, KEFIR_OPT_COMPARISON_FLOAT32_EQUAL,
         KEFIR_OPT_COMPARISON_FLOAT32_GREATER_OR_EQUAL, true)
    FUSE(KEFIR_OPT_COMPARISON_FLOAT32_LESSER, KEFIR_OPT_COMPARISON_FLOAT32_EQUAL,
         KEFIR_OPT_COMPARISON_FLOAT32_LESSER_OR_EQUAL, true)
    FUSE(KEFIR_OPT_COMPARISON_FLOAT64_GREATER, KEFIR_OPT_COMPARISON_FLOAT64_EQUAL,
         KEFIR_OPT_COMPARISON_FLOAT64_GREATER_OR_EQUAL, true)
    FUSE(KEFIR_OPT_COMPARISON_FLOAT64_LESSER, KEFIR_OPT_COMPARISON_FLOAT64_EQUAL,
         KEFIR_OPT_COMPARISON_FLOAT64_LESSER_OR_EQUAL, true)

#undef FUSE

    return KEFIR_OK;
}

static kefir_result_t simplify_bool_and(struct kefir_mem *mem, struct kefir_opt_function *func,
                                        struct kefir_opt_code_structure *structure,
                                        kefir_opt_instruction_ref_t instr_ref,
                                        kefir_opt_instruction_ref_t *replacement_ref) {
    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_ref, &instr));
    REQUIRE(instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_AND ||
                instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_BOOL_AND ||
                instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_BOOL_AND ||
                instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_BOOL_AND,
            KEFIR_OK);

    const struct kefir_opt_instruction *arg1, *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

#define SIMPLIFY_CONST_BOOL_AND(_width)                                                                             \
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT##_width##_BOOL_AND &&                                       \
        (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||                                                    \
         arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {                                                  \
        if (((kefir_uint##_width##_t) arg1->operation.parameters.imm.integer) != 0) {                               \
            REQUIRE_OK(kefir_opt_code_builder_int##_width##_to_bool(mem, &func->code, instr->block_id, arg2->id,    \
                                                                    replacement_ref));                              \
            return KEFIR_OK;                                                                                        \
        } else {                                                                                                    \
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, instr->block_id, 0, replacement_ref)); \
            return KEFIR_OK;                                                                                        \
        }                                                                                                           \
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT##_width##_BOOL_AND &&                                \
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||                                             \
                arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {                                           \
        if (((kefir_uint##_width##_t) arg2->operation.parameters.imm.integer) != 0) {                               \
            REQUIRE_OK(kefir_opt_code_builder_int##_width##_to_bool(mem, &func->code, instr->block_id, arg1->id,    \
                                                                    replacement_ref));                              \
            return KEFIR_OK;                                                                                        \
        } else {                                                                                                    \
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, instr->block_id, 0, replacement_ref)); \
            return KEFIR_OK;                                                                                        \
        }                                                                                                           \
    }
    SIMPLIFY_CONST_BOOL_AND(8)
    else SIMPLIFY_CONST_BOOL_AND(16) else SIMPLIFY_CONST_BOOL_AND(32) else SIMPLIFY_CONST_BOOL_AND(64)
#undef SIMPLIFY_CONST_BOOL_AND
        else if (instr->operation.opcode != KEFIR_OPT_OPCODE_INT8_BOOL_AND && IS_BOOL_INSTR(arg1) &&
                 IS_BOOL_INSTR(arg2)) {
        REQUIRE_OK(kefir_opt_code_builder_int8_bool_and(mem, &func->code, instr->block_id, arg1->id, arg2->id,
                                                        replacement_ref));
        return KEFIR_OK;
    }
#define SIMPLIFY_HALF_BOOL_AND(_width)                                                                          \
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_AND &&                                            \
        arg1->operation.opcode == KEFIR_OPT_OPCODE_INT##_width##_TO_BOOL && IS_BOOL_INSTR(arg2)) {              \
        REQUIRE_OK(kefir_opt_code_builder_int##_width##_bool_and(                                               \
            mem, &func->code, instr->block_id, arg1->operation.parameters.refs[0], arg2->id, replacement_ref)); \
        return KEFIR_OK;                                                                                        \
    }
    else SIMPLIFY_HALF_BOOL_AND(8) else SIMPLIFY_HALF_BOOL_AND(16) else SIMPLIFY_HALF_BOOL_AND(
        32) else SIMPLIFY_HALF_BOOL_AND(64)
#undef SIMPLIFY_HALF_BOOL_AND
#define SIMPLIFY_HALF_BOOL_AND(_width)                                                                          \
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_AND && IS_BOOL_INSTR(arg1) &&                     \
        arg2->operation.opcode == KEFIR_OPT_OPCODE_INT##_width##_TO_BOOL) {                                     \
        REQUIRE_OK(kefir_opt_code_builder_int##_width##_bool_and(                                               \
            mem, &func->code, instr->block_id, arg1->id, arg2->operation.parameters.refs[0], replacement_ref)); \
        return KEFIR_OK;                                                                                        \
    }
        else SIMPLIFY_HALF_BOOL_AND(8) else SIMPLIFY_HALF_BOOL_AND(16) else SIMPLIFY_HALF_BOOL_AND(
            32) else SIMPLIFY_HALF_BOOL_AND(64)
#undef SIMPLIFY_HALF_BOOL_AND
#define SIMPLIFY_HALF_BOOL_AND2(_width)                                                                         \
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT##_width##_BOOL_AND &&                                   \
        arg1->operation.opcode == KEFIR_OPT_OPCODE_INT##_width##_TO_BOOL) {                                     \
        REQUIRE_OK(kefir_opt_code_builder_int##_width##_bool_and(                                               \
            mem, &func->code, instr->block_id, arg1->operation.parameters.refs[0], arg2->id, replacement_ref)); \
        return KEFIR_OK;                                                                                        \
    }
            else SIMPLIFY_HALF_BOOL_AND2(8) else SIMPLIFY_HALF_BOOL_AND2(16) else SIMPLIFY_HALF_BOOL_AND2(
                32) else SIMPLIFY_HALF_BOOL_AND2(64)
#undef SIMPLIFY_HALF_BOOL_AND2
#define SIMPLIFY_HALF_BOOL_AND2(_width)                                                                         \
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT##_width##_BOOL_AND &&                                   \
        arg2->operation.opcode == KEFIR_OPT_OPCODE_INT##_width##_TO_BOOL) {                                     \
        REQUIRE_OK(kefir_opt_code_builder_int##_width##_bool_and(                                               \
            mem, &func->code, instr->block_id, arg1->id, arg2->operation.parameters.refs[0], replacement_ref)); \
        return KEFIR_OK;                                                                                        \
    }
                else SIMPLIFY_HALF_BOOL_AND2(8) else SIMPLIFY_HALF_BOOL_AND2(16) else SIMPLIFY_HALF_BOOL_AND2(
                    32) else SIMPLIFY_HALF_BOOL_AND2(64)
#undef SIMPLIFY_HALF_BOOL_AND2
                    else if ((instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_AND ||
                              instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_BOOL_AND ||
                              instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_BOOL_AND ||
                              instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_BOOL_AND) &&
                             arg1->operation.opcode == instr->operation.opcode &&
                             (arg1->operation.parameters.refs[0] == instr->operation.parameters.refs[1] ||
                              arg1->operation.parameters.refs[1] == instr->operation.parameters.refs[1])) {
        *replacement_ref = arg1->id;
        return KEFIR_OK;
    }
    else if ((instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_AND ||
              instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_BOOL_AND ||
              instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_BOOL_AND ||
              instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_BOOL_AND) &&
             arg2->operation.opcode == instr->operation.opcode &&
             (arg2->operation.parameters.refs[0] == instr->operation.parameters.refs[0] ||
              arg2->operation.parameters.refs[1] == instr->operation.parameters.refs[0])) {
        *replacement_ref = arg2->id;
        return KEFIR_OK;
    }
    else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_AND && arg1->id == arg2->id) {
        REQUIRE_OK(kefir_opt_code_builder_int8_to_bool(mem, &func->code, instr->block_id, arg1->id, replacement_ref));
        return KEFIR_OK;
    }
    else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_BOOL_AND && arg1->id == arg2->id) {
        REQUIRE_OK(kefir_opt_code_builder_int16_to_bool(mem, &func->code, instr->block_id, arg1->id, replacement_ref));
        return KEFIR_OK;
    }
    else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_BOOL_AND && arg1->id == arg2->id) {
        REQUIRE_OK(kefir_opt_code_builder_int32_to_bool(mem, &func->code, instr->block_id, arg1->id, replacement_ref));
        return KEFIR_OK;
    }
    else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_BOOL_AND && arg1->id == arg2->id) {
        REQUIRE_OK(kefir_opt_code_builder_int64_to_bool(mem, &func->code, instr->block_id, arg1->id, replacement_ref));
        return KEFIR_OK;
    }

    REQUIRE(arg1->block_id != instr->block_id, KEFIR_OK);

    kefir_bool_t only_predecessor;
    REQUIRE_OK(kefir_opt_code_structure_block_exclusive_direct_predecessor(structure, arg1->block_id, instr->block_id,
                                                                           &only_predecessor));
    if (only_predecessor) {
        const struct kefir_opt_code_block *arg1_block;
        REQUIRE_OK(kefir_opt_code_container_block(&func->code, arg1->block_id, &arg1_block));

        kefir_opt_instruction_ref_t arg1_block_tail_ref;
        REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&func->code, arg1_block, &arg1_block_tail_ref));
        REQUIRE(arg1_block_tail_ref != KEFIR_ID_NONE, KEFIR_OK);

        const struct kefir_opt_instruction *arg1_block_tail;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1_block_tail_ref, &arg1_block_tail));
        REQUIRE(arg1_block_tail->operation.opcode == KEFIR_OPT_OPCODE_BRANCH, KEFIR_OK);
        REQUIRE(arg1_block_tail->operation.parameters.branch.alternative_block == instr->block_id, KEFIR_OK);

        if (KEFIR_OPT_BRANCH_CONDITION_VARIANT_IS_DIRECT(
                arg1_block_tail->operation.parameters.branch.condition_variant)) {
            const struct kefir_opt_instruction *condition_instr;
            REQUIRE_OK(kefir_opt_code_container_instr(
                &func->code, arg1_block_tail->operation.parameters.branch.condition_ref, &condition_instr));
            REQUIRE(condition_instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_NOT, KEFIR_OK);
            REQUIRE(condition_instr->operation.parameters.refs[0] == arg1->id, KEFIR_OK);
        }
    } else {
        kefir_opt_instruction_ref_t sole_use_ref;
        REQUIRE_OK(kefir_opt_instruction_get_sole_use(&func->code, instr_ref, &sole_use_ref));
        REQUIRE(sole_use_ref != KEFIR_ID_NONE, KEFIR_OK);

        const struct kefir_opt_instruction *sole_use_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, sole_use_ref, &sole_use_instr));
        REQUIRE(sole_use_instr->operation.opcode == KEFIR_OPT_OPCODE_SELECT, KEFIR_OK);
        REQUIRE(sole_use_instr->operation.parameters.refs[0] == instr->operation.parameters.refs[0], KEFIR_OK);
        REQUIRE(sole_use_instr->operation.parameters.refs[1] == instr->operation.parameters.refs[0], KEFIR_OK);
        REQUIRE(sole_use_instr->operation.parameters.refs[2] == instr->id, KEFIR_OK);
    }

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_BOOL_AND:
            REQUIRE_OK(
                kefir_opt_code_builder_int8_to_bool(mem, &func->code, instr->block_id, arg2->id, replacement_ref));
            break;

        case KEFIR_OPT_OPCODE_INT16_BOOL_AND:
            REQUIRE_OK(
                kefir_opt_code_builder_int16_to_bool(mem, &func->code, instr->block_id, arg2->id, replacement_ref));
            break;

        case KEFIR_OPT_OPCODE_INT32_BOOL_AND:
            REQUIRE_OK(
                kefir_opt_code_builder_int32_to_bool(mem, &func->code, instr->block_id, arg2->id, replacement_ref));
            break;

        case KEFIR_OPT_OPCODE_INT64_BOOL_AND:
            REQUIRE_OK(
                kefir_opt_code_builder_int64_to_bool(mem, &func->code, instr->block_id, arg2->id, replacement_ref));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t builder_int_not(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                      kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t ref1,
                                      kefir_opt_instruction_ref_t *result_ref, kefir_opt_opcode_t original) {
    switch (original) {
        case KEFIR_OPT_OPCODE_INT8_NOT:
            REQUIRE_OK(kefir_opt_code_builder_int8_not(mem, code, block_id, ref1, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT16_NOT:
            REQUIRE_OK(kefir_opt_code_builder_int16_not(mem, code, block_id, ref1, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT32_NOT:
            REQUIRE_OK(kefir_opt_code_builder_int32_not(mem, code, block_id, ref1, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT64_NOT:
            REQUIRE_OK(kefir_opt_code_builder_int64_not(mem, code, block_id, ref1, result_ref));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected original instruction opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_not(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       const struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    kefir_opt_opcode_t original_opcode = instr->operation.opcode;
    const kefir_opt_block_id_t block_id = instr->block_id;

    const struct kefir_opt_instruction *arg1;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));

    if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_NOT &&
        (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
         arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
         arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
         arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
         arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
         arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        REQUIRE_OK(builder_int_not(mem, &func->code, block_id, arg1->operation.parameters.refs[0], replacement_ref,
                                   original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_NOT &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        REQUIRE_OK(builder_int_not(mem, &func->code, block_id, arg1->operation.parameters.refs[0], replacement_ref,
                                   original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_NOT &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        REQUIRE_OK(builder_int_not(mem, &func->code, block_id, arg1->operation.parameters.refs[0], replacement_ref,
                                   original_opcode));
    }
    return KEFIR_OK;
}

static kefir_result_t builder_int_to_bool(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                          kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t ref1,
                                          kefir_opt_instruction_ref_t *result_ref, kefir_opt_opcode_t original) {
    switch (original) {
        case KEFIR_OPT_OPCODE_INT8_TO_BOOL:
            REQUIRE_OK(kefir_opt_code_builder_int8_to_bool(mem, code, block_id, ref1, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT16_TO_BOOL:
            REQUIRE_OK(kefir_opt_code_builder_int16_to_bool(mem, code, block_id, ref1, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT32_TO_BOOL:
            REQUIRE_OK(kefir_opt_code_builder_int32_to_bool(mem, code, block_id, ref1, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT64_TO_BOOL:
            REQUIRE_OK(kefir_opt_code_builder_int64_to_bool(mem, code, block_id, ref1, result_ref));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected original instruction opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_to_bool(struct kefir_mem *mem, struct kefir_opt_function *func,
                                           const struct kefir_opt_instruction *instr,
                                           kefir_opt_instruction_ref_t *replacement_ref) {
    kefir_opt_opcode_t original_opcode = instr->operation.opcode;
    const kefir_opt_block_id_t block_id = instr->block_id;

    const struct kefir_opt_instruction *arg1;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));

    if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_TO_BOOL &&
        (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
         arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
         arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
         arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
         arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
         arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        REQUIRE_OK(builder_int_to_bool(mem, &func->code, block_id, arg1->operation.parameters.refs[0], replacement_ref,
                                       original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_TO_BOOL &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        REQUIRE_OK(builder_int_to_bool(mem, &func->code, block_id, arg1->operation.parameters.refs[0], replacement_ref,
                                       original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_TO_BOOL &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        REQUIRE_OK(builder_int_to_bool(mem, &func->code, block_id, arg1->operation.parameters.refs[0], replacement_ref,
                                       original_opcode));
    } else if ((instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_TO_BOOL ||
                instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_TO_BOOL ||
                instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_TO_BOOL ||
                instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_TO_BOOL) &&
               IS_BOOL_INSTR(arg1)) {
        *replacement_ref = arg1->id;
    }
    return KEFIR_OK;
}

static kefir_result_t builder_int_and(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                      kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t ref1,
                                      kefir_opt_instruction_ref_t ref2, kefir_opt_instruction_ref_t *result_ref,
                                      kefir_opt_opcode_t original) {
    switch (original) {
        case KEFIR_OPT_OPCODE_INT8_AND:
            REQUIRE_OK(kefir_opt_code_builder_int8_and(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT16_AND:
            REQUIRE_OK(kefir_opt_code_builder_int16_and(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT32_AND:
            REQUIRE_OK(kefir_opt_code_builder_int32_and(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT64_AND:
            REQUIRE_OK(kefir_opt_code_builder_int64_and(mem, code, block_id, ref1, ref2, result_ref));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected original instruction opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_and(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       const struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    kefir_opt_opcode_t original_opcode = instr->operation.opcode;
    const kefir_opt_block_id_t block_id = instr->block_id;

    const struct kefir_opt_instruction *arg1;
    const struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    const kefir_opt_instruction_ref_t arg1_id = arg1->id;

    if (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
        arg2->operation.parameters.imm.uinteger == ((1ull << 32) - 1)) {
        REQUIRE_OK(
            kefir_opt_code_builder_int64_zero_extend_32bits(mem, &func->code, block_id, arg1_id, replacement_ref));
    } else if (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
               arg2->operation.parameters.imm.uinteger == ((1ull << 16) - 1)) {
        REQUIRE_OK(
            kefir_opt_code_builder_int64_zero_extend_16bits(mem, &func->code, block_id, arg1_id, replacement_ref));
    } else if (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
               arg2->operation.parameters.imm.uinteger == ((1ull << 8) - 1)) {
        REQUIRE_OK(
            kefir_opt_code_builder_int64_zero_extend_8bits(mem, &func->code, block_id, arg1_id, replacement_ref));
    } else if (arg1->operation.opcode == original_opcode && (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                                                             arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *and_arg1;
        const struct kefir_opt_instruction *and_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &and_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &and_arg2));

        const kefir_opt_instruction_ref_t and_arg1_id = and_arg1->id, and_arg2_id = and_arg2->id;

        kefir_uint64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                         ? (kefir_uint64_t) arg2->operation.parameters.imm.integer
                                         : arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (and_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand &= (kefir_uint64_t) and_arg1->operation.parameters.imm.integer;
            operand_ref = and_arg2_id;
        } else if (and_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand &= and_arg1->operation.parameters.imm.uinteger;
            operand_ref = and_arg2_id;
        } else if (and_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand &= (kefir_uint64_t) and_arg2->operation.parameters.imm.integer;
            operand_ref = and_arg1_id;
        } else if (and_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand &= and_arg2->operation.parameters.imm.uinteger;
            operand_ref = and_arg1_id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                builder_int_and(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref, original_opcode));
        }
    } else if (arg2->operation.opcode == original_opcode && (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                                                             arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *and_arg1;
        const struct kefir_opt_instruction *and_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[0], &and_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[1], &and_arg2));

        const kefir_opt_instruction_ref_t and_arg1_id = and_arg1->id, and_arg2_id = and_arg2->id;

        kefir_uint64_t imm_operand = arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                         ? (kefir_uint64_t) arg1->operation.parameters.imm.integer
                                         : arg1->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (and_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand &= (kefir_uint64_t) and_arg1->operation.parameters.imm.integer;
            operand_ref = and_arg2_id;
        } else if (and_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand &= and_arg1->operation.parameters.imm.uinteger;
            operand_ref = and_arg2_id;
        } else if (and_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand &= (kefir_uint64_t) and_arg2->operation.parameters.imm.integer;
            operand_ref = and_arg1_id;
        } else if (and_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand &= and_arg2->operation.parameters.imm.uinteger;
            operand_ref = and_arg1_id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                builder_int_and(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref, original_opcode));
        }
    } else if ((arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) &&
               arg2->operation.opcode != KEFIR_OPT_OPCODE_INT_CONST &&
               arg2->operation.opcode != KEFIR_OPT_OPCODE_UINT_CONST) {
        REQUIRE_OK(builder_int_and(mem, &func->code, block_id, arg2->id, arg1->id, replacement_ref, original_opcode));
    } else if ((arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg1->operation.parameters.imm.integer == 0) ||
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                arg1->operation.parameters.imm.uinteger == 0) ||
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg2->operation.parameters.imm.integer == 0) ||
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                arg2->operation.parameters.imm.uinteger == 0)) {
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, 0, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_AND &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;
        kefir_opt_instruction_ref_t arg2_unwrapped_ref = arg2->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }
        if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg2_unwrapped_ref = arg2->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_and(mem, &func->code, block_id, arg1_unwrapped_ref, arg2_unwrapped_ref, replacement_ref,
                                   original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_AND &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;
        kefir_opt_instruction_ref_t arg2_unwrapped_ref = arg2->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }
        if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg2_unwrapped_ref = arg2->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_and(mem, &func->code, block_id, arg1_unwrapped_ref, arg2_unwrapped_ref, replacement_ref,
                                   original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_AND &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;
        kefir_opt_instruction_ref_t arg2_unwrapped_ref = arg2->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }
        if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg2_unwrapped_ref = arg2->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_and(mem, &func->code, block_id, arg1_unwrapped_ref, arg2_unwrapped_ref, replacement_ref,
                                   original_opcode));
    }
    return KEFIR_OK;
}

static kefir_result_t builder_int_or(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                     kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t ref1,
                                     kefir_opt_instruction_ref_t ref2, kefir_opt_instruction_ref_t *result_ref,
                                     kefir_opt_opcode_t original) {
    switch (original) {
        case KEFIR_OPT_OPCODE_INT8_OR:
            REQUIRE_OK(kefir_opt_code_builder_int8_or(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT16_OR:
            REQUIRE_OK(kefir_opt_code_builder_int16_or(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT32_OR:
            REQUIRE_OK(kefir_opt_code_builder_int32_or(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT64_OR:
            REQUIRE_OK(kefir_opt_code_builder_int64_or(mem, code, block_id, ref1, ref2, result_ref));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected original instruction opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_or(struct kefir_mem *mem, struct kefir_opt_function *func,
                                      const struct kefir_opt_instruction *instr,
                                      kefir_opt_instruction_ref_t *replacement_ref) {
    kefir_opt_opcode_t original_opcode = instr->operation.opcode;
    const kefir_opt_block_id_t block_id = instr->block_id;

    const struct kefir_opt_instruction *arg1;
    const struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    if (arg1->operation.opcode == original_opcode && (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                                                      arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *or_arg1;
        const struct kefir_opt_instruction *or_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &or_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &or_arg2));

        const kefir_opt_instruction_ref_t or_arg1_id = or_arg1->id, or_arg2_id = or_arg2->id;

        kefir_uint64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                         ? (kefir_uint64_t) arg2->operation.parameters.imm.integer
                                         : arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (or_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand |= (kefir_uint64_t) or_arg1->operation.parameters.imm.integer;
            operand_ref = or_arg2_id;
        } else if (or_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand |= or_arg1->operation.parameters.imm.uinteger;
            operand_ref = or_arg2_id;
        } else if (or_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand |= (kefir_uint64_t) or_arg2->operation.parameters.imm.integer;
            operand_ref = or_arg1_id;
        } else if (or_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand |= or_arg2->operation.parameters.imm.uinteger;
            operand_ref = or_arg1_id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                builder_int_or(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref, original_opcode));
        }
    } else if (arg2->operation.opcode == original_opcode && (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                                                             arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *or_arg1;
        const struct kefir_opt_instruction *or_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[0], &or_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[1], &or_arg2));

        const kefir_opt_instruction_ref_t or_arg1_id = or_arg1->id, or_arg2_id = or_arg2->id;

        kefir_uint64_t imm_operand = arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                         ? (kefir_uint64_t) arg1->operation.parameters.imm.integer
                                         : arg1->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (or_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand |= (kefir_uint64_t) or_arg1->operation.parameters.imm.integer;
            operand_ref = or_arg2_id;
        } else if (or_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand |= or_arg1->operation.parameters.imm.uinteger;
            operand_ref = or_arg2_id;
        } else if (or_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand |= (kefir_uint64_t) or_arg2->operation.parameters.imm.integer;
            operand_ref = or_arg1_id;
        } else if (or_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand |= or_arg2->operation.parameters.imm.uinteger;
            operand_ref = or_arg1_id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                builder_int_or(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref, original_opcode));
        }
    } else if ((arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) &&
               arg2->operation.opcode != KEFIR_OPT_OPCODE_INT_CONST &&
               arg2->operation.opcode != KEFIR_OPT_OPCODE_UINT_CONST) {
        REQUIRE_OK(builder_int_or(mem, &func->code, block_id, arg2->id, arg1->id, replacement_ref, original_opcode));
    } else if ((arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg1->operation.parameters.imm.integer == 0) ||
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                arg1->operation.parameters.imm.uinteger == 0)) {
        *replacement_ref = arg2->id;
    } else if ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg2->operation.parameters.imm.integer == 0) ||
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                arg2->operation.parameters.imm.uinteger == 0)) {
        *replacement_ref = arg1->id;
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_OR &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;
        kefir_opt_instruction_ref_t arg2_unwrapped_ref = arg2->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }
        if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg2_unwrapped_ref = arg2->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_or(mem, &func->code, block_id, arg1_unwrapped_ref, arg2_unwrapped_ref, replacement_ref,
                                  original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_OR &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;
        kefir_opt_instruction_ref_t arg2_unwrapped_ref = arg2->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }
        if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg2_unwrapped_ref = arg2->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_or(mem, &func->code, block_id, arg1_unwrapped_ref, arg2_unwrapped_ref, replacement_ref,
                                  original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_OR &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;
        kefir_opt_instruction_ref_t arg2_unwrapped_ref = arg2->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }
        if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg2_unwrapped_ref = arg2->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_or(mem, &func->code, block_id, arg1_unwrapped_ref, arg2_unwrapped_ref, replacement_ref,
                                  original_opcode));
    }
    return KEFIR_OK;
}

static kefir_result_t builder_int_xor(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                      kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t ref1,
                                      kefir_opt_instruction_ref_t ref2, kefir_opt_instruction_ref_t *result_ref,
                                      kefir_opt_opcode_t original) {
    switch (original) {
        case KEFIR_OPT_OPCODE_INT8_XOR:
            REQUIRE_OK(kefir_opt_code_builder_int8_xor(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT16_XOR:
            REQUIRE_OK(kefir_opt_code_builder_int16_xor(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT32_XOR:
            REQUIRE_OK(kefir_opt_code_builder_int32_xor(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT64_XOR:
            REQUIRE_OK(kefir_opt_code_builder_int64_xor(mem, code, block_id, ref1, ref2, result_ref));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected original instruction opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_xor(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       const struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    kefir_opt_opcode_t original_opcode = instr->operation.opcode;
    const kefir_opt_block_id_t block_id = instr->block_id;

    const struct kefir_opt_instruction *arg1;
    const struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    if (arg1->operation.opcode == original_opcode && (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                                                      arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *xor_arg1;
        const struct kefir_opt_instruction *xor_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &xor_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &xor_arg2));

        const kefir_opt_instruction_ref_t xor_arg1_id = xor_arg1->id, xor_arg2_id = xor_arg2->id;

        kefir_uint64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                         ? (kefir_uint64_t) arg2->operation.parameters.imm.integer
                                         : arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (xor_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand ^= (kefir_uint64_t) xor_arg1->operation.parameters.imm.integer;
            operand_ref = xor_arg2_id;
        } else if (xor_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand ^= xor_arg1->operation.parameters.imm.uinteger;
            operand_ref = xor_arg2_id;
        } else if (xor_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand ^= (kefir_uint64_t) xor_arg2->operation.parameters.imm.integer;
            operand_ref = xor_arg1_id;
        } else if (xor_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand ^= xor_arg2->operation.parameters.imm.uinteger;
            operand_ref = xor_arg1_id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                builder_int_xor(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref, original_opcode));
        }
    } else if (arg2->operation.opcode == original_opcode && (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                                                             arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *xor_arg1;
        const struct kefir_opt_instruction *xor_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[0], &xor_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[1], &xor_arg2));

        const kefir_opt_instruction_ref_t xor_arg1_id = xor_arg1->id, xor_arg2_id = xor_arg2->id;

        kefir_uint64_t imm_operand = arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                         ? (kefir_uint64_t) arg1->operation.parameters.imm.integer
                                         : arg1->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (xor_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand ^= (kefir_uint64_t) xor_arg1->operation.parameters.imm.integer;
            operand_ref = xor_arg2_id;
        } else if (xor_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand ^= xor_arg1->operation.parameters.imm.uinteger;
            operand_ref = xor_arg2_id;
        } else if (xor_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand ^= (kefir_uint64_t) xor_arg2->operation.parameters.imm.integer;
            operand_ref = xor_arg1_id;
        } else if (xor_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand ^= xor_arg2->operation.parameters.imm.uinteger;
            operand_ref = xor_arg1_id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_uint_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                builder_int_xor(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref, original_opcode));
        }
    } else if ((arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) &&
               arg2->operation.opcode != KEFIR_OPT_OPCODE_INT_CONST &&
               arg2->operation.opcode != KEFIR_OPT_OPCODE_UINT_CONST) {
        REQUIRE_OK(builder_int_xor(mem, &func->code, block_id, arg2->id, arg1->id, replacement_ref, original_opcode));
    } else if ((arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg1->operation.parameters.imm.integer == 0) ||
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                arg1->operation.parameters.imm.uinteger == 0)) {
        *replacement_ref = arg2->id;
    } else if ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg2->operation.parameters.imm.integer == 0) ||
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                arg2->operation.parameters.imm.uinteger == 0)) {
        *replacement_ref = arg1->id;
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_XOR &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;
        kefir_opt_instruction_ref_t arg2_unwrapped_ref = arg2->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }
        if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg2_unwrapped_ref = arg2->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_xor(mem, &func->code, block_id, arg1_unwrapped_ref, arg2_unwrapped_ref, replacement_ref,
                                   original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_XOR &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;
        kefir_opt_instruction_ref_t arg2_unwrapped_ref = arg2->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }
        if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg2_unwrapped_ref = arg2->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_xor(mem, &func->code, block_id, arg1_unwrapped_ref, arg2_unwrapped_ref, replacement_ref,
                                   original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_XOR &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;
        kefir_opt_instruction_ref_t arg2_unwrapped_ref = arg2->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }
        if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg2_unwrapped_ref = arg2->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_xor(mem, &func->code, block_id, arg1_unwrapped_ref, arg2_unwrapped_ref, replacement_ref,
                                   original_opcode));
    }
    return KEFIR_OK;
}

static kefir_result_t builder_int_add(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                      kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t ref1,
                                      kefir_opt_instruction_ref_t ref2, kefir_opt_instruction_ref_t *result_ref,
                                      kefir_opt_opcode_t original) {
    switch (original) {
        case KEFIR_OPT_OPCODE_INT8_ADD:
            REQUIRE_OK(kefir_opt_code_builder_int8_add(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT16_ADD:
            REQUIRE_OK(kefir_opt_code_builder_int16_add(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT32_ADD:
            REQUIRE_OK(kefir_opt_code_builder_int32_add(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT64_ADD:
            REQUIRE_OK(kefir_opt_code_builder_int64_add(mem, code, block_id, ref1, ref2, result_ref));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected original instruction opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t builder_int_sub(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                      kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t ref1,
                                      kefir_opt_instruction_ref_t ref2, kefir_opt_instruction_ref_t *result_ref,
                                      kefir_opt_opcode_t original) {
    switch (original) {
        case KEFIR_OPT_OPCODE_INT8_SUB:
            REQUIRE_OK(kefir_opt_code_builder_int8_sub(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT16_SUB:
            REQUIRE_OK(kefir_opt_code_builder_int16_sub(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT32_SUB:
            REQUIRE_OK(kefir_opt_code_builder_int32_sub(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT64_SUB:
            REQUIRE_OK(kefir_opt_code_builder_int64_sub(mem, code, block_id, ref1, ref2, result_ref));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected original instruction opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t int_addition_counterpart(kefir_opt_opcode_t opcode, kefir_opt_opcode_t *counterpart) {
    switch (opcode) {
        case KEFIR_OPT_OPCODE_INT8_ADD:
            *counterpart = KEFIR_OPT_OPCODE_INT8_SUB;
            break;

        case KEFIR_OPT_OPCODE_INT16_ADD:
            *counterpart = KEFIR_OPT_OPCODE_INT16_SUB;
            break;

        case KEFIR_OPT_OPCODE_INT32_ADD:
            *counterpart = KEFIR_OPT_OPCODE_INT32_SUB;
            break;

        case KEFIR_OPT_OPCODE_INT64_ADD:
            *counterpart = KEFIR_OPT_OPCODE_INT64_SUB;
            break;

        case KEFIR_OPT_OPCODE_INT8_SUB:
            *counterpart = KEFIR_OPT_OPCODE_INT8_ADD;
            break;

        case KEFIR_OPT_OPCODE_INT16_SUB:
            *counterpart = KEFIR_OPT_OPCODE_INT16_ADD;
            break;

        case KEFIR_OPT_OPCODE_INT32_SUB:
            *counterpart = KEFIR_OPT_OPCODE_INT32_ADD;
            break;

        case KEFIR_OPT_OPCODE_INT64_SUB:
            *counterpart = KEFIR_OPT_OPCODE_INT64_ADD;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_add(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       const struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    kefir_opt_opcode_t original_opcode = instr->operation.opcode, counterpart_opcode = 0;
    REQUIRE_OK(int_addition_counterpart(original_opcode, &counterpart_opcode));
    const kefir_opt_block_id_t block_id = instr->block_id;

    const struct kefir_opt_instruction *arg1;
    const struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    if (arg1->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL &&
        arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
        REQUIRE_OK(kefir_opt_code_builder_ref_local(mem, &func->code, block_id, arg1->id,
                                                    arg2->operation.parameters.imm.integer, replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
        REQUIRE_OK(kefir_opt_code_builder_ref_local(mem, &func->code, block_id, arg1->id,
                                                    arg2->operation.parameters.imm.uinteger, replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) {
        REQUIRE_OK(kefir_opt_code_builder_ref_local(mem, &func->code, block_id, arg2->id,
                                                    arg1->operation.parameters.imm.integer, replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) {
        REQUIRE_OK(kefir_opt_code_builder_ref_local(mem, &func->code, block_id, arg2->id,
                                                    arg1->operation.parameters.imm.uinteger, replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_REF_LOCAL &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
        REQUIRE_OK(kefir_opt_code_builder_ref_local(
            mem, &func->code, block_id, arg1->operation.parameters.refs[0],
            arg1->operation.parameters.offset + arg2->operation.parameters.imm.integer, replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
        REQUIRE_OK(kefir_opt_code_builder_ref_local(
            mem, &func->code, block_id, arg1->operation.parameters.refs[0],
            arg1->operation.parameters.offset + arg2->operation.parameters.imm.uinteger, replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) {
        REQUIRE_OK(kefir_opt_code_builder_ref_local(
            mem, &func->code, block_id, arg2->operation.parameters.refs[0],
            arg2->operation.parameters.offset + arg1->operation.parameters.imm.integer, replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) {
        REQUIRE_OK(kefir_opt_code_builder_ref_local(
            mem, &func->code, block_id, arg2->operation.parameters.refs[0],
            arg2->operation.parameters.offset + arg1->operation.parameters.imm.uinteger, replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
        REQUIRE_OK(kefir_opt_code_builder_get_global(
            mem, &func->code, block_id, arg1->operation.parameters.variable.global_ref,
            arg1->operation.parameters.variable.offset + arg2->operation.parameters.imm.integer, replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
        REQUIRE_OK(kefir_opt_code_builder_get_global(
            mem, &func->code, block_id, arg1->operation.parameters.variable.global_ref,
            arg1->operation.parameters.variable.offset + (kefir_int64_t) arg2->operation.parameters.imm.uinteger,
            replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL) {
        REQUIRE_OK(kefir_opt_code_builder_get_global(
            mem, &func->code, block_id, arg2->operation.parameters.variable.global_ref,
            arg2->operation.parameters.variable.offset + arg1->operation.parameters.imm.integer, replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL) {
        REQUIRE_OK(kefir_opt_code_builder_get_global(
            mem, &func->code, block_id, arg2->operation.parameters.variable.global_ref,
            arg2->operation.parameters.variable.offset + (kefir_int64_t) arg1->operation.parameters.imm.uinteger,
            replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
        REQUIRE_OK(kefir_opt_code_builder_get_thread_local(
            mem, &func->code, block_id, arg1->operation.parameters.variable.global_ref,
            arg1->operation.parameters.variable.offset + arg2->operation.parameters.imm.integer, replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
        REQUIRE_OK(kefir_opt_code_builder_get_thread_local(
            mem, &func->code, block_id, arg1->operation.parameters.variable.global_ref,
            arg1->operation.parameters.variable.offset + (kefir_int64_t) arg2->operation.parameters.imm.uinteger,
            replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL) {
        REQUIRE_OK(kefir_opt_code_builder_get_thread_local(
            mem, &func->code, block_id, arg2->operation.parameters.variable.global_ref,
            arg2->operation.parameters.variable.offset + arg1->operation.parameters.imm.integer, replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
               arg2->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL) {
        REQUIRE_OK(kefir_opt_code_builder_get_thread_local(
            mem, &func->code, block_id, arg2->operation.parameters.variable.global_ref,
            arg2->operation.parameters.variable.offset + (kefir_int64_t) arg1->operation.parameters.imm.uinteger,
            replacement_ref));
    } else if ((arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg1->operation.parameters.imm.integer == 0) ||
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                arg1->operation.parameters.imm.uinteger == 0)) {
        *replacement_ref = arg2->id;
    } else if ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg2->operation.parameters.imm.integer == 0) ||
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                arg2->operation.parameters.imm.uinteger == 0)) {
        *replacement_ref = arg1->id;
    } else if (arg1->operation.opcode == instr->operation.opcode &&
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *add_arg1;
        const struct kefir_opt_instruction *add_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &add_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &add_arg2));

        const kefir_opt_instruction_ref_t add_arg1_id = add_arg1->id, add_arg2_id = add_arg2->id;

        kefir_int64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg2->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (add_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += add_arg1->operation.parameters.imm.integer;
            operand_ref = add_arg2_id;
        } else if (add_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) add_arg1->operation.parameters.imm.uinteger;
            operand_ref = add_arg2_id;
        } else if (add_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += add_arg2->operation.parameters.imm.integer;
            operand_ref = add_arg1_id;
        } else if (add_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) add_arg2->operation.parameters.imm.uinteger;
            operand_ref = add_arg1_id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                builder_int_add(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref, original_opcode));
        }
    } else if (arg2->operation.opcode == instr->operation.opcode &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *add_arg1;
        const struct kefir_opt_instruction *add_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[0], &add_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[1], &add_arg2));

        const kefir_opt_instruction_ref_t add_arg1_id = add_arg1->id, add_arg2_id = add_arg2->id;

        kefir_int64_t imm_operand = arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg1->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg1->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (add_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += add_arg1->operation.parameters.imm.integer;
            operand_ref = add_arg2_id;
        } else if (add_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) add_arg1->operation.parameters.imm.uinteger;
            operand_ref = add_arg2_id;
        } else if (add_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += add_arg2->operation.parameters.imm.integer;
            operand_ref = add_arg1_id;
        } else if (add_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) add_arg2->operation.parameters.imm.uinteger;
            operand_ref = add_arg1_id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                builder_int_add(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref, original_opcode));
        }
    } else if (arg1->operation.opcode == counterpart_opcode &&
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *sub_arg1;
        const struct kefir_opt_instruction *sub_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &sub_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &sub_arg2));

        const kefir_opt_instruction_ref_t sub_arg1_id = sub_arg1->id, sub_arg2_id = sub_arg2->id;

        kefir_int64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg2->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (sub_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += sub_arg1->operation.parameters.imm.integer;
            operand_ref = sub_arg2_id;
        } else if (sub_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) sub_arg1->operation.parameters.imm.uinteger;
            operand_ref = sub_arg2_id;
        } else if (sub_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand = sub_arg2->operation.parameters.imm.integer - imm_operand;
            operand_ref = sub_arg1_id;
        } else if (sub_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand = (kefir_int64_t) sub_arg2->operation.parameters.imm.uinteger - imm_operand;
            operand_ref = sub_arg1_id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            if (operand_ref == sub_arg1_id) {
                REQUIRE_OK(builder_int_sub(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref,
                                           counterpart_opcode));
            } else {
                REQUIRE_OK(builder_int_sub(mem, &func->code, block_id, imm_op_ref, operand_ref, replacement_ref,
                                           counterpart_opcode));
            }
        }
    } else if (arg2->operation.opcode == counterpart_opcode &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *sub_arg1;
        const struct kefir_opt_instruction *sub_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[0], &sub_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[1], &sub_arg2));

        const kefir_opt_instruction_ref_t sub_arg1_id = sub_arg1->id, sub_arg2_id = sub_arg2->id;

        kefir_int64_t imm_operand = arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg1->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg1->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (sub_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += sub_arg1->operation.parameters.imm.integer;
            operand_ref = sub_arg2_id;
        } else if (sub_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) sub_arg1->operation.parameters.imm.uinteger;
            operand_ref = sub_arg2_id;
        } else if (sub_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand = sub_arg2->operation.parameters.imm.integer - imm_operand;
            operand_ref = sub_arg1_id;
        } else if (sub_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand = (kefir_int64_t) sub_arg2->operation.parameters.imm.uinteger - imm_operand;
            operand_ref = sub_arg1_id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            if (operand_ref == sub_arg1_id) {
                REQUIRE_OK(builder_int_sub(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref,
                                           counterpart_opcode));
            } else {
                REQUIRE_OK(builder_int_sub(mem, &func->code, block_id, imm_op_ref, operand_ref, replacement_ref,
                                           counterpart_opcode));
            }
        }
    } else if ((arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) &&
               arg2->operation.opcode != KEFIR_OPT_OPCODE_INT_CONST &&
               arg2->operation.opcode != KEFIR_OPT_OPCODE_UINT_CONST) {
        REQUIRE_OK(builder_int_add(mem, &func->code, block_id, arg2->id, arg1->id, replacement_ref, original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_ADD &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;
        kefir_opt_instruction_ref_t arg2_unwrapped_ref = arg2->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }
        if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg2_unwrapped_ref = arg2->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_add(mem, &func->code, block_id, arg1_unwrapped_ref, arg2_unwrapped_ref, replacement_ref,
                                   original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_ADD &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;
        kefir_opt_instruction_ref_t arg2_unwrapped_ref = arg2->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }
        if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg2_unwrapped_ref = arg2->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_add(mem, &func->code, block_id, arg1_unwrapped_ref, arg2_unwrapped_ref, replacement_ref,
                                   original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_ADD &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;
        kefir_opt_instruction_ref_t arg2_unwrapped_ref = arg2->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }
        if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg2_unwrapped_ref = arg2->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_add(mem, &func->code, block_id, arg1_unwrapped_ref, arg2_unwrapped_ref, replacement_ref,
                                   original_opcode));
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_sub(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       const struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    kefir_opt_opcode_t original_opcode = instr->operation.opcode, counterpart_opcode = 0;
    REQUIRE_OK(int_addition_counterpart(original_opcode, &counterpart_opcode));
    const kefir_opt_block_id_t block_id = instr->block_id;

    const struct kefir_opt_instruction *arg1;
    const struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    if ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg2->operation.parameters.imm.integer == 0) ||
        (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST && arg2->operation.parameters.imm.uinteger == 0)) {
        *replacement_ref = arg1->id;
    } else if (arg1->operation.opcode == counterpart_opcode &&
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *add_arg1;
        const struct kefir_opt_instruction *add_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &add_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &add_arg2));

        const kefir_opt_instruction_ref_t add_arg1_id = add_arg1->id, add_arg2_id = add_arg2->id;

        kefir_int64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg2->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (add_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand -= add_arg1->operation.parameters.imm.integer;
            operand_ref = add_arg2_id;
        } else if (add_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand -= (kefir_int64_t) add_arg1->operation.parameters.imm.uinteger;
            operand_ref = add_arg2_id;
        } else if (add_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand -= add_arg2->operation.parameters.imm.integer;
            operand_ref = add_arg1_id;
        } else if (add_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand -= (kefir_int64_t) add_arg2->operation.parameters.imm.uinteger;
            operand_ref = add_arg1_id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                builder_int_sub(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref, original_opcode));
        }
    } else if (arg2->operation.opcode == counterpart_opcode &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *add_arg1;
        const struct kefir_opt_instruction *add_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[0], &add_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[1], &add_arg2));

        const kefir_opt_instruction_ref_t add_arg1_id = add_arg1->id, add_arg2_id = add_arg2->id;

        kefir_int64_t imm_operand = arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg1->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg1->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (add_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand -= add_arg1->operation.parameters.imm.integer;
            operand_ref = add_arg2_id;
        } else if (add_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand -= (kefir_int64_t) add_arg1->operation.parameters.imm.uinteger;
            operand_ref = add_arg2_id;
        } else if (add_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand -= add_arg2->operation.parameters.imm.integer;
            operand_ref = add_arg1_id;
        } else if (add_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand -= (kefir_int64_t) add_arg2->operation.parameters.imm.uinteger;
            operand_ref = add_arg1_id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                builder_int_sub(mem, &func->code, block_id, imm_op_ref, operand_ref, replacement_ref, original_opcode));
        }
    } else if (arg1->operation.opcode == original_opcode && (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                                                             arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *sub_arg1;
        const struct kefir_opt_instruction *sub_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &sub_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &sub_arg2));

        const kefir_opt_instruction_ref_t sub_arg1_id = sub_arg1->id, sub_arg2_id = sub_arg2->id;

        kefir_int64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg2->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (sub_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand = sub_arg1->operation.parameters.imm.integer - imm_operand;
            operand_ref = sub_arg2_id;
        } else if (sub_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand = (kefir_int64_t) sub_arg1->operation.parameters.imm.uinteger - imm_operand;
            operand_ref = sub_arg2_id;
        } else if (sub_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += sub_arg2->operation.parameters.imm.integer;
            operand_ref = sub_arg1_id;
        } else if (sub_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) sub_arg2->operation.parameters.imm.uinteger;
            operand_ref = sub_arg1_id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            if (operand_ref == sub_arg1_id) {
                REQUIRE_OK(builder_int_sub(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref,
                                           original_opcode));
            } else {
                REQUIRE_OK(builder_int_sub(mem, &func->code, block_id, imm_op_ref, operand_ref, replacement_ref,
                                           original_opcode));
            }
        }
    } else if (arg2->operation.opcode == original_opcode && (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                                                             arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *sub_arg1;
        const struct kefir_opt_instruction *sub_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[0], &sub_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[1], &sub_arg2));

        const kefir_opt_instruction_ref_t sub_arg1_id = sub_arg1->id, sub_arg2_id = sub_arg2->id;

        kefir_int64_t imm_operand = arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg1->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg1->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (sub_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand -= sub_arg1->operation.parameters.imm.integer;
            operand_ref = sub_arg2_id;
        } else if (sub_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand -= (kefir_int64_t) sub_arg1->operation.parameters.imm.uinteger;
            operand_ref = sub_arg2_id;
        } else if (sub_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += sub_arg2->operation.parameters.imm.integer;
            operand_ref = sub_arg1_id;
        } else if (sub_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) sub_arg2->operation.parameters.imm.uinteger;
            operand_ref = sub_arg1_id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            if (operand_ref == sub_arg1_id) {
                REQUIRE_OK(builder_int_sub(mem, &func->code, block_id, imm_op_ref, operand_ref, replacement_ref,
                                           original_opcode));
            } else {
                REQUIRE_OK(kefir_opt_code_builder_int64_add(mem, &func->code, block_id, imm_op_ref, operand_ref,
                                                            replacement_ref));
            }
        }
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_SUB &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;
        kefir_opt_instruction_ref_t arg2_unwrapped_ref = arg2->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }
        if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg2_unwrapped_ref = arg2->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_sub(mem, &func->code, block_id, arg1_unwrapped_ref, arg2_unwrapped_ref, replacement_ref,
                                   original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_SUB &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;
        kefir_opt_instruction_ref_t arg2_unwrapped_ref = arg2->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }
        if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg2_unwrapped_ref = arg2->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_sub(mem, &func->code, block_id, arg1_unwrapped_ref, arg2_unwrapped_ref, replacement_ref,
                                   original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_SUB &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;
        kefir_opt_instruction_ref_t arg2_unwrapped_ref = arg2->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }
        if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg2_unwrapped_ref = arg2->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_sub(mem, &func->code, block_id, arg1_unwrapped_ref, arg2_unwrapped_ref, replacement_ref,
                                   original_opcode));
    }
    return KEFIR_OK;
}

static kefir_result_t builder_int_mul(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                      kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t ref1,
                                      kefir_opt_instruction_ref_t ref2, kefir_opt_instruction_ref_t *result_ref,
                                      kefir_opt_opcode_t original) {
    switch (original) {
        case KEFIR_OPT_OPCODE_INT8_MUL:
            REQUIRE_OK(kefir_opt_code_builder_int8_mul(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT16_MUL:
            REQUIRE_OK(kefir_opt_code_builder_int16_mul(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT32_MUL:
            REQUIRE_OK(kefir_opt_code_builder_int32_mul(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT64_MUL:
            REQUIRE_OK(kefir_opt_code_builder_int64_mul(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_UINT8_MUL:
            REQUIRE_OK(kefir_opt_code_builder_uint8_mul(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_UINT16_MUL:
            REQUIRE_OK(kefir_opt_code_builder_uint16_mul(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_UINT32_MUL:
            REQUIRE_OK(kefir_opt_code_builder_uint32_mul(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_UINT64_MUL:
            REQUIRE_OK(kefir_opt_code_builder_uint64_mul(mem, code, block_id, ref1, ref2, result_ref));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected original instruction opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_mul(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       const struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    kefir_opt_opcode_t original_opcode = instr->operation.opcode;
    const kefir_opt_block_id_t block_id = instr->block_id;

    const struct kefir_opt_instruction *arg1;
    const struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    if ((arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg1->operation.parameters.imm.integer == 1) ||
        (arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST && arg1->operation.parameters.imm.uinteger == 1)) {
        *replacement_ref = arg2->id;
    } else if ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg2->operation.parameters.imm.integer == 1) ||
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                arg2->operation.parameters.imm.uinteger == 1)) {
        *replacement_ref = arg1->id;
    } else if ((arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg1->operation.parameters.imm.integer == 0) ||
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                arg1->operation.parameters.imm.uinteger == 0) ||
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg2->operation.parameters.imm.integer == 0) ||
               (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                arg2->operation.parameters.imm.uinteger == 0)) {
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, 0, replacement_ref));
    } else if (arg1->operation.opcode == original_opcode && (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                                                             arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *mul_arg1;
        const struct kefir_opt_instruction *mul_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &mul_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &mul_arg2));

        const kefir_opt_instruction_ref_t mul_arg1_id = mul_arg1->id, mul_arg2_id = mul_arg2->id;

        kefir_int64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg2->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (mul_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand *= mul_arg1->operation.parameters.imm.integer;
            operand_ref = mul_arg2_id;
        } else if (mul_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand *= (kefir_int64_t) mul_arg1->operation.parameters.imm.uinteger;
            operand_ref = mul_arg2_id;
        } else if (mul_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand *= mul_arg2->operation.parameters.imm.integer;
            operand_ref = mul_arg1_id;
        } else if (mul_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand *= (kefir_int64_t) mul_arg2->operation.parameters.imm.uinteger;
            operand_ref = mul_arg1_id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                builder_int_mul(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref, original_opcode));
        }
    } else if (arg2->operation.opcode == original_opcode && (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                                                             arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *mul_arg1;
        const struct kefir_opt_instruction *mul_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[0], &mul_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg2->operation.parameters.refs[1], &mul_arg2));

        const kefir_opt_instruction_ref_t mul_arg1_id = mul_arg1->id, mul_arg2_id = mul_arg2->id;

        kefir_int64_t imm_operand = arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg1->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg1->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (mul_arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand *= mul_arg1->operation.parameters.imm.integer;
            operand_ref = mul_arg2_id;
        } else if (mul_arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand *= (kefir_int64_t) mul_arg1->operation.parameters.imm.uinteger;
            operand_ref = mul_arg2_id;
        } else if (mul_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand *= mul_arg2->operation.parameters.imm.integer;
            operand_ref = mul_arg1_id;
        } else if (mul_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand *= (kefir_int64_t) mul_arg2->operation.parameters.imm.uinteger;
            operand_ref = mul_arg1_id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                builder_int_mul(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref, original_opcode));
        }
    } else if ((arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) &&
               arg2->operation.opcode != KEFIR_OPT_OPCODE_INT_CONST &&
               arg2->operation.opcode != KEFIR_OPT_OPCODE_UINT_CONST) {
        REQUIRE_OK(builder_int_mul(mem, &func->code, block_id, arg2->id, arg1->id, replacement_ref, original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_MUL &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;
        kefir_opt_instruction_ref_t arg2_unwrapped_ref = arg2->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }
        if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg2_unwrapped_ref = arg2->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_mul(mem, &func->code, block_id, arg1_unwrapped_ref, arg2_unwrapped_ref, replacement_ref,
                                   original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_MUL &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;
        kefir_opt_instruction_ref_t arg2_unwrapped_ref = arg2->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }
        if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg2_unwrapped_ref = arg2->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_mul(mem, &func->code, block_id, arg1_unwrapped_ref, arg2_unwrapped_ref, replacement_ref,
                                   original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_MUL &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;
        kefir_opt_instruction_ref_t arg2_unwrapped_ref = arg2->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }
        if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg2->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg2_unwrapped_ref = arg2->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_mul(mem, &func->code, block_id, arg1_unwrapped_ref, arg2_unwrapped_ref, replacement_ref,
                                   original_opcode));
    }
    return KEFIR_OK;
}
static kefir_result_t simplify_int_div(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       const struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    UNUSED(mem);
    const struct kefir_opt_instruction *arg1;
    const struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    if ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg2->operation.parameters.imm.integer == 1) ||
        (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST && arg2->operation.parameters.imm.uinteger == 1)) {
        *replacement_ref = arg1->id;
    }

    return KEFIR_OK;
}

static kefir_result_t builder_int_shl(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                      kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t ref1,
                                      kefir_opt_instruction_ref_t ref2, kefir_opt_instruction_ref_t *result_ref,
                                      kefir_opt_opcode_t original) {
    switch (original) {
        case KEFIR_OPT_OPCODE_INT8_LSHIFT:
            REQUIRE_OK(kefir_opt_code_builder_int8_lshift(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT16_LSHIFT:
            REQUIRE_OK(kefir_opt_code_builder_int16_lshift(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT32_LSHIFT:
            REQUIRE_OK(kefir_opt_code_builder_int32_lshift(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT64_LSHIFT:
            REQUIRE_OK(kefir_opt_code_builder_int64_lshift(mem, code, block_id, ref1, ref2, result_ref));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected original instruction opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_shl(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       const struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_opcode_t original_opcode = instr->operation.opcode;
    const kefir_opt_block_id_t block_id = instr->block_id;

    const struct kefir_opt_instruction *arg1;
    const struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    if ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg2->operation.parameters.imm.integer == 0) ||
        (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST && arg2->operation.parameters.imm.integer == 0)) {
        *replacement_ref = arg1->id;
    } else if (arg1->operation.opcode == original_opcode && (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                                                             arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *shl_arg1;
        const struct kefir_opt_instruction *shl_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &shl_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &shl_arg2));

        const kefir_opt_instruction_ref_t shl_arg1_id = shl_arg1->id;

        kefir_int64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg2->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (shl_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += shl_arg2->operation.parameters.imm.integer;
            operand_ref = shl_arg1_id;
        } else if (shl_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) shl_arg2->operation.parameters.imm.uinteger;
            operand_ref = shl_arg1_id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                builder_int_shl(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref, original_opcode));
        }
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_LSHIFT &&
               ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg2->operation.parameters.imm.integer >= 8) ||
                (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                 arg2->operation.parameters.imm.uinteger >= 8))) {
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, 0, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_LSHIFT &&
               ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
                 arg2->operation.parameters.imm.integer >= 16) ||
                (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                 arg2->operation.parameters.imm.uinteger >= 16))) {
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, 0, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_LSHIFT &&
               ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
                 arg2->operation.parameters.imm.integer >= 32) ||
                (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                 arg2->operation.parameters.imm.uinteger >= 32))) {
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, 0, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_LSHIFT &&
               ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
                 arg2->operation.parameters.imm.integer >= 64) ||
                (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                 arg2->operation.parameters.imm.uinteger >= 64))) {
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, 0, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_LSHIFT &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_shl(mem, &func->code, block_id, arg1_unwrapped_ref, arg2->id, replacement_ref,
                                   original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_LSHIFT &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_shl(mem, &func->code, block_id, arg1_unwrapped_ref, arg2->id, replacement_ref,
                                   original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_LSHIFT &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_shl(mem, &func->code, block_id, arg1_unwrapped_ref, arg2->id, replacement_ref,
                                   original_opcode));
    }
    return KEFIR_OK;
}

static kefir_result_t builder_int_shr(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                      kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t ref1,
                                      kefir_opt_instruction_ref_t ref2, kefir_opt_instruction_ref_t *result_ref,
                                      kefir_opt_opcode_t original) {
    switch (original) {
        case KEFIR_OPT_OPCODE_INT8_RSHIFT:
            REQUIRE_OK(kefir_opt_code_builder_int8_rshift(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT16_RSHIFT:
            REQUIRE_OK(kefir_opt_code_builder_int16_rshift(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT32_RSHIFT:
            REQUIRE_OK(kefir_opt_code_builder_int32_rshift(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT64_RSHIFT:
            REQUIRE_OK(kefir_opt_code_builder_int64_rshift(mem, code, block_id, ref1, ref2, result_ref));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected original instruction opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_shr(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       const struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_opcode_t original_opcode = instr->operation.opcode;
    const kefir_opt_block_id_t block_id = instr->block_id;

    const struct kefir_opt_instruction *arg1;
    const struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    if ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg2->operation.parameters.imm.integer == 0) ||
        (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST && arg2->operation.parameters.imm.integer == 0)) {
        *replacement_ref = arg1->id;
    } else if (arg1->operation.opcode == original_opcode && (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                                                             arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *shr_arg1;
        const struct kefir_opt_instruction *shr_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &shr_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &shr_arg2));

        const kefir_opt_instruction_ref_t shr_arg1_id = shr_arg1->id;

        kefir_int64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg2->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg2->operation.parameters.imm.uinteger;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (shr_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += shr_arg2->operation.parameters.imm.integer;
            operand_ref = shr_arg1_id;
        } else if (shr_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) shr_arg2->operation.parameters.imm.uinteger;
            operand_ref = shr_arg1_id;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                builder_int_shr(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref, original_opcode));
        }
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_RSHIFT &&
               ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg2->operation.parameters.imm.integer >= 8) ||
                (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                 arg2->operation.parameters.imm.uinteger >= 8))) {
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, 0, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_RSHIFT &&
               ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
                 arg2->operation.parameters.imm.integer >= 16) ||
                (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                 arg2->operation.parameters.imm.uinteger >= 16))) {
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, 0, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_RSHIFT &&
               ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
                 arg2->operation.parameters.imm.integer >= 32) ||
                (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                 arg2->operation.parameters.imm.uinteger >= 32))) {
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, 0, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_RSHIFT &&
               ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
                 arg2->operation.parameters.imm.integer >= 64) ||
                (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                 arg2->operation.parameters.imm.uinteger >= 64))) {
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, 0, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_RSHIFT &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_shr(mem, &func->code, block_id, arg1_unwrapped_ref, arg2->id, replacement_ref,
                                   original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_RSHIFT &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_shr(mem, &func->code, block_id, arg1_unwrapped_ref, arg2->id, replacement_ref,
                                   original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_RSHIFT &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_shr(mem, &func->code, block_id, arg1_unwrapped_ref, arg2->id, replacement_ref,
                                   original_opcode));
    }
    return KEFIR_OK;
}

static kefir_result_t builder_int_sar(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                      kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t ref1,
                                      kefir_opt_instruction_ref_t ref2, kefir_opt_instruction_ref_t *result_ref,
                                      kefir_opt_opcode_t original) {
    switch (original) {
        case KEFIR_OPT_OPCODE_INT8_ARSHIFT:
            REQUIRE_OK(kefir_opt_code_builder_int8_arshift(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT16_ARSHIFT:
            REQUIRE_OK(kefir_opt_code_builder_int16_arshift(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT32_ARSHIFT:
            REQUIRE_OK(kefir_opt_code_builder_int32_arshift(mem, code, block_id, ref1, ref2, result_ref));
            break;

        case KEFIR_OPT_OPCODE_INT64_ARSHIFT:
            REQUIRE_OK(kefir_opt_code_builder_int64_arshift(mem, code, block_id, ref1, ref2, result_ref));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected original instruction opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_sar(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       const struct kefir_opt_instruction *instr,
                                       kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_opcode_t original_opcode = instr->operation.opcode;
    const kefir_opt_block_id_t block_id = instr->block_id;

    const struct kefir_opt_instruction *arg1;
    const struct kefir_opt_instruction *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg2));

    if ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg2->operation.parameters.imm.integer == 0) ||
        (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST && arg2->operation.parameters.imm.integer == 0)) {
        *replacement_ref = arg1->id;
    } else if (arg1->operation.opcode == original_opcode && (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
                                                             arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST)) {

        const struct kefir_opt_instruction *sar_arg1;
        const struct kefir_opt_instruction *sar_arg2;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[0], &sar_arg1));
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, arg1->operation.parameters.refs[1], &sar_arg2));

        const kefir_opt_instruction_ref_t sar_arg1_id = sar_arg1->id;

        kefir_int64_t max_arshift = 0;
        switch (original_opcode) {
            case KEFIR_OPT_OPCODE_INT8_ARSHIFT:
                max_arshift = 8;
                break;

            case KEFIR_OPT_OPCODE_INT16_ARSHIFT:
                max_arshift = 16;
                break;

            case KEFIR_OPT_OPCODE_INT32_ARSHIFT:
                max_arshift = 32;
                break;

            case KEFIR_OPT_OPCODE_INT64_ARSHIFT:
                max_arshift = 64;
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected original instruction opcode");
        }

        kefir_int64_t imm_operand = arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST
                                        ? arg2->operation.parameters.imm.integer
                                        : (kefir_int64_t) arg2->operation.parameters.imm.uinteger;
        const kefir_int64_t original_imm_operand = imm_operand;
        kefir_opt_instruction_ref_t operand_ref = KEFIR_ID_NONE;
        if (sar_arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {
            imm_operand += sar_arg2->operation.parameters.imm.integer;
            operand_ref = sar_arg1_id;
        } else if (sar_arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
            imm_operand += (kefir_int64_t) sar_arg2->operation.parameters.imm.uinteger;
            operand_ref = sar_arg1_id;
        }
        if (original_imm_operand < max_arshift && imm_operand >= max_arshift) {
            imm_operand = max_arshift - 1;
        }

        if (operand_ref != KEFIR_ID_NONE) {
            kefir_opt_instruction_ref_t imm_op_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, imm_operand, &imm_op_ref));
            REQUIRE_OK(
                builder_int_sar(mem, &func->code, block_id, operand_ref, imm_op_ref, replacement_ref, original_opcode));
        }
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_ARSHIFT &&
               ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST && arg2->operation.parameters.imm.integer >= 8) ||
                (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                 arg2->operation.parameters.imm.uinteger >= 8))) {
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, 0, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_ARSHIFT &&
               ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
                 arg2->operation.parameters.imm.integer >= 16) ||
                (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                 arg2->operation.parameters.imm.uinteger >= 16))) {
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, 0, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_ARSHIFT &&
               ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
                 arg2->operation.parameters.imm.integer >= 32) ||
                (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                 arg2->operation.parameters.imm.uinteger >= 32))) {
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, 0, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ARSHIFT &&
               ((arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&
                 arg2->operation.parameters.imm.integer >= 64) ||
                (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&
                 arg2->operation.parameters.imm.uinteger >= 64))) {
        REQUIRE_OK(kefir_opt_code_builder_int_constant(mem, &func->code, block_id, 0, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_ARSHIFT &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_sar(mem, &func->code, block_id, arg1_unwrapped_ref, arg2->id, replacement_ref,
                                   original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_ARSHIFT &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_sar(mem, &func->code, block_id, arg1_unwrapped_ref, arg2->id, replacement_ref,
                                   original_opcode));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_ARSHIFT &&
               (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS)) {
        kefir_opt_instruction_ref_t arg1_unwrapped_ref = arg1->id;

        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS ||
            arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
            arg1_unwrapped_ref = arg1->operation.parameters.refs[0];
        }

        REQUIRE_OK(builder_int_sar(mem, &func->code, block_id, arg1_unwrapped_ref, arg2->id, replacement_ref,
                                   original_opcode));
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_extend(struct kefir_mem *mem, struct kefir_opt_function *func,
                                          const struct kefir_opt_instruction *instr,
                                          kefir_opt_instruction_ref_t *replacement_ref) {
    const struct kefir_opt_instruction *arg1;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));
    if ((arg1->operation.opcode == KEFIR_OPT_OPCODE_INT8_LOAD &&
         arg1->operation.parameters.memory_access.flags.load_extension == KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND &&
         instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS) ||
        (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT16_LOAD &&
         arg1->operation.parameters.memory_access.flags.load_extension == KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND &&
         instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS) ||
        (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT32_LOAD &&
         arg1->operation.parameters.memory_access.flags.load_extension == KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND &&
         instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) ||
        (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT8_LOAD &&
         arg1->operation.parameters.memory_access.flags.load_extension == KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND &&
         instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS) ||
        (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT16_LOAD &&
         arg1->operation.parameters.memory_access.flags.load_extension == KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND &&
         instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS) ||
        (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT32_LOAD &&
         arg1->operation.parameters.memory_access.flags.load_extension == KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND &&
         instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS) ||
        (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS &&
         (instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
          instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
          instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
          instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
          instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS)) ||
        (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS &&
         (instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
          instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
          instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS)) ||
        (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS &&
         (instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
          instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
          instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS)) ||
        (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS &&
         (instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
          instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS))) {
        *replacement_ref = arg1->id;
    } else if ((arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS) &&
               instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS) {
        REQUIRE_OK(kefir_opt_code_builder_int64_zero_extend_8bits(mem, &func->code, instr->block_id,
                                                                  arg1->operation.parameters.refs[0], replacement_ref));
    } else if ((arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS) &&
               instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS) {
        REQUIRE_OK(kefir_opt_code_builder_int64_sign_extend_8bits(mem, &func->code, instr->block_id,
                                                                  arg1->operation.parameters.refs[0], replacement_ref));
    } else if ((arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS) &&
               instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS) {
        REQUIRE_OK(kefir_opt_code_builder_int64_zero_extend_16bits(
            mem, &func->code, instr->block_id, arg1->operation.parameters.refs[0], replacement_ref));
    } else if ((arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
                arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS) &&
               instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS) {
        REQUIRE_OK(kefir_opt_code_builder_int64_sign_extend_16bits(
            mem, &func->code, instr->block_id, arg1->operation.parameters.refs[0], replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS &&
               instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
        REQUIRE_OK(kefir_opt_code_builder_int64_zero_extend_32bits(
            mem, &func->code, instr->block_id, arg1->operation.parameters.refs[0], replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS &&
               instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS) {
        REQUIRE_OK(kefir_opt_code_builder_int64_zero_extend_32bits(
            mem, &func->code, instr->block_id, arg1->operation.parameters.refs[0], replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS &&
               instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS) {
        REQUIRE_OK(kefir_opt_code_builder_int64_sign_extend_32bits(
            mem, &func->code, instr->block_id, arg1->operation.parameters.refs[0], replacement_ref));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS &&
               instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS) {
        REQUIRE_OK(kefir_opt_code_builder_int64_sign_extend_32bits(
            mem, &func->code, instr->block_id, arg1->operation.parameters.refs[0], replacement_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_bits_extract(struct kefir_mem *mem, struct kefir_opt_function *func,
                                            const struct kefir_opt_instruction *instr,
                                            kefir_opt_instruction_ref_t *replacement_ref) {
    const struct kefir_opt_instruction *arg1;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &arg1));

    if (instr->operation.opcode == KEFIR_OPT_OPCODE_BITS_EXTRACT_UNSIGNED &&
        arg1->operation.opcode == KEFIR_OPT_OPCODE_BITS_EXTRACT_UNSIGNED &&
        instr->operation.parameters.bitfield.offset < arg1->operation.parameters.bitfield.length) {
        const kefir_size_t new_offset =
            arg1->operation.parameters.bitfield.offset + instr->operation.parameters.bitfield.offset;
        const kefir_size_t new_length =
            MIN(instr->operation.parameters.bitfield.length,
                arg1->operation.parameters.bitfield.length - instr->operation.parameters.bitfield.offset);
        REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, instr->block_id,
                                                                arg1->operation.parameters.refs[0], new_offset,
                                                                new_length, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_BITS_EXTRACT_SIGNED &&
               arg1->operation.opcode == KEFIR_OPT_OPCODE_BITS_EXTRACT_SIGNED &&
               instr->operation.parameters.bitfield.offset < arg1->operation.parameters.bitfield.length) {
        const kefir_size_t new_offset =
            arg1->operation.parameters.bitfield.offset + instr->operation.parameters.bitfield.offset;
        const kefir_size_t new_length =
            MIN(instr->operation.parameters.bitfield.length,
                arg1->operation.parameters.bitfield.length - instr->operation.parameters.bitfield.offset);
        REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, instr->block_id,
                                                              arg1->operation.parameters.refs[0], new_offset,
                                                              new_length, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_BITS_EXTRACT_UNSIGNED &&
               arg1->operation.opcode == KEFIR_OPT_OPCODE_BITS_EXTRACT_SIGNED &&
               instr->operation.parameters.bitfield.offset < arg1->operation.parameters.bitfield.length &&
               instr->operation.parameters.bitfield.offset + instr->operation.parameters.bitfield.length <=
                   arg1->operation.parameters.bitfield.length) {
        const kefir_size_t new_offset =
            arg1->operation.parameters.bitfield.offset + instr->operation.parameters.bitfield.offset;
        const kefir_size_t new_length = instr->operation.parameters.bitfield.length;
        REQUIRE_OK(kefir_opt_code_builder_bits_extract_unsigned(mem, &func->code, instr->block_id,
                                                                arg1->operation.parameters.refs[0], new_offset,
                                                                new_length, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_BITS_EXTRACT_SIGNED &&
               arg1->operation.opcode == KEFIR_OPT_OPCODE_BITS_EXTRACT_UNSIGNED &&
               instr->operation.parameters.bitfield.offset < arg1->operation.parameters.bitfield.length &&
               instr->operation.parameters.bitfield.offset + instr->operation.parameters.bitfield.length <=
                   arg1->operation.parameters.bitfield.length) {
        const kefir_size_t new_offset =
            arg1->operation.parameters.bitfield.offset + instr->operation.parameters.bitfield.offset;
        const kefir_size_t new_length = instr->operation.parameters.bitfield.length;
        REQUIRE_OK(kefir_opt_code_builder_bits_extract_signed(mem, &func->code, instr->block_id,
                                                              arg1->operation.parameters.refs[0], new_offset,
                                                              new_length, replacement_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_store(struct kefir_mem *mem, struct kefir_opt_function *func, const struct kefir_opt_instruction *instr,
                                         kefir_opt_instruction_ref_t *replacement_ref) {
    const struct kefir_opt_instruction *value_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(
        &func->code, instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF], &value_instr));
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_STORE &&
        (value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
         value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
         value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
         value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
         value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
         value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS)) {
        REQUIRE_OK(kefir_opt_code_builder_int8_store(mem, &func->code, instr->block_id, instr->operation.parameters.refs[0], value_instr->operation.parameters.refs[0], &instr->operation.parameters.memory_access.flags, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_STORE &&
        (value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
         value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
         value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
         value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS)) {
        REQUIRE_OK(kefir_opt_code_builder_int16_store(mem, &func->code, instr->block_id, instr->operation.parameters.refs[0], value_instr->operation.parameters.refs[0], &instr->operation.parameters.memory_access.flags, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_STORE &&
        (value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
         value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS)) {
        REQUIRE_OK(kefir_opt_code_builder_int32_store(mem, &func->code, instr->block_id, instr->operation.parameters.refs[0], value_instr->operation.parameters.refs[0], &instr->operation.parameters.memory_access.flags, replacement_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_atomic_store(struct kefir_mem *mem, struct kefir_opt_function *func, const struct kefir_opt_instruction *instr,
                                         kefir_opt_instruction_ref_t *replacement_ref) {
    const struct kefir_opt_instruction *value_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(
        &func->code, instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF], &value_instr));
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_ATOMIC_STORE8 &&
        (value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
         value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
         value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
         value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
         value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
         value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS)) {
        REQUIRE_OK(kefir_opt_code_builder_atomic_store8(mem, &func->code, instr->block_id, instr->operation.parameters.refs[0], value_instr->operation.parameters.refs[0], instr->operation.parameters.atomic_op.model, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_ATOMIC_STORE16 &&
        (value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
         value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
         value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
         value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS)) {
        REQUIRE_OK(kefir_opt_code_builder_atomic_store16(mem, &func->code, instr->block_id, instr->operation.parameters.refs[0], value_instr->operation.parameters.refs[0], instr->operation.parameters.atomic_op.model, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_ATOMIC_STORE32 &&
        (value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
         value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS)) {
        REQUIRE_OK(kefir_opt_code_builder_atomic_store32(mem, &func->code, instr->block_id, instr->operation.parameters.refs[0], value_instr->operation.parameters.refs[0], instr->operation.parameters.atomic_op.model, replacement_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_int_atomic_cmpxchg(struct kefir_mem *mem, struct kefir_opt_function *func, const struct kefir_opt_instruction *instr,
                                         kefir_opt_instruction_ref_t *replacement_ref) {
    const struct kefir_opt_instruction *compare_value_instr, *new_value_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(
        &func->code, instr->operation.parameters.refs[1], &compare_value_instr));
    REQUIRE_OK(kefir_opt_code_container_instr(
        &func->code, instr->operation.parameters.refs[2], &new_value_instr));
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG8 &&
        (compare_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
         compare_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
         compare_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
         compare_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
         compare_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
         compare_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS)) {
        REQUIRE_OK(kefir_opt_code_builder_atomic_compare_exchange8(mem, &func->code, instr->block_id, instr->operation.parameters.refs[0], compare_value_instr->operation.parameters.refs[0], new_value_instr->id, instr->operation.parameters.atomic_op.model, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG8 &&
        (new_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS ||
         new_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS ||
         new_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
         new_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
         new_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
         new_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS)) {
        REQUIRE_OK(kefir_opt_code_builder_atomic_compare_exchange8(mem, &func->code, instr->block_id, instr->operation.parameters.refs[0], compare_value_instr->id, new_value_instr->operation.parameters.refs[0], instr->operation.parameters.atomic_op.model, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG16 &&
        (compare_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
         compare_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
         compare_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
         compare_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS)) {
        REQUIRE_OK(kefir_opt_code_builder_atomic_compare_exchange16(mem, &func->code, instr->block_id, instr->operation.parameters.refs[0], compare_value_instr->operation.parameters.refs[0], new_value_instr->id, instr->operation.parameters.atomic_op.model, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG16 &&
        (new_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS ||
         new_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS ||
         new_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
         new_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS)) {
        REQUIRE_OK(kefir_opt_code_builder_atomic_compare_exchange16(mem, &func->code, instr->block_id, instr->operation.parameters.refs[0], compare_value_instr->id, new_value_instr->operation.parameters.refs[0], instr->operation.parameters.atomic_op.model, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG32 &&
        (compare_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
         compare_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS)) {
        REQUIRE_OK(kefir_opt_code_builder_atomic_compare_exchange32(mem, &func->code, instr->block_id, instr->operation.parameters.refs[0], compare_value_instr->operation.parameters.refs[0], new_value_instr->id, instr->operation.parameters.atomic_op.model, replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG32 &&
        (new_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS ||
         new_value_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS)) {
        REQUIRE_OK(kefir_opt_code_builder_atomic_compare_exchange32(mem, &func->code, instr->block_id, instr->operation.parameters.refs[0], compare_value_instr->id, new_value_instr->operation.parameters.refs[0], instr->operation.parameters.atomic_op.model, replacement_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t is_unreachable_block(const struct kefir_opt_code_container *code, kefir_opt_block_id_t block_id,
                                           kefir_bool_t *unreachable_ptr) {
    const struct kefir_opt_code_block *block;
    REQUIRE_OK(kefir_opt_code_container_block(code, block_id, &block));

    *unreachable_ptr = false;
    kefir_opt_instruction_ref_t instr_ref;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(code, block, &instr_ref));
    REQUIRE(instr_ref != KEFIR_ID_NONE, KEFIR_OK);
    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
    REQUIRE(instr->operation.opcode == KEFIR_OPT_OPCODE_UNREACHABLE, KEFIR_OK);

    *unreachable_ptr = true;
    REQUIRE_OK(kefir_opt_instruction_prev_control(code, instr_ref, &instr_ref));
    for (; instr_ref != KEFIR_ID_NONE;) {
        REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
        if (instr->operation.opcode != KEFIR_OPT_OPCODE_LOCAL_LIFETIME_MARK) {
            *unreachable_ptr = false;
            break;
        } else {
            REQUIRE_OK(kefir_opt_instruction_prev_control(code, instr_ref, &instr_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_branch(struct kefir_mem *mem, struct kefir_opt_function *func,
                                      struct kefir_opt_code_structure *structure,
                                      const struct kefir_opt_instruction *instr,
                                      kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id,
                               target_block = instr->operation.parameters.branch.target_block,
                               alternative_block = instr->operation.parameters.branch.alternative_block;

    kefir_bool_t target_block_unreachable, alternative_block_unreachable;
    REQUIRE_OK(is_unreachable_block(&func->code, target_block, &target_block_unreachable));
    REQUIRE_OK(is_unreachable_block(&func->code, alternative_block, &alternative_block_unreachable));

    const struct kefir_opt_instruction *arg1;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.branch.condition_ref, &arg1));
    if (target_block == alternative_block) {
        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr->id));
        REQUIRE_OK(kefir_opt_code_builder_finalize_jump(mem, &func->code, block_id, target_block, replacement_ref));
        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
    } else if (target_block_unreachable) {
        const kefir_opt_instruction_ref_t instr_ref = instr->id;
        REQUIRE_OK(
            kefir_opt_code_block_merge_into(mem, &func->code, &func->debug_info, block_id, target_block, false, false));
        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_ref));
        REQUIRE_OK(
            kefir_opt_code_builder_finalize_jump(mem, &func->code, block_id, alternative_block, replacement_ref));
        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
    } else if (alternative_block_unreachable) {
        const kefir_opt_instruction_ref_t instr_ref = instr->id;
        REQUIRE_OK(kefir_opt_code_block_merge_into(mem, &func->code, &func->debug_info, block_id, alternative_block,
                                                   false, false));
        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_ref));
        REQUIRE_OK(kefir_opt_code_builder_finalize_jump(mem, &func->code, block_id, target_block, replacement_ref));
        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_SCALAR_COMPARE) {
        kefir_opt_comparison_operation_t comparison = arg1->operation.parameters.comparison;
        const kefir_opt_instruction_ref_t ref1 = arg1->operation.parameters.refs[0],
                                          ref2 = arg1->operation.parameters.refs[1];
        const kefir_opt_block_id_t block_id = instr->block_id,
                                   target_block = instr->operation.parameters.branch.target_block,
                                   alternative_block = instr->operation.parameters.branch.alternative_block;
        if (KEFIR_OPT_BRANCH_CONDITION_VARIANT_IS_NEGATED(instr->operation.parameters.branch.condition_variant)) {
            REQUIRE_OK(kefir_opt_comparison_operation_inverse(comparison, &comparison));
        }
        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr->id));
        REQUIRE_OK(kefir_opt_code_builder_finalize_branch_compare(mem, &func->code, block_id, comparison, ref1, ref2,
                                                                  target_block, alternative_block, replacement_ref));
        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_NOT &&
               (instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_8BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_16BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_32BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_64BIT)) {
        const kefir_opt_instruction_ref_t condition_ref = arg1->operation.parameters.refs[0];
        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr->id));
        REQUIRE_OK(kefir_opt_code_builder_finalize_branch(mem, &func->code, block_id,
                                                          KEFIR_OPT_BRANCH_CONDITION_NEGATED_8BIT, condition_ref,
                                                          target_block, alternative_block, replacement_ref));

        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT16_BOOL_NOT &&
               (instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_8BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_16BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_32BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_64BIT)) {
        const kefir_opt_instruction_ref_t condition_ref = arg1->operation.parameters.refs[0];
        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr->id));
        REQUIRE_OK(kefir_opt_code_builder_finalize_branch(mem, &func->code, block_id,
                                                          KEFIR_OPT_BRANCH_CONDITION_NEGATED_16BIT, condition_ref,
                                                          target_block, alternative_block, replacement_ref));

        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT32_BOOL_NOT &&
               (instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_8BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_16BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_32BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_64BIT)) {
        const kefir_opt_instruction_ref_t condition_ref = arg1->operation.parameters.refs[0];
        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr->id));
        REQUIRE_OK(kefir_opt_code_builder_finalize_branch(mem, &func->code, block_id,
                                                          KEFIR_OPT_BRANCH_CONDITION_NEGATED_32BIT, condition_ref,
                                                          target_block, alternative_block, replacement_ref));

        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_BOOL_NOT &&
               (instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_8BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_16BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_32BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_64BIT)) {
        const kefir_opt_instruction_ref_t condition_ref = arg1->operation.parameters.refs[0];
        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr->id));
        REQUIRE_OK(kefir_opt_code_builder_finalize_branch(mem, &func->code, block_id,
                                                          KEFIR_OPT_BRANCH_CONDITION_NEGATED_64BIT, condition_ref,
                                                          target_block, alternative_block, replacement_ref));

        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT8_BOOL_NOT &&
               (instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_8BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_16BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_32BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_64BIT)) {
        const kefir_opt_instruction_ref_t condition_ref = arg1->operation.parameters.refs[0];
        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr->id));
        REQUIRE_OK(kefir_opt_code_builder_finalize_branch(mem, &func->code, block_id, KEFIR_OPT_BRANCH_CONDITION_8BIT,
                                                          condition_ref, target_block, alternative_block,
                                                          replacement_ref));

        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT16_BOOL_NOT &&
               (instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_8BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_16BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_32BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_64BIT)) {
        const kefir_opt_instruction_ref_t condition_ref = arg1->operation.parameters.refs[0];
        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr->id));
        REQUIRE_OK(kefir_opt_code_builder_finalize_branch(mem, &func->code, block_id, KEFIR_OPT_BRANCH_CONDITION_16BIT,
                                                          condition_ref, target_block, alternative_block,
                                                          replacement_ref));

        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT32_BOOL_NOT &&
               (instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_8BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_16BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_32BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_64BIT)) {
        const kefir_opt_instruction_ref_t condition_ref = arg1->operation.parameters.refs[0];
        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr->id));
        REQUIRE_OK(kefir_opt_code_builder_finalize_branch(mem, &func->code, block_id, KEFIR_OPT_BRANCH_CONDITION_32BIT,
                                                          condition_ref, target_block, alternative_block,
                                                          replacement_ref));

        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT64_BOOL_NOT &&
               (instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_8BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_16BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_32BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_64BIT)) {
        const kefir_opt_instruction_ref_t condition_ref = arg1->operation.parameters.refs[0];
        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr->id));
        REQUIRE_OK(kefir_opt_code_builder_finalize_branch(mem, &func->code, block_id, KEFIR_OPT_BRANCH_CONDITION_64BIT,
                                                          condition_ref, target_block, alternative_block,
                                                          replacement_ref));
        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
    } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST ||
               arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {
        kefir_bool_t condition = arg1->operation.parameters.imm.integer != 0;
        if (KEFIR_OPT_BRANCH_CONDITION_VARIANT_IS_NEGATED(instr->operation.parameters.branch.condition_variant)) {
            condition = !condition;
        }

        const kefir_opt_instruction_ref_t instr_ref = instr->id;
        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_ref));
        if (condition) {
            REQUIRE_OK(kefir_opt_code_builder_finalize_jump(
                mem, &func->code, block_id, instr->operation.parameters.branch.target_block, replacement_ref));
        } else {
            REQUIRE_OK(kefir_opt_code_builder_finalize_jump(
                mem, &func->code, block_id, instr->operation.parameters.branch.alternative_block, replacement_ref));
        }
        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
    } else if (IS_BOOL_INSTR(arg1) &&
               (instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_16BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_32BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_64BIT)) {
        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr->id));
        REQUIRE_OK(kefir_opt_code_builder_finalize_branch(mem, &func->code, block_id, KEFIR_OPT_BRANCH_CONDITION_8BIT,
                                                          arg1->id, target_block, alternative_block, replacement_ref));

        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
    } else if (IS_BOOL_INSTR(arg1) &&
               (instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_16BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_32BIT ||
                instr->operation.parameters.branch.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_64BIT)) {
        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr->id));
        REQUIRE_OK(kefir_opt_code_builder_finalize_branch(mem, &func->code, block_id,
                                                          KEFIR_OPT_BRANCH_CONDITION_NEGATED_8BIT, arg1->id,
                                                          target_block, alternative_block, replacement_ref));

        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_branch_compare(struct kefir_mem *mem, struct kefir_opt_function *func,
                                              struct kefir_opt_code_structure *structure,
                                              const struct kefir_opt_instruction *instr,
                                              kefir_opt_instruction_ref_t *replacement_ref) {
    const kefir_opt_block_id_t block_id = instr->block_id,
                               target_block = instr->operation.parameters.branch.target_block,
                               alternative_block = instr->operation.parameters.branch.alternative_block;

    if (target_block == alternative_block) {
        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr->id));
        REQUIRE_OK(kefir_opt_code_builder_finalize_jump(mem, &func->code, block_id, target_block, replacement_ref));
        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
    } else {
        kefir_bool_t target_block_unreachable, alternative_block_unreachable;
        REQUIRE_OK(is_unreachable_block(&func->code, target_block, &target_block_unreachable));
        REQUIRE_OK(is_unreachable_block(&func->code, alternative_block, &alternative_block_unreachable));
        if (target_block_unreachable) {
            const kefir_opt_instruction_ref_t instr_ref = instr->id;
            REQUIRE_OK(kefir_opt_code_block_merge_into(mem, &func->code, &func->debug_info, block_id, target_block,
                                                       false, false));
            REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_ref));
            REQUIRE_OK(
                kefir_opt_code_builder_finalize_jump(mem, &func->code, block_id, alternative_block, replacement_ref));
            REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
        } else if (alternative_block_unreachable) {
            const kefir_opt_instruction_ref_t instr_ref = instr->id;
            REQUIRE_OK(kefir_opt_code_block_merge_into(mem, &func->code, &func->debug_info, block_id, alternative_block,
                                                       false, false));
            REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_finalize_jump(mem, &func->code, block_id, target_block, replacement_ref));
            REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_select(struct kefir_mem *mem, struct kefir_opt_function *func,
                                      const struct kefir_opt_instruction *instr,
                                      kefir_opt_instruction_ref_t *replacement_ref) {
    const struct kefir_opt_instruction *condition, *arg1, *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[0], &condition));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[1], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[2], &arg2));

    if (arg1->id == arg2->id) {
        *replacement_ref = arg1->id;
        return KEFIR_OK;
    }

    if (condition->id == arg1->id && IS_BOOL_INSTR(arg1) && IS_BOOL_INSTR(arg2)) {
        if (KEFIR_OPT_BRANCH_CONDITION_VARIANT_IS_DIRECT(instr->operation.parameters.condition_variant)) {
            REQUIRE_OK(kefir_opt_code_builder_int8_bool_or(mem, &func->code, instr->block_id, arg1->id, arg2->id,
                                                           replacement_ref));
        } else {
            REQUIRE_OK(kefir_opt_code_builder_int8_bool_and(mem, &func->code, instr->block_id, arg1->id, arg2->id,
                                                            replacement_ref));
        }
    } else if (IS_BOOL_INSTR(condition) &&
               (instr->operation.parameters.condition_variant == KEFIR_OPT_BRANCH_CONDITION_16BIT ||
                instr->operation.parameters.condition_variant == KEFIR_OPT_BRANCH_CONDITION_32BIT ||
                instr->operation.parameters.condition_variant == KEFIR_OPT_BRANCH_CONDITION_64BIT)) {
        REQUIRE_OK(kefir_opt_code_builder_select(mem, &func->code, instr->block_id, KEFIR_OPT_BRANCH_CONDITION_8BIT,
                                                 condition->id, arg1->id, arg2->id, replacement_ref));

    } else if (IS_BOOL_INSTR(condition) &&
               (instr->operation.parameters.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_16BIT ||
                instr->operation.parameters.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_32BIT ||
                instr->operation.parameters.condition_variant == KEFIR_OPT_BRANCH_CONDITION_NEGATED_64BIT)) {
        REQUIRE_OK(kefir_opt_code_builder_select(mem, &func->code, instr->block_id,
                                                 KEFIR_OPT_BRANCH_CONDITION_NEGATED_8BIT, condition->id, arg1->id,
                                                 arg2->id, replacement_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_select_compare(struct kefir_mem *mem, struct kefir_opt_function *func,
                                              const struct kefir_opt_instruction *instr,
                                              kefir_opt_instruction_ref_t *replacement_ref) {
    const struct kefir_opt_instruction *arg1, *arg2;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[2], &arg1));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr->operation.parameters.refs[3], &arg2));

    if (arg1->id == arg2->id) {
        *replacement_ref = arg1->id;
        return KEFIR_OK;
    }

    if (arg1->operation.opcode == KEFIR_OPT_OPCODE_SCALAR_COMPARE &&
        arg1->operation.parameters.refs[0] == instr->operation.parameters.refs[0] &&
        arg1->operation.parameters.refs[1] == instr->operation.parameters.refs[1] && IS_BOOL_INSTR(arg2)) {
        kefir_opt_comparison_operation_t inverse_arg1_comparison;
        REQUIRE_OK(
            kefir_opt_comparison_operation_inverse(arg1->operation.parameters.comparison, &inverse_arg1_comparison));

        if (inverse_arg1_comparison == instr->operation.parameters.comparison) {
            REQUIRE_OK(kefir_opt_code_builder_int8_bool_and(mem, &func->code, instr->block_id, arg1->id, arg2->id,
                                                            replacement_ref));
        } else if (arg1->operation.parameters.comparison == instr->operation.parameters.comparison) {
            REQUIRE_OK(kefir_opt_code_builder_int8_bool_or(mem, &func->code, instr->block_id, arg1->id, arg2->id,
                                                           replacement_ref));
        }
    }

    return KEFIR_OK;
}

static kefir_result_t simplify_load(struct kefir_mem *mem, struct kefir_opt_function *func,
                                    const struct kefir_opt_instruction *instr,
                                    kefir_opt_instruction_ref_t *replacement_ref) {
    kefir_opt_instruction_ref_t use_instr_ref;
    REQUIRE_OK(kefir_opt_instruction_get_sole_use(&func->code, instr->id, &use_instr_ref));
    REQUIRE(use_instr_ref != KEFIR_ID_NONE, KEFIR_OK);

    const struct kefir_opt_instruction *use_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, use_instr_ref, &use_instr));

    if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_LOAD &&
        instr->operation.parameters.memory_access.flags.load_extension == KEFIR_OPT_MEMORY_LOAD_NOEXTEND &&
        use_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS &&
        use_instr->operation.parameters.refs[0] == instr->id) {
        REQUIRE_OK(kefir_opt_code_builder_int8_load(
            mem, &func->code, instr->block_id, instr->operation.parameters.refs[0],
            &(struct kefir_opt_memory_access_flags) {
                .load_extension = KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND,
                .volatile_access = instr->operation.parameters.memory_access.flags.volatile_access},
            replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_replace_references(mem, &func->code, *replacement_ref, use_instr_ref));
        REQUIRE_OK(
            kefir_opt_code_debug_info_replace_local_variable(mem, &func->debug_info, use_instr_ref, *replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT8_LOAD &&
               instr->operation.parameters.memory_access.flags.load_extension == KEFIR_OPT_MEMORY_LOAD_NOEXTEND &&
               use_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS &&
               use_instr->operation.parameters.refs[0] == instr->id) {
        REQUIRE_OK(kefir_opt_code_builder_int8_load(
            mem, &func->code, instr->block_id, instr->operation.parameters.refs[0],
            &(struct kefir_opt_memory_access_flags) {
                .load_extension = KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND,
                .volatile_access = instr->operation.parameters.memory_access.flags.volatile_access},
            replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_replace_references(mem, &func->code, *replacement_ref, use_instr_ref));
        REQUIRE_OK(
            kefir_opt_code_debug_info_replace_local_variable(mem, &func->debug_info, use_instr_ref, *replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_LOAD &&
               instr->operation.parameters.memory_access.flags.load_extension == KEFIR_OPT_MEMORY_LOAD_NOEXTEND &&
               use_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS &&
               use_instr->operation.parameters.refs[0] == instr->id) {
        REQUIRE_OK(kefir_opt_code_builder_int16_load(
            mem, &func->code, instr->block_id, instr->operation.parameters.refs[0],
            &(struct kefir_opt_memory_access_flags) {
                .load_extension = KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND,
                .volatile_access = instr->operation.parameters.memory_access.flags.volatile_access},
            replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_replace_references(mem, &func->code, *replacement_ref, use_instr_ref));
        REQUIRE_OK(
            kefir_opt_code_debug_info_replace_local_variable(mem, &func->debug_info, use_instr_ref, *replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT16_LOAD &&
               instr->operation.parameters.memory_access.flags.load_extension == KEFIR_OPT_MEMORY_LOAD_NOEXTEND &&
               use_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS &&
               use_instr->operation.parameters.refs[0] == instr->id) {
        REQUIRE_OK(kefir_opt_code_builder_int16_load(
            mem, &func->code, instr->block_id, instr->operation.parameters.refs[0],
            &(struct kefir_opt_memory_access_flags) {
                .load_extension = KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND,
                .volatile_access = instr->operation.parameters.memory_access.flags.volatile_access},
            replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_replace_references(mem, &func->code, *replacement_ref, use_instr_ref));
        REQUIRE_OK(
            kefir_opt_code_debug_info_replace_local_variable(mem, &func->debug_info, use_instr_ref, *replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_LOAD &&
               instr->operation.parameters.memory_access.flags.load_extension == KEFIR_OPT_MEMORY_LOAD_NOEXTEND &&
               use_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS &&
               use_instr->operation.parameters.refs[0] == instr->id) {
        REQUIRE_OK(kefir_opt_code_builder_int32_load(
            mem, &func->code, instr->block_id, instr->operation.parameters.refs[0],
            &(struct kefir_opt_memory_access_flags) {
                .load_extension = KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND,
                .volatile_access = instr->operation.parameters.memory_access.flags.volatile_access},
            replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_replace_references(mem, &func->code, *replacement_ref, use_instr_ref));
        REQUIRE_OK(
            kefir_opt_code_debug_info_replace_local_variable(mem, &func->debug_info, use_instr_ref, *replacement_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INT32_LOAD &&
               instr->operation.parameters.memory_access.flags.load_extension == KEFIR_OPT_MEMORY_LOAD_NOEXTEND &&
               use_instr->operation.opcode == KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS &&
               use_instr->operation.parameters.refs[0] == instr->id) {
        REQUIRE_OK(kefir_opt_code_builder_int32_load(
            mem, &func->code, instr->block_id, instr->operation.parameters.refs[0],
            &(struct kefir_opt_memory_access_flags) {
                .load_extension = KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND,
                .volatile_access = instr->operation.parameters.memory_access.flags.volatile_access},
            replacement_ref));
        REQUIRE_OK(kefir_opt_code_container_replace_references(mem, &func->code, *replacement_ref, use_instr_ref));
        REQUIRE_OK(
            kefir_opt_code_debug_info_replace_local_variable(mem, &func->debug_info, use_instr_ref, *replacement_ref));
    }

    return KEFIR_OK;
}

static kefir_result_t simplify_phi(struct kefir_mem *mem, struct kefir_opt_function *func,
                                   struct kefir_opt_code_structure *structure,
                                   const struct kefir_opt_instruction *phi_instr,
                                   kefir_opt_instruction_ref_t *replacement_ref) {
    kefir_opt_phi_id_t phi_ref = phi_instr->operation.parameters.phi_ref;
    const struct kefir_opt_phi_node *phi_node;
    REQUIRE_OK(kefir_opt_code_container_phi(&func->code, phi_ref, &phi_node));

    REQUIRE(phi_node->number_of_links == 2, KEFIR_OK);

    const kefir_opt_instruction_ref_t phi_instr_ref = phi_instr->id;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, phi_instr_ref, &phi_instr));
    const kefir_opt_block_id_t phi_instr_block_id = phi_instr->block_id;

    const kefir_opt_block_id_t immediate_dominator_block_id =
        structure->blocks[phi_instr->block_id].immediate_dominator;
    REQUIRE(immediate_dominator_block_id != KEFIR_ID_NONE, KEFIR_OK);

    const struct kefir_opt_code_block *immediate_dominator_block;
    REQUIRE_OK(kefir_opt_code_container_block(&func->code, immediate_dominator_block_id, &immediate_dominator_block));

    kefir_opt_instruction_ref_t immediate_dominator_tail_ref;
    REQUIRE_OK(
        kefir_opt_code_block_instr_control_tail(&func->code, immediate_dominator_block, &immediate_dominator_tail_ref));
    REQUIRE(immediate_dominator_tail_ref != KEFIR_ID_NONE, KEFIR_OK);

    const struct kefir_opt_instruction *immediate_dominator_tail;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, immediate_dominator_tail_ref, &immediate_dominator_tail));

    kefir_bool_t comparison = false;
    kefir_opt_branch_condition_variant_t condition_variant;
    kefir_opt_instruction_ref_t condition_ref, comparison_arg1, comparison_arg2;
    kefir_opt_comparison_operation_t comparison_operation;

    if (immediate_dominator_tail->operation.opcode == KEFIR_OPT_OPCODE_BRANCH) {
        condition_variant = immediate_dominator_tail->operation.parameters.branch.condition_variant;
        condition_ref = immediate_dominator_tail->operation.parameters.branch.condition_ref;
    } else if (immediate_dominator_tail->operation.opcode == KEFIR_OPT_OPCODE_BRANCH_COMPARE &&
               KEFIR_OPT_COMPARISON_IS_INTEGRAL(
                   immediate_dominator_tail->operation.parameters.branch.comparison.operation)) {
        comparison_operation = immediate_dominator_tail->operation.parameters.branch.comparison.operation;
        comparison_arg1 = immediate_dominator_tail->operation.parameters.refs[0];
        comparison_arg2 = immediate_dominator_tail->operation.parameters.refs[1];
        comparison = true;
    } else {
        return KEFIR_OK;
    }

    const kefir_opt_block_id_t immediate_dominator_target =
        immediate_dominator_tail->operation.parameters.branch.target_block;
    const kefir_opt_block_id_t immediate_dominator_alternative =
        immediate_dominator_tail->operation.parameters.branch.alternative_block;
    REQUIRE(immediate_dominator_target != immediate_dominator_alternative, KEFIR_OK);

    kefir_opt_instruction_ref_t link_ref1, link_ref2;
    kefir_bool_t move_link1 = false;
    kefir_bool_t move_link2 = false;
    if (immediate_dominator_target == phi_instr->block_id) {
        REQUIRE_OK(
            kefir_opt_code_container_phi_link_for(&func->code, phi_ref, immediate_dominator_block_id, &link_ref1));
    } else {
#define CHECK_TARGET(_dominator_branch, _link_ref, _move_link)                                                     \
    do {                                                                                                           \
        kefir_bool_t is_predecessor;                                                                               \
        REQUIRE_OK(kefir_opt_code_structure_block_exclusive_direct_predecessor(                                    \
            structure, immediate_dominator_block_id, (_dominator_branch), &is_predecessor));                       \
        REQUIRE(is_predecessor, KEFIR_OK);                                                                         \
        REQUIRE_OK(kefir_opt_code_structure_block_direct_predecessor(structure, (_dominator_branch),               \
                                                                     phi_instr->block_id, &is_predecessor));       \
        REQUIRE(is_predecessor, KEFIR_OK);                                                                         \
                                                                                                                   \
        const struct kefir_opt_code_block *branch_block;                                                           \
        REQUIRE_OK(kefir_opt_code_container_block(&func->code, (_dominator_branch), &branch_block));               \
        kefir_opt_instruction_ref_t branch_block_tail, branch_block_tail_prev;                                     \
        REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&func->code, branch_block, &branch_block_tail));        \
        REQUIRE(branch_block_tail != KEFIR_ID_NONE, KEFIR_OK);                                                     \
        REQUIRE_OK(kefir_opt_instruction_prev_control(&func->code, branch_block_tail, &branch_block_tail_prev));   \
        REQUIRE(branch_block_tail_prev == KEFIR_ID_NONE, KEFIR_OK);                                                \
                                                                                                                   \
        REQUIRE_OK(kefir_opt_code_container_phi_link_for(&func->code, phi_ref, (_dominator_branch), (_link_ref))); \
                                                                                                                   \
        const struct kefir_opt_instruction *link_instr;                                                            \
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, *(_link_ref), &link_instr));                        \
                                                                                                                   \
        if (link_instr->block_id == (_dominator_branch)) {                                                         \
            kefir_bool_t can_move_instr;                                                                           \
            REQUIRE_OK(kefir_opt_can_hoist_instruction_with_local_dependencies(                                    \
                structure, *(_link_ref), immediate_dominator_block_id, &can_move_instr));                          \
            REQUIRE(can_move_instr, KEFIR_OK);                                                                     \
            *(_move_link) = true;                                                                                  \
        }                                                                                                          \
    } while (0)
        CHECK_TARGET(immediate_dominator_target, &link_ref1, &move_link1);
    }
    if (immediate_dominator_alternative == phi_instr->block_id) {
        REQUIRE_OK(
            kefir_opt_code_container_phi_link_for(&func->code, phi_ref, immediate_dominator_block_id, &link_ref2));
    } else {
        CHECK_TARGET(immediate_dominator_alternative, &link_ref2, &move_link2);
#undef CHECK_TARGET
    }

    if (move_link1) {
        REQUIRE_OK(kefir_opt_hoist_instruction_with_local_dependencies(mem, &func->code, &func->debug_info, link_ref1,
                                                                       immediate_dominator_block_id, &link_ref1));
    }
    if (move_link2) {
        REQUIRE_OK(kefir_opt_hoist_instruction_with_local_dependencies(mem, &func->code, &func->debug_info, link_ref2,
                                                                       immediate_dominator_block_id, &link_ref2));
    }

    if (!comparison) {
        REQUIRE_OK(kefir_opt_code_builder_select(mem, &func->code, phi_instr_block_id, condition_variant, condition_ref,
                                                 link_ref1, link_ref2, replacement_ref));
    } else {
        REQUIRE_OK(kefir_opt_code_builder_select_compare(mem, &func->code, phi_instr_block_id, comparison_operation,
                                                         comparison_arg1, comparison_arg2, link_ref1, link_ref2,
                                                         replacement_ref));
    }

    return KEFIR_OK;
}

static kefir_result_t simplify_copy_memory(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                           struct kefir_opt_function *func, struct kefir_opt_code_structure *structure,
                                           const struct kefir_opt_instruction *copy_instr,
                                           kefir_opt_instruction_ref_t *replacement_ref, kefir_bool_t *drop_instr) {
    UNUSED(replacement_ref);
    const kefir_opt_instruction_ref_t copy_instr_ref = copy_instr->id;

    const struct kefir_ir_type *copy_instr_type =
        kefir_ir_module_get_named_type(module->ir_module, copy_instr->operation.parameters.type.type_id);
    REQUIRE(copy_instr_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve IR type"));

    kefir_bool_t is_control_flow;
    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(&func->code, copy_instr_ref, &is_control_flow));
    REQUIRE(is_control_flow, KEFIR_OK);

    const struct kefir_opt_instruction *copy_source_instr, *copy_target_instr;
    REQUIRE_OK(
        kefir_opt_code_container_instr(&func->code, copy_instr->operation.parameters.refs[0], &copy_target_instr));
    REQUIRE_OK(
        kefir_opt_code_container_instr(&func->code, copy_instr->operation.parameters.refs[1], &copy_source_instr));
    if (copy_source_instr->operation.opcode == KEFIR_OPT_OPCODE_INVOKE ||
        copy_source_instr->operation.opcode == KEFIR_OPT_OPCODE_INVOKE_VIRTUAL) {
        kefir_opt_instruction_ref_t copy_prev_control_instr_ref;
        REQUIRE_OK(kefir_opt_instruction_prev_control(&func->code, copy_instr_ref, &copy_prev_control_instr_ref));
        REQUIRE(copy_prev_control_instr_ref == copy_source_instr->id, KEFIR_OK);

        const struct kefir_opt_call_node *call_node;
        REQUIRE_OK(kefir_opt_code_container_call(
            &func->code, copy_source_instr->operation.parameters.function_call.call_ref, &call_node));
        REQUIRE(call_node->return_space != KEFIR_ID_NONE, KEFIR_OK);

        const struct kefir_opt_instruction *return_space_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, call_node->return_space, &return_space_instr));
        REQUIRE(return_space_instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL, KEFIR_OK);
        REQUIRE(copy_target_instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL, KEFIR_OK);

        const struct kefir_ir_function_decl *ir_func_decl =
            kefir_ir_module_get_declaration(module->ir_module, call_node->function_declaration_id);
        REQUIRE(ir_func_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR function declaration"));
        REQUIRE(ir_func_decl->result != NULL, KEFIR_OK);

        kefir_bool_t same_type;
        REQUIRE_OK(kefir_ir_type_same(ir_func_decl->result, 0, copy_instr_type,
                                      copy_instr->operation.parameters.type.type_index, &same_type));
        REQUIRE(same_type, KEFIR_OK);

        kefir_bool_t call_sequenced_before_other_uses;
        REQUIRE_OK(kefir_opt_check_all_control_flow_uses_after(
            mem, structure, copy_target_instr->id, copy_source_instr->id, &call_sequenced_before_other_uses));
        REQUIRE(call_sequenced_before_other_uses, KEFIR_OK);

        REQUIRE_OK(kefir_opt_code_container_replace_references(mem, &func->code, copy_target_instr->id,
                                                               return_space_instr->id));
        REQUIRE_OK(kefir_opt_code_debug_info_replace_local_variable(mem, &func->debug_info, return_space_instr->id,
                                                                    copy_target_instr->id));

        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, copy_instr_ref));
        REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, copy_instr_ref));
        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
    } else if (copy_source_instr->operation.opcode == KEFIR_OPT_OPCODE_GET_ARGUMENT &&
               copy_target_instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL &&
               kefir_ir_type_length(copy_instr_type) > 0 &&
               kefir_ir_type_at(copy_instr_type, 0)->typecode == KEFIR_IR_TYPE_STRUCT) {
        kefir_opt_instruction_ref_t arg_sole_use_ref;
        REQUIRE_OK(kefir_opt_instruction_get_sole_use(&func->code, copy_source_instr->id, &arg_sole_use_ref));
        REQUIRE(arg_sole_use_ref == copy_instr->id, KEFIR_OK);

        const kefir_opt_instruction_ref_t source_instr_ref = copy_source_instr->id;
        const kefir_opt_instruction_ref_t target_instr_ref = copy_target_instr->id;

        kefir_result_t res;
        struct kefir_opt_instruction_use_iterator use_iter;
        for (res = kefir_opt_code_container_instruction_use_instr_iter(&func->code, target_instr_ref, &use_iter);
             res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
            if (use_iter.use_instr_ref == copy_instr->id) {
                continue;
            }

            const struct kefir_opt_instruction *use_instr;
            REQUIRE_OK(kefir_opt_code_container_instr(&func->code, use_iter.use_instr_ref, &use_instr));
            if (use_instr->operation.opcode == KEFIR_OPT_OPCODE_LOCAL_LIFETIME_MARK) {
                continue;
            }

            kefir_bool_t before_copy;
            REQUIRE_OK(kefir_opt_code_structure_is_sequenced_before(mem, structure, copy_instr->id,
                                                                    use_iter.use_instr_ref, &before_copy));
            REQUIRE(!before_copy, KEFIR_OK);
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        const struct kefir_ir_type *alloc_instr_type =
            kefir_ir_module_get_named_type(module->ir_module, copy_target_instr->operation.parameters.type.type_id);
        REQUIRE(alloc_instr_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve IR type"));

        kefir_bool_t same_type;
        REQUIRE_OK(kefir_ir_type_same(alloc_instr_type, 0, copy_instr_type, 0, &same_type));
        REQUIRE(same_type, KEFIR_OK);

        REQUIRE_OK(kefir_ir_type_same(func->ir_func->declaration->params,
                                      kefir_ir_type_child_index(func->ir_func->declaration->params,
                                                                copy_source_instr->operation.parameters.index),
                                      copy_instr_type, 0, &same_type));
        REQUIRE(same_type, KEFIR_OK);

        REQUIRE_OK(kefir_opt_code_container_replace_references(mem, &func->code, source_instr_ref, target_instr_ref));

        for (res = kefir_opt_code_container_instruction_use_instr_iter(&func->code, source_instr_ref, &use_iter);
             res == KEFIR_OK;) {
            const struct kefir_opt_instruction *use_instr;
            REQUIRE_OK(kefir_opt_code_container_instr(&func->code, use_iter.use_instr_ref, &use_instr));

            if (use_instr->operation.opcode == KEFIR_OPT_OPCODE_LOCAL_LIFETIME_MARK) {
                REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, use_instr->id));
                REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, use_instr->id));
                res = kefir_opt_code_container_instruction_use_instr_iter(&func->code, source_instr_ref, &use_iter);
            } else {
                res = kefir_opt_code_container_instruction_use_next(&use_iter);
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        *drop_instr = true;
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_bitint_cast(struct kefir_mem *mem, struct kefir_opt_function *func,
                                           const struct kefir_opt_instruction *instr,
                                           kefir_opt_instruction_ref_t *replacement_ref) {
    UNUSED(mem);
    UNUSED(func);
    UNUSED(instr);
    if (instr->operation.parameters.bitwidth == instr->operation.parameters.src_bitwidth) {
        *replacement_ref = instr->operation.parameters.refs[0];
    }
    return KEFIR_OK;
}

static kefir_result_t simplify_unreachable(struct kefir_mem *mem, struct kefir_opt_function *func,
                                           const struct kefir_opt_instruction *instr) {
    UNUSED(mem);
    UNUSED(func);
    UNUSED(instr);

    kefir_bool_t is_control_flow;
    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(&func->code, instr->id, &is_control_flow));
    REQUIRE(is_control_flow, KEFIR_OK);

    kefir_opt_instruction_ref_t prev_control_ref;
    REQUIRE_OK(kefir_opt_instruction_prev_control(&func->code, instr->id, &prev_control_ref));
    while (prev_control_ref != KEFIR_ID_NONE) {
        const struct kefir_opt_instruction *prev_control_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->code, prev_control_ref, &prev_control_instr));

        kefir_bool_t drop_instr = false;
        switch (prev_control_instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_INT8_STORE:
            case KEFIR_OPT_OPCODE_INT16_STORE:
            case KEFIR_OPT_OPCODE_INT32_STORE:
            case KEFIR_OPT_OPCODE_INT64_STORE:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE:
            case KEFIR_OPT_OPCODE_INT8_LOAD:
            case KEFIR_OPT_OPCODE_INT16_LOAD:
            case KEFIR_OPT_OPCODE_INT32_LOAD:
            case KEFIR_OPT_OPCODE_INT64_LOAD:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_LOAD:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_LOAD:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_LOAD:
            case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_LOAD:
                if (prev_control_instr->operation.parameters.memory_access.flags.volatile_access) {
                    prev_control_ref = KEFIR_ID_NONE;
                } else {
                    drop_instr = true;
                }
                break;

            case KEFIR_OPT_OPCODE_BITINT_STORE:
            case KEFIR_OPT_OPCODE_BITINT_LOAD:
                if (prev_control_instr->operation.parameters.bitint_memflags.volatile_access) {
                    prev_control_ref = KEFIR_ID_NONE;
                } else {
                    drop_instr = true;
                }
                break;

            case KEFIR_OPT_OPCODE_VARARG_START:
            case KEFIR_OPT_OPCODE_VARARG_END:
            case KEFIR_OPT_OPCODE_VARARG_COPY:
            case KEFIR_OPT_OPCODE_VARARG_GET:
            case KEFIR_OPT_OPCODE_SCOPE_PUSH:
            case KEFIR_OPT_OPCODE_SCOPE_POP:
            case KEFIR_OPT_OPCODE_FENV_SAVE:
            case KEFIR_OPT_OPCODE_FENV_CLEAR:
            case KEFIR_OPT_OPCODE_FENV_UPDATE:
            case KEFIR_OPT_OPCODE_BITINT_EXTRACT_SIGNED:
            case KEFIR_OPT_OPCODE_BITINT_EXTRACT_UNSIGNED:
            case KEFIR_OPT_OPCODE_BITINT_INSERT:
                drop_instr = true;
                break;

            case KEFIR_OPT_OPCODE_LOCAL_LIFETIME_MARK:
                REQUIRE_OK(kefir_opt_instruction_prev_control(&func->code, prev_control_ref, &prev_control_ref));
                break;

            default:
                prev_control_ref = KEFIR_ID_NONE;
                break;
        }

        if (drop_instr) {
            kefir_opt_instruction_ref_t prev_prev_control_ref;
            REQUIRE_OK(kefir_opt_instruction_prev_control(&func->code, prev_control_ref, &prev_prev_control_ref));

            REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, prev_control_ref));
            prev_control_ref = prev_prev_control_ref;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t op_simplify_apply_impl(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                             struct kefir_opt_function *func,
                                             struct kefir_opt_code_structure *structure) {
    struct kefir_opt_code_container_iterator iter;
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(&func->code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {

        kefir_opt_instruction_ref_t instr_id;
        const struct kefir_opt_instruction *instr = NULL;
        kefir_bool_t fixpoint_reached = false;
        while (!fixpoint_reached) {
            fixpoint_reached = true;
            for (kefir_opt_code_block_instr_head(&func->code, block, &instr_id); instr_id != KEFIR_ID_NONE;) {
                REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_id, &instr));
                REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor_of(&func->debug_info, instr_id));
                kefir_opt_instruction_ref_t replacement_ref = KEFIR_ID_NONE;
                kefir_opt_instruction_ref_t next_instr_ref;
                REQUIRE_OK(kefir_opt_instruction_next_sibling(&func->code, instr_id, &next_instr_ref));
                kefir_bool_t drop_instr = false;
                switch (instr->operation.opcode) {
                    case KEFIR_OPT_OPCODE_INT8_BOOL_NOT:
                    case KEFIR_OPT_OPCODE_INT16_BOOL_NOT:
                    case KEFIR_OPT_OPCODE_INT32_BOOL_NOT:
                    case KEFIR_OPT_OPCODE_INT64_BOOL_NOT:
                        REQUIRE_OK(simplify_bool_not(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT8_BOOL_OR:
                    case KEFIR_OPT_OPCODE_INT16_BOOL_OR:
                    case KEFIR_OPT_OPCODE_INT32_BOOL_OR:
                    case KEFIR_OPT_OPCODE_INT64_BOOL_OR:
                        REQUIRE_OK(simplify_bool_or(mem, func, structure, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT8_BOOL_AND:
                    case KEFIR_OPT_OPCODE_INT16_BOOL_AND:
                    case KEFIR_OPT_OPCODE_INT32_BOOL_AND:
                    case KEFIR_OPT_OPCODE_INT64_BOOL_AND:
                        REQUIRE_OK(simplify_bool_and(mem, func, structure, instr->id, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT8_NOT:
                    case KEFIR_OPT_OPCODE_INT16_NOT:
                    case KEFIR_OPT_OPCODE_INT32_NOT:
                    case KEFIR_OPT_OPCODE_INT64_NOT:
                        REQUIRE_OK(simplify_int_not(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT8_AND:
                    case KEFIR_OPT_OPCODE_INT16_AND:
                    case KEFIR_OPT_OPCODE_INT32_AND:
                    case KEFIR_OPT_OPCODE_INT64_AND:
                        REQUIRE_OK(simplify_int_and(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT8_OR:
                    case KEFIR_OPT_OPCODE_INT16_OR:
                    case KEFIR_OPT_OPCODE_INT32_OR:
                    case KEFIR_OPT_OPCODE_INT64_OR:
                        REQUIRE_OK(simplify_int_or(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT8_XOR:
                    case KEFIR_OPT_OPCODE_INT16_XOR:
                    case KEFIR_OPT_OPCODE_INT32_XOR:
                    case KEFIR_OPT_OPCODE_INT64_XOR:
                        REQUIRE_OK(simplify_int_xor(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT8_ADD:
                    case KEFIR_OPT_OPCODE_INT16_ADD:
                    case KEFIR_OPT_OPCODE_INT32_ADD:
                    case KEFIR_OPT_OPCODE_INT64_ADD:
                        REQUIRE_OK(simplify_int_add(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT8_SUB:
                    case KEFIR_OPT_OPCODE_INT16_SUB:
                    case KEFIR_OPT_OPCODE_INT32_SUB:
                    case KEFIR_OPT_OPCODE_INT64_SUB:
                        REQUIRE_OK(simplify_int_sub(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT8_MUL:
                    case KEFIR_OPT_OPCODE_INT16_MUL:
                    case KEFIR_OPT_OPCODE_INT32_MUL:
                    case KEFIR_OPT_OPCODE_INT64_MUL:
                    case KEFIR_OPT_OPCODE_UINT8_MUL:
                    case KEFIR_OPT_OPCODE_UINT16_MUL:
                    case KEFIR_OPT_OPCODE_UINT32_MUL:
                    case KEFIR_OPT_OPCODE_UINT64_MUL:
                        REQUIRE_OK(simplify_int_mul(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT8_DIV:
                    case KEFIR_OPT_OPCODE_INT16_DIV:
                    case KEFIR_OPT_OPCODE_INT32_DIV:
                    case KEFIR_OPT_OPCODE_INT64_DIV:
                    case KEFIR_OPT_OPCODE_UINT8_DIV:
                    case KEFIR_OPT_OPCODE_UINT16_DIV:
                    case KEFIR_OPT_OPCODE_UINT32_DIV:
                    case KEFIR_OPT_OPCODE_UINT64_DIV:
                        REQUIRE_OK(simplify_int_div(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT8_LSHIFT:
                    case KEFIR_OPT_OPCODE_INT16_LSHIFT:
                    case KEFIR_OPT_OPCODE_INT32_LSHIFT:
                    case KEFIR_OPT_OPCODE_INT64_LSHIFT:
                        REQUIRE_OK(simplify_int_shl(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT8_RSHIFT:
                    case KEFIR_OPT_OPCODE_INT16_RSHIFT:
                    case KEFIR_OPT_OPCODE_INT32_RSHIFT:
                    case KEFIR_OPT_OPCODE_INT64_RSHIFT:
                        REQUIRE_OK(simplify_int_shr(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT8_ARSHIFT:
                    case KEFIR_OPT_OPCODE_INT16_ARSHIFT:
                    case KEFIR_OPT_OPCODE_INT32_ARSHIFT:
                    case KEFIR_OPT_OPCODE_INT64_ARSHIFT:
                        REQUIRE_OK(simplify_int_sar(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS:
                    case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS:
                    case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS:
                    case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS:
                    case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS:
                    case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS:
                        REQUIRE_OK(simplify_int_extend(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_BITS_EXTRACT_SIGNED:
                    case KEFIR_OPT_OPCODE_BITS_EXTRACT_UNSIGNED:
                        REQUIRE_OK(simplify_bits_extract(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT8_STORE:
                    case KEFIR_OPT_OPCODE_INT16_STORE:
                    case KEFIR_OPT_OPCODE_INT32_STORE:
                        REQUIRE_OK(simplify_int_store(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_ATOMIC_STORE8:
                    case KEFIR_OPT_OPCODE_ATOMIC_STORE16:
                    case KEFIR_OPT_OPCODE_ATOMIC_STORE32:
                        REQUIRE_OK(simplify_int_atomic_store(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG8:
                    case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG16:
                    case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG32:
                        REQUIRE_OK(simplify_int_atomic_cmpxchg(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT8_TO_BOOL:
                    case KEFIR_OPT_OPCODE_INT16_TO_BOOL:
                    case KEFIR_OPT_OPCODE_INT32_TO_BOOL:
                    case KEFIR_OPT_OPCODE_INT64_TO_BOOL:
                        REQUIRE_OK(simplify_int_to_bool(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_BRANCH:
                        REQUIRE_OK(simplify_branch(mem, func, structure, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_BRANCH_COMPARE:
                        REQUIRE_OK(simplify_branch_compare(mem, func, structure, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_SELECT:
                        REQUIRE_OK(simplify_select(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_SELECT_COMPARE:
                        REQUIRE_OK(simplify_select_compare(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_PHI:
                        REQUIRE_OK(simplify_phi(mem, func, structure, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_INT8_LOAD:
                    case KEFIR_OPT_OPCODE_INT16_LOAD:
                    case KEFIR_OPT_OPCODE_INT32_LOAD:
                        REQUIRE_OK(simplify_load(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_COPY_MEMORY:
                        REQUIRE_OK(
                            simplify_copy_memory(mem, module, func, structure, instr, &replacement_ref, &drop_instr));
                        break;

                    case KEFIR_OPT_OPCODE_BITINT_CAST_SIGNED:
                    case KEFIR_OPT_OPCODE_BITINT_CAST_UNSIGNED:
                        REQUIRE_OK(simplify_bitint_cast(mem, func, instr, &replacement_ref));
                        break;

                    case KEFIR_OPT_OPCODE_UNREACHABLE:
                        REQUIRE_OK(simplify_unreachable(mem, func, instr));
                        break;

                    default:
                        // Intentionally left blank
                        break;
                }

                if (replacement_ref != KEFIR_ID_NONE) {
                    fixpoint_reached = false;
                    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_id, &instr));
                    REQUIRE_OK(
                        kefir_opt_code_container_replace_references(mem, &func->code, replacement_ref, instr_id));
                    REQUIRE_OK(kefir_opt_code_debug_info_replace_local_variable(mem, &func->debug_info, instr_id,
                                                                                replacement_ref));
                    kefir_bool_t is_control_flow, is_replacement_control_flow;
                    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(&func->code, instr_id, &is_control_flow));
                    if (is_control_flow) {
                        REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(&func->code, replacement_ref,
                                                                              &is_replacement_control_flow));
                        if (!is_replacement_control_flow) {
                            REQUIRE_OK(kefir_opt_code_container_insert_control(&func->code, block->id, instr_id,
                                                                               replacement_ref));
                        }
                        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));
                        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
                    }
                    kefir_opt_instruction_ref_t prev_instr_id = instr_id;
                    REQUIRE_OK(kefir_opt_instruction_next_sibling(&func->code, instr_id, &instr_id));
                    REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, prev_instr_id));
                    REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor(
                        &func->debug_info, KEFIR_OPT_CODE_DEBUG_INSTRUCTION_LOCATION_NONE));
                } else if (drop_instr) {
                    kefir_bool_t is_control_flow;
                    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(&func->code, instr_id, &is_control_flow));
                    if (is_control_flow) {
                        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, instr_id));
                        REQUIRE_OK(kefir_opt_code_structure_drop_sequencing_cache(mem, structure));
                    }
                    kefir_opt_instruction_ref_t prev_instr_id = instr_id;
                    REQUIRE_OK(kefir_opt_instruction_next_sibling(&func->code, instr_id, &instr_id));
                    REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, prev_instr_id));
                    REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor(
                        &func->debug_info, KEFIR_OPT_CODE_DEBUG_INSTRUCTION_LOCATION_NONE));
                } else {
                    instr_id = next_instr_ref;
                }
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t op_simplify_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                        struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                        const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct kefir_opt_code_structure structure;
    REQUIRE_OK(kefir_opt_code_structure_init(&structure));
    kefir_result_t res = kefir_opt_code_structure_build(mem, &structure, &func->code);
    REQUIRE_CHAIN(&res, op_simplify_apply_impl(mem, module, func, &structure));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_structure_free(mem, &structure);
        return res;
    });
    REQUIRE_OK(kefir_opt_code_structure_free(mem, &structure));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassOpSimplify = {
    .name = "op-simplify", .apply = op_simplify_apply, .payload = NULL};
