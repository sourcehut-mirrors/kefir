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
#include "kefir/codegen/opt-system-v-amd64/runtime.h"
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

DEFINE_TRANSLATOR(long_double_comparison) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg1_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg2_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &arg1_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[1], &arg2_allocation));

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

    struct kefir_codegen_opt_sysv_amd64_storage_register result_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        mem, &codegen->xasmgen, &codegen_func->storage, result_allocation, &result_reg, NULL, NULL));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_XOR(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                             kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg)));

    kefir_asm_amd64_xasmgen_register_t result_reg_variant;
    REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(result_reg.reg, &result_reg_variant));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_EQUALS: {
            struct kefir_codegen_opt_sysv_amd64_storage_register tmp_reg;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_any_general_purpose_register(
                mem, &codegen->xasmgen, &codegen_func->storage, &tmp_reg, NULL, NULL));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_XOR(&codegen->xasmgen,
                                                     kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg),
                                                     kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg)));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FUCOMIP(&codegen->xasmgen));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FSTP(
                &codegen->xasmgen,
                kefir_asm_amd64_xasmgen_operand_fpu_register(&codegen->xasmgen_helpers.operands[0], 0)));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SETNP(&codegen->xasmgen,
                                                       kefir_asm_amd64_xasmgen_operand_reg(result_reg_variant)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CMOVNE(&codegen->xasmgen,
                                                        kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                        kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg)));

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen,
                                                                             &codegen_func->storage, &tmp_reg));
        } break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_GREATER:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FCOMIP(&codegen->xasmgen));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FSTP(
                &codegen->xasmgen,
                kefir_asm_amd64_xasmgen_operand_fpu_register(&codegen->xasmgen_helpers.operands[0], 0)));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SETA(&codegen->xasmgen,
                                                      kefir_asm_amd64_xasmgen_operand_reg(result_reg_variant)));
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_LESSER:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FSTP(
                &codegen->xasmgen,
                kefir_asm_amd64_xasmgen_operand_fpu_register(&codegen->xasmgen_helpers.operands[0], 2)));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FCOMIP(&codegen->xasmgen));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FSTP(
                &codegen->xasmgen,
                kefir_asm_amd64_xasmgen_operand_fpu_register(&codegen->xasmgen_helpers.operands[0], 0)));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SETA(&codegen->xasmgen,
                                                      kefir_asm_amd64_xasmgen_operand_reg(result_reg_variant)));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                 result_allocation, result_reg.reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &result_reg));
    return KEFIR_OK;
}

