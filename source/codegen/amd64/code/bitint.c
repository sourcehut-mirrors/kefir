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
#include "kefir/target/abi/amd64/return.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

#define BIGINT_GET_SET_SIGNED_INTEGER_FN "__kefir_bigint_set_signed_integer"
#define BIGINT_GET_SET_UNSIGNED_INTEGER_FN "__kefir_bigint_set_unsigned_integer"
#define BIGINT_CAST_SIGNED_FN "__kefir_bigint_cast_signed"
#define BIGINT_CAST_UNSIGNED_FN "__kefir_bigint_cast_unsigned"

#define QWORD_BITS (KEFIR_AMD64_ABI_QWORD * 8)

static kefir_result_t preserve_regs(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                    kefir_asmcmp_stash_index_t *stash_idx) {
    const kefir_size_t num_of_preserved_gp_regs =
        kefir_abi_amd64_num_of_caller_preserved_general_purpose_registers(function->codegen->abi_variant);
    const kefir_size_t num_of_preserved_sse_regs =
        kefir_abi_amd64_num_of_caller_preserved_sse_registers(function->codegen->abi_variant);

    REQUIRE_OK(kefir_asmcmp_register_stash_new(mem, &function->code.context, stash_idx));

    for (kefir_size_t i = 0; i < num_of_preserved_gp_regs; i++) {
        kefir_asm_amd64_xasmgen_register_t reg;
        REQUIRE_OK(
            kefir_abi_amd64_get_caller_preserved_general_purpose_register(function->codegen->abi_variant, i, &reg));

        REQUIRE_OK(kefir_asmcmp_register_stash_add(mem, &function->code.context, *stash_idx,
                                                   (kefir_asmcmp_physical_register_index_t) reg));
    }

    for (kefir_size_t i = 0; i < num_of_preserved_sse_regs; i++) {
        kefir_asm_amd64_xasmgen_register_t reg;
        REQUIRE_OK(kefir_abi_amd64_get_caller_preserved_sse_register(function->codegen->abi_variant, i, &reg));

        REQUIRE_OK(kefir_asmcmp_register_stash_add(mem, &function->code.context, *stash_idx,
                                                   (kefir_asmcmp_physical_register_index_t) reg));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_activate_stash(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), *stash_idx, NULL));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(bitint_const)(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64_function *function,
                                                                  const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    const struct kefir_bigint *bigint;
    REQUIRE_OK(kefir_ir_module_get_bigint(function->module->ir_module, instruction->operation.parameters.imm.bitint_ref,
                                          &bigint));

    kefir_asmcmp_virtual_register_index_t result_vreg;
    if (bigint->bitwidth <= QWORD_BITS) {
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

        if (instruction->operation.opcode == KEFIR_OPT_OPCODE_BITINT_SIGNED_CONST) {
            kefir_int64_t value;
            REQUIRE_OK(kefir_bigint_get_signed(bigint, &value));
            if (value >= KEFIR_INT32_MIN && value <= KEFIR_INT32_MAX) {
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_INT(value), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_INT(value), NULL));
            }
        } else {
            kefir_uint64_t value;
            REQUIRE_OK(kefir_bigint_get_unsigned(bigint, &value));
            if (value <= KEFIR_INT32_MAX) {
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_INT(value), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_INT(value), NULL));
            }
        }
    } else {
        const kefir_size_t qwords = (bigint->bitwidth + QWORD_BITS - 1) / QWORD_BITS;
        REQUIRE_OK(
            kefir_asmcmp_virtual_register_new_spill_space(mem, &function->code.context, qwords, 1, &result_vreg));

        for (kefir_size_t i = 0; i < qwords; i++) {
            kefir_uint64_t part;
            REQUIRE_OK(kefir_bigint_get_bits(bigint, i * QWORD_BITS, QWORD_BITS / 2, &part));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, i * KEFIR_AMD64_ABI_QWORD,
                                                                           KEFIR_ASMCMP_OPERAND_VARIANT_32BIT),
                                       &KEFIR_ASMCMP_MAKE_INT(part), NULL));

            REQUIRE_OK(kefir_bigint_get_bits(bigint, i * QWORD_BITS + QWORD_BITS / 2, QWORD_BITS / 2, &part));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, i * KEFIR_AMD64_ABI_QWORD + KEFIR_AMD64_ABI_QWORD / 2,
                                                    KEFIR_ASMCMP_OPERAND_VARIANT_32BIT),
                &KEFIR_ASMCMP_MAKE_INT(part), NULL));
        }
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(bitint_get_signed)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));

    if (instruction->operation.parameters.bitwidth <= QWORD_BITS) {
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             result_vreg, arg_vreg, NULL));

        REQUIRE_OK(kefir_asmcmp_amd64_shl(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
            &KEFIR_ASMCMP_MAKE_UINT(QWORD_BITS - instruction->operation.parameters.bitwidth), NULL));

        REQUIRE_OK(kefir_asmcmp_amd64_sar(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
            &KEFIR_ASMCMP_MAKE_UINT(QWORD_BITS - instruction->operation.parameters.bitwidth), NULL));

    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(bitint_get_unsigned)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));

    if (instruction->operation.parameters.bitwidth <= QWORD_BITS) {
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             result_vreg, arg_vreg, NULL));

        REQUIRE_OK(kefir_asmcmp_amd64_shl(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
            &KEFIR_ASMCMP_MAKE_UINT(QWORD_BITS - instruction->operation.parameters.bitwidth), NULL));

        REQUIRE_OK(kefir_asmcmp_amd64_shr(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
            &KEFIR_ASMCMP_MAKE_UINT(QWORD_BITS - instruction->operation.parameters.bitwidth), NULL));

    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t bigint_from_impl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                       const struct kefir_opt_instruction *instruction,
                                       kefir_bool_t signed_construction) {
    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));

    if (instruction->operation.parameters.bitwidth <= QWORD_BITS) {
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             result_vreg, arg_vreg, NULL));
        REQUIRE_OK(kefir_asmcmp_amd64_shl(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
            &KEFIR_ASMCMP_MAKE_UINT(QWORD_BITS - instruction->operation.parameters.bitwidth), NULL));

        if (signed_construction) {
            REQUIRE_OK(kefir_asmcmp_amd64_sar(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
                &KEFIR_ASMCMP_MAKE_UINT(QWORD_BITS - instruction->operation.parameters.bitwidth), NULL));
        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_sar(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
                &KEFIR_ASMCMP_MAKE_UINT(QWORD_BITS - instruction->operation.parameters.bitwidth), NULL));
        }
    } else {
        const char *fn_name =
            signed_construction ? BIGINT_GET_SET_SIGNED_INTEGER_FN : BIGINT_GET_SET_UNSIGNED_INTEGER_FN;
        REQUIRE_OK(kefir_codegen_amd64_module_require_runtime(mem, function->codegen_module, fn_name));

        kefir_asmcmp_stash_index_t stash_idx;
        REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));
        REQUIRE_OK(preserve_regs(mem, function, &stash_idx));

        const kefir_size_t qwords = (instruction->operation.parameters.bitwidth + QWORD_BITS - 1) / QWORD_BITS;
        REQUIRE_OK(
            kefir_asmcmp_virtual_register_new_spill_space(mem, &function->code.context, qwords, 1, &result_vreg));

        kefir_asm_amd64_xasmgen_register_t bigint_placement_phreg, bitwidth_placement_phreg, arg_placement_phreg;
        REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 0,
                                                                      &bigint_placement_phreg));
        REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 1,
                                                                      &bitwidth_placement_phreg));
        REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 2,
                                                                      &arg_placement_phreg));

        kefir_asmcmp_virtual_register_index_t bigint_placement_vreg, bitwidth_placement_vreg, arg_placement_vreg;
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &bigint_placement_vreg));
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &bitwidth_placement_vreg));
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg_placement_vreg));

        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, bigint_placement_vreg,
                                                                      bigint_placement_phreg));
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, bitwidth_placement_vreg,
                                                                      bitwidth_placement_phreg));
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, arg_placement_vreg,
                                                                      arg_placement_phreg));

        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             bigint_placement_vreg, result_vreg, NULL));
        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_VREG(bitwidth_placement_vreg),
                                          &KEFIR_ASMCMP_MAKE_UINT(instruction->operation.parameters.bitwidth), NULL));
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             arg_placement_vreg, arg_vreg, NULL));

        kefir_asmcmp_instruction_index_t call_idx;
        REQUIRE_OK(kefir_asmcmp_amd64_call(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE, fn_name, 0), &call_idx));
        REQUIRE_OK(kefir_asmcmp_register_stash_set_liveness_index(&function->code.context, stash_idx, call_idx));

        REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg, NULL));

        REQUIRE_OK(kefir_asmcmp_amd64_deactivate_stash(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), stash_idx, NULL));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(bitint_from_signed)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(bigint_from_impl(mem, function, instruction, true));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(bitint_from_unsigned)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(bigint_from_impl(mem, function, instruction, false));
    return KEFIR_OK;
}

