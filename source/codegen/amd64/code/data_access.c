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
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(get_local)(struct kefir_mem *mem,
                                                               struct kefir_codegen_amd64_function *function,
                                                               const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE(function->function->locals.type != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid IR function local type"));

    kefir_asmcmp_virtual_register_index_t vreg, tmp_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &vreg));

    const struct kefir_abi_amd64_typeentry_layout *entry = NULL;
    REQUIRE_OK(kefir_abi_amd64_type_layout_at(&function->locals_layout,
                                              instruction->operation.parameters.variable.local_index, &entry));

    const kefir_int64_t offset = entry->relative_offset + instruction->operation.parameters.variable.offset;
    if (offset >= KEFIR_INT16_MIN && offset <= KEFIR_INT16_MAX) {
        REQUIRE_OK(kefir_asmcmp_amd64_lea(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_VAR(offset, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    } else if (offset >= KEFIR_INT32_MIN && offset <= KEFIR_INT32_MAX) {
        REQUIRE_OK(kefir_asmcmp_amd64_lea(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_VAR(0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
        if (offset != 0) {
            REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(vreg), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
        }
    } else {
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));

        REQUIRE_OK(kefir_asmcmp_amd64_lea(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_LOCAL_VAR(0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
        REQUIRE_OK(
            kefir_asmcmp_amd64_movabs(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
        REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_VREG64(vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
    }

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

    if (!function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_amd64_lea(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(symbol, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    } else if (!kefir_ir_module_has_external(function->module->ir_module, symbol) &&
               !kefir_ir_module_has_global(function->module->ir_module, symbol)) {
        REQUIRE_OK(kefir_asmcmp_amd64_lea(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(vreg),
            &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(symbol, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    } else {
        char buf[256];
        snprintf(buf, sizeof(buf), KEFIR_AMD64_GOTPCREL, symbol);

        symbol = kefir_string_pool_insert(mem, &function->code.context.strings, buf, NULL);
        REQUIRE(symbol != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert symbol into string pool"));

        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(vreg),
            &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(symbol, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), NULL));
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
