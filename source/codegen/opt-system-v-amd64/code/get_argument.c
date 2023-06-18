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

    REQUIRE(result_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_POINTER_SPILL_AREA,
            KEFIR_OK);
    REQUIRE(result_allocation->result.parameter_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid function parameter allocation"));

    kefir_size_t offset = result_allocation->result.spill.index;
    for (kefir_size_t i = 0; i < kefir_vector_length(&result_allocation->result.parameter_allocation->container.qwords);
         i++) {
        ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_qword *, qword,
                         kefir_vector_at(&result_allocation->result.parameter_allocation->container.qwords, i));
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

    struct kefir_codegen_opt_sysv_amd64_translate_temporary_register result_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain(
        mem, codegen, result_allocation, codegen_func, &result_reg, NULL, NULL));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg.reg),
        kefir_asm_amd64_xasmgen_operand_indirect(
            &codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
            codegen_func->stack_frame_map.offset.spill_area + offset * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                 result_allocation, result_reg.reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &result_reg));
    return KEFIR_OK;
}
