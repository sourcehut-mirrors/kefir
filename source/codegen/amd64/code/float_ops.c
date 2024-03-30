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
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_to_float32)(struct kefir_mem *mem,
                                                                    struct kefir_codegen_amd64_function *function,
                                                                    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));

    REQUIRE_OK(
        kefir_asmcmp_amd64_cvtsi2ss(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_to_float64)(struct kefir_mem *mem,
                                                                    struct kefir_codegen_amd64_function *function,
                                                                    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));

    REQUIRE_OK(
        kefir_asmcmp_amd64_cvtsi2sd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(uint_to_float)(struct kefir_mem *mem,
                                                                   struct kefir_codegen_amd64_function *function,
                                                                   const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg, tmp_vreg, tmp2_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp2_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));

    kefir_asmcmp_label_index_t sign_label;
    kefir_asmcmp_label_index_t nosign_label;
    REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &sign_label));
    REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &nosign_label));

    REQUIRE_OK(kefir_asmcmp_amd64_pxor(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                                       NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_test(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(arg_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_js(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(sign_label), NULL));

    if (instruction->operation.opcode == KEFIR_OPT_OPCODE_UINT_TO_FLOAT32) {
        REQUIRE_OK(kefir_asmcmp_amd64_cvtsi2ss(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg_vreg), NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_cvtsi2sd(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg_vreg), NULL));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(nosign_label), NULL));

    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, sign_label));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg, arg_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp2_vreg, arg_vreg, NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_INT(1), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(tmp2_vreg), &KEFIR_ASMCMP_MAKE_INT(1), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_or(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp2_vreg), NULL));

    if (instruction->operation.opcode == KEFIR_OPT_OPCODE_UINT_TO_FLOAT32) {
        REQUIRE_OK(kefir_asmcmp_amd64_cvtsi2ss(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));

        REQUIRE_OK(
            kefir_asmcmp_amd64_addss(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(result_vreg), NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_cvtsi2sd(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));

        REQUIRE_OK(
            kefir_asmcmp_amd64_addsd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(result_vreg), NULL));
    }

    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, nosign_label));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(float32_to_int)(struct kefir_mem *mem,
                                                                    struct kefir_codegen_amd64_function *function,
                                                                    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));

    REQUIRE_OK(
        kefir_asmcmp_amd64_cvttss2si(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(float64_to_int)(struct kefir_mem *mem,
                                                                    struct kefir_codegen_amd64_function *function,
                                                                    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));

    REQUIRE_OK(
        kefir_asmcmp_amd64_cvttsd2si(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(float_to_uint)(struct kefir_mem *mem,
                                                                   struct kefir_codegen_amd64_function *function,
                                                                   const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, result_placement_vreg, arg_vreg, arg_placement_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg_placement_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, result_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_RAX));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, arg_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_XMM0));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         arg_placement_vreg, arg_vreg, NULL));

    const char *symbolic_label;
    const char *conv_procedure = instruction->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_TO_UINT
                                     ? KEFIR_AMD64_RUNTIME_FLOAT32_TO_UINT
                                     : KEFIR_AMD64_RUNTIME_FLOAT64_TO_UINT;
    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_format(mem, &function->code.context, &symbolic_label, KEFIR_AMD64_PLT, conv_procedure));
    } else {
        symbolic_label = conv_procedure;
    }
    REQUIRE_OK(kefir_asmcmp_amd64_call(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(symbolic_label, 0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_placement_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_vreg, result_placement_vreg, NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(float_to_float)(struct kefir_mem *mem,
                                                                    struct kefir_codegen_amd64_function *function,
                                                                    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));

    if (instruction->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_TO_FLOAT64) {
        REQUIRE_OK(
            kefir_asmcmp_amd64_cvtss2sd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg_vreg), NULL));
    } else {
        REQUIRE_OK(
            kefir_asmcmp_amd64_cvtsd2ss(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg_vreg), NULL));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(float32_arith_op)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg, arg1_vreg, NULL));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_FLOAT32_ADD:
            REQUIRE_OK(kefir_asmcmp_amd64_addss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_SUB:
            REQUIRE_OK(kefir_asmcmp_amd64_subss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_MUL:
            REQUIRE_OK(kefir_asmcmp_amd64_mulss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            break;

        case KEFIR_OPT_OPCODE_FLOAT32_DIV:
            REQUIRE_OK(kefir_asmcmp_amd64_divss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected instruction opcode");
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(float64_arith_op)(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg, arg1_vreg, NULL));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_FLOAT64_ADD:
            REQUIRE_OK(kefir_asmcmp_amd64_addsd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_SUB:
            REQUIRE_OK(kefir_asmcmp_amd64_subsd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_MUL:
            REQUIRE_OK(kefir_asmcmp_amd64_mulsd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_DIV:
            REQUIRE_OK(kefir_asmcmp_amd64_divsd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected instruction opcode");
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(float_unary_op)(struct kefir_mem *mem,
                                                                    struct kefir_codegen_amd64_function *function,
                                                                    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg, arg1_vreg, NULL));

    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_FLOAT32_NEG:
            if (function->codegen->config->position_independent_code) {
                REQUIRE_OK(kefir_asmcmp_amd64_xorps(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                    &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_AMD64_CONSTANT_FLOAT32_NEG,
                                                             KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                    NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_xorps(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                    &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_AMD64_CONSTANT_FLOAT32_NEG, 0,
                                                               KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                    NULL));
            }
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_NEG:
            if (function->codegen->config->position_independent_code) {
                REQUIRE_OK(kefir_asmcmp_amd64_xorpd(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                    &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_AMD64_CONSTANT_FLOAT64_NEG,
                                                             KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                    NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_xorpd(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                    &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_AMD64_CONSTANT_FLOAT64_NEG, 0,
                                                               KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                    NULL));
            }
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected instruction opcode");
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_FUSION_IMPL(float_comparison)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction, kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *),
    void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));
    REQUIRE(callback != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen instruction fusion callback"));

    struct kefir_codegen_amd64_comparison_match_op fused_comparison_op;
    REQUIRE_OK(
        kefir_codegen_amd64_match_comparison_op(&function->function->code, instruction->id, &fused_comparison_op));
    switch (fused_comparison_op.type) {
        case KEFIR_CODEGEN_AMD64_COMPARISON_NONE:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL_CONST:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 codegen comparison operation");

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL:
            REQUIRE_OK(callback(fused_comparison_op.refs[0], payload));
            REQUIRE_OK(callback(fused_comparison_op.refs[1], payload));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST:
            REQUIRE_OK(callback(fused_comparison_op.refs[0], payload));
            break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_util_translate_float_comparison(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction, const struct kefir_codegen_amd64_comparison_match_op *match_op) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));
    REQUIRE(match_op != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 codegen comparison match"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg, tmp_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, match_op->refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));

    switch (match_op->type) {
        case KEFIR_CODEGEN_AMD64_COMPARISON_NONE:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_ABOVE_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_INT_BELOW_OR_EQUAL_CONST:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 codegen comparison type");

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL:
            REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, match_op->refs[1], &arg2_vreg));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST: {
            kefir_asmcmp_label_index_t label;
            REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label));

            REQUIRE_OK(kefir_hashtree_insert(mem, &function->constants, (kefir_hashtree_key_t) label,
                                             (kefir_hashtree_value_t) match_op->refs[1]));

            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg2_vreg));

            if (function->codegen->config->position_independent_code) {
                REQUIRE_OK(kefir_asmcmp_amd64_movd(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg),
                    &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_INTERNAL(label, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_movd(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg),
                    &KEFIR_ASMCMP_MAKE_INDIRECT_INTERNAL_LABEL(label, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
            }
        } break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST: {
            kefir_asmcmp_label_index_t label;
            REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label));

            REQUIRE_OK(kefir_hashtree_insert(mem, &function->constants, (kefir_hashtree_key_t) label,
                                             (kefir_hashtree_value_t) match_op->refs[1]));

            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg2_vreg));

            if (function->codegen->config->position_independent_code) {
                REQUIRE_OK(kefir_asmcmp_amd64_movq(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg),
                    &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_INTERNAL(label, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_movq(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg),
                    &KEFIR_ASMCMP_MAKE_INDIRECT_INTERNAL_LABEL(label, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
            }
        } break;
    }

    switch (match_op->type) {
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL_CONST:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_ucomiss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL_CONST:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(1), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_ucomiss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL_CONST:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_ucomisd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL_CONST:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(1), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_ucomisd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_comiss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_comisd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_comiss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_comisd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 codegen comparison type");
    }

    switch (match_op->type) {
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_EQUAL_CONST: {
            REQUIRE_OK(kefir_asmcmp_amd64_setnp(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_cmovne(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
        } break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_NOT_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_NOT_EQUAL_CONST: {
            REQUIRE_OK(kefir_asmcmp_amd64_setp(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_cmovne(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
        } break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_seta(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            break;

        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_GREATER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT32_LESSER_OR_EQUAL_CONST:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL:
        case KEFIR_CODEGEN_AMD64_COMPARISON_FLOAT64_LESSER_OR_EQUAL_CONST:
            REQUIRE_OK(kefir_asmcmp_amd64_setae(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 codegen comparison type");
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(float_compare)(struct kefir_mem *mem,
                                                                   struct kefir_codegen_amd64_function *function,
                                                                   const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    struct kefir_codegen_amd64_comparison_match_op match_op;
    REQUIRE_OK(kefir_codegen_amd64_match_comparison_op(&function->function->code, instruction->id, &match_op));
    REQUIRE_OK(kefir_codegen_amd64_util_translate_float_comparison(mem, function, instruction, &match_op));

    return KEFIR_OK;
}
