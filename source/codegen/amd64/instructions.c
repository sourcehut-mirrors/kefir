/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#include "kefir/codegen/amd64/instructions.h"
#include "kefir/codegen/amd64/function.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(error_lowered)(struct kefir_mem *mem,
                                                                   struct kefir_codegen_amd64_function *function,
                                                                   const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected optimizer intruction to be lowered prior to code generation");
}

kefir_result_t kefir_codegen_amd64_function_translate_instruction(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                            const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));
    
    kefir_size_t x87_stack_index = 0;
    struct kefir_codegen_amd64_x87_iterator iter;
    kefir_result_t res;
    kefir_opt_instruction_ref_t stack_instr_ref;
    for (res = kefir_codegen_amd64_x87_iter(&function->translator.x87, &iter, &stack_instr_ref); res == KEFIR_OK; res = kefir_codegen_amd64_x87_next(&iter, &stack_instr_ref), x87_stack_index++) {
        REQUIRE_OK(kefir_codegen_amd64_function_translator_record_x87_at(mem, &function->translator, stack_instr_ref, instruction->id, x87_stack_index));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    REQUIRE(kefir_opt_code_schedule_has(&function->generic.schedule, instruction->id),
            KEFIR_OK);
    const kefir_asmcmp_instruction_index_t begin_idx = kefir_asmcmp_context_instr_length(&function->code.context);
    const kefir_asmcmp_instruction_index_t begin_tail_idx = kefir_asmcmp_context_instr_tail(&function->code.context);

    switch (instruction->operation.opcode) {
#define CASE_INSTR(_id, _opcode)                                                           \
    case _opcode:                                                                          \
        REQUIRE_OK(KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(_id)(mem, function, instruction)); \
        break
        KEFIR_CODEGEN_AMD64_INSTRUCTIONS(CASE_INSTR, ;);
#undef CASE_INSTR
    }

    if (function->codegen->config->debug_info) {
        const kefir_asmcmp_instruction_index_t end_tail_idx = kefir_asmcmp_context_instr_tail(&function->code.context);
        for (kefir_asmcmp_instruction_index_t instr_idx = begin_tail_idx; instr_idx != end_tail_idx;
             instr_idx = kefir_asmcmp_context_instr_next(&function->code.context, instr_idx)) {
            REQUIRE_OK(kefir_codegen_target_ir_code_constructor_metadata_add_code_ref(
                mem, &function->target_ir.target_ir_metadata,
                kefir_asmcmp_context_instr_next(&function->code.context, instr_idx),
                (kefir_codegen_target_ir_metadata_code_ref_t) instruction->id));
        }
    }

    kefir_opt_code_debug_info_code_ref_t instruction_location;
    REQUIRE_OK(kefir_opt_code_debug_info_instruction_code_reference(&function->generic.function->debug_info, instruction->id,
                                                                    &instruction_location));
    if (instruction_location != KEFIR_OPT_CODE_DEBUG_INSTRUCTION_CODE_REF_NONE &&
        function->codegen->config->debug_info) {
        const struct kefir_ir_debug_source_location *source_location;
        kefir_result_t res = kefir_ir_debug_function_source_map_find(
            &function->generic.function->ir_func->debug_info.source_map, instruction_location, &source_location);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            const kefir_asmcmp_instruction_index_t end_idx = kefir_asmcmp_context_instr_length(&function->code.context);
            REQUIRE_OK(kefir_asmcmp_debug_info_source_map_add_location(
                mem, &function->code.context.debug_info.source_map, &function->code.context.strings, begin_idx, end_idx,
                &source_location->location));
        }
    }

    if (instruction->operation.opcode == KEFIR_OPT_OPCODE_GET_ARGUMENT) {
        REQUIRE_OK(kefir_codegen_amd64_function_translator_set_parameter(mem, &function->translator, instruction->operation.parameters.index, instruction->id));
    }
    return KEFIR_OK;
}
