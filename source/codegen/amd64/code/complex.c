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
#include "kefir/codegen/amd64/module.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/target/abi/amd64/type_layout.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

#define PAIR_REAL_INDEX 0
#define PAIR_IMAGINARY_INDEX 1

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float32_real)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg1_real_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(
        kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, PAIR_REAL_INDEX, &arg1_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_vreg, arg1_real_vreg, NULL));

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

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg1_imaginary_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, PAIR_IMAGINARY_INDEX,
                                                     &arg1_imaginary_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_vreg, arg1_imaginary_vreg, NULL));

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

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg1_real_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(
        kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, PAIR_REAL_INDEX, &arg1_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_vreg, arg1_real_vreg, NULL));

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

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg1_imaginary_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, PAIR_IMAGINARY_INDEX,
                                                     &arg1_imaginary_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_vreg, arg1_imaginary_vreg, NULL));

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
    REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(
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
    REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(
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

    kefir_asmcmp_virtual_register_index_t result_vreg, result_real_vreg, result_imaginary_vreg, arg1_vreg, arg2_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_imaginary_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context,
                                                      KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_SINGLE, result_real_vreg,
                                                      result_imaginary_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_real_vreg, arg1_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_imaginary_vreg, arg2_vreg, NULL));

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

    kefir_asmcmp_virtual_register_index_t result_vreg, result_real_vreg, result_imaginary_vreg, arg1_vreg, arg2_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_imaginary_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context,
                                                      KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_DOUBLE, result_real_vreg,
                                                      result_imaginary_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_real_vreg, arg1_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_imaginary_vreg, arg2_vreg, NULL));

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
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));
    REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(
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
        arg2_real_vreg, arg2_imag_vreg, tmp1_vreg, tmp2_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp2_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_imag_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         tmp1_vreg, arg1_real_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         tmp2_vreg, arg1_imag_vreg, NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_cmpeqps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                   &KEFIR_ASMCMP_MAKE_VREG(tmp1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_real_vreg), NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_cmpeqps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                   &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_andps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(tmp1_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movd2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp1_vreg),
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
        arg2_real_vreg, arg2_imag_vreg, tmp1_vreg, tmp2_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp2_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_imag_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         tmp1_vreg, arg1_real_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         tmp2_vreg, arg1_imag_vreg, NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_cmpeqpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                   &KEFIR_ASMCMP_MAKE_VREG(tmp1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_real_vreg), NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_cmpeqpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                   &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_andpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(tmp1_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movd2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp1_vreg),
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
    REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg, tmp_vreg, tmp2_vreg, tmp3_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp3_vreg));

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

    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(3), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fucomip(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_setnp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_sete(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG8(tmp2_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG8(tmp2_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fucomip(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_setnp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_sete(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG8(tmp3_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG8(tmp3_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG8(tmp3_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp2_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movzx(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp3_vreg),
                                        NULL));

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

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg1_real_vreg, arg1_imag_vreg, zero_vreg, tmp1_vreg,
        tmp2_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &zero_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp2_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         tmp1_vreg, arg1_real_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         tmp2_vreg, arg1_imag_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_pxor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(zero_vreg), &KEFIR_ASMCMP_MAKE_VREG(zero_vreg), NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_cmpneqps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(tmp1_vreg), &KEFIR_ASMCMP_MAKE_VREG(zero_vreg), NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_cmpneqps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), &KEFIR_ASMCMP_MAKE_VREG(zero_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_orps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(tmp1_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movd2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp1_vreg),
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

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg1_real_vreg, arg1_imag_vreg, zero_vreg, tmp1_vreg,
        tmp2_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &zero_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp2_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         tmp1_vreg, arg1_real_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         tmp2_vreg, arg1_imag_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_pxor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(zero_vreg), &KEFIR_ASMCMP_MAKE_VREG(zero_vreg), NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_cmpneqpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(tmp1_vreg), &KEFIR_ASMCMP_MAKE_VREG(zero_vreg), NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_cmpneqpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), &KEFIR_ASMCMP_MAKE_VREG(zero_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_orpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(tmp1_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movd2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp1_vreg),
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
    REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, tmp_vreg, tmp2_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp2_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD * 2, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_fldz(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fucomi(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_X87(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(2), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_setp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_setne(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG8(tmp2_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_or(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG8(tmp2_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fucomip(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_setp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_setne(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_or(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_or(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp2_vreg), NULL));

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

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg1_real_vreg, arg1_imag_vreg, arg2_vreg,
        arg2_real_vreg, arg2_imag_vreg, result_real_vreg, result_imag_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context,
                                                      KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_SINGLE, result_real_vreg,
                                                      result_imag_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_real_vreg, arg1_real_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_imag_vreg, arg1_imag_vreg, NULL));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_ADD:
            REQUIRE_OK(kefir_asmcmp_amd64_addss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_real_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_real_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_addss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_vreg), NULL));
            break;

        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_SUB:
            REQUIRE_OK(kefir_asmcmp_amd64_subss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_real_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_real_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_subss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_vreg), NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected opcode");
    }

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

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg1_real_vreg, arg1_imag_vreg, arg2_vreg,
        arg2_real_vreg, arg2_imag_vreg, result_real_vreg, result_imag_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context,
                                                      KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_DOUBLE, result_real_vreg,
                                                      result_imag_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_real_vreg, arg1_real_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_imag_vreg, arg1_imag_vreg, NULL));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_ADD:
            REQUIRE_OK(kefir_asmcmp_amd64_addsd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_real_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_real_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_addsd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_vreg), NULL));
            break;

        case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_SUB:
            REQUIRE_OK(kefir_asmcmp_amd64_subsd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_real_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_real_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_subsd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_vreg), NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected opcode");
    }

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
    REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(
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

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float32_mul)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
    function->codegen_module->constants.complex_float32_mul = true;

    kefir_asmcmp_virtual_register_index_t result_vreg, result_placement_vreg, result_real_vreg, result_imag_vreg,
        arg1_vreg, arg1_real_vreg, arg1_imag_vreg, arg1_placement_vreg, arg2_vreg, arg2_placement_vreg, arg2_real_vreg,
        arg2_imag_vreg, tmp_vreg, tmp2_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg1_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg2_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context,
                                                      KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_SINGLE, result_real_vreg,
                                                      result_imag_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         arg1_placement_vreg, arg1_real_vreg, NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_insertps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(arg1_placement_vreg),
                                    &KEFIR_ASMCMP_MAKE_VREG(arg1_imag_vreg), &KEFIR_ASMCMP_MAKE_UINT(0x10), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         arg2_placement_vreg, arg2_real_vreg, NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_insertps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(arg2_placement_vreg),
                                    &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_vreg), &KEFIR_ASMCMP_MAKE_UINT(0x10), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movaps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG128(tmp_vreg),
                                         &KEFIR_ASMCMP_MAKE_VREG128(arg2_placement_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shufps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG128(tmp_vreg),
                                         &KEFIR_ASMCMP_MAKE_VREG128(arg2_placement_vreg), &KEFIR_ASMCMP_MAKE_UINT(160),
                                         NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mulps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG128(tmp_vreg),
                                        &KEFIR_ASMCMP_MAKE_VREG128(arg1_placement_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_placement_vreg, arg1_placement_vreg, NULL));
    if (function->codegen_module->codegen->config->position_independent_code) {
        REQUIRE_OK(
            kefir_asmcmp_amd64_xorps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG128(result_placement_vreg),
                                     &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                                              KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT32_MUL,
                                                                              KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
                                     NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_xorps(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG128(result_placement_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                       KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT32_MUL, 0,
                                                       KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
            NULL));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_shufps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG128(result_placement_vreg),
                                         &KEFIR_ASMCMP_MAKE_VREG128(result_placement_vreg),
                                         &KEFIR_ASMCMP_MAKE_UINT(177), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         tmp2_vreg, arg2_placement_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shufps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG128(tmp2_vreg), &KEFIR_ASMCMP_MAKE_VREG128(tmp2_vreg),
                                         &KEFIR_ASMCMP_MAKE_UINT(245), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mulps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG128(result_placement_vreg),
                                        &KEFIR_ASMCMP_MAKE_VREG128(tmp2_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_addps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG128(result_placement_vreg),
                                        &KEFIR_ASMCMP_MAKE_VREG128(tmp_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movaps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_placement_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shufps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg), &KEFIR_ASMCMP_MAKE_UINT(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_real_vreg, result_placement_vreg, NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float32_div)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
    function->codegen_module->constants.complex_float32_div = true;

    kefir_asmcmp_virtual_register_index_t result_vreg, result_placement_vreg, result_real_vreg, result_imag_vreg,
        arg1_vreg, arg1_placement_vreg, arg1_real_vreg, arg1_imag_vreg, arg2_vreg, arg2_placement_vreg, arg2_real_vreg,
        arg2_imag_vreg, tmp1_vreg, tmp2_vreg, tmp3_vreg, tmp4_vreg, tmp5_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg1_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg2_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp3_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp4_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp5_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context,
                                                      KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_SINGLE, result_real_vreg,
                                                      result_imag_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         arg1_placement_vreg, arg1_real_vreg, NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_insertps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(arg1_placement_vreg),
                                    &KEFIR_ASMCMP_MAKE_VREG(arg1_imag_vreg), &KEFIR_ASMCMP_MAKE_UINT(0x10), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         arg2_placement_vreg, arg2_real_vreg, NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_insertps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(arg2_placement_vreg),
                                    &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_vreg), &KEFIR_ASMCMP_MAKE_UINT(0x10), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_cvtps2pd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp4_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_placement_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_cvtps2pd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_placement_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movaps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(tmp5_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp4_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movaps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(tmp1_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp4_vreg), NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_unpcklpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(tmp5_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp4_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mulpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(tmp5_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shufpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg),
                                         &KEFIR_ASMCMP_MAKE_UINT(1), NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_unpckhpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(tmp1_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp4_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mulpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(tmp4_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp4_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mulpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp1_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movaps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(tmp3_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp4_vreg), NULL));
    if (function->codegen_module->codegen->config->position_independent_code) {
        REQUIRE_OK(
            kefir_asmcmp_amd64_xorps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG128(tmp2_vreg),
                                     &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                                              KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT32_DIV,
                                                                              KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
                                     NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_xorps(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG128(tmp2_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                       KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT32_DIV, 0,
                                                       KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
            NULL));
    }
    REQUIRE_OK(kefir_asmcmp_amd64_shufpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(tmp3_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp4_vreg),
                                         &KEFIR_ASMCMP_MAKE_UINT(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_addpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(tmp5_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_addpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(tmp4_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp3_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_divpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(tmp5_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp4_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_cvtpd2ps(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_placement_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp5_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movaps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_placement_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shufps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg), &KEFIR_ASMCMP_MAKE_UINT(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_real_vreg, result_placement_vreg, NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float64_mul)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
    function->codegen_module->constants.complex_float64_mul = true;

    kefir_asmcmp_virtual_register_index_t result_vreg, result_real_placement_vreg, result_imag_placement_vreg,
        arg1_vreg, arg1_real_vreg, arg1_imag_vreg, arg1_real_placement_vreg, arg1_imag_placement_vreg, arg2_vreg,
        arg2_real_placement_vreg, arg2_imag_placement_vreg, arg2_real_vreg, arg2_imag_vreg, tmp_vreg, tmp2_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg1_real_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg1_imag_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg2_real_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg2_imag_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_real_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_imag_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_DOUBLE, result_real_placement_vreg,
        result_imag_placement_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         arg1_real_placement_vreg, arg1_real_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         arg1_imag_placement_vreg, arg1_imag_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         arg2_real_placement_vreg, arg2_real_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         arg2_imag_placement_vreg, arg2_imag_vreg, NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_unpcklpd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_real_placement_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_imag_placement_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movaps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
                                         &KEFIR_ASMCMP_MAKE_VREG(arg1_real_placement_vreg), NULL));
    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(
            kefir_asmcmp_amd64_xorps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
                                     &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                                              KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT64_MUL,
                                                                              KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
                                     NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_xorps(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                       KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT64_MUL, 0,
                                                       KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
            NULL));
    }
    REQUIRE_OK(kefir_asmcmp_amd64_shufpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
                                         &KEFIR_ASMCMP_MAKE_UINT(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_unpcklpd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg2_real_placement_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_real_placement_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_unpcklpd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_placement_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_placement_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mulpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(arg2_real_placement_vreg),
                                        &KEFIR_ASMCMP_MAKE_VREG(arg1_real_placement_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mulpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_placement_vreg),
                                        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_addpd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_placement_vreg),
                                        &KEFIR_ASMCMP_MAKE_VREG(arg2_real_placement_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movaps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_imag_placement_vreg),
                                         &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_placement_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movaps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_real_placement_vreg),
                                         &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_placement_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_unpckhpd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_imag_placement_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_placement_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float64_div)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, result_real_vreg, result_imag_vreg, result_placement_vreg,
        arg1_vreg, arg1_placement_vreg, arg1_real_vreg, arg1_imag_vreg, arg2_vreg, arg2_placement_vreg, arg2_real_vreg,
        arg2_imag_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(
        mem, &function->code.context, kefir_abi_amd64_complex_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_double_qword_alignment(function->codegen->abi_variant), &arg1_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(
        mem, &function->code.context, kefir_abi_amd64_complex_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_double_qword_alignment(function->codegen->abi_variant), &arg2_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(
        mem, &function->code.context, kefir_abi_amd64_complex_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_double_qword_alignment(function->codegen->abi_variant), &result_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context,
                                                      KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_DOUBLE, result_real_vreg,
                                                      result_imag_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_placement_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_real_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_placement_vreg, KEFIR_AMD64_ABI_QWORD,
                                                                           KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                       &KEFIR_ASMCMP_MAKE_VREG(arg1_imag_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_placement_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(arg2_real_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_placement_vreg, KEFIR_AMD64_ABI_QWORD,
                                                                           KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                       &KEFIR_ASMCMP_MAKE_VREG(arg2_imag_vreg), NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_fld1(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_placement_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmul1(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_placement_vreg, KEFIR_AMD64_ABI_QWORD,
                                                                          KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmul1(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_faddp1(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_X87(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fdivrp1(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_X87(3), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_placement_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmul1(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_X87(3), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmul1(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_X87(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_placement_vreg, KEFIR_AMD64_ABI_QWORD,
                                                                          KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmulp1(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_X87(4), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(3), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_faddp1(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_X87(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmul1(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_X87(4), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_placement_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmulp1(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_fsubp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_fmulp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_placement_vreg, KEFIR_AMD64_ABI_QWORD,
                                                                           KEFIR_ASMCMP_OPERAND_VARIANT_FP_DOUBLE),
                                       NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_real_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_placement_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg),
                                       &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_placement_vreg, KEFIR_AMD64_ABI_QWORD,
                                                                           KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                       NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_long_double_mul)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));
    REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(
        mem, &function->code.context, kefir_abi_amd64_complex_long_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_long_double_qword_alignment(function->codegen->abi_variant), &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD * 2, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, KEFIR_AMD64_ABI_QWORD * 2, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmul2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_X87(2), &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmul1(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_X87(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fsubp1(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_X87(3), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(3), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmulp1(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmulp1(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_X87(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_faddp1(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_X87(1), NULL));

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

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_long_double_div)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));
    REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));
    function->codegen_module->constants.complex_long_double_div = true;

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(
        mem, &function->code.context, kefir_abi_amd64_complex_long_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_complex_long_double_qword_alignment(function->codegen->abi_variant), &result_vreg));

    kefir_asmcmp_label_index_t label1, label2, label3, label4;
    REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label1));
    REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label2));
    REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label3));
    REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label4));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 2 * KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_fabs(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg2_vreg, 2 * KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_fabs(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fcomip2(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_X87(0), &KEFIR_ASMCMP_MAKE_X87(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_amd64_fld(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                     KEFIR_AMD64_CONSTANT_COMPLEX_LONG_DOUBLE_DIV,
                                                     KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
            NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_fld(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                       KEFIR_AMD64_CONSTANT_COMPLEX_LONG_DOUBLE_DIV, 0,
                                                       KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
            NULL));
    }
    REQUIRE_OK(kefir_asmcmp_amd64_jb(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(label3), NULL));

    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, label1));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fdivr2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_X87(2), &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_X87(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmul2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_X87(0), &KEFIR_ASMCMP_MAKE_X87(3), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fadd2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_X87(0), &KEFIR_ASMCMP_MAKE_X87(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmulp2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_X87(1), &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fdivrp2(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_X87(1), &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_X87(3), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmul2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_X87(0), &KEFIR_ASMCMP_MAKE_X87(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fadd2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_X87(0), &KEFIR_ASMCMP_MAKE_X87(3), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmul2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_X87(0), &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(3), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmulp2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_X87(2), &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(3), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fsubrp2(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_X87(1), &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmulp2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_X87(2), &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(1), NULL));

    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, label2));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(label4), NULL));

    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, label3));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fdiv2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_X87(0), &KEFIR_ASMCMP_MAKE_X87(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmul2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_X87(0), &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fadd2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_X87(0), &KEFIR_ASMCMP_MAKE_X87(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmulp2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_X87(3), &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fdivrp2(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_X87(1), &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_X87(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmul2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_X87(0), &KEFIR_ASMCMP_MAKE_X87(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fadd2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_X87(0), &KEFIR_ASMCMP_MAKE_X87(4), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmul2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_X87(0), &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(4), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmulp2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_X87(2), &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_X87(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fsubrp2(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_X87(2), &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fmulp2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_X87(1), &KEFIR_ASMCMP_MAKE_X87(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(label2), NULL));

    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, label4));
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
    function->codegen_module->constants.complex_float32_neg = true;

    kefir_asmcmp_virtual_register_index_t result_vreg, result_real_vreg, result_imag_vreg, arg1_vreg, arg1_real_vreg,
        arg1_imag_vreg, tmp_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context,
                                                      KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_SINGLE, result_real_vreg,
                                                      result_imag_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         tmp_vreg, arg1_real_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_insertps(mem, &function->code,
                                           kefir_asmcmp_context_instr_tail(&function->code.context),
                                           &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_imag_vreg),
                                           &KEFIR_ASMCMP_MAKE_UINT(0x10), NULL));

    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(
            kefir_asmcmp_amd64_xorps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
                                     &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                                              KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT32_NEG,
                                                                              KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
                                     NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_xorps(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                       KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT32_NEG, 0,
                                                       KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
            NULL));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_movaps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
                                         NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shufps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg), &KEFIR_ASMCMP_MAKE_UINT(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_real_vreg, tmp_vreg, NULL));

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

    kefir_asmcmp_virtual_register_index_t result_vreg, result_real_vreg, result_imag_vreg, arg1_vreg, arg1_real_vreg,
        arg1_imag_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context,
                                                      KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_DOUBLE, result_real_vreg,
                                                      result_imag_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_real_vreg, arg1_real_vreg, NULL));

    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(
            kefir_asmcmp_amd64_xorps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(result_real_vreg),
                                     &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                                              KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT64_NEG,
                                                                              KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
                                     NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_xorps(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(result_real_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                       KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT64_NEG, 0,
                                                       KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
            NULL));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_imag_vreg, arg1_imag_vreg, NULL));

    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(
            kefir_asmcmp_amd64_xorps(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg),
                                     &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                                              KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT64_NEG,
                                                                              KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
                                     NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_xorps(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                       KEFIR_AMD64_CONSTANT_COMPLEX_FLOAT64_NEG, 0,
                                                       KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
            NULL));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_imag_vreg, result_imag_vreg, NULL));

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
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));
    REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));
    function->codegen_module->constants.complex_float64_neg = true;

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(
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

    kefir_asmcmp_virtual_register_index_t result_vreg, result_real_vreg, result_imag_vreg, arg1_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context,
                                                      KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_SINGLE, result_real_vreg,
                                                      result_imag_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_real_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD / 2, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));

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

    kefir_asmcmp_virtual_register_index_t result_vreg, result_real_vreg, result_imag_vreg, arg1_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_imag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context,
                                                      KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_FLOAT_DOUBLE, result_real_vreg,
                                                      result_imag_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_real_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_imag_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg1_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));

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
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));
    REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(
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

    kefir_asmcmp_virtual_register_index_t location_vreg, value_vreg, value_real_vreg, value_imag_vreg;

    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &location_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &value_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, value_vreg, 0, &value_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, value_vreg, 1, &value_imag_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movd(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(value_real_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, KEFIR_AMD64_ABI_QWORD / 2,
                                                                           KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                       &KEFIR_ASMCMP_MAKE_VREG(value_imag_vreg), NULL));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_float64_store)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t location_vreg, value_vreg, value_real_vreg, value_imag_vreg;

    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &location_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &value_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, value_vreg, 0, &value_real_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, value_vreg, 1, &value_imag_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(value_real_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movq(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(value_imag_vreg), NULL));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(complex_long_double_store)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
    REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));

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
