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

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(local_lifetime_mark)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    // Intentionally left blank
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(temporary_object)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg;
    const kefir_size_t size =
        (instruction->operation.parameters.tmp_object.size + KEFIR_AMD64_ABI_QWORD - 1) / KEFIR_AMD64_ABI_QWORD;
    const kefir_size_t alignemnt =
        (instruction->operation.parameters.tmp_object.alignment + KEFIR_AMD64_ABI_QWORD - 1) / KEFIR_AMD64_ABI_QWORD;
    REQUIRE_OK(
        kefir_asmcmp_virtual_register_new_spill_space(mem, &function->code.context, size, alignemnt, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(pair)(struct kefir_mem *mem,
                                                          struct kefir_codegen_amd64_function *function,
                                                          const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg_vreg;
    kefir_result_t res =
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, arg_vreg));
    }
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(alloc_local)(struct kefir_mem *mem,
                                                                 struct kefir_codegen_amd64_function *function,
                                                                 const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_id_t variable_id;
    REQUIRE_OK(kefir_codegen_local_variable_allocator_mark_alive(mem, &function->variable_allocator, instruction->id,
                                                                 &variable_id));

    kefir_asmcmp_virtual_register_index_t vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_local_variable(mem, &function->code.context, variable_id, 0, &vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, vreg));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(ref_local)(struct kefir_mem *mem,
                                                               struct kefir_codegen_amd64_function *function,
                                                               const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_id_t variable_id;
    REQUIRE_OK(kefir_codegen_local_variable_allocator_mark_alive(
        mem, &function->variable_allocator, instruction->operation.parameters.refs[0], &variable_id));

    kefir_asmcmp_virtual_register_index_t vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_local_variable(mem, &function->code.context, variable_id,
                                                                instruction->operation.parameters.offset, &vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, vreg));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(get_global)(struct kefir_mem *mem,
                                                                struct kefir_codegen_amd64_function *function,
                                                                const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t vreg, tmp_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &vreg));

    const char *symbol = kefir_ir_module_get_named_symbol(function->module->ir_module,
                                                          instruction->operation.parameters.variable.global_ref);
    REQUIRE(symbol != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find named IR symbol"));

    const struct kefir_ir_identifier *ir_identifier;
    REQUIRE_OK(kefir_ir_module_get_identifier(function->module->ir_module, symbol, &ir_identifier));

    if (!function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_amd64_lea(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE, ir_identifier->symbol, 0,
                                                       KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
            NULL));
    } else if (ir_identifier->scope == KEFIR_IR_IDENTIFIER_SCOPE_LOCAL) {
        REQUIRE_OK(kefir_asmcmp_amd64_lea(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(vreg),
            &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE, ir_identifier->symbol,
                                                     KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
            NULL));
    } else {
        symbol = kefir_string_pool_insert(mem, &function->code.context.strings, ir_identifier->symbol, NULL);
        REQUIRE(symbol != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert symbol into string pool"));

        REQUIRE_OK(
            kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                   &KEFIR_ASMCMP_MAKE_VREG64(vreg),
                                   &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_ASMCMP_EXTERNAL_LABEL_GOTPCREL,
                                                                            symbol, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                                   NULL));
    }

    const kefir_int64_t offset = instruction->operation.parameters.variable.offset;
    if (offset < KEFIR_INT32_MIN || offset > KEFIR_INT32_MAX) {
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
        REQUIRE_OK(
            kefir_asmcmp_amd64_movabs(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
        REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_VREG64(vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
    } else if (offset != 0) {
        REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_VREG64(vreg), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, vreg));

    return KEFIR_OK;
}
