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

DEFINE_TRANSLATOR(float_binary_op) {
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

    struct kefir_codegen_opt_sysv_amd64_storage_register result_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        mem, &codegen->xasmgen, &codegen_func->storage, result_allocation, &result_reg,
        kefir_codegen_opt_sysv_amd64_filter_regs_allocation,
        (const struct kefir_codegen_opt_sysv_amd64_register_allocation *[]){arg2_allocation, NULL}));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                arg1_allocation, result_reg.reg));

    struct kefir_codegen_opt_amd64_sysv_storage_handle arg2_handle;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_FLOATING_POINTER_REGISTER |
            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_MEMORY,
        arg2_allocation, &arg2_handle, NULL, NULL));

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_load(&codegen->xasmgen, &codegen_func->stack_frame_map,
                                                                  arg2_allocation, &arg2_handle.location));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_FLOAT32_ADD:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_ADDSS(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                                    &codegen->xasmgen_helpers.operands[0], &arg2_handle.location)));
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_ADD:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_ADDSD(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                                    &codegen->xasmgen_helpers.operands[0], &arg2_handle.location)));
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_SUB:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_SUBSS(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                                    &codegen->xasmgen_helpers.operands[0], &arg2_handle.location)));
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_SUB:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_SUBSD(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                                    &codegen->xasmgen_helpers.operands[0], &arg2_handle.location)));
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_MUL:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_MULSS(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                                    &codegen->xasmgen_helpers.operands[0], &arg2_handle.location)));
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_MUL:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_MULSD(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                                    &codegen->xasmgen_helpers.operands[0], &arg2_handle.location)));
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_DIV:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_DIVSS(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                                    &codegen->xasmgen_helpers.operands[0], &arg2_handle.location)));
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_DIV:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_DIVSD(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                kefir_codegen_opt_amd64_sysv_storage_location_operand(
                                                    &codegen->xasmgen_helpers.operands[0], &arg2_handle.location)));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                 result_allocation, result_reg.reg));

    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &arg2_handle));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &result_reg));
    return KEFIR_OK;
}
