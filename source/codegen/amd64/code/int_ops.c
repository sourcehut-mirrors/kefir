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

#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/function.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_amd64_match_comparison_op(const struct kefir_opt_code_container *code,
                                                       kefir_opt_instruction_ref_t instr_ref,
                                                       struct kefir_codegen_amd64_comparison_match_op *match_op) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(instr_ref != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid instruction reference"));
    REQUIRE(match_op != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                              "Expected valid pointer to AMD64 codegen comparison match operation"));

    match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_NONE;

    struct kefir_opt_instruction *instr, *condition_instr2, *condition_instr3, *arg_instr, *arg2_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(code, instr_ref, &instr));
    switch (instr->operation.opcode) {
#define OP(_instr, _opcode, _const_opcode1, _const_opcode2)                                                    \
    do {                                                                                                       \
        REQUIRE_OK(kefir_opt_code_container_instr(code, (_instr)->operation.parameters.refs[0], &arg_instr));  \
        REQUIRE_OK(kefir_opt_code_container_instr(code, (_instr)->operation.parameters.refs[1], &arg2_instr)); \
        if (arg_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {                                       \
            match_op->type = (_const_opcode2);                                                                 \
            match_op->refs[0] = arg2_instr->id;                                                                \
            match_op->refs[1] = arg_instr->id;                                                                 \
            match_op->int_value = arg_instr->operation.parameters.imm.integer;                                 \
        } else if (arg_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {                               \
            match_op->type = (_const_opcode2);                                                                 \
            match_op->refs[0] = arg2_instr->id;                                                                \
            match_op->refs[1] = arg_instr->id;                                                                 \
            match_op->int_value = arg_instr->operation.parameters.imm.uinteger;                                \
        } else if (arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST) {                               \
            match_op->type = (_const_opcode1);                                                                 \
            match_op->refs[0] = arg_instr->id;                                                                 \
            match_op->refs[1] = arg2_instr->id;                                                                \
            match_op->int_value = arg2_instr->operation.parameters.imm.integer;                                \
        } else if (arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST) {                              \
            match_op->type = (_const_opcode1);                                                                 \
            match_op->refs[0] = arg_instr->id;                                                                 \
            match_op->refs[1] = arg2_instr->id;                                                                \
            match_op->int_value = arg2_instr->operation.parameters.imm.uinteger;                               \
        } else {                                                                                               \
            match_op->type = (_opcode);                                                                        \
            match_op->refs[0] = arg_instr->id;                                                                 \
            match_op->refs[1] = arg2_instr->id;                                                                \
        }                                                                                                      \
    } while (0)
#define F32OP(_instr, _opcode, _const_opcode1, _const_opcode2)                                                 \
    do {                                                                                                       \
        REQUIRE_OK(kefir_opt_code_container_instr(code, (_instr)->operation.parameters.refs[0], &arg_instr));  \
        REQUIRE_OK(kefir_opt_code_container_instr(code, (_instr)->operation.parameters.refs[1], &arg2_instr)); \
        if (arg_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_CONST) {                                   \
            match_op->type = (_const_opcode2);                                                                 \
            match_op->refs[0] = arg2_instr->id;                                                                \
            match_op->refs[1] = arg_instr->id;                                                                 \
            match_op->float32_value = arg_instr->operation.parameters.imm.float32;                             \
        } else if (arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_CONST) {                           \
            match_op->type = (_const_opcode1);                                                                 \
            match_op->refs[0] = arg_instr->id;                                                                 \
            match_op->refs[1] = arg2_instr->id;                                                                \
            match_op->float32_value = arg2_instr->operation.parameters.imm.float32;                            \
        } else {                                                                                               \
            match_op->type = (_opcode);                                                                        \
            match_op->refs[0] = arg_instr->id;                                                                 \
            match_op->refs[1] = arg2_instr->id;                                                                \
        }                                                                                                      \
    } while (0)
#define F64OP(_instr, _opcode, _const_opcode1, _const_opcode2)                                                 \
    do {                                                                                                       \
        REQUIRE_OK(kefir_opt_code_container_instr(code, (_instr)->operation.parameters.refs[0], &arg_instr));  \
        REQUIRE_OK(kefir_opt_code_container_instr(code, (_instr)->operation.parameters.refs[1], &arg2_instr)); \
        if (arg_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT64_CONST) {                                   \
            match_op->type = (_const_opcode2);                                                                 \
            match_op->refs[0] = arg2_instr->id;                                                                \
            match_op->refs[1] = arg_instr->id;                                                                 \
            match_op->float64_value = arg_instr->operation.parameters.imm.float64;                             \
        } else if (arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT64_CONST) {                           \
            match_op->type = (_const_opcode1);                                                                 \
            match_op->refs[0] = arg_instr->id;                                                                 \
            match_op->refs[1] = arg2_instr->id;                                                                \
            match_op->float64_value = arg2_instr->operation.parameters.imm.float64;                            \
        } else {                                                                                               \
            match_op->type = (_opcode);                                                                        \
            match_op->refs[0] = arg_instr->id;                                                                 \
            match_op->refs[1] = arg2_instr->id;                                                                \
        }                                                                                                      \
    } while (0)

        case KEFIR_OPT_OPCODE_INT_EQUALS:
            OP(instr, KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL, KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL_CONST,
               KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL_CONST);
            break;

        case KEFIR_OPT_OPCODE_INT_LESSER:
            OP(instr, KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER, KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_CONST,
               KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_CONST);
            break;

        case KEFIR_OPT_OPCODE_INT_GREATER:
            OP(instr, KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER, KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_CONST,
               KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_CONST);
            break;

        case KEFIR_OPT_OPCODE_INT_BELOW:
            OP(instr, KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW, KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_CONST,
               KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_CONST);
            break;

        case KEFIR_OPT_OPCODE_INT_ABOVE:
            OP(instr, KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE, KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_CONST,
               KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_CONST);
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_EQUALS:
            F32OP(instr, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL_CONST,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL_CONST);
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_GREATER:
            F32OP(instr, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_CONST,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_CONST);
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_GREATER_OR_EQUALS:
            F32OP(instr, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST);
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_LESSER:
            F32OP(instr, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_CONST,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_CONST);
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_LESSER_OR_EQUALS:
            F32OP(instr, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST);
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_EQUALS:
            F64OP(instr, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL_CONST,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL_CONST);
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_GREATER:
            F64OP(instr, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_CONST,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_CONST);
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_GREATER_OR_EQUALS:
            F64OP(instr, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST);
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_LESSER:
            F64OP(instr, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_CONST,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_CONST);
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_LESSER_OR_EQUALS:
            F64OP(instr, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST);
            break;

        case KEFIR_OPT_OPCODE_BOOL_OR: {
            struct kefir_codegen_amd64_comparison_match_op left_op, right_op;
            REQUIRE_OK(kefir_codegen_amd64_match_comparison_op(code, instr->operation.parameters.refs[0], &left_op));
            REQUIRE_OK(kefir_codegen_amd64_match_comparison_op(code, instr->operation.parameters.refs[1], &right_op));

            const kefir_codegen_amd64_comparison_match_op_type_t op1 = MIN(left_op.type, right_op.type);
            const kefir_codegen_amd64_comparison_match_op_type_t op2 = MAX(left_op.type, right_op.type);
            if (op1 != KEFIR_CODEGEN_AMD64_COMPARISON_NONE && op2 != KEFIR_CODEGEN_AMD64_COMPARISON_NONE) {
                const kefir_opt_instruction_ref_t *op2_refs = op1 == left_op.type ? right_op.refs : left_op.refs;
                switch (op1) {
                    case KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL:
                        if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER &&
                            ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||
                             (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) {
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL;
                            match_op->refs[0] = op2_refs[0];
                            match_op->refs[1] = op2_refs[1];
                        } else if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER &&
                                   ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||
                                    (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) {
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL;
                            match_op->refs[0] = op2_refs[0];
                            match_op->refs[1] = op2_refs[1];
                        } else if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE &&
                                   ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||
                                    (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) {
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL;
                            match_op->refs[0] = op2_refs[0];
                            match_op->refs[1] = op2_refs[1];
                        } else if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW &&
                                   ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||
                                    (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) {
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL;
                            match_op->refs[0] = op2_refs[0];
                            match_op->refs[1] = op2_refs[1];
                        }
                        break;

                    case KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL_CONST:
                        if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_CONST &&
                            ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||
                             (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) {
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL_CONST;
                            match_op->refs[0] = op2_refs[0];
                            match_op->refs[1] = op2_refs[1];
                            match_op->int_value = left_op.int_value;
                        } else if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_CONST &&
                                   ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||
                                    (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) {
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL_CONST;
                            match_op->refs[0] = op2_refs[0];
                            match_op->refs[1] = op2_refs[1];
                            match_op->int_value = left_op.int_value;
                        } else if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_CONST &&
                                   ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||
                                    (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) {
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL_CONST;
                            match_op->refs[0] = op2_refs[0];
                            match_op->refs[1] = op2_refs[1];
                            match_op->int_value = left_op.int_value;
                        } else if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_CONST &&
                                   ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||
                                    (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) {
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL_CONST;
                            match_op->refs[0] = op2_refs[0];
                            match_op->refs[1] = op2_refs[1];
                            match_op->int_value = left_op.int_value;
                        }
                        break;

                    default:
                        // Intentionally left blank
                        break;
                }
            }
        } break;

        case KEFIR_OPT_OPCODE_BOOL_NOT:
            REQUIRE_OK(kefir_opt_code_container_instr(code, instr->operation.parameters.refs[0], &condition_instr2));
            switch (condition_instr2->operation.opcode) {
                case KEFIR_OPT_OPCODE_INT_EQUALS:
                    OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_INT_NOT_EQUAL,
                       KEFIR_CODEGEN_AMD64_COMPARISON_INT_NOT_EQUAL_CONST,
                       KEFIR_CODEGEN_AMD64_COMPARISON_INT_NOT_EQUAL_CONST);
                    break;

                case KEFIR_OPT_OPCODE_INT_GREATER:
                    OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL,
                       KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL_CONST,
                       KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL_CONST);
                    break;

                case KEFIR_OPT_OPCODE_INT_LESSER:
                    OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL,
                       KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL_CONST,
                       KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL_CONST);
                    break;

                case KEFIR_OPT_OPCODE_INT_ABOVE:
                    OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL,
                       KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL_CONST,
                       KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL_CONST);
                    break;

                case KEFIR_OPT_OPCODE_INT_BELOW:
                    OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL,
                       KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL_CONST,
                       KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL_CONST);
                    break;

                case KEFIR_OPT_OPCODE_FLOAT32_EQUALS:
                    F32OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL_CONST,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL_CONST);
                    break;

                case KEFIR_OPT_OPCODE_FLOAT32_LESSER_OR_EQUALS:
                    F32OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_CONST,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_CONST);
                    break;

                case KEFIR_OPT_OPCODE_FLOAT32_LESSER:
                    F32OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST);
                    break;

                case KEFIR_OPT_OPCODE_FLOAT32_GREATER_OR_EQUALS:
                    F32OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_CONST,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_CONST);
                    break;

                case KEFIR_OPT_OPCODE_FLOAT32_GREATER:
                    F32OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST);
                    break;

                case KEFIR_OPT_OPCODE_FLOAT64_EQUALS:
                    F32OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL_CONST,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL_CONST);
                    break;

                case KEFIR_OPT_OPCODE_FLOAT64_LESSER_OR_EQUALS:
                    F32OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_CONST,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_CONST);
                    break;

                case KEFIR_OPT_OPCODE_FLOAT64_LESSER:
                    F32OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST);
                    break;

                case KEFIR_OPT_OPCODE_FLOAT64_GREATER_OR_EQUALS:
                    F32OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_CONST,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_CONST);
                    break;

                case KEFIR_OPT_OPCODE_FLOAT64_GREATER:
                    F32OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST);
                    break;

                case KEFIR_OPT_OPCODE_BOOL_OR: {
                    struct kefir_codegen_amd64_comparison_match_op compound_op;
                    REQUIRE_OK(kefir_codegen_amd64_match_comparison_op(code, condition_instr2->id, &compound_op));
                    switch (compound_op.type) {
                        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL:
                            *match_op = compound_op;
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER;
                            break;

                        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL_CONST:
                            *match_op = compound_op;
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_CONST;
                            break;

                        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL:
                            *match_op = compound_op;
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER;
                            break;

                        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL_CONST:
                            *match_op = compound_op;
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_CONST;
                            break;

                        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL:
                            *match_op = compound_op;
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW;
                            break;

                        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL_CONST:
                            *match_op = compound_op;
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_CONST;
                            break;

                        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL:
                            *match_op = compound_op;
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE;
                            break;

                        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL_CONST:
                            *match_op = compound_op;
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_CONST;
                            break;

                        default:
                            // Intentionally left blank
                            break;
                    }
                } break;

                case KEFIR_OPT_OPCODE_BOOL_NOT:
                    REQUIRE_OK(kefir_opt_code_container_instr(code, condition_instr2->operation.parameters.refs[0],
                                                              &condition_instr3));
                    switch (condition_instr3->operation.opcode) {
                        case KEFIR_OPT_OPCODE_INT_EQUALS:
                            OP(condition_instr3, KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL,
                               KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL_CONST,
                               KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL_CONST);
                            break;

                        case KEFIR_OPT_OPCODE_INT_LESSER:
                            OP(condition_instr3, KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER,
                               KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_CONST,
                               KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_CONST);
                            break;

                        case KEFIR_OPT_OPCODE_INT_GREATER:
                            OP(condition_instr3, KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER,
                               KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_CONST,
                               KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_CONST);
                            break;

                        case KEFIR_OPT_OPCODE_INT_BELOW:
                            OP(condition_instr3, KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW,
                               KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_CONST,
                               KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_CONST);
                            break;

                        case KEFIR_OPT_OPCODE_INT_ABOVE:
                            OP(condition_instr3, KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE,
                               KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_CONST,
                               KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_CONST);
                            break;

                        case KEFIR_OPT_OPCODE_FLOAT32_EQUALS:
                            F32OP(condition_instr3, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL_CONST,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL_CONST);
                            break;

                        case KEFIR_OPT_OPCODE_FLOAT32_GREATER:
                            F32OP(condition_instr3, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_CONST,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_CONST);
                            break;

                        case KEFIR_OPT_OPCODE_FLOAT32_GREATER_OR_EQUALS:
                            F32OP(condition_instr3, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST);
                            break;

                        case KEFIR_OPT_OPCODE_FLOAT32_LESSER:
                            F32OP(condition_instr3, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_CONST,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_CONST);
                            break;

                        case KEFIR_OPT_OPCODE_FLOAT32_LESSER_OR_EQUALS:
                            F32OP(condition_instr3, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST);
                            break;

                        case KEFIR_OPT_OPCODE_FLOAT64_EQUALS:
                            F64OP(condition_instr3, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL_CONST,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL_CONST);
                            break;

                        case KEFIR_OPT_OPCODE_FLOAT64_GREATER:
                            F64OP(condition_instr3, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_CONST,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_CONST);
                            break;

                        case KEFIR_OPT_OPCODE_FLOAT64_GREATER_OR_EQUALS:
                            F64OP(condition_instr3, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST);
                            break;

                        case KEFIR_OPT_OPCODE_FLOAT64_LESSER:
                            F64OP(condition_instr3, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_CONST,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_CONST);
                            break;

                        case KEFIR_OPT_OPCODE_FLOAT64_LESSER_OR_EQUALS:
                            F64OP(condition_instr3, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST);
                            break;

                        case KEFIR_OPT_OPCODE_BOOL_OR:
                            REQUIRE_OK(kefir_codegen_amd64_match_comparison_op(code, condition_instr3->id, match_op));
                            break;

                        default:
                            // Intentionally left blank
                            break;
                    }
                    break;

                default:
                    // Intentionally left blank
                    break;
            }
            break;

