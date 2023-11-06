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
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_const)(struct kefir_mem *mem,
                                                               struct kefir_codegen_amd64_function *function,
                                                               const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    if (instruction->operation.parameters.imm.integer >= KEFIR_INT32_MIN &&
        instruction->operation.parameters.imm.integer <= KEFIR_INT32_MAX) {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
                                          &KEFIR_ASMCMP_MAKE_INT(instruction->operation.parameters.imm.integer), NULL));
    } else {
        REQUIRE_OK(
            kefir_asmcmp_amd64_movabs(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
                                      &KEFIR_ASMCMP_MAKE_INT(instruction->operation.parameters.imm.integer), NULL));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(uint_const)(struct kefir_mem *mem,
                                                                struct kefir_codegen_amd64_function *function,
                                                                const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    if (instruction->operation.parameters.imm.uinteger <= (kefir_uint64_t) KEFIR_INT32_MAX) {
        REQUIRE_OK(
            kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                   &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
                                   &KEFIR_ASMCMP_MAKE_UINT(instruction->operation.parameters.imm.uinteger), NULL));
    } else {
        REQUIRE_OK(
            kefir_asmcmp_amd64_movabs(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
                                      &KEFIR_ASMCMP_MAKE_UINT(instruction->operation.parameters.imm.uinteger), NULL));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(string_ref)(struct kefir_mem *mem,
                                                                struct kefir_codegen_amd64_function *function,
                                                                const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    char buf[256];
    snprintf(buf, sizeof(buf), KEFIR_AMD64_STRING_LITERAL, instruction->operation.parameters.imm.string_ref);
    const char *string_ref = kefir_string_pool_insert(mem, &function->code.context.strings, buf, NULL);
    REQUIRE(string_ref != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert string reference into string pool"));

    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_amd64_lea(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
            &KEFIR_ASMCMP_MAKE_RIP_INDIRECT(string_ref, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_lea(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_LABEL(string_ref, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(block_label)(struct kefir_mem *mem,
                                                                 struct kefir_codegen_amd64_function *function,
                                                                 const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    struct kefir_hashtree_node *label_node;
    REQUIRE_OK(kefir_hashtree_at(&function->labels,
                                 (kefir_hashtree_key_t) instruction->operation.parameters.imm.block_ref, &label_node));
    ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, target_label, label_node->value);

    const char *symbol;
    REQUIRE_OK(kefir_codegen_amd64_function_format_label(mem, function, target_label, &symbol));

    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_amd64_lea(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
            &KEFIR_ASMCMP_MAKE_RIP_INDIRECT(symbol, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    } else {
        REQUIRE_OK(
            kefir_asmcmp_amd64_lea(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                   &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_LABEL(symbol), NULL));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(float32_const)(struct kefir_mem *mem,
                                                                   struct kefir_codegen_amd64_function *function,
                                                                   const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));

    kefir_asmcmp_label_index_t label;
    REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label));

    REQUIRE_OK(kefir_hashtree_insert(mem, &function->constants, (kefir_hashtree_key_t) label,
                                     (kefir_hashtree_value_t) instruction->id));

    const char *symbolic_label;
    REQUIRE_OK(kefir_codegen_amd64_function_format_label(mem, function, label, &symbolic_label));

    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_amd64_movd(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
            &KEFIR_ASMCMP_MAKE_RIP_INDIRECT(symbolic_label, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_movd(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_LABEL(symbolic_label, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(float64_const)(struct kefir_mem *mem,
                                                                   struct kefir_codegen_amd64_function *function,
                                                                   const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));

    kefir_asmcmp_label_index_t label;
    REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label));

    REQUIRE_OK(kefir_hashtree_insert(mem, &function->constants, (kefir_hashtree_key_t) label,
                                     (kefir_hashtree_value_t) instruction->id));

    const char *symbolic_label;
    REQUIRE_OK(kefir_codegen_amd64_function_format_label(mem, function, label, &symbolic_label));

    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_amd64_movq(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
            &KEFIR_ASMCMP_MAKE_RIP_INDIRECT(symbolic_label, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_movq(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_LABEL(symbolic_label, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(long_double_const)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.imm.long_double.storage,
                                                    &result_vreg));

    kefir_asmcmp_label_index_t label;
    REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label));

    REQUIRE_OK(kefir_hashtree_insert(mem, &function->constants, (kefir_hashtree_key_t) label,
                                     (kefir_hashtree_value_t) instruction->id));

    const char *symbolic_label;
    REQUIRE_OK(kefir_codegen_amd64_function_format_label(mem, function, label, &symbolic_label));

    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_amd64_fld(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_RIP_INDIRECT(symbolic_label, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_fld(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_INDIRECT_LABEL(symbolic_label, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}
