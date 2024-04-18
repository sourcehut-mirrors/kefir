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

    const struct kefir_opt_instruction *instr, *condition_instr2, *condition_instr3, *arg_instr, *arg2_instr;
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

#define CASE(_instr, _op, _res_op, _res2_op)                         \
    case KEFIR_OPT_OPCODE_INT8_##_op:                                \
        OP((_instr), KEFIR_CODEGEN_AMD64_COMPARISON_INT8_##_res_op,  \
           KEFIR_CODEGEN_AMD64_COMPARISON_INT8_##_res_op##_CONST,    \
           KEFIR_CODEGEN_AMD64_COMPARISON_INT8_##_res2_op##_CONST);  \
        break;                                                       \
    case KEFIR_OPT_OPCODE_INT16_##_op:                               \
        OP((_instr), KEFIR_CODEGEN_AMD64_COMPARISON_INT16_##_res_op, \
           KEFIR_CODEGEN_AMD64_COMPARISON_INT16_##_res_op##_CONST,   \
           KEFIR_CODEGEN_AMD64_COMPARISON_INT16_##_res2_op##_CONST); \
        break;                                                       \
    case KEFIR_OPT_OPCODE_INT32_##_op:                               \
        OP((_instr), KEFIR_CODEGEN_AMD64_COMPARISON_INT32_##_res_op, \
           KEFIR_CODEGEN_AMD64_COMPARISON_INT32_##_res_op##_CONST,   \
           KEFIR_CODEGEN_AMD64_COMPARISON_INT32_##_res2_op##_CONST); \
        break;                                                       \
    case KEFIR_OPT_OPCODE_INT64_##_op:                               \
        OP((_instr), KEFIR_CODEGEN_AMD64_COMPARISON_INT64_##_res_op, \
           KEFIR_CODEGEN_AMD64_COMPARISON_INT64_##_res_op##_CONST,   \
           KEFIR_CODEGEN_AMD64_COMPARISON_INT64_##_res2_op##_CONST); \
        break;

        CASE(instr, EQUALS, EQUAL, EQUAL)
        CASE(instr, LESSER, LESSER, GREATER)
        CASE(instr, GREATER, GREATER, LESSER)
        CASE(instr, BELOW, BELOW, ABOVE)
        CASE(instr, ABOVE, ABOVE, BELOW)

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

        case KEFIR_OPT_OPCODE_FLOAT32_LESSER:
            F32OP(instr, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_CONST,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_CONST);
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

        case KEFIR_OPT_OPCODE_FLOAT64_LESSER:
            F64OP(instr, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_CONST,
                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_CONST);
            break;

        case KEFIR_OPT_OPCODE_INT8_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT16_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT32_BOOL_OR:
        case KEFIR_OPT_OPCODE_INT64_BOOL_OR: {
            struct kefir_codegen_amd64_comparison_match_op left_op, right_op;
            REQUIRE_OK(kefir_codegen_amd64_match_comparison_op(code, instr->operation.parameters.refs[0], &left_op));
            REQUIRE_OK(kefir_codegen_amd64_match_comparison_op(code, instr->operation.parameters.refs[1], &right_op));

            const kefir_codegen_amd64_comparison_match_op_type_t op1 = MIN(left_op.type, right_op.type);
            const kefir_codegen_amd64_comparison_match_op_type_t op2 = MAX(left_op.type, right_op.type);
            if (op1 != KEFIR_CODEGEN_AMD64_COMPARISON_NONE && op2 != KEFIR_CODEGEN_AMD64_COMPARISON_NONE) {
                const kefir_opt_instruction_ref_t *op2_refs = op1 == left_op.type ? right_op.refs : left_op.refs;
                switch (op1) {
#define FUSED_CASE(_variant)                                                                         \
    case KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_EQUAL:                                       \
        if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_GREATER &&                         \
            ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||         \
             (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) {        \
            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_GREATER_OR_EQUAL;        \
            match_op->refs[0] = op2_refs[0];                                                         \
            match_op->refs[1] = op2_refs[1];                                                         \
        } else if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_LESSER &&                   \
                   ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||  \
                    (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) { \
            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_LESSER_OR_EQUAL;         \
            match_op->refs[0] = op2_refs[0];                                                         \
            match_op->refs[1] = op2_refs[1];                                                         \
        } else if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_ABOVE &&                    \
                   ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||  \
                    (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) { \
            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_ABOVE_OR_EQUAL;          \
            match_op->refs[0] = op2_refs[0];                                                         \
            match_op->refs[1] = op2_refs[1];                                                         \
        } else if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_BELOW &&                    \
                   ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||  \
                    (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) { \
            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_BELOW_OR_EQUAL;          \
            match_op->refs[0] = op2_refs[0];                                                         \
            match_op->refs[1] = op2_refs[1];                                                         \
        }                                                                                            \
        break;                                                                                       \
                                                                                                     \
    case KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_EQUAL_CONST:                                 \
        if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_GREATER_CONST &&                   \
            ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||         \
             (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) {        \
            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_GREATER_OR_EQUAL_CONST;  \
            match_op->refs[0] = op2_refs[0];                                                         \
            match_op->refs[1] = op2_refs[1];                                                         \
            match_op->int_value = left_op.int_value;                                                 \
        } else if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_LESSER_CONST &&             \
                   ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||  \
                    (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) { \
            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_LESSER_OR_EQUAL_CONST;   \
            match_op->refs[0] = op2_refs[0];                                                         \
            match_op->refs[1] = op2_refs[1];                                                         \
            match_op->int_value = left_op.int_value;                                                 \
        } else if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_ABOVE_CONST &&              \
                   ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||  \
                    (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) { \
            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_ABOVE_OR_EQUAL_CONST;    \
            match_op->refs[0] = op2_refs[0];                                                         \
            match_op->refs[1] = op2_refs[1];                                                         \
            match_op->int_value = left_op.int_value;                                                 \
        } else if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_BELOW_CONST &&              \
                   ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||  \
                    (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) { \
            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_BELOW_OR_EQUAL_CONST;    \
            match_op->refs[0] = op2_refs[0];                                                         \
            match_op->refs[1] = op2_refs[1];                                                         \
            match_op->int_value = left_op.int_value;                                                 \
        }                                                                                            \
        break;

                    FUSED_CASE(8)
                    FUSED_CASE(16)
                    FUSED_CASE(32)
                    FUSED_CASE(64)

#undef FUSED_CASE

                    case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL:
                        if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER &&
                            ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||
                             (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) {
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL;
                            match_op->refs[0] = op2_refs[0];
                            match_op->refs[1] = op2_refs[1];
                        } else if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER &&
                                   ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||
                                    (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) {
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL;
                            match_op->refs[0] = op2_refs[0];
                            match_op->refs[1] = op2_refs[1];
                        }
                        break;

                    case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL_CONST:
                        if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_CONST &&
                            ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||
                             (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) {
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST;
                            match_op->refs[0] = op2_refs[0];
                            match_op->refs[1] = op2_refs[1];
                            match_op->float32_value = left_op.float32_value;
                        } else if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_CONST &&
                                   ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||
                                    (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) {
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST;
                            match_op->refs[0] = op2_refs[0];
                            match_op->refs[1] = op2_refs[1];
                            match_op->float32_value = left_op.float32_value;
                        }
                        break;

                    case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL:
                        if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER &&
                            ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||
                             (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) {
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL;
                            match_op->refs[0] = op2_refs[0];
                            match_op->refs[1] = op2_refs[1];
                        } else if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER &&
                                   ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||
                                    (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) {
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL;
                            match_op->refs[0] = op2_refs[0];
                            match_op->refs[1] = op2_refs[1];
                        }
                        break;

                    case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL_CONST:
                        if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_CONST &&
                            ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||
                             (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) {
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST;
                            match_op->refs[0] = op2_refs[0];
                            match_op->refs[1] = op2_refs[1];
                            match_op->float64_value = left_op.float64_value;
                        } else if (op2 == KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_CONST &&
                                   ((left_op.refs[0] == right_op.refs[0] && left_op.refs[1] == right_op.refs[1]) ||
                                    (left_op.refs[0] == right_op.refs[1] && left_op.refs[1] == right_op.refs[0]))) {
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST;
                            match_op->refs[0] = op2_refs[0];
                            match_op->refs[1] = op2_refs[1];
                            match_op->float64_value = left_op.float64_value;
                        }
                        break;

                    default:
                        // Intentionally left blank
                        break;
                }
            }
        } break;

        case KEFIR_OPT_OPCODE_INT8_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT16_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT32_BOOL_NOT:
        case KEFIR_OPT_OPCODE_INT64_BOOL_NOT:
            REQUIRE_OK(kefir_opt_code_container_instr(code, instr->operation.parameters.refs[0], &condition_instr2));
            switch (condition_instr2->operation.opcode) {
                CASE(condition_instr2, EQUALS, NOT_EQUAL, NOT_EQUAL)
                CASE(condition_instr2, GREATER, LESSER_OR_EQUAL, GREATER_OR_EQUAL)
                CASE(condition_instr2, LESSER, GREATER_OR_EQUAL, LESSER_OR_EQUAL)
                CASE(condition_instr2, BELOW, ABOVE_OR_EQUAL, BELOW_OR_EQUAL)
                CASE(condition_instr2, ABOVE, BELOW_OR_EQUAL, ABOVE_OR_EQUAL)

                case KEFIR_OPT_OPCODE_FLOAT32_EQUALS:
                    F32OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL_CONST,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL_CONST);
                    break;

                case KEFIR_OPT_OPCODE_FLOAT32_LESSER:
                    F32OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST);
                    break;

                case KEFIR_OPT_OPCODE_FLOAT32_GREATER:
                    F32OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST);
                    break;

                case KEFIR_OPT_OPCODE_FLOAT64_EQUALS:
                    F64OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL_CONST,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL_CONST);
                    break;

                case KEFIR_OPT_OPCODE_FLOAT64_LESSER:
                    F64OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST);
                    break;

                case KEFIR_OPT_OPCODE_FLOAT64_GREATER:
                    F64OP(condition_instr2, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST,
                          KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST);
                    break;

                case KEFIR_OPT_OPCODE_INT8_BOOL_OR:
                case KEFIR_OPT_OPCODE_INT16_BOOL_OR:
                case KEFIR_OPT_OPCODE_INT32_BOOL_OR:
                case KEFIR_OPT_OPCODE_INT64_BOOL_OR: {
                    struct kefir_codegen_amd64_comparison_match_op compound_op;
                    REQUIRE_OK(kefir_codegen_amd64_match_comparison_op(code, condition_instr2->id, &compound_op));
                    switch (compound_op.type) {
#define REVERSE_CASE(_variant)                                                         \
    case KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_GREATER_OR_EQUAL:              \
        *match_op = compound_op;                                                       \
        match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_LESSER;        \
        break;                                                                         \
                                                                                       \
    case KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_GREATER_OR_EQUAL_CONST:        \
        *match_op = compound_op;                                                       \
        match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_LESSER_CONST;  \
        break;                                                                         \
                                                                                       \
    case KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_LESSER_OR_EQUAL:               \
        *match_op = compound_op;                                                       \
        match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_GREATER;       \
        break;                                                                         \
                                                                                       \
    case KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_LESSER_OR_EQUAL_CONST:         \
        *match_op = compound_op;                                                       \
        match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_GREATER_CONST; \
        break;                                                                         \
                                                                                       \
    case KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_ABOVE_OR_EQUAL:                \
        *match_op = compound_op;                                                       \
        match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_BELOW;         \
        break;                                                                         \
                                                                                       \
    case KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_ABOVE_OR_EQUAL_CONST:          \
        *match_op = compound_op;                                                       \
        match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_BELOW_CONST;   \
        break;                                                                         \
                                                                                       \
    case KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_BELOW_OR_EQUAL:                \
        *match_op = compound_op;                                                       \
        match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_ABOVE;         \
        break;                                                                         \
                                                                                       \
    case KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_BELOW_OR_EQUAL_CONST:          \
        *match_op = compound_op;                                                       \
        match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_INT##_variant##_ABOVE_CONST;   \
        break;

                        REVERSE_CASE(8)
                        REVERSE_CASE(16)
                        REVERSE_CASE(32)
                        REVERSE_CASE(64)

#undef REVERSE_CASE

                        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL:
                            *match_op = compound_op;
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER;
                            break;

                        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST:
                            *match_op = compound_op;
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_CONST;
                            break;

                        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL:
                            *match_op = compound_op;
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER;
                            break;

                        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST:
                            *match_op = compound_op;
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_CONST;
                            break;

                        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL:
                            *match_op = compound_op;
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER;
                            break;

                        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST:
                            *match_op = compound_op;
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_CONST;
                            break;

                        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL:
                            *match_op = compound_op;
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER;
                            break;

                        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST:
                            *match_op = compound_op;
                            match_op->type = KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_CONST;
                            break;

                        default:
                            // Intentionally left blank
                            break;
                    }
                } break;

                case KEFIR_OPT_OPCODE_INT8_BOOL_NOT:
                case KEFIR_OPT_OPCODE_INT16_BOOL_NOT:
                case KEFIR_OPT_OPCODE_INT32_BOOL_NOT:
                case KEFIR_OPT_OPCODE_INT64_BOOL_NOT:
                    REQUIRE_OK(kefir_opt_code_container_instr(code, condition_instr2->operation.parameters.refs[0],
                                                              &condition_instr3));
                    switch (condition_instr3->operation.opcode) {
                        CASE(condition_instr3, EQUALS, EQUAL, EQUAL)
                        CASE(condition_instr3, LESSER, LESSER, GREATER)
                        CASE(condition_instr3, GREATER, GREATER, LESSER)
                        CASE(condition_instr3, BELOW, BELOW, ABOVE)
                        CASE(condition_instr3, ABOVE, ABOVE, BELOW)

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

                        case KEFIR_OPT_OPCODE_FLOAT32_LESSER:
                            F32OP(condition_instr3, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_CONST,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_CONST);
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

                        case KEFIR_OPT_OPCODE_FLOAT64_LESSER:
                            F64OP(condition_instr3, KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_CONST,
                                  KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_CONST);
                            break;

                        case KEFIR_OPT_OPCODE_INT8_BOOL_OR:
                        case KEFIR_OPT_OPCODE_INT16_BOOL_OR:
                        case KEFIR_OPT_OPCODE_INT32_BOOL_OR:
                        case KEFIR_OPT_OPCODE_INT64_BOOL_OR:
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

#undef CASE
#undef OP
#undef F32OP
#undef F64OP

        default:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

enum int_arithmetics_op_type {
    INT_ARITHMETICS_ADD8,
    INT_ARITHMETICS_ADD8_CONST,
    INT_ARITHMETICS_ADD16,
    INT_ARITHMETICS_ADD16_CONST,
    INT_ARITHMETICS_ADD32,
    INT_ARITHMETICS_ADD32_CONST,
    INT_ARITHMETICS_ADD64,
    INT_ARITHMETICS_ADD64_CONST,
    INT_ARITHMETICS_SUB8,
    INT_ARITHMETICS_SUB8_CONST,
    INT_ARITHMETICS_SUB16,
    INT_ARITHMETICS_SUB16_CONST,
    INT_ARITHMETICS_SUB32,
    INT_ARITHMETICS_SUB32_CONST,
    INT_ARITHMETICS_SUB64,
    INT_ARITHMETICS_SUB64_CONST,
    INT_ARITHMETICS_MUL8,
    INT_ARITHMETICS_MUL8_CONST,
    INT_ARITHMETICS_MUL16,
    INT_ARITHMETICS_MUL16_CONST,
    INT_ARITHMETICS_MUL32,
    INT_ARITHMETICS_MUL32_CONST,
    INT_ARITHMETICS_MUL64,
    INT_ARITHMETICS_MUL64_CONST,
    INT_ARITHMETICS_AND8,
    INT_ARITHMETICS_AND8_CONST,
    INT_ARITHMETICS_AND16,
    INT_ARITHMETICS_AND16_CONST,
    INT_ARITHMETICS_AND32,
    INT_ARITHMETICS_AND32_CONST,
    INT_ARITHMETICS_AND64,
    INT_ARITHMETICS_AND64_CONST,
    INT_ARITHMETICS_OR8,
    INT_ARITHMETICS_OR8_CONST,
    INT_ARITHMETICS_OR16,
    INT_ARITHMETICS_OR16_CONST,
    INT_ARITHMETICS_OR32,
    INT_ARITHMETICS_OR32_CONST,
    INT_ARITHMETICS_OR64,
    INT_ARITHMETICS_OR64_CONST,
    INT_ARITHMETICS_XOR8,
    INT_ARITHMETICS_XOR8_CONST,
    INT_ARITHMETICS_XOR16,
    INT_ARITHMETICS_XOR16_CONST,
    INT_ARITHMETICS_XOR32,
    INT_ARITHMETICS_XOR32_CONST,
    INT_ARITHMETICS_XOR64,
    INT_ARITHMETICS_XOR64_CONST,
    INT_ARITHMETICS_SHL8,
    INT_ARITHMETICS_SHL8_CONST,
    INT_ARITHMETICS_SHL16,
    INT_ARITHMETICS_SHL16_CONST,
    INT_ARITHMETICS_SHL32,
    INT_ARITHMETICS_SHL32_CONST,
    INT_ARITHMETICS_SHL64,
    INT_ARITHMETICS_SHL64_CONST,
    INT_ARITHMETICS_SHR8,
    INT_ARITHMETICS_SHR8_CONST,
    INT_ARITHMETICS_SHR16,
    INT_ARITHMETICS_SHR16_CONST,
    INT_ARITHMETICS_SHR32,
    INT_ARITHMETICS_SHR32_CONST,
    INT_ARITHMETICS_SHR64,
    INT_ARITHMETICS_SHR64_CONST,
    INT_ARITHMETICS_SAR8,
    INT_ARITHMETICS_SAR8_CONST,
    INT_ARITHMETICS_SAR16,
    INT_ARITHMETICS_SAR16_CONST,
    INT_ARITHMETICS_SAR32,
    INT_ARITHMETICS_SAR32_CONST,
    INT_ARITHMETICS_SAR64,
    INT_ARITHMETICS_SAR64_CONST,
    INT_ARITHMETICS_NOT8,
    INT_ARITHMETICS_NOT16,
    INT_ARITHMETICS_NOT32,
    INT_ARITHMETICS_NOT64,
    INT_ARITHMETICS_NEG8,
    INT_ARITHMETICS_NEG16,
    INT_ARITHMETICS_NEG32,
    INT_ARITHMETICS_NEG64
};

struct int_arithmetics_op {
    enum int_arithmetics_op_type type;
    kefir_opt_instruction_ref_t refs[2];
    union {
        kefir_int64_t int_value;
    };
};

static kefir_result_t match_int_arithmetics(struct kefir_codegen_amd64_function *function,
                                            const struct kefir_opt_instruction *instruction,
                                            struct int_arithmetics_op *op) {
    const struct kefir_opt_instruction *arg1, *arg2;
    switch (instruction->operation.opcode) {
#define OP(_opcode, _opcode_const, _direct, _reverse, _const_min, _const_max)                               \
    do {                                                                                                    \
        REQUIRE_OK(kefir_opt_code_container_instr(&function->function->code,                                \
                                                  instruction->operation.parameters.refs[0], &arg1));       \
        REQUIRE_OK(kefir_opt_code_container_instr(&function->function->code,                                \
                                                  instruction->operation.parameters.refs[1], &arg2));       \
        if (arg1->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&                                         \
            arg1->operation.parameters.imm.integer >= (_const_min) &&                                       \
            arg1->operation.parameters.imm.integer <= (_const_max) && (_reverse)) {                         \
            op->type = (_opcode_const);                                                                     \
            op->refs[0] = arg2->id;                                                                         \
            op->refs[1] = arg1->id;                                                                         \
            op->int_value = arg1->operation.parameters.imm.integer;                                         \
        } else if (arg1->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&                                 \
                   (kefir_int64_t) arg1->operation.parameters.imm.uinteger >= (_const_min) &&               \
                   (kefir_int64_t) arg1->operation.parameters.imm.uinteger <= (_const_max) && (_reverse)) { \
            op->type = (_opcode_const);                                                                     \
            op->refs[0] = arg2->id;                                                                         \
            op->refs[1] = arg1->id;                                                                         \
            op->int_value = (kefir_int64_t) arg1->operation.parameters.imm.uinteger;                        \
        } else if (arg2->operation.opcode == KEFIR_OPT_OPCODE_INT_CONST &&                                  \
                   arg2->operation.parameters.imm.integer >= (_const_min) &&                                \
                   arg2->operation.parameters.imm.integer <= (_const_max) && (_direct)) {                   \
            op->type = (_opcode_const);                                                                     \
            op->refs[0] = arg1->id;                                                                         \
            op->refs[1] = arg2->id;                                                                         \
            op->int_value = arg2->operation.parameters.imm.integer;                                         \
        } else if (arg2->operation.opcode == KEFIR_OPT_OPCODE_UINT_CONST &&                                 \
                   (kefir_int64_t) arg2->operation.parameters.imm.uinteger >= (_const_min) &&               \
                   (kefir_int64_t) arg2->operation.parameters.imm.uinteger <= (_const_max) && (_direct)) {  \
            op->type = (_opcode_const);                                                                     \
            op->refs[0] = arg1->id;                                                                         \
            op->refs[1] = arg2->id;                                                                         \
            op->int_value = (kefir_int64_t) arg2->operation.parameters.imm.uinteger;                        \
        } else {                                                                                            \
            op->type = (_opcode);                                                                           \
            op->refs[0] = arg1->id;                                                                         \
            op->refs[1] = arg2->id;                                                                         \
        }                                                                                                   \
    } while (0)
#define UNARY_OP(_opcode)                                        \
    do {                                                         \
        op->type = (_opcode);                                    \
        op->refs[0] = instruction->operation.parameters.refs[0]; \
    } while (0)

        case KEFIR_OPT_OPCODE_INT8_ADD:
            OP(INT_ARITHMETICS_ADD8, INT_ARITHMETICS_ADD8_CONST, true, true, KEFIR_INT8_MIN, KEFIR_INT8_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT16_ADD:
            OP(INT_ARITHMETICS_ADD16, INT_ARITHMETICS_ADD16_CONST, true, true, KEFIR_INT16_MIN, KEFIR_INT16_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT32_ADD:
            OP(INT_ARITHMETICS_ADD32, INT_ARITHMETICS_ADD32_CONST, true, true, KEFIR_INT32_MIN, KEFIR_INT32_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT64_ADD:
            OP(INT_ARITHMETICS_ADD64, INT_ARITHMETICS_ADD64_CONST, true, true, KEFIR_INT32_MIN, KEFIR_INT32_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT8_SUB:
            OP(INT_ARITHMETICS_SUB8, INT_ARITHMETICS_SUB8_CONST, true, false, KEFIR_INT8_MIN, KEFIR_INT8_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT16_SUB:
            OP(INT_ARITHMETICS_SUB16, INT_ARITHMETICS_SUB16_CONST, true, false, KEFIR_INT16_MIN, KEFIR_INT16_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT32_SUB:
            OP(INT_ARITHMETICS_SUB32, INT_ARITHMETICS_SUB32_CONST, true, false, KEFIR_INT32_MIN, KEFIR_INT32_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT64_SUB:
            OP(INT_ARITHMETICS_SUB64, INT_ARITHMETICS_SUB64_CONST, true, false, KEFIR_INT32_MIN, KEFIR_INT32_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT8_MUL:
            OP(INT_ARITHMETICS_MUL8, INT_ARITHMETICS_MUL8_CONST, true, true, KEFIR_INT8_MIN, KEFIR_INT8_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT16_MUL:
            OP(INT_ARITHMETICS_MUL16, INT_ARITHMETICS_MUL16_CONST, true, true, KEFIR_INT16_MIN, KEFIR_INT16_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT32_MUL:
            OP(INT_ARITHMETICS_MUL32, INT_ARITHMETICS_MUL32_CONST, true, true, KEFIR_INT32_MIN, KEFIR_INT32_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT64_MUL:
            OP(INT_ARITHMETICS_MUL64, INT_ARITHMETICS_MUL64_CONST, true, true, KEFIR_INT32_MIN, KEFIR_INT32_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT8_AND:
            OP(INT_ARITHMETICS_AND8, INT_ARITHMETICS_AND8_CONST, true, true, KEFIR_INT8_MIN, KEFIR_INT8_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT16_AND:
            OP(INT_ARITHMETICS_AND16, INT_ARITHMETICS_AND16_CONST, true, true, KEFIR_INT16_MIN, KEFIR_INT16_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT32_AND:
            OP(INT_ARITHMETICS_AND32, INT_ARITHMETICS_AND32_CONST, true, true, KEFIR_INT32_MIN, KEFIR_INT32_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT64_AND:
            OP(INT_ARITHMETICS_AND64, INT_ARITHMETICS_AND64_CONST, true, true, KEFIR_INT32_MIN, KEFIR_INT32_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT8_OR:
            OP(INT_ARITHMETICS_OR8, INT_ARITHMETICS_OR8_CONST, true, true, KEFIR_INT8_MIN, KEFIR_INT8_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT16_OR:
            OP(INT_ARITHMETICS_OR16, INT_ARITHMETICS_OR16_CONST, true, true, KEFIR_INT16_MIN, KEFIR_INT16_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT32_OR:
            OP(INT_ARITHMETICS_OR32, INT_ARITHMETICS_OR32_CONST, true, true, KEFIR_INT32_MIN, KEFIR_INT32_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT64_OR:
            OP(INT_ARITHMETICS_OR64, INT_ARITHMETICS_OR64_CONST, true, true, KEFIR_INT32_MIN, KEFIR_INT32_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT8_XOR:
            OP(INT_ARITHMETICS_XOR8, INT_ARITHMETICS_XOR8_CONST, true, true, KEFIR_INT8_MIN, KEFIR_INT8_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT16_XOR:
            OP(INT_ARITHMETICS_XOR16, INT_ARITHMETICS_XOR16_CONST, true, true, KEFIR_INT16_MIN, KEFIR_INT16_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT32_XOR:
            OP(INT_ARITHMETICS_XOR32, INT_ARITHMETICS_XOR32_CONST, true, true, KEFIR_INT32_MIN, KEFIR_INT32_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT64_XOR:
            OP(INT_ARITHMETICS_XOR64, INT_ARITHMETICS_XOR64_CONST, true, true, KEFIR_INT32_MIN, KEFIR_INT32_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT8_LSHIFT:
            OP(INT_ARITHMETICS_SHL8, INT_ARITHMETICS_SHL8_CONST, true, false, KEFIR_INT8_MIN, KEFIR_INT8_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT16_LSHIFT:
            OP(INT_ARITHMETICS_SHL16, INT_ARITHMETICS_SHL16_CONST, true, false, KEFIR_INT8_MIN, KEFIR_INT8_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT32_LSHIFT:
            OP(INT_ARITHMETICS_SHL32, INT_ARITHMETICS_SHL32_CONST, true, false, KEFIR_INT8_MIN, KEFIR_INT8_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT64_LSHIFT:
            OP(INT_ARITHMETICS_SHL64, INT_ARITHMETICS_SHL64_CONST, true, false, KEFIR_INT8_MIN, KEFIR_INT8_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT8_RSHIFT:
            OP(INT_ARITHMETICS_SHR8, INT_ARITHMETICS_SHR8_CONST, true, false, KEFIR_INT8_MIN, KEFIR_INT8_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT16_RSHIFT:
            OP(INT_ARITHMETICS_SHR16, INT_ARITHMETICS_SHR16_CONST, true, false, KEFIR_INT8_MIN, KEFIR_INT8_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT32_RSHIFT:
            OP(INT_ARITHMETICS_SHR32, INT_ARITHMETICS_SHR32_CONST, true, false, KEFIR_INT8_MIN, KEFIR_INT8_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT64_RSHIFT:
            OP(INT_ARITHMETICS_SHR64, INT_ARITHMETICS_SHR64_CONST, true, false, KEFIR_INT8_MIN, KEFIR_INT8_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT8_ARSHIFT:
            OP(INT_ARITHMETICS_SAR8, INT_ARITHMETICS_SAR8_CONST, true, false, KEFIR_INT8_MIN, KEFIR_INT8_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT16_ARSHIFT:
            OP(INT_ARITHMETICS_SAR16, INT_ARITHMETICS_SAR16_CONST, true, false, KEFIR_INT8_MIN, KEFIR_INT8_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT32_ARSHIFT:
            OP(INT_ARITHMETICS_SAR32, INT_ARITHMETICS_SAR32_CONST, true, false, KEFIR_INT8_MIN, KEFIR_INT8_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT64_ARSHIFT:
            OP(INT_ARITHMETICS_SAR64, INT_ARITHMETICS_SAR64_CONST, true, false, KEFIR_INT8_MIN, KEFIR_INT8_MAX);
            break;

        case KEFIR_OPT_OPCODE_INT8_NOT:
            UNARY_OP(INT_ARITHMETICS_NOT8);
            break;

        case KEFIR_OPT_OPCODE_INT16_NOT:
            UNARY_OP(INT_ARITHMETICS_NOT16);
            break;

        case KEFIR_OPT_OPCODE_INT32_NOT:
            UNARY_OP(INT_ARITHMETICS_NOT32);
            break;

        case KEFIR_OPT_OPCODE_INT64_NOT:
            UNARY_OP(INT_ARITHMETICS_NOT64);
            break;

        case KEFIR_OPT_OPCODE_INT8_NEG:
            UNARY_OP(INT_ARITHMETICS_NEG8);
            break;

        case KEFIR_OPT_OPCODE_INT16_NEG:
            UNARY_OP(INT_ARITHMETICS_NEG16);
            break;

        case KEFIR_OPT_OPCODE_INT32_NEG:
            UNARY_OP(INT_ARITHMETICS_NEG32);
            break;

        case KEFIR_OPT_OPCODE_INT64_NEG:
            UNARY_OP(INT_ARITHMETICS_NEG64);
            break;

#undef OP
#undef UNARY_OP

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 codegen integer arithmetics operation type");
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_FUSION_IMPL(int_arithmetics)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction, kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
    void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));
    REQUIRE(callback != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen instruction fusion callback"));

    struct int_arithmetics_op op = {0};
    REQUIRE_OK(match_int_arithmetics(function, instruction, &op));
    switch (op.type) {
        case INT_ARITHMETICS_ADD8:
        case INT_ARITHMETICS_ADD16:
        case INT_ARITHMETICS_ADD32:
        case INT_ARITHMETICS_ADD64:
        case INT_ARITHMETICS_SUB8:
        case INT_ARITHMETICS_SUB16:
        case INT_ARITHMETICS_SUB32:
        case INT_ARITHMETICS_SUB64:
        case INT_ARITHMETICS_MUL8:
        case INT_ARITHMETICS_MUL16:
        case INT_ARITHMETICS_MUL32:
        case INT_ARITHMETICS_MUL64:
        case INT_ARITHMETICS_AND8:
        case INT_ARITHMETICS_AND16:
        case INT_ARITHMETICS_AND32:
        case INT_ARITHMETICS_AND64:
        case INT_ARITHMETICS_OR8:
        case INT_ARITHMETICS_OR16:
        case INT_ARITHMETICS_OR32:
        case INT_ARITHMETICS_OR64:
        case INT_ARITHMETICS_XOR8:
        case INT_ARITHMETICS_XOR16:
        case INT_ARITHMETICS_XOR32:
        case INT_ARITHMETICS_XOR64:
        case INT_ARITHMETICS_SHL8:
        case INT_ARITHMETICS_SHL16:
        case INT_ARITHMETICS_SHL32:
        case INT_ARITHMETICS_SHL64:
        case INT_ARITHMETICS_SHR8:
        case INT_ARITHMETICS_SHR16:
        case INT_ARITHMETICS_SHR32:
        case INT_ARITHMETICS_SHR64:
        case INT_ARITHMETICS_SAR8:
        case INT_ARITHMETICS_SAR16:
        case INT_ARITHMETICS_SAR32:
        case INT_ARITHMETICS_SAR64:
            REQUIRE_OK(callback(op.refs[0], payload));
            REQUIRE_OK(callback(op.refs[1], payload));
            break;

        case INT_ARITHMETICS_ADD8_CONST:
        case INT_ARITHMETICS_ADD16_CONST:
        case INT_ARITHMETICS_ADD32_CONST:
        case INT_ARITHMETICS_ADD64_CONST:
        case INT_ARITHMETICS_SUB8_CONST:
        case INT_ARITHMETICS_SUB16_CONST:
        case INT_ARITHMETICS_SUB32_CONST:
        case INT_ARITHMETICS_SUB64_CONST:
        case INT_ARITHMETICS_OR8_CONST:
        case INT_ARITHMETICS_OR16_CONST:
        case INT_ARITHMETICS_OR32_CONST:
        case INT_ARITHMETICS_OR64_CONST:
        case INT_ARITHMETICS_XOR8_CONST:
        case INT_ARITHMETICS_XOR16_CONST:
        case INT_ARITHMETICS_XOR32_CONST:
        case INT_ARITHMETICS_XOR64_CONST:
        case INT_ARITHMETICS_SHL8_CONST:
        case INT_ARITHMETICS_SHL16_CONST:
        case INT_ARITHMETICS_SHL32_CONST:
        case INT_ARITHMETICS_SHL64_CONST:
        case INT_ARITHMETICS_SHR8_CONST:
        case INT_ARITHMETICS_SHR16_CONST:
        case INT_ARITHMETICS_SHR32_CONST:
        case INT_ARITHMETICS_SHR64_CONST:
        case INT_ARITHMETICS_SAR8_CONST:
        case INT_ARITHMETICS_SAR16_CONST:
        case INT_ARITHMETICS_SAR32_CONST:
        case INT_ARITHMETICS_SAR64_CONST:
        case INT_ARITHMETICS_NOT8:
        case INT_ARITHMETICS_NOT16:
        case INT_ARITHMETICS_NOT32:
        case INT_ARITHMETICS_NOT64:
        case INT_ARITHMETICS_NEG8:
        case INT_ARITHMETICS_NEG16:
        case INT_ARITHMETICS_NEG32:
        case INT_ARITHMETICS_NEG64:
            REQUIRE_OK(callback(op.refs[0], payload));
            break;

        case INT_ARITHMETICS_MUL8_CONST:
        case INT_ARITHMETICS_MUL16_CONST:
        case INT_ARITHMETICS_MUL32_CONST:
        case INT_ARITHMETICS_MUL64_CONST:
        case INT_ARITHMETICS_AND8_CONST:
        case INT_ARITHMETICS_AND16_CONST:
        case INT_ARITHMETICS_AND32_CONST:
        case INT_ARITHMETICS_AND64_CONST:
            if (op.int_value != 0) {
                REQUIRE_OK(callback(op.refs[0], payload));
            }
            break;
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_arithmetics)(struct kefir_mem *mem,
                                                                     struct kefir_codegen_amd64_function *function,
                                                                     const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    struct int_arithmetics_op op = {0};
    REQUIRE_OK(match_int_arithmetics(function, instruction, &op));
    switch (op.type) {
#define OP(_operation, _variant)                                                                                       \
    do {                                                                                                               \
        kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;                                       \
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, op.refs[0], &arg1_vreg));                            \
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, op.refs[1], &arg2_vreg));                            \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,                                     \
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));    \
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,                                     \
                                                             kefir_asmcmp_context_instr_tail(&function->code.context), \
                                                             result_vreg, arg1_vreg, NULL));                           \
        REQUIRE_OK(kefir_asmcmp_amd64_##_operation(                                                                    \
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                            \
            &KEFIR_ASMCMP_MAKE_VREG##_variant(result_vreg), &KEFIR_ASMCMP_MAKE_VREG##_variant(arg2_vreg), NULL));      \
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));             \
    } while (0)
#define CONST_OP(_operation, _variant, _skip_on_zero)                                                                  \
    do {                                                                                                               \
        kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;                                                  \
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, op.refs[0], &arg1_vreg));                            \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,                                     \
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));    \
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,                                     \
                                                             kefir_asmcmp_context_instr_tail(&function->code.context), \
                                                             result_vreg, arg1_vreg, NULL));                           \
        if (!(_skip_on_zero) || op.int_value != 0) {                                                                   \
            REQUIRE(                                                                                                   \
                op.int_value >= KEFIR_INT32_MIN && op.int_value <= KEFIR_INT32_MAX,                                    \
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 codegen integer arithmetics constant value"));  \
            REQUIRE_OK(kefir_asmcmp_amd64_##_operation(                                                                \
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                        \
                &KEFIR_ASMCMP_MAKE_VREG##_variant(result_vreg), &KEFIR_ASMCMP_MAKE_INT(op.int_value), NULL));          \
        }                                                                                                              \
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));             \
    } while (0)
#define SHIFT_OP(_op, _variant)                                                                                        \
    do {                                                                                                               \
        kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg, arg2_placement_vreg;                  \
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, op.refs[0], &arg1_vreg));                            \
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, op.refs[1], &arg2_vreg));                            \
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
            &KEFIR_ASMCMP_MAKE_VREG##_variant(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(arg2_placement_vreg), NULL));     \
                                                                                                                       \
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));             \
    } while (false)
#define CONST_SHIFT_OP(_op, _variant)                                                                                  \
    do {                                                                                                               \
        kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;                                                  \
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, op.refs[0], &arg1_vreg));                            \
                                                                                                                       \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,                                     \
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));    \
                                                                                                                       \
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,                                     \
                                                             kefir_asmcmp_context_instr_tail(&function->code.context), \
                                                             result_vreg, arg1_vreg, NULL));                           \
                                                                                                                       \
        if (op.int_value != 0) {                                                                                       \
            REQUIRE_OK(kefir_asmcmp_amd64_##_op(                                                                       \
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                        \
                &KEFIR_ASMCMP_MAKE_VREG##_variant(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(op.int_value & 0xffu), NULL)); \
        }                                                                                                              \
                                                                                                                       \
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));             \
    } while (0)
#define UNARY_OP(_op, _variant)                                                                                        \
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
                                            &KEFIR_ASMCMP_MAKE_VREG##_variant(result_vreg), NULL));                    \
                                                                                                                       \
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));             \
    } while (0)

        case INT_ARITHMETICS_ADD8:
            OP(add, 8);
            break;

        case INT_ARITHMETICS_ADD16:
            OP(add, 16);
            break;

        case INT_ARITHMETICS_ADD32:
            OP(add, 32);
            break;

        case INT_ARITHMETICS_ADD64:
            OP(add, 64);
            break;

        case INT_ARITHMETICS_SUB8:
            OP(sub, 8);
            break;

        case INT_ARITHMETICS_SUB16:
            OP(sub, 16);
            break;

        case INT_ARITHMETICS_SUB32:
            OP(sub, 32);
            break;

        case INT_ARITHMETICS_SUB64:
            OP(sub, 64);
            break;

        case INT_ARITHMETICS_MUL8:
            OP(imul, 8);
            break;

        case INT_ARITHMETICS_MUL16:
            OP(imul, 16);
            break;

        case INT_ARITHMETICS_MUL32:
            OP(imul, 32);
            break;

        case INT_ARITHMETICS_MUL64:
            OP(imul, 64);
            break;

        case INT_ARITHMETICS_AND8:
            OP(and, 8);
            break;

        case INT_ARITHMETICS_AND16:
            OP(and, 16);
            break;

        case INT_ARITHMETICS_AND32:
            OP(and, 32);
            break;

        case INT_ARITHMETICS_AND64:
            OP(and, 64);
            break;

        case INT_ARITHMETICS_OR8:
            OP(or, 8);
            break;

        case INT_ARITHMETICS_OR16:
            OP(or, 16);
            break;

        case INT_ARITHMETICS_OR32:
            OP(or, 32);
            break;

        case INT_ARITHMETICS_OR64:
            OP(or, 64);
            break;

        case INT_ARITHMETICS_XOR8:
            OP(xor, 8);
            break;

        case INT_ARITHMETICS_XOR16:
            OP(xor, 16);
            break;

        case INT_ARITHMETICS_XOR32:
            OP(xor, 32);
            break;

        case INT_ARITHMETICS_XOR64:
            OP(xor, 64);
            break;

        case INT_ARITHMETICS_SHL8:
            SHIFT_OP(shl, 8);
            break;

        case INT_ARITHMETICS_SHL16:
            SHIFT_OP(shl, 16);
            break;

        case INT_ARITHMETICS_SHL32:
            SHIFT_OP(shl, 32);
            break;

        case INT_ARITHMETICS_SHL64:
            SHIFT_OP(shl, 64);
            break;

        case INT_ARITHMETICS_SHR8:
            SHIFT_OP(shr, 8);
            break;

        case INT_ARITHMETICS_SHR16:
            SHIFT_OP(shr, 16);
            break;

        case INT_ARITHMETICS_SHR32:
            SHIFT_OP(shr, 32);
            break;

        case INT_ARITHMETICS_SHR64:
            SHIFT_OP(shr, 64);
            break;

        case INT_ARITHMETICS_SAR8:
            SHIFT_OP(sar, 8);
            break;

        case INT_ARITHMETICS_SAR16:
            SHIFT_OP(sar, 16);
            break;

        case INT_ARITHMETICS_SAR32:
            SHIFT_OP(sar, 32);
            break;

        case INT_ARITHMETICS_SAR64:
            SHIFT_OP(sar, 64);
            break;

        case INT_ARITHMETICS_ADD8_CONST:
            CONST_OP(add, 8, true);
            break;

        case INT_ARITHMETICS_ADD16_CONST:
            CONST_OP(add, 16, true);
            break;

        case INT_ARITHMETICS_ADD32_CONST:
            CONST_OP(add, 32, true);
            break;

        case INT_ARITHMETICS_ADD64_CONST:
            CONST_OP(add, 64, true);
            break;

        case INT_ARITHMETICS_SUB8_CONST:
            CONST_OP(sub, 8, true);
            break;

        case INT_ARITHMETICS_SUB16_CONST:
            CONST_OP(sub, 16, true);
            break;

        case INT_ARITHMETICS_SUB32_CONST:
            CONST_OP(sub, 32, true);
            break;

        case INT_ARITHMETICS_SUB64_CONST:
            CONST_OP(sub, 64, true);
            break;

        case INT_ARITHMETICS_OR8_CONST:
            CONST_OP(or, 8, true);
            break;

        case INT_ARITHMETICS_OR16_CONST:
            CONST_OP(or, 16, true);
            break;

        case INT_ARITHMETICS_OR32_CONST:
            CONST_OP(or, 32, true);
            break;

        case INT_ARITHMETICS_OR64_CONST:
            CONST_OP(or, 64, true);
            break;

        case INT_ARITHMETICS_XOR8_CONST:
            CONST_OP(xor, 8, true);
            break;

        case INT_ARITHMETICS_XOR16_CONST:
            CONST_OP(xor, 16, true);
            break;

        case INT_ARITHMETICS_XOR32_CONST:
            CONST_OP(xor, 32, true);
            break;

        case INT_ARITHMETICS_XOR64_CONST:
            CONST_OP(xor, 64, true);
            break;

        case INT_ARITHMETICS_SHL8_CONST:
            CONST_SHIFT_OP(shl, 8);
            break;

        case INT_ARITHMETICS_SHL16_CONST:
            CONST_SHIFT_OP(shl, 16);
            break;

        case INT_ARITHMETICS_SHL32_CONST:
            CONST_SHIFT_OP(shl, 32);
            break;

        case INT_ARITHMETICS_SHL64_CONST:
            CONST_SHIFT_OP(shl, 64);
            break;

        case INT_ARITHMETICS_SHR8_CONST:
            CONST_SHIFT_OP(shr, 8);
            break;

        case INT_ARITHMETICS_SHR16_CONST:
            CONST_SHIFT_OP(shr, 16);
            break;

        case INT_ARITHMETICS_SHR32_CONST:
            CONST_SHIFT_OP(shr, 32);
            break;

        case INT_ARITHMETICS_SHR64_CONST:
            CONST_SHIFT_OP(shr, 64);
            break;

        case INT_ARITHMETICS_SAR8_CONST:
            CONST_SHIFT_OP(sar, 8);
            break;

        case INT_ARITHMETICS_SAR16_CONST:
            CONST_SHIFT_OP(sar, 16);
            break;

        case INT_ARITHMETICS_SAR32_CONST:
            CONST_SHIFT_OP(sar, 32);
            break;

        case INT_ARITHMETICS_SAR64_CONST:
            CONST_SHIFT_OP(sar, 64);
            break;

        case INT_ARITHMETICS_NOT8:
            UNARY_OP(not, 8);
            break;

        case INT_ARITHMETICS_NOT16:
            UNARY_OP(not, 16);
            break;

        case INT_ARITHMETICS_NOT32:
            UNARY_OP(not, 32);
            break;

        case INT_ARITHMETICS_NOT64:
            UNARY_OP(not, 64);
            break;

        case INT_ARITHMETICS_NEG8:
            UNARY_OP(neg, 8);
            break;

        case INT_ARITHMETICS_NEG16:
            UNARY_OP(neg, 16);
            break;

        case INT_ARITHMETICS_NEG32:
            UNARY_OP(neg, 32);
            break;

        case INT_ARITHMETICS_NEG64:
            UNARY_OP(neg, 64);
            break;

        case INT_ARITHMETICS_MUL8_CONST: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            if (op.int_value == 0) {
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), &KEFIR_ASMCMP_MAKE_INT(0), NULL));
            } else if (op.int_value == 1) {
                REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, op.refs[0], &arg1_vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                    arg1_vreg, NULL));
            } else {
                REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, op.refs[0], &arg1_vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_movzx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG16(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(arg1_vreg), NULL));
                REQUIRE_OK(kefir_asmcmp_amd64_imul3(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG16(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(result_vreg),
                    &KEFIR_ASMCMP_MAKE_INT(op.int_value), NULL));
            }
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case INT_ARITHMETICS_MUL16_CONST: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            if (op.int_value == 0) {
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG16(result_vreg), &KEFIR_ASMCMP_MAKE_INT(0), NULL));
            } else if (op.int_value == 1) {
                REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, op.refs[0], &arg1_vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                    arg1_vreg, NULL));
            } else {
                REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, op.refs[0], &arg1_vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_imul3(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG16(result_vreg), &KEFIR_ASMCMP_MAKE_VREG16(arg1_vreg),
                    &KEFIR_ASMCMP_MAKE_INT(op.int_value), NULL));
            }
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case INT_ARITHMETICS_MUL32_CONST: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            if (op.int_value == 0) {
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_INT(0), NULL));
            } else if (op.int_value == 1) {
                REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, op.refs[0], &arg1_vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                    arg1_vreg, NULL));
            } else {
                REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, op.refs[0], &arg1_vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_imul3(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(arg1_vreg),
                    &KEFIR_ASMCMP_MAKE_INT(op.int_value), NULL));
            }
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case INT_ARITHMETICS_MUL64_CONST: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            if (op.int_value == 0) {
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_INT(0), NULL));
            } else if (op.int_value == 1) {
                REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, op.refs[0], &arg1_vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                    arg1_vreg, NULL));
            } else {
                REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, op.refs[0], &arg1_vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_imul3(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg1_vreg),
                    &KEFIR_ASMCMP_MAKE_INT(op.int_value), NULL));
            }
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case INT_ARITHMETICS_AND8_CONST: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            if (op.int_value == 0) {
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), &KEFIR_ASMCMP_MAKE_INT(0), NULL));
            } else {
                REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, op.refs[0], &arg1_vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                    arg1_vreg, NULL));
                REQUIRE_OK(kefir_asmcmp_amd64_and(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), &KEFIR_ASMCMP_MAKE_INT(op.int_value), NULL));
            }
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case INT_ARITHMETICS_AND16_CONST: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            if (op.int_value == 0) {
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG16(result_vreg), &KEFIR_ASMCMP_MAKE_INT(0), NULL));
            } else {
                REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, op.refs[0], &arg1_vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                    arg1_vreg, NULL));
                REQUIRE_OK(kefir_asmcmp_amd64_and(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG16(result_vreg), &KEFIR_ASMCMP_MAKE_INT(op.int_value), NULL));
            }
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case INT_ARITHMETICS_AND32_CONST: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            if (op.int_value == 0) {
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_INT(0), NULL));
            } else {
                REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, op.refs[0], &arg1_vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                    arg1_vreg, NULL));
                REQUIRE_OK(kefir_asmcmp_amd64_and(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_INT(op.int_value), NULL));
            }
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case INT_ARITHMETICS_AND64_CONST: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            if (op.int_value == 0) {
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_INT(0), NULL));
            } else {
                REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, op.refs[0], &arg1_vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                    arg1_vreg, NULL));
                REQUIRE_OK(kefir_asmcmp_amd64_and(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_INT(op.int_value), NULL));
            }
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

