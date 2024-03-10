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

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float32_real)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float32_imaginary)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD / 2, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float64_real)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float64_imaginary)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_long_double_real)(
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

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_long_double_imaginary)(
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

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 2 * KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float32_from)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_complex_float_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_float_qword_alignment(function->codegen->abi_variant), &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, KEFIR_AMD64_ABI_QWORD / 2,
                                                                           KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                       &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float64_from)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_complex_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_double_qword_alignment(function->codegen->abi_variant), &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_long_double_from)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_complex_long_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_long_double_qword_alignment(function->codegen->abi_variant), &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 2 * KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float32_equals)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg1_real_vreg, arg1_imag_vreg, arg2_vreg,
        arg2_real_vreg, arg2_imag_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg1_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg1_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg2_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg2_imag_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_real_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_imag_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD / 2, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg2_real_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, KEFIR_AMD64_ABI_QWORD / 2, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_cmpeqps(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_real_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_real_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_cmpeqps(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_imag_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_andps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(arg1_real_vreg),
                                        &KEFIR_ASMCMP_MAKE_VREG(arg1_imag_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movd2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_real_vreg),
                                        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_INT(1), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float64_equals)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg1_real_vreg, arg1_imag_vreg, arg2_vreg,
        arg2_real_vreg, arg2_imag_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg1_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg1_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg2_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg2_imag_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_real_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_imag_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg2_real_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_cmpeqpd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_real_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_real_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_cmpeqpd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_imag_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_andpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(arg1_real_vreg),
                                        &KEFIR_ASMCMP_MAKE_VREG(arg1_imag_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movd2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_real_vreg),
                                        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_INT(1), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_long_double_equals)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, result_placement_vreg, arg1_vreg, arg2_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, result_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_RAX));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD * 2, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, KEFIR_AMD64_ABI_QWORD * 2, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));

    const char *symbolic_label = KEFIR_AMD64_RUNTIME_COMPLEX_LONG_DOUBLE_EQUALS;
    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_format(mem, &function->code.context, &symbolic_label, KEFIR_AMD64_PLT,
                                       KEFIR_AMD64_RUNTIME_COMPLEX_LONG_DOUBLE_EQUALS));
    }
    REQUIRE_OK(kefir_asmcmp_amd64_call(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(symbolic_label, 0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_vreg, result_placement_vreg, NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float32_truncate_1bit)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg1_real_vreg, arg1_imag_vreg, zero_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg1_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg1_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &zero_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_real_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_imag_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD / 2, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_pxor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(zero_vreg), &KEFIR_ASMCMP_MAKE_VREG(zero_vreg), NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_cmpneqps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(arg1_real_vreg), &KEFIR_ASMCMP_MAKE_VREG(zero_vreg), NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_cmpneqps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(arg1_imag_vreg), &KEFIR_ASMCMP_MAKE_VREG(zero_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_orps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(arg1_real_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_imag_vreg),
                                       NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movd2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_real_vreg),
                                        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_INT(1), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float64_truncate_1bit)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg1_real_vreg, arg1_imag_vreg, zero_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg1_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg1_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &zero_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_real_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_imag_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_pxor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(zero_vreg), &KEFIR_ASMCMP_MAKE_VREG(zero_vreg), NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_cmpneqpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(arg1_real_vreg), &KEFIR_ASMCMP_MAKE_VREG(zero_vreg), NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_cmpneqpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(arg1_imag_vreg), &KEFIR_ASMCMP_MAKE_VREG(zero_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_orpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(arg1_real_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_imag_vreg),
                                       NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movd2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_real_vreg),
                                        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_INT(1), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_long_double_truncate_1bit)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, result_placement_vreg, arg1_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, result_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_RAX));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD * 2, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));

    const char *symbolic_label = KEFIR_AMD64_RUNTIME_COMPLEX_LONG_DOUBLE_TRUNCATE_1BIT;
    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_format(mem, &function->code.context, &symbolic_label, KEFIR_AMD64_PLT,
                                       KEFIR_AMD64_RUNTIME_COMPLEX_LONG_DOUBLE_TRUNCATE_1BIT));
    }
    REQUIRE_OK(kefir_asmcmp_amd64_call(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(symbolic_label, 0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_vreg, result_placement_vreg, NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float32_add_sub)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg, result_real_vreg, result_imag_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_complex_float_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_float_qword_alignment(function->codegen->abi_variant), &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_imag_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_real_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD / 2, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_ADD:
            REQUIRE_OK(kefir_asmcmp_amd64_addss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_real_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
            REQUIRE_OK(
                kefir_asmcmp_amd64_addss(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg),
                                         &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, KEFIR_AMD64_ABI_QWORD / 2,
                                                                             KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                         NULL));
            break;

        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_SUB:
            REQUIRE_OK(kefir_asmcmp_amd64_subss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_real_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
            REQUIRE_OK(
                kefir_asmcmp_amd64_subss(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg),
                                         &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, KEFIR_AMD64_ABI_QWORD / 2,
                                                                             KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                         NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected opcode");
    }

    REQUIRE_OK(kefir_asmcmp_amd64_movd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(result_real_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, KEFIR_AMD64_ABI_QWORD / 2,
                                                                           KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                       &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float64_add_sub)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg, result_real_vreg, result_imag_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_complex_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_double_qword_alignment(function->codegen->abi_variant), &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_imag_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_real_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_ADD:
            REQUIRE_OK(kefir_asmcmp_amd64_addsd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_real_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
            REQUIRE_OK(
                kefir_asmcmp_amd64_addsd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg),
                                         &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, KEFIR_AMD64_ABI_QWORD,
                                                                             KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                         NULL));
            break;

        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_SUB:
            REQUIRE_OK(kefir_asmcmp_amd64_subsd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_real_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
            REQUIRE_OK(
                kefir_asmcmp_amd64_subsd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg),
                                         &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, KEFIR_AMD64_ABI_QWORD,
                                                                             KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                         NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected opcode");
    }

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(result_real_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_long_double_add_sub)(
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
        mem, &function->code.context, kefir_abi_amd64_complex_long_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_long_double_qword_alignment(function->codegen->abi_variant), &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_ADD:
            REQUIRE_OK(kefir_asmcmp_amd64_faddp(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
            break;

        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_SUB:
            REQUIRE_OK(kefir_asmcmp_amd64_fsubp(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected opcode");
    }

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, KEFIR_AMD64_ABI_QWORD * 2, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD * 2, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_ADD:
            REQUIRE_OK(kefir_asmcmp_amd64_faddp(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
            break;

        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_SUB:
            REQUIRE_OK(kefir_asmcmp_amd64_fsubp(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected opcode");
    }

    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, KEFIR_AMD64_ABI_QWORD * 2, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t call_runtime(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                   const char *routine) {
    const char *symbolic_label = routine;
    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_format(mem, &function->code.context, &symbolic_label, KEFIR_AMD64_PLT, routine));
    }
    REQUIRE_OK(kefir_asmcmp_amd64_call(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(symbolic_label, 0), NULL));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float32_mul_div)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, result_placement_vreg, arg1_vreg, arg1_placement_vreg, arg2_vreg,
        arg2_placement_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_complex_float_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_float_qword_alignment(function->codegen->abi_variant), &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg1_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg2_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, result_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_XMM0));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, arg1_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_XMM0));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, arg2_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_XMM1));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_placement_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg2_placement_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_MUL:
            REQUIRE_OK(call_runtime(mem, function, KEFIR_AMD64_RUNTIME_COMPLEX_FLOAT32_MUL));
            break;

        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_DIV:
            REQUIRE_OK(call_runtime(mem, function, KEFIR_AMD64_RUNTIME_COMPLEX_FLOAT32_DIV));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected opcode");
    }

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(result_placement_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float64_mul_div)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, result_real_placement_vreg, result_imag_placement_vreg,
        arg1_vreg, arg1_real_placement_vreg, arg1_imag_placement_vreg, arg2_vreg, arg2_real_placement_vreg,
        arg2_imag_placement_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_complex_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_double_qword_alignment(function->codegen->abi_variant), &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_real_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_imag_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg1_real_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg1_imag_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg2_real_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg2_imag_placement_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, result_real_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_XMM0));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, result_imag_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_XMM1));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, arg1_real_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_XMM0));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, arg1_imag_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_XMM1));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, arg2_real_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_XMM2));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, arg2_imag_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_XMM3));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_real_placement_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_imag_placement_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg2_real_placement_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_placement_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_MUL:
            REQUIRE_OK(call_runtime(mem, function, KEFIR_AMD64_RUNTIME_COMPLEX_FLOAT64_MUL));
            break;

        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_DIV:
            REQUIRE_OK(call_runtime(mem, function, KEFIR_AMD64_RUNTIME_COMPLEX_FLOAT64_DIV));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected opcode");
    }

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(result_real_placement_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(result_imag_placement_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_long_double_mul_div)(
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
        mem, &function->code.context, kefir_abi_amd64_complex_long_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_long_double_qword_alignment(function->codegen->abi_variant), &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, KEFIR_AMD64_ABI_QWORD * 2, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD * 2, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_MUL:
            REQUIRE_OK(call_runtime(mem, function, KEFIR_AMD64_RUNTIME_COMPLEX_LONG_DOUBLE_MUL));
            break;

        case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_DIV:
            REQUIRE_OK(call_runtime(mem, function, KEFIR_AMD64_RUNTIME_COMPLEX_LONG_DOUBLE_DIV));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected opcode");
    }

    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, KEFIR_AMD64_ABI_QWORD * 2, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float32_neg)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, tmp_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_complex_float_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_float_qword_alignment(function->codegen->abi_variant), &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_xorps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                 &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
                                 &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT32_NEG,
                                                                          KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
                                 NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float64_neg)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, tmp_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_complex_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_double_qword_alignment(function->codegen->abi_variant), &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_xorps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                 &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
                                 &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT64_NEG,
                                                                          KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
                                 NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_xorps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                 &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
                                 &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT64_NEG,
                                                                          KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
                                 NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_long_double_neg)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_complex_long_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_long_double_qword_alignment(function->codegen->abi_variant), &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_fchs(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD * 2, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_fchs(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, KEFIR_AMD64_ABI_QWORD * 2, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float32_load)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, tmp_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_complex_float_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_float_qword_alignment(function->codegen->abi_variant), &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float64_load)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, tmp_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_complex_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_double_qword_alignment(function->codegen->abi_variant), &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_long_double_load)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_complex_long_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_long_double_qword_alignment(function->codegen->abi_variant), &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD * 2, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, KEFIR_AMD64_ABI_QWORD * 2, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float32_store)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t location_vreg, value_vreg, tmp_vreg;

    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &location_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &value_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(value_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float64_store)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t location_vreg, value_vreg, tmp_vreg;

    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &location_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &value_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(value_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(value_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_long_double_store)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t location_vreg, value_vreg;

    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &location_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &value_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
        &KEFIR_ASMCMP_MAKE_INT(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, 3 * KEFIR_AMD64_ABI_QWORD,
                                                                          KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                                      &KEFIR_ASMCMP_MAKE_INT(0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(value_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(value_vreg, KEFIR_AMD64_ABI_QWORD * 2, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, KEFIR_AMD64_ABI_QWORD * 2,
                                                                           KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
                                       NULL));
    return KEFIR_OK;
}