#undef OP
#undef F32OP
#undef F64OP

        default:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

#define DEFINE_BINARY_OP(_operation)                                                                                   \
    do {                                                                                                               \
        kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;                                       \
        REQUIRE_OK(                                                                                                    \
            kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));    \
        REQUIRE_OK(                                                                                                    \
            kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));    \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,                                     \
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));    \
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,                                     \
                                                             kefir_asmcmp_context_instr_tail(&function->code.context), \
                                                             result_vreg, arg1_vreg, NULL));                           \
        REQUIRE_OK(kefir_asmcmp_amd64_##_operation(                                                                    \
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                            \
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_vreg), NULL));                      \
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));             \
    } while (0)

#define BINARY_CONST_OP(_operation)                                                                                    \
    do {                                                                                                               \
        kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, tmp_vreg;                                        \
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.ref_imm.refs[0],   \
                                                        &arg1_vreg));                                                  \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,                                     \
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));    \
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,                                     \
                                                             kefir_asmcmp_context_instr_tail(&function->code.context), \
                                                             result_vreg, arg1_vreg, NULL));                           \
                                                                                                                       \
        if (instruction->operation.parameters.ref_imm.integer >= KEFIR_INT32_MIN &&                                    \
            instruction->operation.parameters.ref_imm.integer <= INT32_MAX) {                                          \
            REQUIRE_OK(kefir_asmcmp_amd64_##_operation(                                                                \
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                        \
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),                                                                \
                &KEFIR_ASMCMP_MAKE_INT(instruction->operation.parameters.ref_imm.integer), NULL));                     \
        } else {                                                                                                       \
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,                                 \
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));   \
            REQUIRE_OK(kefir_asmcmp_amd64_movabs(                                                                      \
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                        \
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),                                                                   \
                &KEFIR_ASMCMP_MAKE_INT(instruction->operation.parameters.ref_imm.integer), NULL));                     \
            REQUIRE_OK(kefir_asmcmp_amd64_##_operation(                                                                \
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                        \
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));                   \
        }                                                                                                              \
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));             \
    } while (0)

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_add)(struct kefir_mem *mem,
                                                             struct kefir_codegen_amd64_function *function,
                                                             const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_BINARY_OP(add);
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_add_const)(struct kefir_mem *mem,
                                                                   struct kefir_codegen_amd64_function *function,
                                                                   const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    BINARY_CONST_OP(add);
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_sub)(struct kefir_mem *mem,
                                                             struct kefir_codegen_amd64_function *function,
                                                             const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_BINARY_OP(sub);
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_sub_const)(struct kefir_mem *mem,
                                                                   struct kefir_codegen_amd64_function *function,
                                                                   const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    BINARY_CONST_OP(sub);
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_mul)(struct kefir_mem *mem,
                                                             struct kefir_codegen_amd64_function *function,
                                                             const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_BINARY_OP(imul);
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_mul_const)(struct kefir_mem *mem,
                                                                   struct kefir_codegen_amd64_function *function,
                                                                   const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, tmp_vreg;
    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.ref_imm.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    if (instruction->operation.parameters.ref_imm.integer >= KEFIR_INT32_MIN &&
        instruction->operation.parameters.ref_imm.integer <= INT32_MAX) {
        REQUIRE_OK(
            kefir_asmcmp_amd64_imul3(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg1_vreg),
                                     &KEFIR_ASMCMP_MAKE_INT(instruction->operation.parameters.ref_imm.integer), NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             result_vreg, arg1_vreg, NULL));
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
        REQUIRE_OK(
            kefir_asmcmp_amd64_movabs(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
                                      &KEFIR_ASMCMP_MAKE_INT(instruction->operation.parameters.ref_imm.integer), NULL));
        REQUIRE_OK(
            kefir_asmcmp_amd64_imul(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
    }
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_and)(struct kefir_mem *mem,
                                                             struct kefir_codegen_amd64_function *function,
                                                             const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_BINARY_OP(and);
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_and_const)(struct kefir_mem *mem,
                                                                   struct kefir_codegen_amd64_function *function,
                                                                   const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    BINARY_CONST_OP(and);
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_or)(struct kefir_mem *mem,
                                                            struct kefir_codegen_amd64_function *function,
                                                            const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_BINARY_OP(or);
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_or_const)(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64_function *function,
                                                                  const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    BINARY_CONST_OP(or);
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_xor)(struct kefir_mem *mem,
                                                             struct kefir_codegen_amd64_function *function,
                                                             const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_BINARY_OP(xor);
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_xor_const)(struct kefir_mem *mem,
                                                                   struct kefir_codegen_amd64_function *function,
                                                                   const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    BINARY_CONST_OP(xor);
    return KEFIR_OK;
}

#define EXTEND_OP(_op, _width)                                                                                      \
    do {                                                                                                            \
        kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;                                                \
        REQUIRE_OK(                                                                                                 \
            kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));  \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,                                  \
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg)); \
        REQUIRE_OK(kefir_asmcmp_amd64_##_op(                                                                        \
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                         \
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG##_width(arg_vreg), NULL));              \
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));          \
    } while (false)

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_zero_extend8)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    EXTEND_OP(movzx, 8);
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_zero_extend16)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    EXTEND_OP(movzx, 16);
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_zero_extend32)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(arg_vreg),
                                      NULL));
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_sign_extend8)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    EXTEND_OP(movsx, 8);
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_sign_extend16)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    EXTEND_OP(movsx, 16);
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_sign_extend32)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    EXTEND_OP(movsx, 32);
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_trunc1)(struct kefir_mem *mem,
                                                                struct kefir_codegen_amd64_function *function,
                                                                const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_test(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(arg_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_setne(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movzx(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(result_vreg),
                                        NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

#define DEFINE_DIV_MOD(_impl, _res)                                                                                    \
    do {                                                                                                               \
        kefir_asmcmp_virtual_register_index_t result_vreg, result_placement_vreg, arg1_vreg,                           \
            arg1_placement_upper_vreg, arg1_placement_lower_vreg, arg2_vreg;                                           \
        REQUIRE_OK(                                                                                                    \
            kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));    \
        REQUIRE_OK(                                                                                                    \
            kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));    \
                                                                                                                       \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(                                                                  \
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg1_placement_upper_vreg)); \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(                                                                  \
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg1_placement_lower_vreg)); \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,                                     \
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));    \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(                                                                  \
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_placement_vreg));     \
                                                                                                                       \
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, arg1_placement_upper_vreg, \
                                                                      KEFIR_AMD64_XASMGEN_REGISTER_RDX));              \
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, arg1_placement_lower_vreg, \
                                                                      KEFIR_AMD64_XASMGEN_REGISTER_RAX));              \
        REQUIRE_OK(                                                                                                    \
            kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, result_placement_vreg, (_res)));  \
                                                                                                                       \
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,                                     \
                                                             kefir_asmcmp_context_instr_tail(&function->code.context), \
                                                             arg1_placement_lower_vreg, arg1_vreg, NULL));             \
        REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(mem, &function->code,                                     \
                                                             kefir_asmcmp_context_instr_tail(&function->code.context), \
                                                             arg1_placement_upper_vreg, NULL));                        \
                                                                                                                       \
        _impl                                                                                                          \
                                                                                                                       \
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(                                                      \
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,           \
                result_placement_vreg, NULL));                                                                         \
                                                                                                                       \
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));             \
    } while (false)

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_div)(struct kefir_mem *mem,
                                                             struct kefir_codegen_amd64_function *function,
                                                             const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_DIV_MOD(
        {
            REQUIRE_OK(kefir_asmcmp_amd64_cqo(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_idiv(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG64(arg2_vreg), NULL));
        },
        KEFIR_AMD64_XASMGEN_REGISTER_RAX);

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_mod)(struct kefir_mem *mem,
                                                             struct kefir_codegen_amd64_function *function,
                                                             const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_DIV_MOD(
        {
            REQUIRE_OK(kefir_asmcmp_amd64_cqo(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_idiv(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG64(arg2_vreg), NULL));
        },
        KEFIR_AMD64_XASMGEN_REGISTER_RDX);

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(uint_div)(struct kefir_mem *mem,
                                                              struct kefir_codegen_amd64_function *function,
                                                              const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_DIV_MOD(
        {
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg1_placement_upper_vreg), &KEFIR_ASMCMP_MAKE_INT(0), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_div(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(arg2_vreg), NULL));
        },
        KEFIR_AMD64_XASMGEN_REGISTER_RAX);

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(uint_mod)(struct kefir_mem *mem,
                                                              struct kefir_codegen_amd64_function *function,
                                                              const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_DIV_MOD(
        {
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg1_placement_upper_vreg), &KEFIR_ASMCMP_MAKE_INT(0), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_div(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(arg2_vreg), NULL));
        },
        KEFIR_AMD64_XASMGEN_REGISTER_RDX);

    return KEFIR_OK;
}