static kefir_result_t bigint_cast_impl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                       const struct kefir_opt_instruction *instruction, kefir_bool_t signed_cast) {
    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));

    if (instruction->operation.parameters.bitwidth <= QWORD_BITS &&
        instruction->operation.parameters.src_bitwidth <= QWORD_BITS) {
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             result_vreg, arg_vreg, NULL));

        REQUIRE_OK(kefir_asmcmp_amd64_shl(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
            &KEFIR_ASMCMP_MAKE_UINT(QWORD_BITS - instruction->operation.parameters.bitwidth), NULL));

        if (signed_cast) {
            REQUIRE_OK(kefir_asmcmp_amd64_sar(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
                &KEFIR_ASMCMP_MAKE_UINT(QWORD_BITS - instruction->operation.parameters.bitwidth), NULL));
        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_shr(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
                &KEFIR_ASMCMP_MAKE_UINT(QWORD_BITS - instruction->operation.parameters.bitwidth), NULL));
        }
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    } else if (instruction->operation.parameters.bitwidth <= QWORD_BITS) {
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));

        REQUIRE_OK(kefir_asmcmp_amd64_shl(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
            &KEFIR_ASMCMP_MAKE_UINT(QWORD_BITS - instruction->operation.parameters.bitwidth), NULL));

        if (signed_cast) {
            REQUIRE_OK(kefir_asmcmp_amd64_sar(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
                &KEFIR_ASMCMP_MAKE_UINT(QWORD_BITS - instruction->operation.parameters.bitwidth), NULL));
        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_shr(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
                &KEFIR_ASMCMP_MAKE_UINT(QWORD_BITS - instruction->operation.parameters.bitwidth), NULL));
        }
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    } else if (instruction->operation.parameters.src_bitwidth <= QWORD_BITS) {
        REQUIRE_OK(bigint_from_impl(mem, function, instruction, signed_cast));
    } else {
        const char *fn_name = signed_cast ? BIGINT_CAST_SIGNED_FN : BIGINT_CAST_UNSIGNED_FN;
        REQUIRE_OK(kefir_codegen_amd64_module_require_runtime(mem, function->codegen_module, fn_name));

        kefir_asmcmp_stash_index_t stash_idx;
        REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));
        REQUIRE_OK(preserve_regs(mem, function, &stash_idx));

        const kefir_size_t qwords = (instruction->operation.parameters.bitwidth + QWORD_BITS - 1) / QWORD_BITS;
        const kefir_size_t src_qwords = (instruction->operation.parameters.src_bitwidth + QWORD_BITS - 1) / QWORD_BITS;
        REQUIRE_OK(
            kefir_asmcmp_virtual_register_new_spill_space(mem, &function->code.context, qwords, 1, &result_vreg));

        kefir_asmcmp_virtual_register_index_t tmp_vreg;
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
        for (kefir_size_t i = 0; i < MIN(qwords, src_qwords); i++) {
            REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
                                              &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg_vreg, i * KEFIR_AMD64_ABI_QWORD,
                                                                                  KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                              NULL));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, i * KEFIR_AMD64_ABI_QWORD,
                                                                           KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                       &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
        }

        kefir_asm_amd64_xasmgen_register_t bigint_placement_phreg, src_bitwidth_placement_phreg,
            bitwidth_placement_phreg;
        REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 0,
                                                                      &bigint_placement_phreg));
        REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 1,
                                                                      &src_bitwidth_placement_phreg));
        REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 2,
                                                                      &bitwidth_placement_phreg));

        kefir_asmcmp_virtual_register_index_t bigint_placement_vreg, src_bitwidth_placement_vreg,
            bitwidth_placement_vreg;
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &bigint_placement_vreg));
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &src_bitwidth_placement_vreg));
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &bitwidth_placement_vreg));

        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, bigint_placement_vreg,
                                                                      bigint_placement_phreg));
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, src_bitwidth_placement_vreg,
                                                                      src_bitwidth_placement_phreg));
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, bitwidth_placement_vreg,
                                                                      bitwidth_placement_phreg));

        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             bigint_placement_vreg, result_vreg, NULL));
        REQUIRE_OK(
            kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                   &KEFIR_ASMCMP_MAKE_VREG(src_bitwidth_placement_vreg),
                                   &KEFIR_ASMCMP_MAKE_UINT(instruction->operation.parameters.src_bitwidth), NULL));
        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_VREG(bitwidth_placement_vreg),
                                          &KEFIR_ASMCMP_MAKE_UINT(instruction->operation.parameters.bitwidth), NULL));

        kefir_asmcmp_instruction_index_t call_idx;
        REQUIRE_OK(kefir_asmcmp_amd64_call(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE, fn_name, 0), &call_idx));
        REQUIRE_OK(kefir_asmcmp_register_stash_set_liveness_index(&function->code.context, stash_idx, call_idx));
        REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg, NULL));

        REQUIRE_OK(kefir_asmcmp_amd64_deactivate_stash(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), stash_idx, NULL));
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(bitint_cast_signed)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(bigint_cast_impl(mem, function, instruction, true));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(bitint_cast_unsigned)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(bigint_cast_impl(mem, function, instruction, false));
    return KEFIR_OK;
}
