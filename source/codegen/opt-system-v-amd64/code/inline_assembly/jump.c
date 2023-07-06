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
#include "kefir/codegen/opt-system-v-amd64/code_impl.h"
#include "kefir/codegen/opt-system-v-amd64/runtime.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_codegen_opt_sysv_amd64_inline_assembly_jump_trampolines(
    struct kefir_mem *mem, struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen inline assembly context"));
    REQUIRE(!kefir_hashtree_empty(&context->inline_assembly->jump_targets), KEFIR_OK);

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_map_registers(
        mem, context->codegen, context->function, context->func_analysis, context->codegen_func,
        context->instr->block_id, context->inline_assembly->default_jump_target));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
        &context->codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_label(
            &context->codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_helpers_format(
                &context->codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK,
                context->function->ir_func->name, context->inline_assembly->default_jump_target))));

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&context->inline_assembly->jump_targets, &iter);
         node != NULL; node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(kefir_id_t, target_id, node->key);
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block, node->value);

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(
            &context->codegen->xasmgen, KEFIR_OPT_AMD64_SYSTEM_V_INLINE_ASSEMBLY_JUMP_TRAMPOLINE_LABEL,
            context->function->ir_func->name, context->inline_assembly->node_id, target_id));

        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_inline_assembly_store_outputs(context));
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_inline_assembly_restore_state(mem, context));
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_map_registers(mem, context->codegen, context->function,
                                                              context->func_analysis, context->codegen_func,
                                                              context->instr->block_id, target_block));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
            &context->codegen->xasmgen,
            kefir_asm_amd64_xasmgen_operand_label(
                &context->codegen->xasmgen_helpers.operands[0],
                kefir_asm_amd64_xasmgen_helpers_format(&context->codegen->xasmgen_helpers,
                                                       KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK,
                                                       context->function->ir_func->name, target_block))));
    }
    return KEFIR_OK;
}
