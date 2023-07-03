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

DEFINE_TRANSLATOR(vararg_start) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &arg_allocation));

    struct kefir_codegen_opt_amd64_sysv_storage_handle arg_handle;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER |
            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_RDONLY,
        arg_allocation, &arg_handle, NULL, NULL));

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(&codegen->xasmgen, &codegen_func->stack_frame_map,
                                                                  arg_allocation, &arg_handle.location));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_pointer(
            &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_DWORD,
            kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                     kefir_asm_amd64_xasmgen_operand_reg(arg_handle.location.reg), 0)),
        kefir_asm_amd64_xasmgen_operand_immu(
            &codegen->xasmgen_helpers.operands[2],
            KEFIR_AMD64_SYSV_ABI_QWORD * codegen_func->declaration.parameters.location.integer_register)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_pointer(
            &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_DWORD,
            kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                     kefir_asm_amd64_xasmgen_operand_reg(arg_handle.location.reg),
                                                     KEFIR_AMD64_SYSV_ABI_QWORD / 2)),
        kefir_asm_amd64_xasmgen_operand_immu(
            &codegen->xasmgen_helpers.operands[2],
            KEFIR_AMD64_SYSV_ABI_QWORD * KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTER_COUNT +
                2 * KEFIR_AMD64_SYSV_ABI_QWORD * codegen_func->declaration.parameters.location.sse_register)));

    struct kefir_codegen_opt_amd64_sysv_storage_handle tmp_handle;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER, NULL, &tmp_handle, NULL, NULL));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg),
        kefir_asm_amd64_xasmgen_operand_indirect(
            &codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
            2 * KEFIR_AMD64_SYSV_ABI_QWORD + codegen_func->declaration.parameters.location.stack_offset)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(arg_handle.location.reg),
                                                 KEFIR_AMD64_SYSV_ABI_QWORD),
        kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg),
        kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                                                 codegen_func->stack_frame_map.offset.register_save_area)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(arg_handle.location.reg),
                                                 2 * KEFIR_AMD64_SYSV_ABI_QWORD),
        kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg)));

    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &tmp_handle));

    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &arg_handle));
    return KEFIR_OK;
}

DEFINE_TRANSLATOR(vararg_copy) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *source_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *target_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &source_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[1], &target_allocation));

    struct kefir_codegen_opt_amd64_sysv_storage_handle source_handle, target_handle, tmp_handle;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER |
            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_RDONLY,
        source_allocation, &source_handle, kefir_codegen_opt_sysv_amd64_storage_filter_regs_allocation,
        (const struct kefir_codegen_opt_sysv_amd64_register_allocation *[]){target_allocation, NULL}));
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER |
            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_RDONLY,
        target_allocation, &target_handle, NULL, NULL));
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER, NULL, &tmp_handle, NULL, NULL));

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(&codegen->xasmgen, &codegen_func->stack_frame_map,
                                                                  source_allocation, &source_handle.location));
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(&codegen->xasmgen, &codegen_func->stack_frame_map,
                                                                  target_allocation, &target_handle.location));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg),
        kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(source_handle.location.reg), 0)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(target_handle.location.reg), 0),
        kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg),
        kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(source_handle.location.reg),
                                                 KEFIR_AMD64_SYSV_ABI_QWORD)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(target_handle.location.reg),
                                                 KEFIR_AMD64_SYSV_ABI_QWORD),
        kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg),
        kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(source_handle.location.reg),
                                                 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(target_handle.location.reg),
                                                 2 * KEFIR_AMD64_SYSV_ABI_QWORD),
        kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg)));

    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &tmp_handle));
    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &target_handle));
    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &source_handle));
    return KEFIR_OK;
}
