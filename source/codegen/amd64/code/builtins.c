/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t translate_atomic_seq_cst_fence(struct kefir_mem *mem,
                                                     struct kefir_codegen_amd64_function *function) {
    REQUIRE_OK(
        kefir_asmcmp_amd64_lock(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_or(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(KEFIR_AMD64_XASMGEN_REGISTER_RSP, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
        &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
    return KEFIR_OK;
}

static kefir_result_t translate_atomic_seq_cst_test_and_set(struct kefir_mem *mem,
                                                            struct kefir_codegen_amd64_function *function,
                                                            const struct kefir_opt_instruction *instruction,
                                                            const struct kefir_opt_call_node *call_node) {
    UNUSED(instruction);
    kefir_asmcmp_virtual_register_index_t result_vreg, argument_vreg;

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                            "Expected a single argument for __kefir_builtin_atomic_seq_cst_test_and_set"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(1), NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_lock(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xchg(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_8BIT),
        &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_movzx(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG8(result_vreg),
                                        NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t translate_atomic_seq_cst_clear(struct kefir_mem *mem,
                                                     struct kefir_codegen_amd64_function *function,
                                                     const struct kefir_opt_instruction *instruction,
                                                     const struct kefir_opt_call_node *call_node) {
    UNUSED(instruction);
    kefir_asmcmp_virtual_register_index_t tmp_vreg, argument_vreg;

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                            "Expected a single argument for __kefir_builtin_atomic_seq_cst_test_and_set"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_lock(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xchg(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_8BIT),
        &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
    return KEFIR_OK;
}

static kefir_result_t translate_trap(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                     const struct kefir_opt_instruction *instruction) {
    UNUSED(instruction);
    REQUIRE_OK(
        kefir_asmcmp_amd64_ud2(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
    return KEFIR_OK;
}

static kefir_result_t translate_return_address(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                               const struct kefir_opt_instruction *instruction,
                                               const struct kefir_opt_call_node *call_node) {
    kefir_asmcmp_virtual_register_index_t return_address_vreg, frame_address_vreg, argument_vreg, counter_vreg;
    kefir_asmcmp_label_index_t loop_begin_label, loop_end_label;

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_require_frame_pointer(&function->stack_frame));

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected a single argument for __kefir_builtin_return_address"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &return_address_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &frame_address_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &counter_vreg));

    REQUIRE_OK(
        kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &loop_begin_label));
    REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &loop_end_label));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         counter_vreg, argument_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(frame_address_vreg),
                                      &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RBP), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(return_address_vreg),
                                      &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(frame_address_vreg, KEFIR_AMD64_ABI_QWORD,
                                                                          KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                      NULL));

    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, loop_begin_label));

    REQUIRE_OK(kefir_asmcmp_amd64_cmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(counter_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_je(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(loop_end_label), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(return_address_vreg),
                                      &KEFIR_ASMCMP_MAKE_VREG(return_address_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(frame_address_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(frame_address_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_test(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(frame_address_vreg),
                                       &KEFIR_ASMCMP_MAKE_VREG(frame_address_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_jz(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(loop_end_label), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(return_address_vreg),
                                      &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(frame_address_vreg, KEFIR_AMD64_ABI_QWORD,
                                                                          KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_dec(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(counter_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(loop_begin_label), NULL));

    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, loop_end_label));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, return_address_vreg));
    return KEFIR_OK;
}

static kefir_result_t translate_frame_address(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                              const struct kefir_opt_instruction *instruction,
                                              const struct kefir_opt_call_node *call_node) {
    kefir_asmcmp_virtual_register_index_t frame_address_vreg, argument_vreg, counter_vreg;
    kefir_asmcmp_label_index_t loop_begin_label, loop_end_label;

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_require_frame_pointer(&function->stack_frame));

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected a single argument for __kefir_builtin_frame_address"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &frame_address_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &counter_vreg));

    REQUIRE_OK(
        kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &loop_begin_label));
    REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &loop_end_label));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         counter_vreg, argument_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(frame_address_vreg),
                                      &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RBP), NULL));

    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, loop_begin_label));

    REQUIRE_OK(kefir_asmcmp_amd64_cmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(counter_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_je(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(loop_end_label), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(frame_address_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(frame_address_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_test(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(frame_address_vreg),
                                       &KEFIR_ASMCMP_MAKE_VREG(frame_address_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_jz(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(loop_end_label), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_dec(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(counter_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(loop_begin_label), NULL));

    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, loop_end_label));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, frame_address_vreg));
    return KEFIR_OK;
}

static kefir_result_t translate_ffs(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                    const struct kefir_opt_instruction *instruction,
                                    const struct kefir_opt_call_node *call_node) {
    kefir_asmcmp_virtual_register_index_t argument_vreg, result_vreg, tmp_vreg;

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected a single argument for __kefir_builtin_ffs"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_bsf(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(argument_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), &KEFIR_ASMCMP_MAKE_INT(-1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_cmove(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg),
                                        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_INT(1), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t translate_clz(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                    const struct kefir_opt_instruction *instruction,
                                    const struct kefir_opt_call_node *call_node) {
    kefir_asmcmp_virtual_register_index_t argument_vreg, result_vreg;

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected a single argument for __kefir_builtin_clz"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_bsr(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(argument_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_INT(31), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t translate_ctz(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                    const struct kefir_opt_instruction *instruction,
                                    const struct kefir_opt_call_node *call_node) {
    kefir_asmcmp_virtual_register_index_t argument_vreg, result_vreg;

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected a single argument for __kefir_builtin_ctz"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(result_vreg),
                                      NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_rep(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_bsf(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(argument_vreg),
                                      NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t translate_clrsb(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                      const struct kefir_opt_instruction *instruction,
                                      const struct kefir_opt_call_node *call_node) {
    kefir_asmcmp_virtual_register_index_t argument_vreg, result_vreg, tmp_vreg;

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected a single argument for __kefir_builtin_clrsb"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(argument_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(result_vreg),
                                      NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg, argument_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_sar(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(31), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_or(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_bsr(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(result_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(31), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t translate_popcount(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                         const struct kefir_opt_instruction *instruction,
                                         const struct kefir_opt_call_node *call_node) {
    kefir_asmcmp_virtual_register_index_t argument_vreg, result_vreg, tmp_vreg;

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected a single argument for __kefir_builtin_popcount"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(argument_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(1431655765),
                                      NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg, argument_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_sub(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG32(result_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(858993459),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(858993459), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG32(result_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(4), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(252645135),
                                      NULL));
    ;
    REQUIRE_OK(kefir_asmcmp_amd64_imul3(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(result_vreg),
                                        &KEFIR_ASMCMP_MAKE_UINT(16843009), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(24), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t translate_parity(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                       const struct kefir_opt_instruction *instruction,
                                       const struct kefir_opt_call_node *call_node) {
    kefir_asmcmp_virtual_register_index_t argument_vreg, tmp_vreg, result_vreg;

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected a single argument for __kefir_builtin_parity"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, tmp_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_RAX));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG32(argument_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(16), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG32(argument_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg),
                                      &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_AH), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_setnp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movzx(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg, tmp_vreg, NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t translate_ffsl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                     const struct kefir_opt_instruction *instruction,
                                     const struct kefir_opt_call_node *call_node) {
    kefir_asmcmp_virtual_register_index_t argument_vreg, tmp_vreg, result_vreg;

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                            "Expected a single argument for __kefir_builtin_ffsl/__kefir_builtin_ffsll"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg, argument_vreg, NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_bsf(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_INT(-1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_cmove(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG(result_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_lea(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG32(result_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp_vreg, 1, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t translate_clzl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                     const struct kefir_opt_instruction *instruction,
                                     const struct kefir_opt_call_node *call_node) {
    kefir_asmcmp_virtual_register_index_t argument_vreg, result_vreg;

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                            "Expected a single argument for __kefir_builtin_clzl/__kefir_builtin_clzll"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_bsr(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(argument_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(63), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t translate_ctzl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                     const struct kefir_opt_instruction *instruction,
                                     const struct kefir_opt_call_node *call_node) {
    kefir_asmcmp_virtual_register_index_t argument_vreg, result_vreg;

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                            "Expected a single argument for __kefir_builtin_ctzl/__kefir_builtin_ctzll"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(result_vreg),
                                      NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_rep(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_bsf(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(argument_vreg),
                                      NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t translate_clrsbl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                       const struct kefir_opt_instruction *instruction,
                                       const struct kefir_opt_call_node *call_node) {
    kefir_asmcmp_virtual_register_index_t argument_vreg, result_vreg, tmp_vreg;

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                            "Expected a single argument for __kefir_builtin_clrsbl/__kefir_builtin_clrsbll"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(argument_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                                      NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg, argument_vreg, NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_sar(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(63), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_or(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_bsr(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(63), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t translate_popcountl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                          const struct kefir_opt_instruction *instruction,
                                          const struct kefir_opt_call_node *call_node) {
    kefir_asmcmp_virtual_register_index_t argument_vreg, result_vreg, tmp_vreg, tmp2_vreg;

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                            "Expected a single argument for __kefir_builtin_popcountl/__kefir_builtin_popcountll"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp2_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(argument_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(1), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movabs(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
                                         &KEFIR_ASMCMP_MAKE_UINT(6148914691236517205ull), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG(result_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         tmp2_vreg, argument_vreg, NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_sub(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movabs(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                                         &KEFIR_ASMCMP_MAKE_UINT(3689348814741910323ull), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG(result_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), &KEFIR_ASMCMP_MAKE_UINT(2), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG(result_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(4), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG(result_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movabs(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg),
                                         &KEFIR_ASMCMP_MAKE_UINT(1085102592571150095ull), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movabs(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                                         &KEFIR_ASMCMP_MAKE_UINT(72340172838076673ull), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_imul(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp2_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(56), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t translate_parityl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                        const struct kefir_opt_instruction *instruction,
                                        const struct kefir_opt_call_node *call_node) {
    kefir_asmcmp_virtual_register_index_t argument_vreg, result_vreg, tmp_vreg, tmp_result_vreg;

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                            "Expected a single argument for __kefir_builtin_parityl/__kefir_builtin_parityll"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, tmp_result_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_RAX));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(tmp_result_vreg), &KEFIR_ASMCMP_MAKE_VREG(argument_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(tmp_result_vreg), &KEFIR_ASMCMP_MAKE_UINT(32), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(tmp_result_vreg),
                                      &KEFIR_ASMCMP_MAKE_VREG32(argument_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG32(tmp_result_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(16), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(tmp_result_vreg), &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_xor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG8(tmp_result_vreg),
                                      &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_AH), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_setnp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG8(tmp_result_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_movzx(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG32(tmp_result_vreg),
                                        &KEFIR_ASMCMP_MAKE_VREG8(tmp_result_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_vreg, tmp_result_vreg, NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t translate_bswap16(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                        const struct kefir_opt_instruction *instruction,
                                        const struct kefir_opt_call_node *call_node) {
    kefir_asmcmp_virtual_register_index_t argument_vreg, result_vreg;

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected a single argument for __kefir_builtin_bswap16"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_vreg, argument_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_rol(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG16(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(8), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t translate_bswap32(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                        const struct kefir_opt_instruction *instruction,
                                        const struct kefir_opt_call_node *call_node) {
    kefir_asmcmp_virtual_register_index_t argument_vreg, result_vreg;

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected a single argument for __kefir_builtin_bswap32"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_vreg, argument_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_bswap(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t translate_bswap64(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                        const struct kefir_opt_instruction *instruction,
                                        const struct kefir_opt_call_node *call_node) {
    kefir_asmcmp_virtual_register_index_t argument_vreg, result_vreg;

    REQUIRE(call_node->argument_count == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected a single argument for __kefir_builtin_bswap64"));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[0], &argument_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_vreg, argument_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_bswap(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(result_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t translate_flt_rounds(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                           const struct kefir_opt_instruction *instruction,
                                           const struct kefir_opt_call_node *call_node) {
    kefir_asmcmp_virtual_register_index_t tmp_area_vreg, result_vreg;

    REQUIRE(call_node->argument_count == 0,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected no arguments for __kefir_builtin_flt_rounds"));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(mem, &function->code.context, 1, 1,
                                                                                 &tmp_area_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_stmxcsr(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp_area_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp_area_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(3), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0xc00), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_translate_builtin(struct kefir_mem *mem,
                                                     struct kefir_codegen_amd64_function *function,
                                                     const struct kefir_opt_instruction *instruction,
                                                     kefir_bool_t *found_builtin) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    const struct kefir_opt_call_node *call_node = NULL;
    REQUIRE_OK(kefir_opt_code_container_call(&function->function->code,
                                             instruction->operation.parameters.function_call.call_ref, &call_node));

    const struct kefir_ir_function_decl *ir_func_decl =
        kefir_ir_module_get_declaration(function->module->ir_module, call_node->function_declaration_id);
    REQUIRE(ir_func_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR function declaration"));

    if (strcmp(ir_func_decl->name, "__kefir_builtin_atomic_seq_cst_fence") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_atomic_seq_cst_fence(mem, function));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_atomic_seq_cst_test_and_set") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_atomic_seq_cst_test_and_set(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_atomic_seq_cst_clear") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_atomic_seq_cst_clear(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_trap") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_trap(mem, function, instruction));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_return_address") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_return_address(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_frame_address") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_frame_address(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_ffs") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_ffs(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_clz") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_clz(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_ctz") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_ctz(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_clrsb") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_clrsb(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_popcount") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_popcount(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_parity") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_parity(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_ffsl") == 0 ||
               strcmp(ir_func_decl->name, "__kefir_builtin_ffsll") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_ffsl(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_clzl") == 0 ||
               strcmp(ir_func_decl->name, "__kefir_builtin_clzll") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_clzl(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_ctzl") == 0 ||
               strcmp(ir_func_decl->name, "__kefir_builtin_ctzll") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_ctzl(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_clrsbl") == 0 ||
               strcmp(ir_func_decl->name, "__kefir_builtin_clrsbll") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_clrsbl(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_popcountl") == 0 ||
               strcmp(ir_func_decl->name, "__kefir_builtin_popcountll") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_popcountl(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_parityl") == 0 ||
               strcmp(ir_func_decl->name, "__kefir_builtin_parityll") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_parityl(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_bswap16") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_bswap16(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_bswap16") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_bswap16(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_bswap32") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_bswap32(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_bswap64") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_bswap64(mem, function, instruction, call_node));
    } else if (strcmp(ir_func_decl->name, "__kefir_builtin_flt_rounds") == 0) {
        ASSIGN_PTR(found_builtin, true);
        REQUIRE_OK(translate_flt_rounds(mem, function, instruction, call_node));
    } else {
        ASSIGN_PTR(found_builtin, false);
    }
    return KEFIR_OK;
}
