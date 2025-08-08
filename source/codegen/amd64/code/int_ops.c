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

#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/function.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_arithmetics)(struct kefir_mem *mem,
                                                                     struct kefir_codegen_amd64_function *function,
                                                                     const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    switch (instruction->operation.opcode) {
#define OP(_operation, _variant)                                                                                       \
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
            &KEFIR_ASMCMP_MAKE_VREG##_variant(result_vreg), &KEFIR_ASMCMP_MAKE_VREG##_variant(arg2_vreg), NULL));      \
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));             \
    } while (0)
#define SHIFT_OP(_op, _variant)                                                                                        \
    do {                                                                                                               \
        kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg, arg2_placement_vreg;                  \
        REQUIRE_OK(                                                                                                    \
            kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));    \
        REQUIRE_OK(                                                                                                    \
            kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));    \
                                                                                                                       \
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,                                     \
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));    \
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,                                     \
                                                             kefir_asmcmp_context_instr_tail(&function->code.context), \
                                                             result_vreg, arg1_vreg, NULL));                           \
        const struct kefir_asmcmp_virtual_register *vreg2;                                                             \
        REQUIRE_OK(kefir_asmcmp_virtual_register_get(&function->code.context, arg2_vreg, &vreg2));                     \
        if (vreg2->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_IMMEDIATE_INTEGER &&                                          \
            (vreg2->parameters.immediate_int >= 0 && vreg2->parameters.immediate_int <= KEFIR_INT8_MAX)) {             \
            REQUIRE_OK(kefir_asmcmp_amd64_##_op(                                                                       \
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                        \
                &KEFIR_ASMCMP_MAKE_VREG##_variant(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(arg2_vreg), NULL));           \
        } else {                                                                                                       \
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(                                                              \
                mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg2_placement_vreg));   \
                                                                                                                       \
            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, arg2_placement_vreg,   \
                                                                          KEFIR_AMD64_XASMGEN_REGISTER_RCX));          \
                                                                                                                       \
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(                                                      \
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), arg2_placement_vreg,   \
                arg2_vreg, NULL));                                                                                     \
            REQUIRE_OK(kefir_asmcmp_amd64_##_op(                                                                       \
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                        \
                &KEFIR_ASMCMP_MAKE_VREG##_variant(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(arg2_placement_vreg), NULL)); \
        }                                                                                                              \
                                                                                                                       \
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));             \
    } while (false)
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

        case KEFIR_OPT_OPCODE_INT8_ADD:
            OP(add, 8);
            break;

        case KEFIR_OPT_OPCODE_INT16_ADD:
            OP(add, 16);
            break;

        case KEFIR_OPT_OPCODE_INT32_ADD:
            OP(add, 32);
            break;

        case KEFIR_OPT_OPCODE_INT64_ADD:
            OP(add, 64);
            break;

        case KEFIR_OPT_OPCODE_INT8_SUB:
            OP(sub, 8);
            break;

        case KEFIR_OPT_OPCODE_INT16_SUB:
            OP(sub, 16);
            break;

        case KEFIR_OPT_OPCODE_INT32_SUB:
            OP(sub, 32);
            break;

        case KEFIR_OPT_OPCODE_INT64_SUB:
            OP(sub, 64);
            break;

        case KEFIR_OPT_OPCODE_INT8_MUL:
        case KEFIR_OPT_OPCODE_UINT8_MUL: {
            kefir_asmcmp_virtual_register_index_t result_vreg, result_placement_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(
                mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_placement_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, result_placement_vreg,
                                                                          KEFIR_AMD64_XASMGEN_REGISTER_RAX));

            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_placement_vreg,
                arg1_vreg, NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_imul1(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG8(arg2_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                result_placement_vreg, NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_OPCODE_INT16_MUL:
        case KEFIR_OPT_OPCODE_UINT16_MUL:
            OP(imul, 16);
            break;

        case KEFIR_OPT_OPCODE_INT32_MUL:
        case KEFIR_OPT_OPCODE_UINT32_MUL: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            const struct kefir_asmcmp_virtual_register *vreg2;
            REQUIRE_OK(kefir_asmcmp_virtual_register_get(&function->code.context, arg2_vreg, &vreg2));
            if (vreg2->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_IMMEDIATE_INTEGER) {
                REQUIRE_OK(kefir_asmcmp_amd64_imul3(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(arg1_vreg),
                    &KEFIR_ASMCMP_MAKE_VREG32(arg2_vreg), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                    arg1_vreg, NULL));

                REQUIRE_OK(kefir_asmcmp_amd64_imul(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(arg2_vreg), NULL));
            }
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_OPCODE_INT64_MUL:
        case KEFIR_OPT_OPCODE_UINT64_MUL: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            const struct kefir_asmcmp_virtual_register *vreg2;
            REQUIRE_OK(kefir_asmcmp_virtual_register_get(&function->code.context, arg2_vreg, &vreg2));
            if (vreg2->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_IMMEDIATE_INTEGER) {
                REQUIRE_OK(kefir_asmcmp_amd64_imul3(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg1_vreg),
                    &KEFIR_ASMCMP_MAKE_VREG32(arg2_vreg), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                    arg1_vreg, NULL));

                REQUIRE_OK(kefir_asmcmp_amd64_imul(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_vreg), NULL));
            }
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_OPCODE_INT8_AND:
            OP(and, 8);
            break;

        case KEFIR_OPT_OPCODE_INT16_AND:
            OP(and, 16);
            break;

        case KEFIR_OPT_OPCODE_INT32_AND:
            OP(and, 32);
            break;

        case KEFIR_OPT_OPCODE_INT64_AND:
            OP(and, 64);
            break;

        case KEFIR_OPT_OPCODE_INT8_OR:
            OP(or, 8);
            break;

        case KEFIR_OPT_OPCODE_INT16_OR:
            OP(or, 16);
            break;

        case KEFIR_OPT_OPCODE_INT32_OR:
            OP(or, 32);
            break;

        case KEFIR_OPT_OPCODE_INT64_OR:
            OP(or, 64);
            break;

        case KEFIR_OPT_OPCODE_INT8_XOR:
            OP(xor, 8);
            break;

        case KEFIR_OPT_OPCODE_INT16_XOR:
            OP(xor, 16);
            break;

        case KEFIR_OPT_OPCODE_INT32_XOR:
            OP(xor, 32);
            break;

        case KEFIR_OPT_OPCODE_INT64_XOR:
            OP(xor, 64);
            break;

        case KEFIR_OPT_OPCODE_INT8_LSHIFT:
            SHIFT_OP(shl, 8);
            break;

        case KEFIR_OPT_OPCODE_INT16_LSHIFT:
            SHIFT_OP(shl, 16);
            break;

        case KEFIR_OPT_OPCODE_INT32_LSHIFT:
            SHIFT_OP(shl, 32);
            break;

        case KEFIR_OPT_OPCODE_INT64_LSHIFT:
            SHIFT_OP(shl, 64);
            break;

        case KEFIR_OPT_OPCODE_INT8_RSHIFT:
            SHIFT_OP(shr, 8);
            break;

        case KEFIR_OPT_OPCODE_INT16_RSHIFT:
            SHIFT_OP(shr, 16);
            break;

        case KEFIR_OPT_OPCODE_INT32_RSHIFT:
            SHIFT_OP(shr, 32);
            break;

        case KEFIR_OPT_OPCODE_INT64_RSHIFT:
            SHIFT_OP(shr, 64);
            break;

        case KEFIR_OPT_OPCODE_INT8_ARSHIFT:
            SHIFT_OP(sar, 8);
            break;

        case KEFIR_OPT_OPCODE_INT16_ARSHIFT:
            SHIFT_OP(sar, 16);
            break;

        case KEFIR_OPT_OPCODE_INT32_ARSHIFT:
            SHIFT_OP(sar, 32);
            break;

        case KEFIR_OPT_OPCODE_INT64_ARSHIFT:
            SHIFT_OP(sar, 64);
            break;

        case KEFIR_OPT_OPCODE_INT8_NOT:
            UNARY_OP(not, 8);
            break;

        case KEFIR_OPT_OPCODE_INT16_NOT:
            UNARY_OP(not, 16);
            break;

        case KEFIR_OPT_OPCODE_INT32_NOT:
            UNARY_OP(not, 32);
            break;

        case KEFIR_OPT_OPCODE_INT64_NOT:
            UNARY_OP(not, 64);
            break;

        case KEFIR_OPT_OPCODE_INT8_NEG:
            UNARY_OP(neg, 8);
            break;

        case KEFIR_OPT_OPCODE_INT16_NEG:
            UNARY_OP(neg, 16);
            break;

        case KEFIR_OPT_OPCODE_INT32_NEG:
            UNARY_OP(neg, 32);
            break;

        case KEFIR_OPT_OPCODE_INT64_NEG:
            UNARY_OP(neg, 64);
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");

