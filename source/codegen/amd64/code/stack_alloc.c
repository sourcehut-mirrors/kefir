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

#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/function.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t ensure_dynamic_scope_vreg(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function) {
    if (function->dynamic_scope_vreg == KEFIR_ASMCMP_INDEX_NONE) {
        REQUIRE_OK(kefir_asmcmp_virtual_register_new_direct_spill_space_allocation(mem, &function->code.context, 1,
                                                                                   &function->dynamic_scope_vreg));
        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, function->prologue_tail,
                                          &KEFIR_ASMCMP_MAKE_VREG64(function->dynamic_scope_vreg),
                                          &KEFIR_ASMCMP_MAKE_UINT(0), &function->prologue_tail));

        REQUIRE_OK(kefir_codegen_amd64_stack_frame_varying_stack_pointer(&function->stack_frame));
    }
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(stack_alloc)(struct kefir_mem *mem,
                                                                 struct kefir_codegen_amd64_function *function,
                                                                 const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg, size_vreg, alignment_vreg, tmp_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(
        function, instruction->operation.parameters.stack_allocation.size_ref, &size_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(
        function, instruction->operation.parameters.stack_allocation.alignment_ref, &alignment_vreg));

    REQUIRE_OK(ensure_dynamic_scope_vreg(mem, function));

    if (!instruction->operation.parameters.stack_allocation.within_scope) {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(function->dynamic_scope_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_sub(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                      &KEFIR_ASMCMP_MAKE_VREG64(size_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
                                      &KEFIR_ASMCMP_MAKE_UINT(2 * KEFIR_AMD64_ABI_QWORD), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_cmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(alignment_vreg),
                                      NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_cmovl(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(alignment_vreg),
                                        NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_neg(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                      &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
                                      &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RSP), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(push_scope)(struct kefir_mem *mem,
                                                                struct kefir_codegen_amd64_function *function,
                                                                const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    REQUIRE_OK(ensure_dynamic_scope_vreg(mem, function));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
                                      &KEFIR_ASMCMP_MAKE_VREG64(function->dynamic_scope_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_sub(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                      &KEFIR_ASMCMP_MAKE_UINT(2 * KEFIR_AMD64_ABI_QWORD), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(KEFIR_AMD64_XASMGEN_REGISTER_RSP, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
        &KEFIR_ASMCMP_MAKE_VREG64(function->dynamic_scope_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(function->dynamic_scope_vreg),
                                      &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RSP), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(pop_scope)(struct kefir_mem *mem,
                                                               struct kefir_codegen_amd64_function *function,
                                                               const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(ensure_dynamic_scope_vreg(mem, function));

    kefir_asmcmp_label_index_t label;
    REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label));

    const char *symbolic_label;
    REQUIRE_OK(kefir_codegen_amd64_function_format_label(mem, function, label, &symbolic_label));
    ;

    REQUIRE_OK(kefir_asmcmp_amd64_cmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(function->dynamic_scope_vreg),
                                      &KEFIR_ASMCMP_MAKE_UINT(0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_jz(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_LABEL(symbolic_label, 0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                      &KEFIR_ASMCMP_MAKE_VREG64(function->dynamic_scope_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG64(function->dynamic_scope_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(KEFIR_AMD64_XASMGEN_REGISTER_RSP, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
        NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                      &KEFIR_ASMCMP_MAKE_UINT(2 * KEFIR_AMD64_ABI_QWORD), NULL));

    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, label));
    return KEFIR_OK;
}
