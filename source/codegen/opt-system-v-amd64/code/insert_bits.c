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

DEFINE_TRANSLATOR(insert_bits) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL,
                                                                  *bitfield_allocation = NULL, *value_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.bitfield.base_ref, &bitfield_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.bitfield.value_ref, &value_allocation));

    struct kefir_codegen_opt_amd64_sysv_storage_handle result_handle;
    struct kefir_codegen_opt_amd64_sysv_storage_handle bitfield_handle;
    struct kefir_codegen_opt_amd64_sysv_storage_handle tmp_handle;

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER |
            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_OWNER,
        result_allocation, &result_handle, kefir_codegen_opt_sysv_amd64_storage_filter_regs_allocation,
        (const struct kefir_codegen_opt_sysv_amd64_register_allocation *[]){bitfield_allocation, NULL}));
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER, NULL, &bitfield_handle, NULL, NULL));
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER, NULL, &tmp_handle, NULL, NULL));

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(&codegen->xasmgen, &codegen_func->stack_frame_map,
                                                                  value_allocation, &result_handle.location));
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(&codegen->xasmgen, &codegen_func->stack_frame_map,
                                                                  bitfield_allocation, &bitfield_handle.location));

    const kefir_uint64_t offset = instr->operation.parameters.bitfield.offset;
    const kefir_uint64_t length = instr->operation.parameters.bitfield.length;

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVABS(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg),
        kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                             ~(((1ull << length) - 1) << offset))));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_AND(&codegen->xasmgen,
                                             kefir_asm_amd64_xasmgen_operand_reg(bitfield_handle.location.reg),
                                             kefir_asm_amd64_xasmgen_operand_reg(tmp_handle.location.reg)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SHL(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
        kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 64 - length)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SHR(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
        kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 64 - (length + offset))));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_OR(&codegen->xasmgen,
                                            kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
                                            kefir_asm_amd64_xasmgen_operand_reg(bitfield_handle.location.reg)));

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_store(&codegen->xasmgen, &codegen_func->stack_frame_map,
                                                                   result_allocation, &result_handle.location));

    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &tmp_handle));
    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &bitfield_handle));
    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &result_handle));
    return KEFIR_OK;
}
