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

DEFINE_TRANSLATOR(comparison) {
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
        (const struct kefir_codegen_opt_sysv_amd64_register_allocation *[]){arg1_allocation, arg2_allocation, NULL}));

    struct kefir_codegen_opt_sysv_amd64_storage_register arg1_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        mem, &codegen->xasmgen, &codegen_func->storage, arg1_allocation, &arg1_reg, NULL, NULL));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                arg1_allocation, arg1_reg.reg));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_XOR(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                             kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CMP(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(arg1_reg.reg),
        kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                            &codegen_func->stack_frame_map, arg2_allocation)));

    kefir_asm_amd64_xasmgen_register_t result_variant;
    REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(result_reg.reg, &result_variant));
    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT_EQUALS:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_SETE(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_variant)));
            break;

        case KEFIR_OPT_OPCODE_INT_GREATER:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_SETG(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_variant)));
            break;

        case KEFIR_OPT_OPCODE_INT_LESSER:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_SETL(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_variant)));
            break;

        case KEFIR_OPT_OPCODE_INT_ABOVE:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_SETA(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_variant)));
            break;

        case KEFIR_OPT_OPCODE_INT_BELOW:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_SETB(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_variant)));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                 result_allocation, result_reg.reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &arg1_reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &result_reg));
    return KEFIR_OK;
}