#define DEFINE_SHIFT(_op)                                                                                              \
    do {                                                                                                               \
        kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg, arg2_placement_vreg;                  \
        REQUIRE_OK(                                                                                                    \
            kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));    \
        REQUIRE_OK(                                                                                                    \
            kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));    \
                                                                                                                       \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,                                     \
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));    \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(                                                                  \
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg2_placement_vreg));       \
                                                                                                                       \
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, arg2_placement_vreg,       \
                                                                      KEFIR_AMD64_XASMGEN_REGISTER_RCX));              \
                                                                                                                       \
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,                                     \
                                                             kefir_asmcmp_context_instr_tail(&function->code.context), \
                                                             arg2_placement_vreg, arg2_vreg, NULL));                   \
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,                                     \
                                                             kefir_asmcmp_context_instr_tail(&function->code.context), \
                                                             result_vreg, arg1_vreg, NULL));                           \
                                                                                                                       \
        REQUIRE_OK(kefir_asmcmp_amd64_##_op(                                                                           \
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                            \
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(arg2_placement_vreg), NULL));             \
                                                                                                                       \
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));             \
    } while (false)

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_shl)(struct kefir_mem *mem,
                                                             struct kefir_codegen_amd64_function *function,
                                                             const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_SHIFT(shl);

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_shr)(struct kefir_mem *mem,
                                                             struct kefir_codegen_amd64_function *function,
                                                             const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_SHIFT(shr);

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_sar)(struct kefir_mem *mem,
                                                             struct kefir_codegen_amd64_function *function,
                                                             const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_SHIFT(sar);

    return KEFIR_OK;
}