#undef OP
#undef CONST_OP
#undef SHIFT_OP
#undef CONST_SHIFT_OP
#undef UNARY_OP
    }
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

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_to_bool)(struct kefir_mem *mem,
                                                                 struct kefir_codegen_amd64_function *function,
                                                                 const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_TO_BOOL:
            REQUIRE_OK(
                kefir_asmcmp_amd64_test(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG8(arg_vreg), &KEFIR_ASMCMP_MAKE_VREG8(arg_vreg), NULL));
            break;

        case KEFIR_OPT_OPCODE_INT16_TO_BOOL:
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG16(arg_vreg), &KEFIR_ASMCMP_MAKE_VREG16(arg_vreg), NULL));
            break;

        case KEFIR_OPT_OPCODE_INT32_TO_BOOL:
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(arg_vreg), &KEFIR_ASMCMP_MAKE_VREG32(arg_vreg), NULL));
            break;

        case KEFIR_OPT_OPCODE_INT64_TO_BOOL:
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg_vreg), NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer instuction opcode");
    }
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

#define DEFINE_DIV_MOD8(_impl, _res)                                                                                  \
    do {                                                                                                              \
        kefir_asmcmp_virtual_register_index_t result_vreg, result_placement_vreg, arg1_vreg, arg1_placement_vreg,     \
            arg2_vreg;                                                                                                \
        REQUIRE_OK(                                                                                                   \
            kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));   \
        REQUIRE_OK(                                                                                                   \
            kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));   \
                                                                                                                      \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(                                                                 \
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg1_placement_vreg));      \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,                                    \
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));   \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(                                                                 \
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_placement_vreg));    \
                                                                                                                      \
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, arg1_placement_vreg,      \
                                                                      KEFIR_AMD64_XASMGEN_REGISTER_RAX));             \
        REQUIRE_OK(                                                                                                   \
            kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, result_placement_vreg, (_res))); \
                                                                                                                      \
        _impl                                                                                                         \
                                                                                                                      \
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));        \
    } while (0)

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_div)(struct kefir_mem *mem,
                                                             struct kefir_codegen_amd64_function *function,
                                                             const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_DIV:
            DEFINE_DIV_MOD8(
                {
                    REQUIRE_OK(kefir_asmcmp_amd64_movsx(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG16(arg1_placement_vreg), &KEFIR_ASMCMP_MAKE_VREG8(arg1_vreg), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_idiv(mem, &function->code,
                                                       kefir_asmcmp_context_instr_tail(&function->code.context),
                                                       &KEFIR_ASMCMP_MAKE_VREG8(arg2_vreg), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                                      kefir_asmcmp_context_instr_tail(&function->code.context),
                                                      &KEFIR_ASMCMP_MAKE_VREG8(result_vreg),
                                                      &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_AL), NULL));
                },
                KEFIR_AMD64_XASMGEN_REGISTER_RAX);
            break;

        case KEFIR_OPT_OPCODE_INT16_DIV:
            DEFINE_DIV_MOD(
                {
                    REQUIRE_OK(kefir_asmcmp_amd64_cwd(mem, &function->code,
                                                      kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_idiv(mem, &function->code,
                                                       kefir_asmcmp_context_instr_tail(&function->code.context),
                                                       &KEFIR_ASMCMP_MAKE_VREG16(arg2_vreg), NULL));
                },
                KEFIR_AMD64_XASMGEN_REGISTER_RAX);
            break;

        case KEFIR_OPT_OPCODE_INT32_DIV:
            DEFINE_DIV_MOD(
                {
                    REQUIRE_OK(kefir_asmcmp_amd64_cdq(mem, &function->code,
                                                      kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_idiv(mem, &function->code,
                                                       kefir_asmcmp_context_instr_tail(&function->code.context),
                                                       &KEFIR_ASMCMP_MAKE_VREG32(arg2_vreg), NULL));
                },
                KEFIR_AMD64_XASMGEN_REGISTER_RAX);
            break;

        case KEFIR_OPT_OPCODE_INT64_DIV:
            DEFINE_DIV_MOD(
                {
                    REQUIRE_OK(kefir_asmcmp_amd64_cqo(mem, &function->code,
                                                      kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_idiv(mem, &function->code,
                                                       kefir_asmcmp_context_instr_tail(&function->code.context),
                                                       &KEFIR_ASMCMP_MAKE_VREG64(arg2_vreg), NULL));
                },
                KEFIR_AMD64_XASMGEN_REGISTER_RAX);
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer opcode");
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_mod)(struct kefir_mem *mem,
                                                             struct kefir_codegen_amd64_function *function,
                                                             const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_MOD:
            DEFINE_DIV_MOD8(
                {
                    REQUIRE_OK(kefir_asmcmp_amd64_movsx(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG16(arg1_placement_vreg), &KEFIR_ASMCMP_MAKE_VREG8(arg1_vreg), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_idiv(mem, &function->code,
                                                       kefir_asmcmp_context_instr_tail(&function->code.context),
                                                       &KEFIR_ASMCMP_MAKE_VREG8(arg2_vreg), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                                      kefir_asmcmp_context_instr_tail(&function->code.context),
                                                      &KEFIR_ASMCMP_MAKE_VREG8(result_vreg),
                                                      &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_AH), NULL));
                },
                KEFIR_AMD64_XASMGEN_REGISTER_RDX);
            break;

        case KEFIR_OPT_OPCODE_INT16_MOD:
            DEFINE_DIV_MOD(
                {
                    REQUIRE_OK(kefir_asmcmp_amd64_cwd(mem, &function->code,
                                                      kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_idiv(mem, &function->code,
                                                       kefir_asmcmp_context_instr_tail(&function->code.context),
                                                       &KEFIR_ASMCMP_MAKE_VREG16(arg2_vreg), NULL));
                },
                KEFIR_AMD64_XASMGEN_REGISTER_RDX);
            break;

        case KEFIR_OPT_OPCODE_INT32_MOD:
            DEFINE_DIV_MOD(
                {
                    REQUIRE_OK(kefir_asmcmp_amd64_cdq(mem, &function->code,
                                                      kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_idiv(mem, &function->code,
                                                       kefir_asmcmp_context_instr_tail(&function->code.context),
                                                       &KEFIR_ASMCMP_MAKE_VREG32(arg2_vreg), NULL));
                },
                KEFIR_AMD64_XASMGEN_REGISTER_RDX);
            break;

        case KEFIR_OPT_OPCODE_INT64_MOD:
            DEFINE_DIV_MOD(
                {
                    REQUIRE_OK(kefir_asmcmp_amd64_cqo(mem, &function->code,
                                                      kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_idiv(mem, &function->code,
                                                       kefir_asmcmp_context_instr_tail(&function->code.context),
                                                       &KEFIR_ASMCMP_MAKE_VREG64(arg2_vreg), NULL));
                },
                KEFIR_AMD64_XASMGEN_REGISTER_RDX);
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer opcode");
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(uint_div)(struct kefir_mem *mem,
                                                              struct kefir_codegen_amd64_function *function,
                                                              const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_UINT8_DIV:
            DEFINE_DIV_MOD8(
                {
                    REQUIRE_OK(kefir_asmcmp_amd64_movzx(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG16(arg1_placement_vreg), &KEFIR_ASMCMP_MAKE_VREG8(arg1_vreg), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_div(mem, &function->code,
                                                      kefir_asmcmp_context_instr_tail(&function->code.context),
                                                      &KEFIR_ASMCMP_MAKE_VREG8(arg2_vreg), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                                      kefir_asmcmp_context_instr_tail(&function->code.context),
                                                      &KEFIR_ASMCMP_MAKE_VREG8(result_vreg),
                                                      &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_AL), NULL));
                },
                KEFIR_AMD64_XASMGEN_REGISTER_RAX);
            break;

        case KEFIR_OPT_OPCODE_UINT16_DIV:
            DEFINE_DIV_MOD(
                {
                    REQUIRE_OK(kefir_asmcmp_amd64_mov(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG16(arg1_placement_upper_vreg), &KEFIR_ASMCMP_MAKE_INT(0), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_div(mem, &function->code,
                                                      kefir_asmcmp_context_instr_tail(&function->code.context),
                                                      &KEFIR_ASMCMP_MAKE_VREG16(arg2_vreg), NULL));
                },
                KEFIR_AMD64_XASMGEN_REGISTER_RAX);
            break;

        case KEFIR_OPT_OPCODE_UINT32_DIV:
            DEFINE_DIV_MOD(
                {
                    REQUIRE_OK(kefir_asmcmp_amd64_mov(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG32(arg1_placement_upper_vreg), &KEFIR_ASMCMP_MAKE_INT(0), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_div(mem, &function->code,
                                                      kefir_asmcmp_context_instr_tail(&function->code.context),
                                                      &KEFIR_ASMCMP_MAKE_VREG32(arg2_vreg), NULL));
                },
                KEFIR_AMD64_XASMGEN_REGISTER_RAX);
            break;

        case KEFIR_OPT_OPCODE_UINT64_DIV:
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
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer opcode");
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(uint_mod)(struct kefir_mem *mem,
                                                              struct kefir_codegen_amd64_function *function,
                                                              const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_UINT8_MOD:
            DEFINE_DIV_MOD8(
                {
                    REQUIRE_OK(kefir_asmcmp_amd64_movzx(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG16(arg1_placement_vreg), &KEFIR_ASMCMP_MAKE_VREG8(arg1_vreg), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_div(mem, &function->code,
                                                      kefir_asmcmp_context_instr_tail(&function->code.context),
                                                      &KEFIR_ASMCMP_MAKE_VREG8(arg2_vreg), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                                      kefir_asmcmp_context_instr_tail(&function->code.context),
                                                      &KEFIR_ASMCMP_MAKE_VREG8(result_vreg),
                                                      &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_AH), NULL));
                },
                KEFIR_AMD64_XASMGEN_REGISTER_RDX);
            break;

        case KEFIR_OPT_OPCODE_UINT16_MOD:
            DEFINE_DIV_MOD(
                {
                    REQUIRE_OK(kefir_asmcmp_amd64_mov(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG16(arg1_placement_upper_vreg), &KEFIR_ASMCMP_MAKE_INT(0), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_div(mem, &function->code,
                                                      kefir_asmcmp_context_instr_tail(&function->code.context),
                                                      &KEFIR_ASMCMP_MAKE_VREG16(arg2_vreg), NULL));
                },
                KEFIR_AMD64_XASMGEN_REGISTER_RDX);
            break;

        case KEFIR_OPT_OPCODE_UINT32_MOD:
            DEFINE_DIV_MOD(
                {
                    REQUIRE_OK(kefir_asmcmp_amd64_mov(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG32(arg1_placement_upper_vreg), &KEFIR_ASMCMP_MAKE_INT(0), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_div(mem, &function->code,
                                                      kefir_asmcmp_context_instr_tail(&function->code.context),
                                                      &KEFIR_ASMCMP_MAKE_VREG32(arg2_vreg), NULL));
                },
                KEFIR_AMD64_XASMGEN_REGISTER_RDX);
            break;

        case KEFIR_OPT_OPCODE_UINT64_MOD:
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
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer opcode");
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_bool_not)(struct kefir_mem *mem,
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
    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_BOOL_NOT:
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG8(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG8(arg1_vreg), NULL));
            break;

        case KEFIR_OPT_OPCODE_INT16_BOOL_NOT:
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG16(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG16(arg1_vreg), NULL));
            break;

        case KEFIR_OPT_OPCODE_INT32_BOOL_NOT:
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG32(arg1_vreg), NULL));
            break;

        case KEFIR_OPT_OPCODE_INT64_BOOL_NOT:
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg1_vreg), NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer instruction opcode");
    }
    REQUIRE_OK(kefir_asmcmp_amd64_sete(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_FUSION_IMPL(int_bool_or)(
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
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_BELOW_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_BELOW_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_BELOW_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_BELOW_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL:
            callback(fused_comparison_op.refs[0], payload);
            callback(fused_comparison_op.refs[1], payload);
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_ABOVE_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_ABOVE_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_ABOVE_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_ABOVE_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_BELOW_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_BELOW_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_BELOW_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_BELOW_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST:
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

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_bool_or)(struct kefir_mem *mem,
                                                                 struct kefir_codegen_amd64_function *function,
                                                                 const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    struct kefir_codegen_amd64_comparison_match_op match_op;
    REQUIRE_OK(kefir_codegen_amd64_match_comparison_op(&function->function->code, instruction->id, &match_op));
    switch (match_op.type) {
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_BELOW_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_BELOW_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_BELOW_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_BELOW_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_ABOVE_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_ABOVE_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_ABOVE_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_ABOVE_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_BELOW_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_BELOW_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_BELOW_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_BELOW_OR_EQUAL_CONST:
            REQUIRE_OK(translate_int_comparison(mem, function, instruction, &match_op));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST:
            REQUIRE_OK(kefir_codegen_amd64_util_translate_float_comparison(mem, function, instruction, &match_op));
            break;

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

            switch (instruction->operation.opcode) {
                case KEFIR_OPT_OPCODE_INT8_BOOL_OR:
                    REQUIRE_OK(kefir_asmcmp_amd64_or(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(arg2_vreg), NULL));
                    break;

                case KEFIR_OPT_OPCODE_INT16_BOOL_OR:
                    REQUIRE_OK(kefir_asmcmp_amd64_or(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG16(result_vreg), &KEFIR_ASMCMP_MAKE_VREG16(arg2_vreg), NULL));
                    break;

                case KEFIR_OPT_OPCODE_INT32_BOOL_OR:
                    REQUIRE_OK(kefir_asmcmp_amd64_or(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(arg2_vreg), NULL));
                    break;

                case KEFIR_OPT_OPCODE_INT64_BOOL_OR:
                    REQUIRE_OK(kefir_asmcmp_amd64_or(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_vreg), NULL));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer instruction opcode");
            }

            REQUIRE_OK(kefir_asmcmp_amd64_setne(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_movzx(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 codegen comparison fusion result");
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_bool_and)(struct kefir_mem *mem,
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

    struct kefir_asmcmp_value arg1, arg2;
    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_BOOL_AND:
            arg1 = KEFIR_ASMCMP_MAKE_VREG8(arg1_vreg);
            arg2 = KEFIR_ASMCMP_MAKE_VREG8(arg2_vreg);
            break;

        case KEFIR_OPT_OPCODE_INT16_BOOL_AND:
            arg1 = KEFIR_ASMCMP_MAKE_VREG16(arg1_vreg);
            arg2 = KEFIR_ASMCMP_MAKE_VREG16(arg2_vreg);
            break;

        case KEFIR_OPT_OPCODE_INT32_BOOL_AND:
            arg1 = KEFIR_ASMCMP_MAKE_VREG32(arg1_vreg);
            arg2 = KEFIR_ASMCMP_MAKE_VREG32(arg2_vreg);
            break;

        case KEFIR_OPT_OPCODE_INT64_BOOL_AND:
            arg1 = KEFIR_ASMCMP_MAKE_VREG64(arg1_vreg);
            arg2 = KEFIR_ASMCMP_MAKE_VREG64(arg2_vreg);
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer instruction opcode");
    }

    REQUIRE_OK(kefir_asmcmp_amd64_test(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &arg1, &arg1, NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_setne(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_test(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &arg2, &arg2, NULL));

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

        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_ABOVE:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_ABOVE:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_ABOVE:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_ABOVE:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_BELOW:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_BELOW:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_BELOW:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_BELOW:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_BELOW_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_BELOW_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_BELOW_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_BELOW_OR_EQUAL:
            REQUIRE_OK(callback(fused_comparison_op.refs[0], payload));
            REQUIRE_OK(callback(fused_comparison_op.refs[1], payload));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_ABOVE_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_ABOVE_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_ABOVE_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_ABOVE_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_ABOVE_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_ABOVE_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_ABOVE_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_ABOVE_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_BELOW_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_BELOW_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_BELOW_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_BELOW_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT8_BELOW_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT16_BELOW_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT32_BELOW_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT64_BELOW_OR_EQUAL_CONST:
            REQUIRE_OK(callback(fused_comparison_op.refs[0], payload));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t translate_int_comparison(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                               const struct kefir_opt_instruction *instruction,
                                               struct kefir_codegen_amd64_comparison_match_op *match_op) {
    switch (match_op->type) {
#define DEFINE_COMPARISON(_type, _op, _variant)                                                                        \
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
                &KEFIR_ASMCMP_MAKE_VREG##_variant(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG##_variant(arg2_vreg), NULL));    \
            REQUIRE_OK(kefir_asmcmp_amd64_##_op(mem, &function->code,                                                  \
                                                kefir_asmcmp_context_instr_tail(&function->code.context),              \
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));                         \
                                                                                                                       \
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));         \
        } while (false);                                                                                               \
        break

        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_EQUAL, sete, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_EQUAL, sete, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_EQUAL, sete, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_EQUAL, sete, 64);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_NOT_EQUAL, setne, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_NOT_EQUAL, setne, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_NOT_EQUAL, setne, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_NOT_EQUAL, setne, 64);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_GREATER, setg, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_GREATER, setg, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_GREATER, setg, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_GREATER, setg, 64);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_GREATER_OR_EQUAL, setge, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_GREATER_OR_EQUAL, setge, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_GREATER_OR_EQUAL, setge, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_GREATER_OR_EQUAL, setge, 64);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_LESSER, setl, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_LESSER, setl, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_LESSER, setl, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_LESSER, setl, 64);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_LESSER_OR_EQUAL, setle, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_LESSER_OR_EQUAL, setle, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_LESSER_OR_EQUAL, setle, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_LESSER_OR_EQUAL, setle, 64);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_ABOVE, seta, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_ABOVE, seta, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_ABOVE, seta, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_ABOVE, seta, 64);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_ABOVE_OR_EQUAL, setae, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_ABOVE_OR_EQUAL, setae, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_ABOVE_OR_EQUAL, setae, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_ABOVE_OR_EQUAL, setae, 64);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_BELOW, setb, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_BELOW, setb, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_BELOW, setb, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_BELOW, setb, 64);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_BELOW_OR_EQUAL, setbe, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_BELOW_OR_EQUAL, setbe, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_BELOW_OR_EQUAL, setbe, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_BELOW_OR_EQUAL, setbe, 64);

#undef DEFINE_COMPARISON

#define DEFINE_COMPARISON(_type, _op, _variant)                                                                        \
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
                    &KEFIR_ASMCMP_MAKE_VREG##_variant(arg1_vreg), &KEFIR_ASMCMP_MAKE_INT(match_op->int_value), NULL)); \
            } else {                                                                                                   \
                REQUIRE_OK(kefir_asmcmp_virtual_register_new(                                                          \
                    mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));          \
                REQUIRE_OK(kefir_asmcmp_amd64_movabs(                                                                  \
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                    \
                    &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_INT(match_op->int_value), NULL));          \
                REQUIRE_OK(kefir_asmcmp_amd64_cmp(                                                                     \
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                    \
                    &KEFIR_ASMCMP_MAKE_VREG##_variant(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG##_variant(tmp_vreg), NULL)); \
            }                                                                                                          \
            REQUIRE_OK(kefir_asmcmp_amd64_##_op(mem, &function->code,                                                  \
                                                kefir_asmcmp_context_instr_tail(&function->code.context),              \
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));                         \
                                                                                                                       \
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));         \
        } while (0);                                                                                                   \
        break

        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_EQUAL_CONST, sete, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_EQUAL_CONST, sete, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_EQUAL_CONST, sete, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_EQUAL_CONST, sete, 64);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_NOT_EQUAL_CONST, setne, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_NOT_EQUAL_CONST, setne, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_NOT_EQUAL_CONST, setne, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_NOT_EQUAL_CONST, setne, 64);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_GREATER_CONST, setg, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_GREATER_CONST, setg, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_GREATER_CONST, setg, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_GREATER_CONST, setg, 64);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_GREATER_OR_EQUAL_CONST, setge, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_GREATER_OR_EQUAL_CONST, setge, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_GREATER_OR_EQUAL_CONST, setge, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_GREATER_OR_EQUAL_CONST, setge, 64);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_LESSER_CONST, setl, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_LESSER_CONST, setl, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_LESSER_CONST, setl, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_LESSER_CONST, setl, 64);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_LESSER_OR_EQUAL_CONST, setle, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_LESSER_OR_EQUAL_CONST, setle, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_LESSER_OR_EQUAL_CONST, setle, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_LESSER_OR_EQUAL_CONST, setle, 64);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_ABOVE_CONST, seta, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_ABOVE_CONST, seta, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_ABOVE_CONST, seta, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_ABOVE_CONST, seta, 64);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_ABOVE_OR_EQUAL_CONST, setae, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_ABOVE_OR_EQUAL_CONST, setae, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_ABOVE_OR_EQUAL_CONST, setae, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_ABOVE_OR_EQUAL_CONST, setae, 64);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_BELOW_CONST, setb, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_BELOW_CONST, setb, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_BELOW_CONST, setb, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_BELOW_CONST, setb, 64);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT8_BELOW_OR_EQUAL_CONST, setbe, 8);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT16_BELOW_OR_EQUAL_CONST, setbe, 16);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT32_BELOW_OR_EQUAL_CONST, setbe, 32);
        DEFINE_COMPARISON(KEFIR_CODEGEN_AMD64_COMPARISON_INT64_BELOW_OR_EQUAL_CONST, setbe, 64);

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
