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

#include "kefir/codegen/opt-system-v-amd64/code_impl.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

DEFINE_TRANSLATOR(long_double_binary_op) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg1_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg2_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *storage_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &arg1_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[1], &arg2_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[2], &storage_allocation));

    struct kefir_codegen_opt_sysv_amd64_storage_register arg_reg;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        mem, &codegen->xasmgen, &codegen_func->storage, arg2_allocation, &arg_reg, NULL, NULL));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                arg2_allocation, arg_reg.reg));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FLD(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_pointer(
            &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_TBYTE,
            kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                     kefir_asm_amd64_xasmgen_operand_reg(arg_reg.reg), 0))));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &arg_reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        mem, &codegen->xasmgen, &codegen_func->storage, arg1_allocation, &arg_reg, NULL, NULL));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                arg1_allocation, arg_reg.reg));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FLD(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_pointer(
            &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_TBYTE,
            kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                     kefir_asm_amd64_xasmgen_operand_reg(arg_reg.reg), 0))));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &arg_reg));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_ADD:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FADDP(&codegen->xasmgen));
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_SUB:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FSUBP(&codegen->xasmgen));
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_MUL:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FMULP(&codegen->xasmgen));
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_DIV:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FDIVP(&codegen->xasmgen));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    struct kefir_codegen_opt_sysv_amd64_storage_register result_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        mem, &codegen->xasmgen, &codegen_func->storage, result_allocation, &result_reg, NULL, NULL));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                storage_allocation, result_reg.reg));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FSTP(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_pointer(
            &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_TBYTE,
            kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                     kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg), 0))));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                 result_allocation, result_reg.reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &result_reg));
    return KEFIR_OK;
}

DEFINE_TRANSLATOR(long_double_unary_op) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg1_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *storage_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &arg1_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[1], &storage_allocation));

    struct kefir_codegen_opt_sysv_amd64_storage_register arg_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        mem, &codegen->xasmgen, &codegen_func->storage, arg1_allocation, &arg_reg, NULL, NULL));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                arg1_allocation, arg_reg.reg));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FLD(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_pointer(
            &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_TBYTE,
            kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                     kefir_asm_amd64_xasmgen_operand_reg(arg_reg.reg), 0))));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &arg_reg));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_NEG:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FCHS(&codegen->xasmgen));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    struct kefir_codegen_opt_sysv_amd64_storage_register result_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        mem, &codegen->xasmgen, &codegen_func->storage, result_allocation, &result_reg, NULL, NULL));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                storage_allocation, result_reg.reg));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FSTP(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_pointer(
            &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_TBYTE,
            kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                     kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg), 0))));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                 result_allocation, result_reg.reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &result_reg));
    return KEFIR_OK;
}

DEFINE_TRANSLATOR(long_double_store) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *source_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *target_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.memory_access.value, &source_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.memory_access.location, &target_allocation));

    struct kefir_codegen_opt_sysv_amd64_storage_register source_reg;
    struct kefir_codegen_opt_sysv_amd64_storage_register target_reg;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        mem, &codegen->xasmgen, &codegen_func->storage, source_allocation, &source_reg,
        kefir_codegen_opt_sysv_amd64_filter_regs_allocation,
        (const struct kefir_codegen_opt_sysv_amd64_register_allocation *[]){target_allocation, NULL}));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        mem, &codegen->xasmgen, &codegen_func->storage, target_allocation, &target_reg, NULL, NULL));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                source_allocation, source_reg.reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                target_allocation, target_reg.reg));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FLD(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_pointer(
            &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_TBYTE,
            kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                     kefir_asm_amd64_xasmgen_operand_reg(source_reg.reg), 0))));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FSTP(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_pointer(
            &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_TBYTE,
            kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                     kefir_asm_amd64_xasmgen_operand_reg(target_reg.reg), 0))));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &target_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &source_reg));

    return KEFIR_OK;
}