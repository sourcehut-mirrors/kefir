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

DEFINE_TRANSLATOR(constant) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));

    struct kefir_codegen_opt_sysv_amd64_storage_temporary_register result_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_temporary_general_purpose_register(
        mem, &codegen->xasmgen, &codegen_func->storage, result_allocation, &result_reg, NULL, NULL));

    kefir_bool_t unsigned_integer = false;
    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_UINT_CONST:
            unsigned_integer = true;
            // Fallthrough
        case KEFIR_OPT_OPCODE_INT_CONST: {
            if (instr->operation.parameters.imm.integer >= KEFIR_INT32_MIN &&
                instr->operation.parameters.imm.integer <= KEFIR_INT32_MAX) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                    !unsigned_integer
                        ? kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                              instr->operation.parameters.imm.integer)
                        : kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                                               instr->operation.parameters.imm.uinteger)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVABS(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                    !unsigned_integer
                        ? kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                                              instr->operation.parameters.imm.integer)
                        : kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                                               instr->operation.parameters.imm.uinteger)));
            }
        } break;

        case KEFIR_OPT_OPCODE_STRING_REF:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_label(
                        &codegen->xasmgen_helpers.operands[1],
                        kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers,
                                                               KEFIR_AMD64_SYSTEM_V_STRING_LITERAL,
                                                               instr->operation.parameters.imm.string_ref)),
                    0)));
            break;

        case KEFIR_OPT_OPCODE_BLOCK_LABEL:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
                kefir_asm_amd64_xasmgen_operand_label(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_helpers_format(
                        &codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK, function->ir_func->name,
                        instr->operation.parameters.imm.block_ref))));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                 result_allocation, result_reg.reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_temporary_register(mem, &codegen->xasmgen,
                                                                               &codegen_func->storage, &result_reg));
    return KEFIR_OK;
}
