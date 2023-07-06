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
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t inline_assembly_impl(struct kefir_mem *mem,
                                           struct kefir_codegen_opt_sysv_amd64_inline_assembly_context *context) {
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_inline_assembly_mark_clobbers(mem, context));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_inline_assembly_allocate_parameters(mem, context));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_inline_assembly_prepare_state(mem, context));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_inline_assembly_format(mem, context));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_inline_assembly_store_outputs(context));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_inline_assembly_restore_state(mem, context));
    return KEFIR_OK;
}

DEFINE_TRANSLATOR(inline_assembly) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    struct kefir_codegen_opt_sysv_amd64_inline_assembly_context context = {
        .codegen = codegen,
        .function = function,
        .codegen_func = codegen_func,
        .dirty_cc = false,
        .stack_input_parameters = {.initialized = false, .count = 0},
        .stack_output_parameters = {.count = 0},
        .stack_map = {
            .input_parameter_offset = 0, .output_parameter_offset = 0, .preserved_reg_offset = 0, .total_size = 0}};
    REQUIRE_OK(kefir_opt_code_container_inline_assembly(&function->code, instr->operation.parameters.inline_asm_ref,
                                                        &context.inline_assembly));
    context.ir_inline_assembly =
        kefir_ir_module_get_inline_assembly(module->ir_module, context.inline_assembly->inline_asm_id);
    REQUIRE(context.ir_inline_assembly != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR inline assembly"));

    REQUIRE_OK(kefir_hashtreeset_init(&context.dirty_registers, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_list_init(&context.available_registers));
    REQUIRE_OK(kefir_string_builder_init(&context.formatted_asm));
    context.parameter_allocation =
        KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_entry) *
                              context.inline_assembly->parameter_count);
    REQUIRE(context.parameter_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate inline assembly parameter entries"));
    memset(context.parameter_allocation, 0,
           sizeof(struct kefir_codegen_opt_sysv_amd64_inline_assembly_parameter_allocation_entry) *
               context.inline_assembly->parameter_count);

    kefir_result_t res = inline_assembly_impl(mem, &context);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, context.parameter_allocation);
        kefir_string_builder_free(mem, &context.formatted_asm);
        kefir_list_free(mem, &context.available_registers);
        kefir_hashtreeset_free(mem, &context.dirty_registers);
    });

    KEFIR_FREE(mem, context.parameter_allocation);
    res = kefir_string_builder_free(mem, &context.formatted_asm);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &context.available_registers);
        kefir_hashtreeset_free(mem, &context.dirty_registers);
    });
    res = kefir_list_free(mem, &context.available_registers);
    REQUIRE_ELSE(res == KEFIR_OK, { kefir_hashtreeset_free(mem, &context.dirty_registers); });
    REQUIRE_OK(kefir_hashtreeset_free(mem, &context.dirty_registers));

    return KEFIR_OK;
}
