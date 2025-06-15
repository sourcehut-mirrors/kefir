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

#define QWORD_BITS (KEFIR_AMD64_ABI_QWORD * 8)

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(bitint_const)(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64_function *function,
                                                                  const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    const struct kefir_bigint *bigint;
    REQUIRE_OK(kefir_ir_module_get_bigint(function->module->ir_module, instruction->operation.parameters.imm.bitint_ref,
                                          &bigint));

    REQUIRE(bigint->bitwidth > QWORD_BITS,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                            "Expected smaller bitint contstants to be lowered prior to code generation"));

    kefir_asmcmp_virtual_register_index_t result_vreg;
    const kefir_size_t qwords = (bigint->bitwidth + QWORD_BITS - 1) / QWORD_BITS;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(mem, &function->code.context, qwords, 1, &result_vreg));

    for (kefir_size_t i = 0; i < qwords; i++) {
        kefir_uint64_t part;
        REQUIRE_OK(kefir_bigint_get_bits(bigint, i * QWORD_BITS, QWORD_BITS / 2, &part));
        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
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

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(bitint_error_lowered)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                           "Expected bitint optimizer intruction to be lowered prior to code generation");
}

static kefir_result_t small_bitint_load_impl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                             kefir_size_t bitwidth, kefir_opt_memory_load_extension_t load_ext,
                                             kefir_asmcmp_instruction_index_t arg_vreg,
                                             kefir_asmcmp_instruction_index_t *result_vreg_ptr) {
    kefir_asmcmp_virtual_register_index_t result_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    if (bitwidth <= 8) {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG8(result_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    } else if (bitwidth <= 16) {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG16(result_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    } else if (bitwidth <= 32) {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG32(result_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    }

    switch (load_ext) {
        case KEFIR_OPT_MEMORY_LOAD_NOEXTEND:
            // Intentionally left blank
            break;

        case KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND:
            REQUIRE_OK(kefir_asmcmp_amd64_shl(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(QWORD_BITS - bitwidth), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_sar(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(QWORD_BITS - bitwidth), NULL));
            break;

        case KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND:
            REQUIRE_OK(kefir_asmcmp_amd64_shl(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(QWORD_BITS - bitwidth), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_shr(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(QWORD_BITS - bitwidth), NULL));
            break;
    }

    *result_vreg_ptr = result_vreg;
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(bitint_load)(struct kefir_mem *mem,
                                                                 struct kefir_codegen_amd64_function *function,
                                                                 const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));

    if (instruction->operation.parameters.bitwidth <= QWORD_BITS) {
        REQUIRE_OK(small_bitint_load_impl(mem, function, instruction->operation.parameters.bitwidth,
                                          instruction->operation.parameters.bitint_memflags.load_extension, arg_vreg,
                                          &result_vreg));
    } else {
        const kefir_size_t qwords = (instruction->operation.parameters.bitwidth + QWORD_BITS - 1) / QWORD_BITS;
        REQUIRE_OK(
            kefir_asmcmp_virtual_register_new_spill_space(mem, &function->code.context, qwords, 1, &result_vreg));
        REQUIRE_OK(
            kefir_codegen_amd64_copy_memory(mem, function, result_vreg, arg_vreg, qwords * KEFIR_AMD64_ABI_QWORD));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(bitint_store)(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64_function *function,
                                                                  const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t location_vreg, value_vreg;
    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &location_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &value_vreg));

    if (instruction->operation.parameters.bitwidth <= QWORD_BITS) {
        if (instruction->operation.parameters.bitwidth <= 8) {
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                &KEFIR_ASMCMP_MAKE_VREG8(value_vreg), NULL));
        } else if (instruction->operation.parameters.bitwidth <= 16) {
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                &KEFIR_ASMCMP_MAKE_VREG16(value_vreg), NULL));
        } else if (instruction->operation.parameters.bitwidth <= 32) {
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                &KEFIR_ASMCMP_MAKE_VREG32(value_vreg), NULL));
        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                &KEFIR_ASMCMP_MAKE_VREG(value_vreg), NULL));
        }
    } else {
        const kefir_size_t qwords = (instruction->operation.parameters.bitwidth + QWORD_BITS - 1) / QWORD_BITS;
        REQUIRE_OK(
            kefir_codegen_amd64_copy_memory(mem, function, location_vreg, value_vreg, qwords * KEFIR_AMD64_ABI_QWORD));
    }
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(bitint_atomic_load)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));

    if (instruction->operation.parameters.bitwidth <= QWORD_BITS) {
        REQUIRE_OK(small_bitint_load_impl(mem, function, instruction->operation.parameters.bitwidth,
                                          instruction->operation.parameters.bitint_memflags.load_extension, arg_vreg,
                                          &result_vreg));
    } else {
        const kefir_size_t qwords = (instruction->operation.parameters.bitwidth + QWORD_BITS - 1) / QWORD_BITS;
        REQUIRE_OK(
            kefir_asmcmp_virtual_register_new_spill_space(mem, &function->code.context, qwords, 1, &result_vreg));

        kefir_asmcmp_stash_index_t stash_idx;
        REQUIRE_OK(kefir_codegen_amd64_function_call_preserve_regs(mem, function, NULL, &stash_idx, NULL));

        kefir_asmcmp_virtual_register_index_t size_placement_vreg, target_ptr_placement_vreg, source_ptr_vreg,
            source_ptr_placement_vreg, memorder_placement_vreg;
        kefir_asm_amd64_xasmgen_register_t size_phreg, target_ptr_phreg, source_ptr_phreg, memorder_phreg;
        REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 0, &size_phreg));
        REQUIRE_OK(
            kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 1, &source_ptr_phreg));
        REQUIRE_OK(
            kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 2, &target_ptr_phreg));

        kefir_asmcmp_external_label_relocation_t atomic_copy_fn_loction =
            function->codegen->config->position_independent_code ? KEFIR_ASMCMP_EXTERNAL_LABEL_PLT
                                                                 : KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE;

        REQUIRE_OK(
            kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 3, &memorder_phreg));
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &size_placement_vreg));
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &target_ptr_placement_vreg));
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &source_ptr_placement_vreg));
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &memorder_placement_vreg));
        REQUIRE_OK(
            kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, size_placement_vreg, size_phreg));
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, target_ptr_placement_vreg,
                                                                      target_ptr_phreg));
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, source_ptr_placement_vreg,
                                                                      source_ptr_phreg));
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, memorder_placement_vreg,
                                                                      memorder_phreg));

        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0],
                                                        &source_ptr_vreg));
        REQUIRE_OK(kefir_asmcmp_amd64_lea(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(target_ptr_placement_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             source_ptr_placement_vreg, source_ptr_vreg, NULL));

        kefir_int64_t memorder;
        REQUIRE_OK(kefir_codegen_amd64_get_atomic_memorder(instruction->operation.parameters.bitint_atomic_memorder,
                                                           &memorder));

        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_VREG(size_placement_vreg),
                                          &KEFIR_ASMCMP_MAKE_UINT(qwords * KEFIR_AMD64_ABI_QWORD), NULL));
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(memorder_placement_vreg), &KEFIR_ASMCMP_MAKE_INT(memorder), NULL));

        kefir_asmcmp_instruction_index_t call_idx;
        REQUIRE_OK(kefir_asmcmp_amd64_call(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(atomic_copy_fn_loction, LIBATOMIC_LOAD, 0), &call_idx));
        REQUIRE_OK(kefir_asmcmp_register_stash_set_liveness_index(&function->code.context, stash_idx, call_idx));

        REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg, NULL));

        REQUIRE_OK(kefir_asmcmp_amd64_deactivate_stash(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), stash_idx, NULL));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}
