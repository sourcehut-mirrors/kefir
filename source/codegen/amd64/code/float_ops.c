/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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
#include "kefir/codegen/amd64/module.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_amd64_function_float_to_int(struct kefir_mem *mem,
                                                         struct kefir_codegen_amd64_function *function,
                                                         kefir_opt_instruction_ref_t arg_ref,
                                                         kefir_opt_instruction_ref_t result_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, arg_ref, &arg_vreg));

    REQUIRE_OK(
        kefir_asmcmp_amd64_cvttss2si(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, result_ref, result_vreg));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_float_to_uint(struct kefir_mem *mem,
                                                          struct kefir_codegen_amd64_function *function,
                                                          kefir_opt_instruction_ref_t arg_ref,
                                                          kefir_opt_instruction_ref_t result_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, arg_ref, &arg_vreg));

    function->codegen_module->constants.float32_to_uint = true;
    kefir_asmcmp_label_index_t overflow_label, no_overflow_label;
    REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &overflow_label));
    REQUIRE_OK(
        kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &no_overflow_label));

    kefir_asmcmp_virtual_register_index_t tmp_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp_vreg));

    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(
            kefir_asmcmp_amd64_comiss(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(arg_vreg),
                                      &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                                               KEFIR_AMD64_CONSTANT_FLOAT32_TO_UINT,
                                                                               KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                      NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_comiss(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(arg_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                       KEFIR_AMD64_CONSTANT_FLOAT32_TO_UINT, 0,
                                                       KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
            NULL));
    }
    REQUIRE_OK(kefir_asmcmp_amd64_jnb(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(overflow_label), NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_cvttss2si(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(no_overflow_label), NULL));

    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, overflow_label));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg, arg_vreg, NULL));
    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(
            kefir_asmcmp_amd64_subss(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
                                     &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                                              KEFIR_AMD64_CONSTANT_FLOAT32_TO_UINT,
                                                                              KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                     NULL));
    } else {
        REQUIRE_OK(
            kefir_asmcmp_amd64_subss(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
                                     &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                                                KEFIR_AMD64_CONSTANT_FLOAT32_TO_UINT, 0,
                                                                                KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                     NULL));
    }
    REQUIRE_OK(
        kefir_asmcmp_amd64_cvttss2si(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_btc(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_INT(63), NULL));
    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, no_overflow_label));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, result_ref, result_vreg));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_double_to_int(struct kefir_mem *mem,
                                                          struct kefir_codegen_amd64_function *function,
                                                          kefir_opt_instruction_ref_t arg_ref,
                                                          kefir_opt_instruction_ref_t result_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, arg_ref, &arg_vreg));

    REQUIRE_OK(
        kefir_asmcmp_amd64_cvttsd2si(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg_vreg), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, result_ref, result_vreg));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_double_to_uint(struct kefir_mem *mem,
                                                           struct kefir_codegen_amd64_function *function,
                                                           kefir_opt_instruction_ref_t arg_ref,
                                                           kefir_opt_instruction_ref_t result_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));

    kefir_asmcmp_virtual_register_index_t result_vreg, arg_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, arg_ref, &arg_vreg));

    function->codegen_module->constants.float64_to_uint = true;
    kefir_asmcmp_label_index_t overflow_label, no_overflow_label;
    REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &overflow_label));
    REQUIRE_OK(
        kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &no_overflow_label));

    kefir_asmcmp_virtual_register_index_t tmp_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp_vreg));

    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(
            kefir_asmcmp_amd64_comisd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(arg_vreg),
                                      &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                                               KEFIR_AMD64_CONSTANT_FLOAT64_TO_UINT,
                                                                               KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                      NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_comisd(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(arg_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                       KEFIR_AMD64_CONSTANT_FLOAT64_TO_UINT, 0,
                                                       KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
            NULL));
    }
    REQUIRE_OK(kefir_asmcmp_amd64_jnb(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(overflow_label), NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_cvttsd2si(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(no_overflow_label), NULL));

    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, overflow_label));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg, arg_vreg, NULL));
    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(
            kefir_asmcmp_amd64_subsd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
                                     &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                                              KEFIR_AMD64_CONSTANT_FLOAT64_TO_UINT,
                                                                              KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                     NULL));
    } else {
        REQUIRE_OK(
            kefir_asmcmp_amd64_subsd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
                                     &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                                                KEFIR_AMD64_CONSTANT_FLOAT64_TO_UINT, 0,
                                                                                KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                     NULL));
    }
    REQUIRE_OK(
        kefir_asmcmp_amd64_cvttsd2si(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_btc(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_INT(63), NULL));
    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, no_overflow_label));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, result_ref, result_vreg));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_int_to_float(struct kefir_mem *mem,
                                                         struct kefir_codegen_amd64_function *function,
                                                         kefir_asmcmp_instruction_index_t arg_vreg,
                                                         kefir_opt_instruction_ref_t result_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));

    REQUIRE_OK(
        kefir_asmcmp_amd64_cvtsi2ss(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg_vreg), NULL));
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, result_ref, result_vreg));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_int_to_double(struct kefir_mem *mem,
                                                          struct kefir_codegen_amd64_function *function,
                                                          kefir_asmcmp_virtual_register_index_t arg_vreg,
                                                          kefir_opt_instruction_ref_t result_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));

    REQUIRE_OK(
        kefir_asmcmp_amd64_cvtsi2sd(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg_vreg), NULL));
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, result_ref, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t kefir_codegen_amd64_function_uint_to_float_impl(struct kefir_mem *mem,
                                                                      struct kefir_codegen_amd64_function *function,
                                                                      kefir_asmcmp_virtual_register_index_t arg_vreg,
                                                                      kefir_opt_instruction_ref_t result_ref,
                                                                      kefir_bool_t float32) {
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));

    kefir_asmcmp_virtual_register_index_t result_vreg, tmp_vreg, tmp2_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp2_vreg));

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

    if (float32) {
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

    if (float32) {
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
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, result_ref, result_vreg));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_uint_to_float(struct kefir_mem *mem,
                                                          struct kefir_codegen_amd64_function *function,
                                                          kefir_asmcmp_virtual_register_index_t arg_vreg,
                                                          kefir_opt_instruction_ref_t result_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));

    REQUIRE_OK(kefir_codegen_amd64_function_uint_to_float_impl(mem, function, arg_vreg, result_ref, true));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_uint_to_double(struct kefir_mem *mem,
                                                           struct kefir_codegen_amd64_function *function,
                                                           kefir_asmcmp_virtual_register_index_t arg_vreg,
                                                           kefir_opt_instruction_ref_t result_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));

    REQUIRE_OK(kefir_codegen_amd64_function_uint_to_float_impl(mem, function, arg_vreg, result_ref, false));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_to_float32)(struct kefir_mem *mem,
                                                                    struct kefir_codegen_amd64_function *function,
                                                                    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_int_to_float(mem, function, arg_vreg, instruction->id));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_to_float64)(struct kefir_mem *mem,
                                                                    struct kefir_codegen_amd64_function *function,
                                                                    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_int_to_double(mem, function, arg_vreg, instruction->id));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(uint_to_float)(struct kefir_mem *mem,
                                                                   struct kefir_codegen_amd64_function *function,
                                                                   const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_uint_to_float_impl(
        mem, function, arg_vreg, instruction->id, instruction->operation.opcode == KEFIR_OPT_OPCODE_UINT_TO_FLOAT32));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(float32_to_int)(struct kefir_mem *mem,
                                                                    struct kefir_codegen_amd64_function *function,
                                                                    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_function_float_to_int(mem, function, instruction->operation.parameters.refs[0],
                                                         instruction->id));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(float64_to_int)(struct kefir_mem *mem,
                                                                    struct kefir_codegen_amd64_function *function,
                                                                    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    REQUIRE_OK(kefir_codegen_amd64_function_double_to_int(mem, function, instruction->operation.parameters.refs[0],
                                                          instruction->id));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(float_to_uint)(struct kefir_mem *mem,
                                                                   struct kefir_codegen_amd64_function *function,
                                                                   const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    if (instruction->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_TO_UINT) {
        REQUIRE_OK(kefir_codegen_amd64_function_float_to_uint(mem, function, instruction->operation.parameters.refs[0],
                                                              instruction->id));
    } else {
        REQUIRE_OK(kefir_codegen_amd64_function_double_to_uint(mem, function, instruction->operation.parameters.refs[0],
                                                               instruction->id));
    }
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
            function->codegen_module->constants.float32_neg = true;
            if (function->codegen->config->position_independent_code) {
                REQUIRE_OK(kefir_asmcmp_amd64_xorps(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                    &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                             KEFIR_AMD64_CONSTANT_FLOAT32_NEG,
                                                             KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                    NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_xorps(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                    &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                               KEFIR_AMD64_CONSTANT_FLOAT32_NEG, 0,
                                                               KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                    NULL));
            }
            break;

        case KEFIR_OPT_OPCODE_FLOAT64_NEG:
            function->codegen_module->constants.float64_neg = true;
            if (function->codegen->config->position_independent_code) {
                REQUIRE_OK(kefir_asmcmp_amd64_xorpd(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                    &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                             KEFIR_AMD64_CONSTANT_FLOAT64_NEG,
                                                             KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                    NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_xorpd(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
                    &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE,
                                                               KEFIR_AMD64_CONSTANT_FLOAT64_NEG, 0,
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
