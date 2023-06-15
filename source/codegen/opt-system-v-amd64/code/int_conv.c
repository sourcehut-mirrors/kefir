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

DEFINE_TRANSLATOR(int_conv) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *target_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &target_allocation));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *source_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &source_allocation));

    struct kefir_codegen_opt_sysv_amd64_translate_temporary_register source_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain(
        mem, codegen, source_allocation, codegen_func, &source_reg, NULL, NULL));

    struct kefir_codegen_opt_sysv_amd64_translate_temporary_register target_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain(
        mem, codegen, target_allocation, codegen_func, &target_reg, NULL, NULL));
    if (source_reg.borrow) {
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation_into(codegen, &codegen_func->stack_frame_map,
                                                                         source_allocation, source_reg.reg));
    }

    kefir_asm_amd64_xasmgen_register_t source_variant_reg, target_variant_reg;
    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_INT64_TRUNCATE_1BIT:
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(target_reg.reg, &target_variant_reg));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_TEST(&codegen->xasmgen,
                                                      kefir_asm_amd64_xasmgen_operand_reg(target_reg.reg),
                                                      kefir_asm_amd64_xasmgen_operand_reg(source_reg.reg)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SETNE(&codegen->xasmgen,
                                                       kefir_asm_amd64_xasmgen_operand_reg(target_variant_reg)));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVZX(&codegen->xasmgen,
                                                       kefir_asm_amd64_xasmgen_operand_reg(target_reg.reg),
                                                       kefir_asm_amd64_xasmgen_operand_reg(target_variant_reg)));
            break;

        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS:
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(source_reg.reg, &source_variant_reg));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVSX(&codegen->xasmgen,
                                                       kefir_asm_amd64_xasmgen_operand_reg(target_reg.reg),
                                                       kefir_asm_amd64_xasmgen_operand_reg(source_variant_reg)));
            break;

        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS:
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register16(source_reg.reg, &source_variant_reg));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVSX(&codegen->xasmgen,
                                                       kefir_asm_amd64_xasmgen_operand_reg(target_reg.reg),
                                                       kefir_asm_amd64_xasmgen_operand_reg(source_variant_reg)));
            break;

        case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS:
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(source_reg.reg, &source_variant_reg));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVSX(&codegen->xasmgen,
                                                       kefir_asm_amd64_xasmgen_operand_reg(target_reg.reg),
                                                       kefir_asm_amd64_xasmgen_operand_reg(source_variant_reg)));
            break;

        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS:
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register8(source_reg.reg, &source_variant_reg));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVZX(&codegen->xasmgen,
                                                       kefir_asm_amd64_xasmgen_operand_reg(target_reg.reg),
                                                       kefir_asm_amd64_xasmgen_operand_reg(source_variant_reg)));
            break;

        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS:
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register16(source_reg.reg, &source_variant_reg));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVZX(&codegen->xasmgen,
                                                       kefir_asm_amd64_xasmgen_operand_reg(target_reg.reg),
                                                       kefir_asm_amd64_xasmgen_operand_reg(source_variant_reg)));
            break;

        case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS:
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(source_reg.reg, &source_variant_reg));
            REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(target_reg.reg, &target_variant_reg));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
                                                     kefir_asm_amd64_xasmgen_operand_reg(target_variant_reg),
                                                     kefir_asm_amd64_xasmgen_operand_reg(source_variant_reg)));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    if (target_reg.borrow) {
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation_from(codegen, &codegen_func->stack_frame_map,
                                                                          target_allocation, target_reg.reg));
    }

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &target_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &source_reg));
    return KEFIR_OK;
}
