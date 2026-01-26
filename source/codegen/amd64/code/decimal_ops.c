/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(decimal_const)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));

    kefir_asmcmp_label_index_t label;
    REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label));

    REQUIRE_OK(kefir_hashtree_insert(mem, &function->constants, (kefir_hashtree_key_t) label,
                                     (kefir_hashtree_value_t) instruction->id));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_DECIMAL32_CONST:
            if (function->codegen->config->position_independent_code) {
                REQUIRE_OK(kefir_asmcmp_amd64_movd(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                    &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_INTERNAL(label, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_movd(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                    &KEFIR_ASMCMP_MAKE_INDIRECT_INTERNAL_LABEL(label, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
            }
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_CONST:
            if (function->codegen->config->position_independent_code) {
                REQUIRE_OK(kefir_asmcmp_amd64_movq(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                    &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_INTERNAL(label, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_movq(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                    &KEFIR_ASMCMP_MAKE_INDIRECT_INTERNAL_LABEL(label, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
            }
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_CONST:
            if (function->codegen->config->position_independent_code) {
                REQUIRE_OK(kefir_asmcmp_amd64_movdqu(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                    &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_INTERNAL(label, KEFIR_ASMCMP_OPERAND_VARIANT_128BIT), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_movdqu(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                    &KEFIR_ASMCMP_MAKE_INDIRECT_INTERNAL_LABEL(label, KEFIR_ASMCMP_OPERAND_VARIANT_128BIT), NULL));
            }
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpectedoptimizr instruction opcode");
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(decimal_load)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg, location_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &location_vreg));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_DECIMAL32_LOAD:
            REQUIRE_OK(kefir_asmcmp_amd64_movd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_LOAD:
            REQUIRE_OK(kefir_asmcmp_amd64_movq(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_LOAD:
            REQUIRE_OK(kefir_asmcmp_amd64_movdqu(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_128BIT), NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpectedoptimizr instruction opcode");
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(decimal_store)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t location_vreg, value_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &location_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &value_vreg));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_DECIMAL32_STORE:
            REQUIRE_OK(kefir_asmcmp_amd64_movd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                &KEFIR_ASMCMP_MAKE_VREG(value_vreg), NULL));
            break;

        case KEFIR_OPT_OPCODE_DECIMAL64_STORE:
            REQUIRE_OK(kefir_asmcmp_amd64_movq(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                &KEFIR_ASMCMP_MAKE_VREG(value_vreg), NULL));
            break;

        case KEFIR_OPT_OPCODE_DECIMAL128_STORE:
            REQUIRE_OK(kefir_asmcmp_amd64_movdqu(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
                &KEFIR_ASMCMP_MAKE_VREG(value_vreg), NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpectedoptimizr instruction opcode");
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(decimal_neg32)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg, tmp_vreg, arg_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        tmp_vreg, arg_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_add(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INT(-2147483648), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_vreg, tmp_vreg, NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(decimal_neg64)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg, tmp_vreg, arg_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        tmp_vreg, arg_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_btc(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INT(63), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        result_vreg, tmp_vreg, NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(decimal_neg128)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg, tmp_vreg, tmp2_vreg, arg_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(mem, &function->code.context, 2, 2, &tmp2_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_produce_virtual_register(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        tmp2_vreg, NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movdqu(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
        &KEFIR_ASMCMP_MAKE_VREG(arg_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp2_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_btc(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INT(63), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp2_vreg, KEFIR_AMD64_ABI_QWORD, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movdqu(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp2_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_128BIT), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}
