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

DEFINE_TRANSLATOR(load) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *source_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.memory_access.location, &source_allocation));

    struct kefir_codegen_opt_sysv_amd64_storage_register source_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_shared_allocated_register(
        mem, &codegen->xasmgen, &codegen_func->storage, source_allocation, &source_reg, NULL, NULL));

    struct kefir_codegen_opt_sysv_amd64_storage_register result_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        mem, &codegen->xasmgen, &codegen_func->storage, result_allocation, &result_reg, NULL, NULL));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                source_allocation, source_reg.reg));

    kefir_asm_amd64_xasmgen_register_t result_variant_reg;
    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT8_LOAD_SIGNED:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVSX(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_asm_amd64_xasmgen_operand_pointer(
                    &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_BYTE,
                    kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                             kefir_asm_amd64_xasmgen_operand_reg(source_reg.reg), 0))));
            break;

        case KEFIR_OPT_OPCODE_INT8_LOAD_UNSIGNED:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVZX(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_asm_amd64_xasmgen_operand_pointer(
                    &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_BYTE,
                    kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                             kefir_asm_amd64_xasmgen_operand_reg(source_reg.reg), 0))));
            break;

        case KEFIR_OPT_OPCODE_INT16_LOAD_SIGNED:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVSX(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_asm_amd64_xasmgen_operand_pointer(
                    &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_WORD,
                    kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                             kefir_asm_amd64_xasmgen_operand_reg(source_reg.reg), 0))));
            break;

        case KEFIR_OPT_OPCODE_INT16_LOAD_UNSIGNED:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVZX(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_asm_amd64_xasmgen_operand_pointer(
                    &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_WORD,
                    kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                             kefir_asm_amd64_xasmgen_operand_reg(source_reg.reg), 0))));
            break;

        case KEFIR_OPT_OPCODE_INT32_LOAD_SIGNED:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVSX(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_asm_amd64_xasmgen_operand_pointer(
                    &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_DWORD,
                    kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                             kefir_asm_amd64_xasmgen_operand_reg(source_reg.reg), 0))));
            break;

        case KEFIR_OPT_OPCODE_INT32_LOAD_UNSIGNED:
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(result_reg.reg, &result_variant_reg));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_variant_reg),
                kefir_asm_amd64_xasmgen_operand_pointer(
                    &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_DWORD,
                    kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                             kefir_asm_amd64_xasmgen_operand_reg(source_reg.reg), 0))));
            break;

        case KEFIR_OPT_OPCODE_INT64_LOAD:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_asm_amd64_xasmgen_operand_pointer(
                    &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_QWORD,
                    kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                             kefir_asm_amd64_xasmgen_operand_reg(source_reg.reg), 0))));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                 result_allocation, result_reg.reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &result_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &source_reg));
    return KEFIR_OK;
}
