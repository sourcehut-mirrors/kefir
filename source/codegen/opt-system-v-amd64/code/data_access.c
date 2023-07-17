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

DEFINE_TRANSLATOR(data_access) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));

    struct kefir_codegen_opt_amd64_sysv_storage_handle result_handle;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_GENERAL_PURPOSE_REGISTER |
            KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_REGISTER_ALLOCATION_OWNER,
        result_allocation, &result_handle, NULL, NULL));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_GET_LOCAL: {
            const struct kefir_abi_sysv_amd64_typeentry_layout *entry = NULL;
            REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_at(&codegen_func->locals_layout,
                                                           instr->operation.parameters.index, &entry));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                    codegen_func->stack_frame_map.offset.local_area + entry->relative_offset)));
        } break;

        case KEFIR_OPT_OPCODE_GET_GLOBAL: {
            const char *symbol =
                kefir_ir_module_get_named_symbol(module->ir_module, instr->operation.parameters.ir_ref);
            REQUIRE(symbol != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find named IR symbol"));

            if (!codegen->config->position_independent_code) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_label(
                            &codegen->xasmgen_helpers.operands[1],
                            kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, "%s", symbol)),
                        0)));
            } else if (!kefir_ir_module_has_external(module->ir_module, symbol) &&
                       !kefir_ir_module_has_global(module->ir_module, symbol)) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
                    kefir_asm_amd64_xasmgen_operand_rip_indirection(&codegen->xasmgen_helpers.operands[0], symbol)));
            } else {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_handle.location.reg),
                    kefir_asm_amd64_xasmgen_operand_pointer(
                        &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_QWORD,
                        kefir_asm_amd64_xasmgen_operand_rip_indirection(
                            &codegen->xasmgen_helpers.operands[1],
                            kefir_asm_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, KEFIR_AMD64_GOTPCREL,
                                                                   symbol)))));
            }
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
    }

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_store(&codegen->xasmgen, &codegen_func->stack_frame_map,
                                                                   result_allocation, &result_handle.location));

    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &result_handle));
    return KEFIR_OK;
}
