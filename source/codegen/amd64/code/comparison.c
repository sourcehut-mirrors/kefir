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
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(int_comparison)(struct kefir_mem *mem,
                                                                    struct kefir_codegen_amd64_function *function,
                                                                    const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    switch (instruction->operation.parameters.comparison) {
#define DEFINE_COMPARISON(_opcode, _op, _variant)                                                                      \
    case (_opcode):                                                                                                    \
        do {                                                                                                           \
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;                                   \
            REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0],       \
                                                            &arg1_vreg));                                              \
            REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1],       \
                                                            &arg2_vreg));                                              \
                                                                                                                       \
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(                                                              \
                mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));           \
                                                                                                                       \
            REQUIRE_OK(                                                                                                \
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), \
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_INT(0), NULL));      \
            REQUIRE_OK(kefir_asmcmp_amd64_cmp(                                                                         \
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),                        \
                &KEFIR_ASMCMP_MAKE_VREG##_variant(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG##_variant(arg2_vreg), NULL));    \
            REQUIRE_OK(kefir_asmcmp_amd64_##_op(mem, &function->code,                                                  \
                                                kefir_asmcmp_context_instr_tail(&function->code.context),              \
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));                         \
                                                                                                                       \
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));         \
        } while (false);                                                                                               \
        break

        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT8_EQUALS, sete, 8);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT16_EQUALS, sete, 16);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT32_EQUALS, sete, 32);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT64_EQUALS, sete, 64);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT8_NOT_EQUALS, setne, 8);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT16_NOT_EQUALS, setne, 16);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT32_NOT_EQUALS, setne, 32);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT64_NOT_EQUALS, setne, 64);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT8_GREATER, setg, 8);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT16_GREATER, setg, 16);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT32_GREATER, setg, 32);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT64_GREATER, setg, 64);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT8_GREATER_OR_EQUALS, setge, 8);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT16_GREATER_OR_EQUALS, setge, 16);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT32_GREATER_OR_EQUALS, setge, 32);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT64_GREATER_OR_EQUALS, setge, 64);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT8_LESSER, setl, 8);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT16_LESSER, setl, 16);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT32_LESSER, setl, 32);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT64_LESSER, setl, 64);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT8_LESSER_OR_EQUALS, setle, 8);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT16_LESSER_OR_EQUALS, setle, 16);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT32_LESSER_OR_EQUALS, setle, 32);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT64_LESSER_OR_EQUALS, setle, 64);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT8_ABOVE, seta, 8);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT16_ABOVE, seta, 16);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT32_ABOVE, seta, 32);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT64_ABOVE, seta, 64);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT8_ABOVE_OR_EQUALS, setae, 8);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT16_ABOVE_OR_EQUALS, setae, 16);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT32_ABOVE_OR_EQUALS, setae, 32);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT64_ABOVE_OR_EQUALS, setae, 64);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT8_BELOW, setb, 8);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT16_BELOW, setb, 16);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT32_BELOW, setb, 32);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT64_BELOW, setb, 64);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT8_BELOW_OR_EQUALS, setbe, 8);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT16_BELOW_OR_EQUALS, setbe, 16);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT32_BELOW_OR_EQUALS, setbe, 32);
        DEFINE_COMPARISON(KEFIR_OPT_COMPARISON_INT64_BELOW_OR_EQUALS, setbe, 64);

#undef DEFINE_COMPARISON

        case KEFIR_OPT_COMPARISON_FLOAT32_EQUAL: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg, tmp_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));

            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_ucomiss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_setnp(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_cmovne(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_EQUAL: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg, tmp_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(1), NULL));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));

            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_ucomiss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_setp(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_cmovne(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT32_GREATER: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_comiss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seta(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT32_GREATER_OR_EQUAL: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_comiss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setae(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT32_LESSER: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_comiss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seta(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT32_LESSER_OR_EQUAL: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_comiss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setae(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_ucomiss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setp(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setbe(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER_OR_EQUAL: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_ucomiss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setp(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setb(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_ucomiss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setp(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setbe(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER_OR_EQUAL: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_ucomiss(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setp(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setb(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT64_EQUAL: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg, tmp_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));

            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_ucomisd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_setnp(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_cmovne(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_EQUAL: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg, tmp_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(1), NULL));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));

            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_ucomisd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_setp(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_cmovne(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT64_GREATER: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_comisd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seta(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT64_GREATER_OR_EQUAL: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_comisd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setae(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT64_LESSER: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_comisd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seta(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT64_LESSER_OR_EQUAL: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_comisd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setae(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_ucomisd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setp(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setbe(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER_OR_EQUAL: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_ucomisd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setp(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setb(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_ucomisd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setp(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setbe(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER_OR_EQUAL: {
            kefir_asmcmp_virtual_register_index_t result_vreg, arg1_vreg, arg2_vreg;
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
            REQUIRE_OK(
                kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(
                kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_UINT(0), NULL));
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&function->stack_frame));
            REQUIRE_OK(kefir_asmcmp_amd64_ucomisd(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(arg2_vreg), &KEFIR_ASMCMP_MAKE_VREG(arg1_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setp(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setb(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected comparison operator type");
    }
    return KEFIR_OK;
}