DEFINE_TRANSLATOR(long_double_conversion_to) {
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

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT_TO_LONG_DOUBLE:
            if (arg1_allocation->result.type ==
                KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(
                    &codegen->xasmgen,
                    kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                        &codegen->xasmgen_helpers.operands[0], &codegen_func->stack_frame_map, arg1_allocation)));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FILD(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_pointer(
                        &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_QWORD,
                        kefir_asm_amd64_xasmgen_operand_indirect(
                            &codegen->xasmgen_helpers.operands[1],
                            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0))));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                    kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                        KEFIR_AMD64_SYSV_ABI_QWORD)));
            } else {
                REQUIRE(arg1_allocation->result.type !=
                            KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected integral register allocation"));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FILD(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_pointer(
                        &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_QWORD,
                        kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                            &codegen->xasmgen_helpers.operands[0], &codegen_func->stack_frame_map, arg1_allocation))));
            }
            break;

        case KEFIR_OPT_OPCODE_UINT_TO_LONG_DOUBLE: {
            struct kefir_codegen_opt_sysv_amd64_storage_register arg_reg;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
                mem, &codegen->xasmgen, &codegen_func->storage, arg1_allocation, &arg_reg,
                kefir_codegen_opt_sysv_amd64_filter_regs_allocation,
                (const struct kefir_codegen_opt_sysv_amd64_register_allocation *[]){storage_allocation, NULL}));

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                        arg1_allocation, arg_reg.reg));

            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(arg_reg.reg)));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FILD(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_pointer(
                                       &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_QWORD,
                                       kefir_asm_amd64_xasmgen_operand_indirect(
                                           &codegen->xasmgen_helpers.operands[1],
                                           kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0))));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                    KEFIR_AMD64_SYSV_ABI_QWORD)));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_TEST(&codegen->xasmgen,
                                                      kefir_asm_amd64_xasmgen_operand_reg(arg_reg.reg),
                                                      kefir_asm_amd64_xasmgen_operand_reg(arg_reg.reg)));

            kefir_id_t unsigned_label = codegen_func->nonblock_labels++;
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JNS(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_label(
                                       &codegen->xasmgen_helpers.operands[0],
                                       kefir_asm_amd64_xasmgen_helpers_format(
                                           &codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL,
                                           function->ir_func->name, instr->block_id, unsigned_label))));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FADD(
                &codegen->xasmgen,
                kefir_asm_amd64_xasmgen_operand_pointer(
                    &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_FP_SINGLE,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[1],
                        kefir_asm_amd64_xasmgen_operand_label(
                            &codegen->xasmgen_helpers.operands[2],
                            kefir_asm_amd64_xasmgen_helpers_format(
                                &codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_CONSTANT_UINT_TO_LD,
                                function->ir_func->name)),
                        0))));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL,
                                                 function->ir_func->name, instr->block_id, unsigned_label));

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen,
                                                                             &codegen_func->storage, &arg_reg));
        } break;

        case KEFIR_OPT_OPCODE_FLOAT32_TO_LONG_DOUBLE:
            if (arg1_allocation->result.type ==
                KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER) {
                kefir_asm_amd64_xasmgen_register_t arg_reg_variant;
                REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(arg1_allocation->result.reg, &arg_reg_variant));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                    kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                        KEFIR_AMD64_SYSV_ABI_QWORD / 2)));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0),
                    kefir_asm_amd64_xasmgen_operand_reg(arg_reg_variant)));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FLD(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_pointer(
                        &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_FP_SINGLE,
                        kefir_asm_amd64_xasmgen_operand_indirect(
                            &codegen->xasmgen_helpers.operands[1],
                            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0))));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                    kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                        KEFIR_AMD64_SYSV_ABI_QWORD / 2)));
            } else if (arg1_allocation->result.type ==
                       KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                    kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                        KEFIR_AMD64_SYSV_ABI_QWORD / 2)));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVD(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0),
                    kefir_asm_amd64_xasmgen_operand_reg(arg1_allocation->result.reg)));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FLD(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_pointer(
                        &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_FP_SINGLE,
                        kefir_asm_amd64_xasmgen_operand_indirect(
                            &codegen->xasmgen_helpers.operands[1],
                            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0))));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                    kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                        KEFIR_AMD64_SYSV_ABI_QWORD / 2)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FLD(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_pointer(
                        &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_FP_SINGLE,
                        kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                            &codegen->xasmgen_helpers.operands[0], &codegen_func->stack_frame_map, arg1_allocation))));
            }
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_TO_LONG_DOUBLE:
            if (arg1_allocation->result.type ==
                KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(arg1_allocation->result.reg)));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FLD(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_pointer(
                        &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_FP_DOUBLE,
                        kefir_asm_amd64_xasmgen_operand_indirect(
                            &codegen->xasmgen_helpers.operands[1],
                            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0))));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                    kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                        KEFIR_AMD64_SYSV_ABI_QWORD)));
            } else if (arg1_allocation->result.type ==
                       KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                    kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                        KEFIR_AMD64_SYSV_ABI_QWORD)));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0),
                    kefir_asm_amd64_xasmgen_operand_reg(arg1_allocation->result.reg)));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FLD(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_pointer(
                        &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_FP_DOUBLE,
                        kefir_asm_amd64_xasmgen_operand_indirect(
                            &codegen->xasmgen_helpers.operands[1],
                            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), 0))));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                    kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                        KEFIR_AMD64_SYSV_ABI_QWORD)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FLD(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_pointer(
                        &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_FP_DOUBLE,
                        kefir_codegen_opt_sysv_amd64_reg_allocation_operand(
                            &codegen->xasmgen_helpers.operands[0], &codegen_func->stack_frame_map, arg1_allocation))));
            }
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

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &result_reg));
    return KEFIR_OK;
}

