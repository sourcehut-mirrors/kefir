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

DEFINE_TRANSLATOR(unary_op) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg1_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &arg1_allocation));

    struct kefir_codegen_opt_sysv_amd64_storage_temporary_register result_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_general_purpose_register(
        mem, &codegen->xasmgen, &codegen_func->storage, result_allocation, &result_reg, NULL, NULL));
    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT_NOT:
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                        arg1_allocation, result_reg.reg));
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_NOT(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg)));
            break;

        case KEFIR_OPT_OPCODE_INT_NEG:
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                        arg1_allocation, result_reg.reg));
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_NEG(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg)));
            break;

        case KEFIR_OPT_OPCODE_BOOL_NOT: {
            struct kefir_codegen_opt_sysv_amd64_storage_temporary_register arg1_reg;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_general_purpose_register(
                mem, &codegen->xasmgen, &codegen_func->storage, arg1_allocation, &arg1_reg, NULL, NULL));

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                        arg1_allocation, arg1_reg.reg));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_XOR(&codegen->xasmgen,
                                                     kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                                                     kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_TEST(&codegen->xasmgen,
                                                      kefir_asm_amd64_xasmgen_operand_reg(arg1_reg.reg),
                                                      kefir_asm_amd64_xasmgen_operand_reg(arg1_reg.reg)));

            kefir_asm_amd64_xasmgen_register_t result_variant;
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(result_reg.reg, &result_variant));

            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_SETE(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_variant)));

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen,
                                                                             &codegen_func->storage, &arg1_reg));
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                 result_allocation, result_reg.reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, &codegen_func->storage,
                                                                     &result_reg));
    return KEFIR_OK;
}
