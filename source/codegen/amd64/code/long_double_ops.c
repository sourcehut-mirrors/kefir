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
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/target/abi/amd64/type_layout.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(long_double_binary_op)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_long_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_long_double_qword_alignment(function->codegen->abi_variant), &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_ADD:
            REQUIRE_OK(kefir_asmcmp_amd64_faddp(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_SUB:
            REQUIRE_OK(kefir_asmcmp_amd64_fsubp(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_MUL:
            REQUIRE_OK(kefir_asmcmp_amd64_fmulp(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_DIV:
            REQUIRE_OK(kefir_asmcmp_amd64_fdivp(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected instruction opcode");
    }

    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(long_double_neg)(struct kefir_mem *mem,
                                                                     struct kefir_codegen_amd64_function *function,
                                                                     const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_long_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_long_double_qword_alignment(function->codegen->abi_variant), &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_fchs(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(long_double_equals)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, tmp_vreg, arg1_vreg, arg2_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fucomip(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fstp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_setnp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_cmovne(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
                                         NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(long_double_greater)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fcomip(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fstp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_seta(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(long_double_less)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fcomip(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fstp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_seta(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_to_long_double)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_long_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_long_double_qword_alignment(function->codegen->abi_variant), &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_push(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(arg1_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fild(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(KEFIR_AMD64_XASMGEN_REGISTER_RSP, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
        NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                      &KEFIR_ASMCMP_MAKE_UINT(KEFIR_AMD64_ABI_QWORD), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(uint_to_long_double)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_long_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_long_double_qword_alignment(function->codegen->abi_variant), &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_push(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(arg1_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fild(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(KEFIR_AMD64_XASMGEN_REGISTER_RSP, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
        NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                      &KEFIR_ASMCMP_MAKE_UINT(KEFIR_AMD64_ABI_QWORD), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_test(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));

    kefir_asmcmp_label_index_t nosign_label;
    REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &nosign_label));

    REQUIRE_OK(kefir_asmcmp_amd64_jns(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(nosign_label), NULL));

    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(
            kefir_asmcmp_amd64_fadd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_AMD64_CONSTANT_UINT_TO_LONG_DOUBLE,
                                                                             KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE),
                                    NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_fadd(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_AMD64_CONSTANT_UINT_TO_LONG_DOUBLE, 0,
                                                       KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE),
            NULL));
    }

    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, nosign_label));

    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(float32_to_long_double)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, tmp_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_float_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_float_qword_alignment(function->codegen->abi_variant), &tmp_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_long_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_long_double_qword_alignment(function->codegen->abi_variant), &result_vreg));

    const struct kefir_asmcmp_virtual_register *arg1;
    REQUIRE_OK(kefir_asmcmp_virtual_register_get(&function->code.context, arg1_vreg, &arg1));
    if (arg1->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT) {
        REQUIRE_OK(kefir_asmcmp_amd64_movq(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
            &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
            &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(float64_to_long_double)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, tmp_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_float_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_float_qword_alignment(function->codegen->abi_variant), &tmp_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_long_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_long_double_qword_alignment(function->codegen->abi_variant), &result_vreg));

    const struct kefir_asmcmp_virtual_register *arg1;
    REQUIRE_OK(kefir_asmcmp_virtual_register_get(&function->code.context, arg1_vreg, &arg1));
    if (arg1->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT) {
        REQUIRE_OK(kefir_asmcmp_amd64_movq(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
            &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
            &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(long_double_to_int)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, result_placement_vreg, arg1_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_placement_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, result_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_RAX));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    const char *symbolic_label = KEFIR_AMD64_RUNTIME_LONG_DOUBLE_TO_INT;
    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_format(mem, &function->code.context, &symbolic_label, KEFIR_AMD64_PLT,
                                       KEFIR_AMD64_RUNTIME_LONG_DOUBLE_TO_INT));
    }
    REQUIRE_OK(kefir_asmcmp_amd64_call(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(symbolic_label, 0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_placement_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_vreg, result_placement_vreg, NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(long_double_to_uint)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, result_placement_vreg, arg1_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_placement_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, result_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_RAX));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    const char *symbolic_label = KEFIR_AMD64_RUNTIME_LONG_DOUBLE_TO_UINT;
    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_format(mem, &function->code.context, &symbolic_label, KEFIR_AMD64_PLT,
                                       KEFIR_AMD64_RUNTIME_LONG_DOUBLE_TO_UINT));
    }
    REQUIRE_OK(kefir_asmcmp_amd64_call(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(symbolic_label, 0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_placement_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_vreg, result_placement_vreg, NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(long_double_to_float32)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fstp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(KEFIR_AMD64_XASMGEN_REGISTER_RSP, -4,
                                                                            KEFIR_ASMCMP_OPERAND_VARIANT_FP_SINGLE),
                                       NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                                       &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(KEFIR_AMD64_XASMGEN_REGISTER_RSP, -4,
                                                                            KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                       NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(long_double_to_float64)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fstp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(KEFIR_AMD64_XASMGEN_REGISTER_RSP, -8,
                                                                            KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE),
                                       NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                                       &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(KEFIR_AMD64_XASMGEN_REGISTER_RSP, -8,
                                                                            KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                       NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}
