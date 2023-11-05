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

#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/function.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/target/abi/amd64/parameters.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t vararg_start_impl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                        const struct kefir_opt_instruction *instruction) {
    kefir_asmcmp_virtual_register_index_t arg_vreg, tmp_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));

    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter_requirements reqs;
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(&function->abi_function_declaration, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_requirements(parameters, &reqs));

    REQUIRE_OK(
        kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                               &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT),
                               &KEFIR_ASMCMP_MAKE_UINT(KEFIR_AMD64_ABI_QWORD * reqs.general_purpose_regs), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg_vreg, 4, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT),
        &KEFIR_ASMCMP_MAKE_UINT(KEFIR_AMD64_ABI_QWORD * kefir_abi_amd64_num_of_general_purpose_parameter_registers(
                                                            KEFIR_ABI_AMD64_VARIANT_SYSTEM_V) +
                                2 * KEFIR_AMD64_ABI_QWORD * reqs.sse_regs),
        NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_lea(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(KEFIR_AMD64_XASMGEN_REGISTER_RBP, 2 * KEFIR_AMD64_ABI_QWORD + reqs.stack,
                                             KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                               &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg_vreg, 8, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                               &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_lea(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
                                      &KEFIR_ASMCMP_MAKE_INDIRECT_VARARG(0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                      NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                               &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg_vreg, 16, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                               &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(vararg_start)(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64_function *function,
                                                                  const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    switch (function->codegen->abi_variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            REQUIRE_OK(vararg_start_impl(mem, function, instruction));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(vararg_end)(struct kefir_mem *mem,
                                                                struct kefir_codegen_amd64_function *function,
                                                                const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    switch (function->codegen->abi_variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}

static kefir_result_t vararg_copy_impl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                       const struct kefir_opt_instruction *instruction) {
    kefir_asmcmp_virtual_register_index_t source_vreg, target_vreg, tmp_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &source_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &target_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(source_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                               &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                               &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(source_vreg, 8, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                               &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, 8, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                               &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(source_vreg, 16, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                               &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, 16, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                               &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(vararg_copy)(struct kefir_mem *mem,
                                                                 struct kefir_codegen_amd64_function *function,
                                                                 const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    switch (function->codegen->abi_variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            REQUIRE_OK(vararg_copy_impl(mem, function, instruction));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}
