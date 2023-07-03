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

DEFINE_TRANSLATOR(get_argument) {
    DEFINE_TRANSLATOR_PROLOGUE;

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));

    REQUIRE(result_allocation->result.backing_storage_type !=
                KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_BACKING_STORAGE_NONE,
            KEFIR_OK);

    if (result_allocation->result.backing_storage_type ==
        KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_BACKING_STORAGE_INDIRECT) {
        struct kefir_codegen_opt_amd64_sysv_storage_handle result_handle;
        REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
            mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER |
                KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_OWNER,
            result_allocation, &result_handle, NULL, NULL));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
            kefir_asm_amd64_xasmgen_operand_indirect(
                &codegen->xasmgen_helpers.operands[0],
                kefir_asm_amd64_xasmgen_operand_reg(result_allocation->result.backing_storage.indirect.base_register),
                result_allocation->result.backing_storage.indirect.offset)));

        REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_store(
            &codegen->xasmgen, &codegen_func->stack_frame_map, result_allocation, &result_handle.location));

        REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                &result_handle));
        return KEFIR_OK;
    }

    REQUIRE(result_allocation->result.backing_storage_type ==
                    KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_BACKING_STORAGE_SPILL_AREA &&
                result_allocation->result.register_aggregate_allocation != NULL,
            KEFIR_OK);

    kefir_size_t offset = result_allocation->result.backing_storage.spill.index;
    for (kefir_size_t i = 0;
         i < kefir_vector_length(&result_allocation->result.register_aggregate_allocation->container.qwords); i++) {
        ASSIGN_DECL_CAST(
            struct kefir_abi_sysv_amd64_qword *, qword,
            kefir_vector_at(&result_allocation->result.register_aggregate_allocation->container.qwords, i));
        switch (qword->klass) {
            case KEFIR_AMD64_SYSV_PARAM_INTEGER:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                        codegen_func->stack_frame_map.offset.spill_area + (offset + i) * KEFIR_AMD64_SYSV_ABI_QWORD),
                    kefir_asm_amd64_xasmgen_operand_reg(
                        KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTERS[qword->location])));
                break;

            case KEFIR_AMD64_SYSV_PARAM_SSE:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PEXTRQ(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                        codegen_func->stack_frame_map.offset.spill_area + (offset + i) * KEFIR_AMD64_SYSV_ABI_QWORD),
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_ABI_SYSV_AMD64_PARAMETER_SSE_REGISTERS[qword->location]),
                    kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[1], 0)));
                break;

            case KEFIR_AMD64_SYSV_PARAM_NO_CLASS:
                // Intentionally left blank
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                       "Aggregates with non-INTEGER and non-SSE members are not supported yet");
        }
    }

    struct kefir_codegen_opt_amd64_sysv_storage_handle result_handle;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER |
            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_OWNER,
        result_allocation, &result_handle, NULL, NULL));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
        kefir_asm_amd64_xasmgen_operand_indirect(
            &codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
            codegen_func->stack_frame_map.offset.spill_area + offset * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_store(&codegen->xasmgen, &codegen_func->stack_frame_map,
                                                                   result_allocation, &result_handle.location));

    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &result_handle));

    for (kefir_size_t i = 0;
         i < kefir_vector_length(&result_allocation->result.register_aggregate_allocation->container.qwords); i++) {
        ASSIGN_DECL_CAST(
            struct kefir_abi_sysv_amd64_qword *, qword,
            kefir_vector_at(&result_allocation->result.register_aggregate_allocation->container.qwords, i));
        switch (qword->klass) {
            case KEFIR_AMD64_SYSV_PARAM_INTEGER:
                REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_mark_register_unused(
                    mem, &codegen_func->storage, KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTERS[qword->location]));
                break;

            case KEFIR_AMD64_SYSV_PARAM_SSE:
                // Intentionally left blank
                break;

            case KEFIR_AMD64_SYSV_PARAM_NO_CLASS:
                // Intentionally left blank
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                       "Aggregates with non-INTEGER and non-SSE members are not supported yet");
        }
    }
    return KEFIR_OK;
}
