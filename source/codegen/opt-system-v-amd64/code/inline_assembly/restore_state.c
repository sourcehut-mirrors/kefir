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

#include "kefir/codegen/opt-system-v-amd64/inline_assembly.h"
#include "kefir/codegen/opt-system-v-amd64/function.h"
#include "kefir/target/abi/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_codegen_opt_sysv_amd64_inline_assembly_restore_state(
    struct kefir_mem *mem, struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen inline assembly context"));

    if (context->stack_map.preserved_reg_offset > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &context->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_imm(&context->codegen->xasmgen_helpers.operands[0],
                                                context->stack_map.preserved_reg_offset)));
    }

    kefir_size_t reg_offset = context->stack_map.preserved_reg_size - KEFIR_AMD64_ABI_QWORD;
    if (context->dirty_cc) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POPFQ(&context->codegen->xasmgen));
        reg_offset -= KEFIR_AMD64_ABI_QWORD;
    }

    struct kefir_hashtreeset_iterator iter;
    kefir_result_t res;
    for (res = kefir_hashtreeset_iter(&context->dirty_registers, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_asm_amd64_xasmgen_register_t, reg, iter.entry);

        kefir_bool_t occupied;
        REQUIRE_OK(
            kefir_codegen_opt_sysv_amd64_storage_is_register_occupied(&context->codegen_func->storage, reg, &occupied));
        if (!occupied) {
            continue;
        }

        if (!kefir_asm_amd64_xasmgen_register_is_floating_point(reg)) {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &context->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg),
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &context->codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), reg_offset)));
        } else {
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                &context->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg),
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &context->codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP), reg_offset)));
        }
        reg_offset -= KEFIR_AMD64_ABI_QWORD;
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    if (context->stack_map.preserved_reg_size > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &context->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_imm(
                &context->codegen->xasmgen_helpers.operands[0],
                context->stack_map.preserved_reg_size - (context->dirty_cc ? KEFIR_AMD64_ABI_QWORD : 0))));
    }
    return KEFIR_OK;
}
