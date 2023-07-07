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

DEFINE_TRANSLATOR(stack_alloc) {
    DEFINE_TRANSLATOR_PROLOGUE;
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "%s", "STACK ALLOC"));

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *size_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *alignment_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.stack_allocation.size_ref, &size_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.stack_allocation.alignment_ref,
        &alignment_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));

    if (!instr->operation.parameters.stack_allocation.within_scope) {
        struct kefir_codegen_opt_amd64_sysv_storage_handle tmp_handle;
        REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
            mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER, NULL, &tmp_handle,
            kefir_codegen_opt_sysv_amd64_storage_filter_regs_allocation,
            (const struct kefir_codegen_opt_sysv_amd64_register_allocation *[]){size_allocation, alignment_allocation,
                                                                                NULL}));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_XOR(&codegen->xasmgen,
                                                 kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg),
                                                 kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg)));

        REQUIRE_OK(
            KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
                                          kefir_asm_amd64_xasmgen_operand_indirect(
                                              &codegen->xasmgen_helpers.operands[0],
                                              kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                                              codegen_func->stack_frame_map.offset.dynamic_scope),
                                          kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg)));

        REQUIRE_OK(
            kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &tmp_handle));
    }

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
        kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                            &codegen_func->stack_frame_map, size_allocation)));

    struct kefir_codegen_opt_amd64_sysv_storage_handle alignment_handle;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER, alignment_allocation, &alignment_handle,
        NULL, NULL));

    struct kefir_codegen_opt_amd64_sysv_storage_handle tmp_handle;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER, NULL, &tmp_handle, NULL, NULL));

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(&codegen->xasmgen, &codegen_func->stack_frame_map,
                                                                  alignment_allocation, &alignment_handle.location));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg),
        kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0], 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CMP(&codegen->xasmgen,
                                             kefir_asm_amd64_xasmgen_operand_reg(alignment_handle.location.reg),
                                             kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CMOVL(&codegen->xasmgen,
                                               kefir_asm_amd64_xasmgen_operand_reg(alignment_handle.location.reg),
                                               kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_NEG(&codegen->xasmgen,
                                             kefir_asm_amd64_xasmgen_operand_reg(alignment_handle.location.reg)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_AND(&codegen->xasmgen,
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                             kefir_asm_amd64_xasmgen_operand_reg(alignment_handle.location.reg)));

    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &tmp_handle));
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage,
                                                            &alignment_handle));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                            &codegen_func->stack_frame_map, result_allocation),
        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP)));

    return KEFIR_OK;
}
