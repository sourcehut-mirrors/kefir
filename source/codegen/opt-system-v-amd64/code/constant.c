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

DEFINE_TRANSLATOR(constant) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &reg_allocation));
    kefir_bool_t unsigned_integer = false;
    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_UINT_CONST:
            unsigned_integer = true;
            // Fallthrough
        case KEFIR_OPT_OPCODE_INT_CONST: {
            struct kefir_codegen_opt_sysv_amd64_translate_temporary_register result_reg;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain(
                mem, codegen, reg_allocation, codegen_func, &result_reg, NULL, NULL));

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

            if (result_reg.borrow) {
                REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation_from(
                    codegen, &codegen_func->stack_frame_map, reg_allocation, result_reg.reg));
            }

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &result_reg));
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }
    return KEFIR_OK;
}