DEFINE_TRANSLATOR(long_double_conversion_from) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &arg_allocation));

    struct kefir_codegen_opt_sysv_amd64_storage_register arg_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        mem, &codegen->xasmgen, &codegen_func->storage, arg_allocation, &arg_reg,
        kefir_codegen_opt_sysv_amd64_filter_regs_allocation,
        (const struct kefir_codegen_opt_sysv_amd64_register_allocation *[]){arg_allocation, NULL}));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map, arg_allocation,
                                                                arg_reg.reg));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FLD(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_pointer(
            &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_TBYTE,
            kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                     kefir_asm_amd64_xasmgen_operand_reg(arg_reg.reg), 0))));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &arg_reg));

    struct kefir_codegen_opt_sysv_amd64_storage_register result_reg;

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_INT:
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
                mem, &codegen->xasmgen, &codegen_func->storage, result_allocation, KEFIR_AMD64_XASMGEN_REGISTER_RAX,
                &result_reg));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
                &codegen->xasmgen,
                kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0],
                                                      KEFIR_OPT_AMD64_SYSTEM_V_RUNTIME_LONG_DOUBLE_TO_INT)));
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_UINT:
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
                mem, &codegen->xasmgen, &codegen_func->storage, result_allocation, KEFIR_AMD64_XASMGEN_REGISTER_RAX,
                &result_reg));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
                &codegen->xasmgen,
                kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0],
                                                      KEFIR_OPT_AMD64_SYSTEM_V_RUNTIME_LONG_DOUBLE_TO_UINT)));
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_TRUNCATE_1BIT:
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
                mem, &codegen->xasmgen, &codegen_func->storage, result_allocation, KEFIR_AMD64_XASMGEN_REGISTER_RAX,
                &result_reg));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
                &codegen->xasmgen,
                kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0],
                                                      KEFIR_OPT_AMD64_SYSTEM_V_RUNTIME_LONG_DOUBLE_TRUNC_1BIT)));
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_FLOAT32:
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_floating_point_allocated_register(
                mem, &codegen->xasmgen, &codegen_func->storage, result_allocation, &result_reg, NULL, NULL));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FSTP(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_pointer(
                                       &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_FP_SINGLE,
                                       kefir_asm_amd64_xasmgen_operand_indirect(
                                           &codegen->xasmgen_helpers.operands[1],
                                           kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                           -KEFIR_AMD64_SYSV_ABI_QWORD / 2))));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVD(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &codegen->xasmgen_helpers.operands[1],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                    -KEFIR_AMD64_SYSV_ABI_QWORD / 2)));
            break;

        case KEFIR_OPT_OPCODE_LONG_DOUBLE_TO_FLOAT64:
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_floating_point_allocated_register(
                mem, &codegen->xasmgen, &codegen_func->storage, result_allocation, &result_reg, NULL, NULL));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FSTP(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_pointer(
                                       &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_FP_DOUBLE,
                                       kefir_asm_amd64_xasmgen_operand_indirect(
                                           &codegen->xasmgen_helpers.operands[1],
                                           kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                           -KEFIR_AMD64_SYSV_ABI_QWORD))));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &codegen->xasmgen_helpers.operands[1],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                    -KEFIR_AMD64_SYSV_ABI_QWORD)));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                 result_allocation, result_reg.reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &result_reg));

    return KEFIR_OK;
}