#undef OP
#undef SHIFT_OP
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
        _impl REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(                                                    \
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), arg1_placement_upper_vreg, \
            NULL));                                                                                                    \
        REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(mem, &function->code,                                     \
                                                             kefir_asmcmp_context_instr_tail(&function->code.context), \
                                                             arg1_placement_lower_vreg, NULL));                        \
                                                                                                                       \
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,                                     \
                                                             kefir_asmcmp_context_instr_tail(&function->code.context), \
                                                             result_vreg, result_placement_vreg, NULL));               \
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

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_bool_or)(struct kefir_mem *mem,
                                                                 struct kefir_codegen_amd64_function *function,
                                                                 const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg, arg1_vreg, NULL));

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

    REQUIRE_OK(kefir_asmcmp_amd64_setne(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movzx(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(result_vreg),
                                        NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));

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

    kefir_bool_t full_test1 = true, full_test2 = true;

    const struct kefir_opt_instruction *arg1_instr, *arg2_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->function->code, instruction->operation.parameters.refs[0],
                                              &arg1_instr));
    REQUIRE_OK(kefir_opt_code_container_instr(&function->function->code, instruction->operation.parameters.refs[1],
                                              &arg2_instr));
#define SKIP_FULL_TEST(_opcode)                                                                      \
    ((_opcode) == KEFIR_OPT_OPCODE_SCALAR_COMPARE || (_opcode) == KEFIR_OPT_OPCODE_INT8_BOOL_AND ||  \
     (_opcode) == KEFIR_OPT_OPCODE_INT16_BOOL_AND || (_opcode) == KEFIR_OPT_OPCODE_INT32_BOOL_AND || \
     (_opcode) == KEFIR_OPT_OPCODE_INT64_BOOL_AND || (_opcode) == KEFIR_OPT_OPCODE_INT8_BOOL_OR ||   \
     (_opcode) == KEFIR_OPT_OPCODE_INT16_BOOL_OR || (_opcode) == KEFIR_OPT_OPCODE_INT32_BOOL_OR ||   \
     (_opcode) == KEFIR_OPT_OPCODE_INT64_BOOL_OR || (_opcode) == KEFIR_OPT_OPCODE_INT8_BOOL_NOT ||   \
     (_opcode) == KEFIR_OPT_OPCODE_INT16_BOOL_NOT || (_opcode) == KEFIR_OPT_OPCODE_INT32_BOOL_NOT || \
     (_opcode) == KEFIR_OPT_OPCODE_INT64_BOOL_NOT)
    if (SKIP_FULL_TEST(arg1_instr->operation.opcode)) {
        full_test1 = false;
    }
    if (SKIP_FULL_TEST(arg2_instr->operation.opcode)) {
        full_test2 = false;
    }
#undef SKIP_FULL_TEST

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

    if (full_test1) {
        REQUIRE_OK(kefir_asmcmp_amd64_test(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), &arg1, &arg1, NULL));

        REQUIRE_OK(kefir_asmcmp_amd64_setne(mem, &function->code,
                                            kefir_asmcmp_context_instr_tail(&function->code.context),
                                            &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             result_vreg, arg1_vreg, NULL));
    }

    if (full_test2) {
        REQUIRE_OK(kefir_asmcmp_amd64_test(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), &arg2, &arg2, NULL));

        REQUIRE_OK(kefir_asmcmp_amd64_setne(mem, &function->code,
                                            kefir_asmcmp_context_instr_tail(&function->code.context),
                                            &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg, arg2_vreg, NULL));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movzx(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg),
                                        NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));

    return KEFIR_OK;
}
