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