#define DEFINE_SHIFT_CONST(_op)                                                                                        \
    do {                                                                                                               \
        kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;                                                  \
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.ref_imm.refs[0],   \
                                                        &arg1_vreg));                                                  \
                                                                                                                       \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,                                     \
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));    \
                                                                                                                       \
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,                                     \
                                                             kefir_asmcmp_context_instr_tail(&function->code.context), \
                                                             result_vreg, arg1_vreg, NULL));                           \
                                                                                                                       \
        REQUIRE_OK(kefir_asmcmp_amd64_##_op(                                                                           \
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                            \
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),                                                                    \
            &KEFIR_ASMCMP_MAKE_UINT(instruction->operation.parameters.ref_imm.integer & 0xffu), NULL));                \
                                                                                                                       \
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));             \
    } while (0)

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_shl_const)(struct kefir_mem *mem,
                                                                   struct kefir_codegen_amd64_function *function,
                                                                   const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_SHIFT_CONST(shl);

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_shr_const)(struct kefir_mem *mem,
                                                                   struct kefir_codegen_amd64_function *function,
                                                                   const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_SHIFT_CONST(shr);

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_sar_const)(struct kefir_mem *mem,
                                                                   struct kefir_codegen_amd64_function *function,
                                                                   const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_SHIFT_CONST(sar);

    return KEFIR_OK;
}

