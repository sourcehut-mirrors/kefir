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

DEFINE_TRANSLATOR(get_local) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &reg_allocation));

    const struct kefir_abi_sysv_amd64_typeentry_layout *entry = NULL;
    REQUIRE_OK(
        kefir_abi_sysv_amd64_type_layout_at(&codegen_func->locals_layout, instr->operation.parameters.index, &entry));

    struct kefir_codegen_opt_sysv_amd64_translate_temporary_register tmp_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain(mem, codegen, reg_allocation,
                                                                                      codegen_func, &tmp_reg));

    REQUIRE_OK(
        KEFIR_AMD64_XASMGEN_INSTR_LEA(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg),
                                      kefir_asm_amd64_xasmgen_operand_indirect(
                                          &codegen->xasmgen_helpers.operands[0],
                                          kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                                          codegen_func->stack_frame_map.offset.local_area + entry->relative_offset)));
    if (tmp_reg.borrow) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen,
            kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&codegen->xasmgen_helpers.operands[0],
                                                                &codegen_func->stack_frame_map, reg_allocation),
            kefir_asm_amd64_xasmgen_operand_reg(tmp_reg.reg)));
    }

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &tmp_reg));
    return KEFIR_OK;
}
