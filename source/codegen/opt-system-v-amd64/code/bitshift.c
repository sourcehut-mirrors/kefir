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

DEFINE_TRANSLATOR(bitshift) {
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

    REQUIRE(arg2_allocation->result.type != KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                            "Expected non-floating-point allocation for the second argument of integral operation"));

    struct kefir_codegen_opt_sysv_amd64_storage_register shift_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
        mem, &codegen->xasmgen, &codegen_func->storage, arg2_allocation, KEFIR_AMD64_XASMGEN_REGISTER_RCX, &shift_reg));

    struct kefir_codegen_opt_sysv_amd64_storage_register result_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        mem, &codegen->xasmgen, &codegen_func->storage, result_allocation, &result_reg,
        kefir_codegen_opt_sysv_amd64_filter_regs_allocation,
        (const struct kefir_codegen_opt_sysv_amd64_register_allocation *[]){arg2_allocation, NULL}));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                arg1_allocation, result_reg.reg));

    kefir_asm_amd64_xasmgen_register_t shift_variant;
    REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(shift_reg.reg, &shift_variant));
    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT_LSHIFT:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SHL(&codegen->xasmgen,
                                                     kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                     kefir_asm_amd64_xasmgen_operand_reg(shift_variant)));
            break;

        case KEFIR_OPT_OPCODE_INT_RSHIFT:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SHR(&codegen->xasmgen,
                                                     kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                     kefir_asm_amd64_xasmgen_operand_reg(shift_variant)));
            break;

        case KEFIR_OPT_OPCODE_INT_ARSHIFT:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SAR(&codegen->xasmgen,
                                                     kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                     kefir_asm_amd64_xasmgen_operand_reg(shift_variant)));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                 result_allocation, result_reg.reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &result_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &shift_reg));
    return KEFIR_OK;
}