#define DEFINE_UNARY(_op)                                                                                              \
    do {                                                                                                               \
        kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;                                                  \
        REQUIRE_OK(                                                                                                    \
            kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));    \
                                                                                                                       \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,                                     \
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));    \
                                                                                                                       \
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,                                     \
                                                             kefir_asmcmp_context_instr_tail(&function->code.context), \
                                                             result_vreg, arg1_vreg, NULL));                           \
                                                                                                                       \
        REQUIRE_OK(kefir_asmcmp_amd64_##_op(mem, &function->code,                                                      \
                                            kefir_asmcmp_context_instr_tail(&function->code.context),                  \
                                            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), NULL));                            \
                                                                                                                       \
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));             \
    } while (false)

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_not)(struct kefir_mem *mem,
                                                             struct kefir_codegen_amd64_function *function,
                                                             const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_UNARY(not );

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_neg)(struct kefir_mem *mem,
                                                             struct kefir_codegen_amd64_function *function,
                                                             const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    DEFINE_UNARY(neg);

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(bool_not)(struct kefir_mem *mem,
                                                              struct kefir_codegen_amd64_function *function,
                                                              const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_INT(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_test(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_sete(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_FUSION_IMPL(bool_or)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction, kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
    void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));
    REQUIRE(callback != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen instruction fusion callback"));

    struct kefir_codegen_amd64_comparison_match_op fused_comparison_op;
    REQUIRE_OK(
        kefir_codegen_amd64_match_comparison_op(&function->function->code, instruction->id, &fused_comparison_op));

    switch (fused_comparison_op.type) {
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL:
            callback(fused_comparison_op.refs[0], payload);
            callback(fused_comparison_op.refs[1], payload);
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL_CONST:
            callback(fused_comparison_op.refs[0], payload);
            break;

        default:
            callback(instruction->operation.parameters.refs[0], payload);
            callback(instruction->operation.parameters.refs[1], payload);
            break;
    }

    return KEFIR_OK;
}
static kefir_result_t translate_int_comparison(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                               const struct kefir_opt_instruction *,
                                               struct kefir_codegen_amd64_comparison_match_op *);

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(bool_or)(struct kefir_mem *mem,
                                                             struct kefir_codegen_amd64_function *function,
                                                             const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    struct kefir_codegen_amd64_comparison_match_op match_op;
    REQUIRE_OK(kefir_codegen_amd64_match_comparison_op(&function->function->code, instruction->id, &match_op));
    switch (match_op.type) {
        case KEFIR_CODEGEN_AMD64_COMPARISON_NONE: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));

            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg, arg1_vreg,
                NULL));

            REQUIRE_OK(
                kefir_asmcmp_amd64_or(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_setne(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_movzx(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        default:
            REQUIRE_OK(translate_int_comparison(mem, function, instruction, &match_op));
            break;
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(bool_and)(struct kefir_mem *mem,
                                                              struct kefir_codegen_amd64_function *function,
                                                              const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg, tmp_vreg, arg1_vreg, arg2_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_test(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_setne(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_test(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_setne(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movzx(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg),
                                        NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_FUSION_IMPL(int_comparison)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction, kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
    void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));
    REQUIRE(callback != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen instruction fusion callback"));

    struct kefir_codegen_amd64_comparison_match_op fused_comparison_op;
    REQUIRE_OK(
        kefir_codegen_amd64_match_comparison_op(&function->function->code, instruction->id, &fused_comparison_op));
    switch (fused_comparison_op.type) {
        case KEFIR_CODEGEN_AMD64_COMPARISON_NONE:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 codegen comparison operation");

        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL:
            REQUIRE_OK(callback(fused_comparison_op.refs[0], payload));
            REQUIRE_OK(callback(fused_comparison_op.refs[1], payload));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL_CONST:
            REQUIRE_OK(callback(fused_comparison_op.refs[0], payload));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t translate_int_comparison(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                               const struct kefir_opt_instruction *instruction,
                                               struct kefir_codegen_amd64_comparison_match_op *match_op) {
    switch (match_op->type) {
#define DEFINE_COMPARISON(_type, _op)                                                                                  \
    case (_type):                                                                                                      \
        do {                                                                                                           \
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;                                   \
            REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, match_op->refs[0], &arg1_vreg));                 \
            REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, match_op->refs[1], &arg2_vreg));                 \
                                                                                                                       \
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(                                                              \
                mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));           \
                                                                                                                       \
            REQUIRE_OK(                                                                                                \
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), \
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_INT(0), NULL));      \
            REQUIRE_OK(kefir_asmcmp_amd64_cmp(                                                                         \
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                        \
                &KEFIR_ASMCMP_MAKE_VREG64(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_vreg), NULL));                    \
            REQUIRE_OK(kefir_asmcmp_amd64_##_op(mem, &function->code,                                                  \
                                                kefir_asmcmp_context_instr_tail(&function->code.context),              \
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));                         \
                                                                                                                       \
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));         \
        } while (false);                                                                                               \
        break

        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL, sete);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_NOT_EQUAL, setne);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER, setg);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL, setge);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER, setl);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL, setle);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE, seta);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL, setae);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW, setb);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL, setbe);

