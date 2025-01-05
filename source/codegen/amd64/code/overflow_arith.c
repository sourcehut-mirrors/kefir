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

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(add_overflow)(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64_function *function,
                                                                  const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t overflow_flag_vreg, arg1_vreg, arg1_tmp_vreg, result_vreg, arg2_vreg,
        arg2_tmp_vreg, result_ptr_vreg, tmp_vreg, tmp_flag_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &overflow_flag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg1_tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg2_tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_flag_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[2], &result_ptr_vreg));

    const struct kefir_ir_type *args_type =
        kefir_ir_module_get_named_type(function->module->ir_module, instruction->operation.parameters.type.type_id);
    REQUIRE(args_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type"));

    const struct kefir_ir_typeentry *args_typeentry =
        kefir_ir_type_at(args_type, instruction->operation.parameters.type.type_index);
    REQUIRE(args_typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type entry"));
    const kefir_ir_typecode_t arg1_type = args_typeentry->typecode;
    args_typeentry = kefir_ir_type_at(args_type, instruction->operation.parameters.type.type_index + 1);
    REQUIRE(args_typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type entry"));
    const kefir_ir_typecode_t arg2_type = args_typeentry->typecode;
    args_typeentry = kefir_ir_type_at(args_type, instruction->operation.parameters.type.type_index + 2);
    REQUIRE(args_typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type entry"));
    const kefir_ir_typecode_t result_type = args_typeentry->typecode;

    const kefir_bool_t arg1_signed = instruction->operation.parameters.overflow_arith.signedness & 1;
    const kefir_bool_t arg2_signed = (instruction->operation.parameters.overflow_arith.signedness >> 1) & 1;
    const kefir_bool_t result_signed = (instruction->operation.parameters.overflow_arith.signedness >> 2) & 1;

    kefir_bool_t left_ulong = false;
    kefir_bool_t right_ulong = false;
    switch (arg1_type) {
        case KEFIR_IR_TYPE_INT8:
        case KEFIR_IR_TYPE_BOOL:
        case KEFIR_IR_TYPE_CHAR:
            if (arg1_signed) {
                REQUIRE_OK(kefir_asmcmp_amd64_movsx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(arg1_tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG8(arg1_vreg), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_movzx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(arg1_tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG8(arg1_vreg), NULL));
            }
            break;

        case KEFIR_IR_TYPE_INT16:
        case KEFIR_IR_TYPE_SHORT:
            if (arg1_signed) {
                REQUIRE_OK(kefir_asmcmp_amd64_movsx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(arg1_tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG16(arg1_vreg), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_movzx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(arg1_tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG16(arg1_vreg), NULL));
            }
            break;

        case KEFIR_IR_TYPE_INT32:
        case KEFIR_IR_TYPE_INT:
            if (arg1_signed) {
                REQUIRE_OK(kefir_asmcmp_amd64_movsx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(arg1_tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG32(arg1_vreg), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG32(arg1_tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG32(arg1_vreg), NULL));
            }
            break;

        case KEFIR_IR_TYPE_INT64:
        case KEFIR_IR_TYPE_LONG:
        case KEFIR_IR_TYPE_WORD:
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), arg1_tmp_vreg,
                arg1_vreg, NULL));
            left_ulong = !arg1_signed;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected overflow arithmetics argument type");
    }

    switch (arg2_type) {
        case KEFIR_IR_TYPE_INT8:
        case KEFIR_IR_TYPE_BOOL:
        case KEFIR_IR_TYPE_CHAR:
            if (arg2_signed) {
                REQUIRE_OK(kefir_asmcmp_amd64_movsx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(arg2_tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG8(arg2_vreg), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_movzx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(arg2_tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG8(arg2_vreg), NULL));
            }
            break;

        case KEFIR_IR_TYPE_INT16:
        case KEFIR_IR_TYPE_SHORT:
            if (arg2_signed) {
                REQUIRE_OK(kefir_asmcmp_amd64_movsx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(arg2_tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG16(arg2_vreg), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_movzx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(arg2_tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG16(arg2_vreg), NULL));
            }
            break;

        case KEFIR_IR_TYPE_INT32:
        case KEFIR_IR_TYPE_INT:
            if (arg2_signed) {
                REQUIRE_OK(kefir_asmcmp_amd64_movsx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(arg2_tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG32(arg2_vreg), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG32(arg2_tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG32(arg2_vreg), NULL));
            }
            break;

        case KEFIR_IR_TYPE_INT64:
        case KEFIR_IR_TYPE_LONG:
        case KEFIR_IR_TYPE_WORD:
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), arg2_tmp_vreg,
                arg2_vreg, NULL));
            right_ulong = !arg2_signed;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected overflow arithmetics argument type");
    }

    kefir_bool_t result_ulong = false;
    if (!result_signed && (result_type == KEFIR_IR_TYPE_INT64 || result_type == KEFIR_IR_TYPE_LONG ||
                           result_type == KEFIR_IR_TYPE_WORD)) {
        result_ulong = true;
    }

    if (!result_ulong) {
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             result_vreg, arg1_tmp_vreg, NULL));
        REQUIRE_OK(kefir_asmcmp_amd64_add(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_tmp_vreg), NULL));
        if (left_ulong && right_ulong) {
            REQUIRE_OK(kefir_asmcmp_amd64_sets(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_cmp(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setb(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(tmp_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_or(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp_flag_vreg), NULL));
        } else if (left_ulong) {
            REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(0x8000000000000000ull), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_cmp(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg1_tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setb(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
        } else if (right_ulong) {
            REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(0x8000000000000000ull), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_cmp(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setb(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_seto(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
        }
    } else {
        if (left_ulong && right_ulong) {
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                arg1_tmp_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setc(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
        } else if (right_ulong) {
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                arg1_tmp_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_xor(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(0x8000000000000000ull), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg2_tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seto(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
        } else if (left_ulong) {
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                arg2_tmp_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_xor(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(0x8000000000000000ull), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg1_tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg1_tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seto(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                arg1_tmp_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg1_tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_sar(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(63), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_sar(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_UINT(63), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_adc(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
        }
    }

    kefir_bool_t result_64bit = false;
    switch (result_type) {
        case KEFIR_IR_TYPE_INT8:
        case KEFIR_IR_TYPE_BOOL:
        case KEFIR_IR_TYPE_CHAR:
            if (result_signed) {
                REQUIRE_OK(kefir_asmcmp_amd64_movsx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_movzx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            }
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_ptr_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_8BIT),
                &KEFIR_ASMCMP_MAKE_VREG8(result_vreg), NULL));
            break;

        case KEFIR_IR_TYPE_INT16:
        case KEFIR_IR_TYPE_SHORT:
            if (result_signed) {
                REQUIRE_OK(kefir_asmcmp_amd64_movsx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG16(result_vreg), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_movzx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG16(result_vreg), NULL));
            }
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_ptr_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_16BIT),
                &KEFIR_ASMCMP_MAKE_VREG16(result_vreg), NULL));
            break;

        case KEFIR_IR_TYPE_INT32:
        case KEFIR_IR_TYPE_INT:
            if (result_signed) {
                REQUIRE_OK(kefir_asmcmp_amd64_movsx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), NULL));
            }
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_ptr_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT),
                &KEFIR_ASMCMP_MAKE_VREG32(result_vreg), NULL));
            break;

        case KEFIR_IR_TYPE_INT64:
        case KEFIR_IR_TYPE_LONG:
        case KEFIR_IR_TYPE_WORD:
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_ptr_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), NULL));
            result_64bit = true;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected overflow arithmetics argument type");
    }

    if (!result_64bit) {
        REQUIRE_OK(
            kefir_asmcmp_amd64_cmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                   &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
        REQUIRE_OK(kefir_asmcmp_amd64_setne(mem, &function->code,
                                            kefir_asmcmp_context_instr_tail(&function->code.context),
                                            &KEFIR_ASMCMP_MAKE_VREG8(tmp_flag_vreg), NULL));
        REQUIRE_OK(kefir_asmcmp_amd64_or(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg),
                                         &KEFIR_ASMCMP_MAKE_VREG8(tmp_flag_vreg), NULL));
    }
    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_UINT(1), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, overflow_flag_vreg));
    return KEFIR_OK;
}
