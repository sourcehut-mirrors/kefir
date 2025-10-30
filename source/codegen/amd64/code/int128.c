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
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_to_bool)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t value_vreg, lower_half_vreg, upper_half_vreg, result_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &value_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, value_vreg, 0, &lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, value_vreg, 1, &upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_vreg, lower_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_or(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(upper_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_setne(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movzx(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_trunacate_int64)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t value_vreg, lower_half_vreg, result_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &value_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, value_vreg, 0, &lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_vreg, lower_half_vreg, NULL)); 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_zero_extend_64bits)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg_vreg, lower_half_vreg, upper_half_vreg, result_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERAL_PURPOSE, lower_half_vreg, upper_half_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        lower_half_vreg, arg_vreg, NULL)); 
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG32(upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG32(upper_half_vreg), NULL));
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_sign_extend_64bits)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg_vreg, lower_half_vreg, upper_half_vreg, result_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERAL_PURPOSE, lower_half_vreg, upper_half_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        lower_half_vreg, arg_vreg, NULL)); 
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        upper_half_vreg, arg_vreg, NULL)); 
    REQUIRE_OK(kefir_asmcmp_amd64_sar(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG64(upper_half_vreg), &KEFIR_ASMCMP_MAKE_UINT(KEFIR_AMD64_ABI_QWORD * 8 - 1), NULL));
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_load)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t location_vreg, lower_half_vreg, upper_half_vreg, result_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &location_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERAL_PURPOSE, lower_half_vreg, upper_half_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(lower_half_vreg), &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(upper_half_vreg), &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_store)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t location_vreg, value_vreg, lower_half_vreg, upper_half_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &location_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &value_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, value_vreg, 0, &lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, value_vreg, 1, &upper_half_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), &KEFIR_ASMCMP_MAKE_VREG(lower_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), &KEFIR_ASMCMP_MAKE_VREG(upper_half_vreg), NULL));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_add)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg1_vreg, arg1_lower_half_vreg, arg1_upper_half_vreg, arg2_vreg, arg2_lower_half_vreg, arg2_upper_half_vreg, result_lower_half_vreg, result_upper_half_vreg, result_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERAL_PURPOSE, result_lower_half_vreg, result_upper_half_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_lower_half_vreg, arg1_lower_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_upper_half_vreg, arg1_upper_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_lower_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_lower_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_adc(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_upper_half_vreg), NULL));
 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_sub)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg1_vreg, arg1_lower_half_vreg, arg1_upper_half_vreg, arg2_vreg, arg2_lower_half_vreg, arg2_upper_half_vreg, result_lower_half_vreg, result_upper_half_vreg, result_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERAL_PURPOSE, result_lower_half_vreg, result_upper_half_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_lower_half_vreg, arg1_lower_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_upper_half_vreg, arg1_upper_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_sub(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_lower_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_lower_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_sbb(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_upper_half_vreg), NULL));
 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_and)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg1_vreg, arg1_lower_half_vreg, arg1_upper_half_vreg, arg2_vreg, arg2_lower_half_vreg, arg2_upper_half_vreg, result_lower_half_vreg, result_upper_half_vreg, result_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERAL_PURPOSE, result_lower_half_vreg, result_upper_half_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_lower_half_vreg, arg1_lower_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_upper_half_vreg, arg1_upper_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_lower_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_lower_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_upper_half_vreg), NULL));
 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_or)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg1_vreg, arg1_lower_half_vreg, arg1_upper_half_vreg, arg2_vreg, arg2_lower_half_vreg, arg2_upper_half_vreg, result_lower_half_vreg, result_upper_half_vreg, result_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERAL_PURPOSE, result_lower_half_vreg, result_upper_half_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_lower_half_vreg, arg1_lower_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_upper_half_vreg, arg1_upper_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_or(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_lower_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_lower_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_or(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_upper_half_vreg), NULL));
 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_xor)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg1_vreg, arg1_lower_half_vreg, arg1_upper_half_vreg, arg2_vreg, arg2_lower_half_vreg, arg2_upper_half_vreg, result_lower_half_vreg, result_upper_half_vreg, result_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERAL_PURPOSE, result_lower_half_vreg, result_upper_half_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_lower_half_vreg, arg1_lower_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_upper_half_vreg, arg1_upper_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_lower_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_lower_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_upper_half_vreg), NULL));
 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_mul)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg1_vreg, arg1_lower_half_vreg, arg1_upper_half_vreg, arg2_vreg, arg2_lower_half_vreg, arg2_upper_half_vreg, result_lower_half_vreg, result_upper_half_vreg, result_vreg,
        tmp_vreg, tmp2_vreg, tmp3_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp3_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERAL_PURPOSE, result_lower_half_vreg, result_upper_half_vreg, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, tmp2_vreg, KEFIR_AMD64_XASMGEN_REGISTER_RAX));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, tmp3_vreg, KEFIR_AMD64_XASMGEN_REGISTER_RDX));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_upper_half_vreg, arg1_upper_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        tmp_vreg, arg2_upper_half_vreg, NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_imul(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_lower_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_imul(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_lower_half_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        tmp2_vreg, arg1_lower_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        tmp3_vreg, arg2_lower_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mul(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp3_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_lower_half_vreg, tmp2_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp3_vreg), NULL));
 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_lshift)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg1_vreg, arg1_lower_half_vreg, arg1_upper_half_vreg, arg2_vreg, result_lower_half_vreg, result_upper_half_vreg, result_vreg,
        tmp_vreg, tmp2_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERAL_PURPOSE, result_lower_half_vreg, result_upper_half_vreg, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, tmp_vreg, KEFIR_AMD64_XASMGEN_REGISTER_RCX));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        tmp_vreg, arg2_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_upper_half_vreg, arg1_upper_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        tmp2_vreg, arg1_lower_half_vreg, NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_shld(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_lower_half_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shl(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG32(result_lower_half_vreg), &KEFIR_ASMCMP_MAKE_VREG32(result_lower_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_test(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(KEFIR_AMD64_ABI_QWORD * 8), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_cmovne(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_cmove(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_lower_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), NULL));
 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_arshift)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg1_vreg, arg1_lower_half_vreg, arg1_upper_half_vreg, arg2_vreg, result_lower_half_vreg, result_upper_half_vreg, result_vreg,
        tmp_vreg, tmp2_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERAL_PURPOSE, result_lower_half_vreg, result_upper_half_vreg, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, tmp_vreg, KEFIR_AMD64_XASMGEN_REGISTER_RCX));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        tmp_vreg, arg2_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_lower_half_vreg, arg1_lower_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_upper_half_vreg, arg1_upper_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        tmp2_vreg, arg1_upper_half_vreg, NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_shrd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_lower_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_sar(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_sar(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), &KEFIR_ASMCMP_MAKE_UINT(63), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_test(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(KEFIR_AMD64_ABI_QWORD * 8), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_cmovne(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_lower_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(result_upper_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_cmovne(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), NULL));
 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_rshift)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg1_vreg, arg1_lower_half_vreg, arg1_upper_half_vreg, arg2_vreg, result_lower_half_vreg, result_upper_half_vreg, result_vreg,
        tmp_vreg, tmp2_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERAL_PURPOSE, result_lower_half_vreg, result_upper_half_vreg, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, tmp_vreg, KEFIR_AMD64_XASMGEN_REGISTER_RCX));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        tmp_vreg, arg2_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_lower_half_vreg, arg1_lower_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        tmp2_vreg, arg1_upper_half_vreg, NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_shrd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_lower_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG32(result_upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG32(result_upper_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_test(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(KEFIR_AMD64_ABI_QWORD * 8), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_cmovne(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_lower_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_cmove(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), NULL));
 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_equal)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg1_vreg, arg1_lower_half_vreg, arg1_upper_half_vreg, arg2_vreg, arg2_lower_half_vreg, arg2_upper_half_vreg, result_vreg,
        tmp_vreg, tmp2_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp2_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        tmp_vreg, arg1_lower_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        tmp2_vreg, arg1_upper_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_lower_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_upper_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_or(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_sete(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_greater)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg1_vreg, arg1_lower_half_vreg, arg1_upper_half_vreg, arg2_vreg, arg2_lower_half_vreg, arg2_upper_half_vreg, result_vreg,
        tmp_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        tmp_vreg, arg2_upper_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_cmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg2_lower_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_lower_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_sbb(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_upper_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_setl(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movzx(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_less)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg1_vreg, arg1_lower_half_vreg, arg1_upper_half_vreg, arg2_vreg, arg2_lower_half_vreg, arg2_upper_half_vreg, result_vreg,
        tmp_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        tmp_vreg, arg1_upper_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_cmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_lower_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_lower_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_sbb(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_upper_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_setl(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movzx(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_above)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg1_vreg, arg1_lower_half_vreg, arg1_upper_half_vreg, arg2_vreg, arg2_lower_half_vreg, arg2_upper_half_vreg, result_vreg,
        tmp_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        tmp_vreg, arg2_upper_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_cmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg2_lower_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_lower_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_sbb(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_upper_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_setb(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movzx(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_below)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg1_vreg, arg1_lower_half_vreg, arg1_upper_half_vreg, arg2_vreg, arg2_lower_half_vreg, arg2_upper_half_vreg, result_vreg,
        tmp_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 0, &arg2_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg2_vreg, 1, &arg2_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        tmp_vreg, arg1_upper_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_cmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(arg1_lower_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_lower_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_sbb(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_upper_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_setb(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movzx(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_neg)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg1_vreg, arg1_lower_half_vreg, arg1_upper_half_vreg, result_lower_half_vreg, result_upper_half_vreg, result_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERAL_PURPOSE, result_lower_half_vreg, result_upper_half_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_lower_half_vreg, arg1_lower_half_vreg, NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG32(result_upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG32(result_upper_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_neg(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_lower_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_sbb(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_upper_half_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_upper_half_vreg), NULL));
 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_not)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg1_vreg, arg1_lower_half_vreg, arg1_upper_half_vreg, result_lower_half_vreg, result_upper_half_vreg, result_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 0, &arg1_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, arg1_vreg, 1, &arg1_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_pair(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR_GENERAL_PURPOSE, result_lower_half_vreg, result_upper_half_vreg, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_lower_half_vreg, arg1_lower_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_upper_half_vreg, arg1_upper_half_vreg, NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_not(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_lower_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_not(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_upper_half_vreg), NULL));
 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int128_bool_not)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t value_vreg, lower_half_vreg, upper_half_vreg, result_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &value_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, value_vreg, 0, &lower_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_pair_at(&function->code.context, value_vreg, 1, &upper_half_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_vreg, lower_half_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_or(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(upper_half_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_sete(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movzx(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
 
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}