#undef DEFINE_COMPARISON

#define DEFINE_COMPARISON(_type, _op)                                                                                  \
    case (_type):                                                                                                      \
        do {                                                                                                           \
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, tmp_vreg;                                    \
            REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, match_op->refs[0], &arg1_vreg));                 \
                                                                                                                       \
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(                                                              \
                mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));           \
                                                                                                                       \
            REQUIRE_OK(                                                                                                \
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), \
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_INT(0), NULL));      \
            if (match_op->int_value >= KEFIR_INT32_MIN && match_op->int_value <= KEFIR_INT32_MAX) {                    \
                REQUIRE_OK(kefir_asmcmp_amd64_cmp(                                                                     \
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                    \
                    &KEFIR_ASMCMP_MAKE_VREG64(arg1_vreg), &KEFIR_ASMCMP_MAKE_INT(match_op->int_value), NULL));         \
            } else {                                                                                                   \
                REQUIRE_OK(kefir_asmcmp_virtual_register_new(                                                          \
                    mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));          \
                REQUIRE_OK(kefir_asmcmp_amd64_movabs(                                                                  \
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                    \
                    &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_INT(match_op->int_value), NULL));          \
                REQUIRE_OK(kefir_asmcmp_amd64_cmp(                                                                     \
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                    \
                    &KEFIR_ASMCMP_MAKE_VREG64(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));                 \
            }                                                                                                          \
            REQUIRE_OK(kefir_asmcmp_amd64_##_op(mem, &function->code,                                                  \
                                                kefir_asmcmp_context_instr_tail(&function->code.context),              \
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));                         \
                                                                                                                       \
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));         \
        } while (0);                                                                                                   \
        break

        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL_CONST, sete);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_NOT_EQUAL_CONST, setne);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_CONST, setg);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL_CONST, setge);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_CONST, setl);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL_CONST, setle);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_CONST, seta);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL_CONST, setae);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_CONST, setb);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL_CONST, setbe);

#undef DEFINE_COMPARISON

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected comparison operator type");
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_comparison)(struct kefir_mem *mem,
                                                                    struct kefir_codegen_amd64_function *function,
                                                                    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    struct kefir_codegen_amd64_comparison_match_op match_op;
    REQUIRE_OK(kefir_codegen_amd64_match_comparison_op(&function->function->code, instruction->id, &match_op));
    REQUIRE_OK(translate_int_comparison(mem, function, instruction, &match_op));

    return KEFIR_OK;
}
