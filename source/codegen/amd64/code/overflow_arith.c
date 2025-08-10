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
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t expand_arg(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                 kefir_ir_typecode_t typecode, kefir_bool_t signed_type,
                                 kefir_asmcmp_virtual_register_index_t dst_vreg,
                                 kefir_asmcmp_virtual_register_index_t src_vreg, kefir_bool_t *ulong) {
    *ulong = false;
    switch (typecode) {
        case KEFIR_IR_TYPE_INT8:
            if (signed_type) {
                REQUIRE_OK(kefir_asmcmp_amd64_movsx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(dst_vreg), &KEFIR_ASMCMP_MAKE_VREG8(src_vreg), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_movzx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(dst_vreg), &KEFIR_ASMCMP_MAKE_VREG8(src_vreg), NULL));
            }
            break;

        case KEFIR_IR_TYPE_INT16:
            if (signed_type) {
                REQUIRE_OK(kefir_asmcmp_amd64_movsx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(dst_vreg), &KEFIR_ASMCMP_MAKE_VREG16(src_vreg), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_movzx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(dst_vreg), &KEFIR_ASMCMP_MAKE_VREG16(src_vreg), NULL));
            }
            break;

        case KEFIR_IR_TYPE_INT32:
            if (signed_type) {
                REQUIRE_OK(kefir_asmcmp_amd64_movsx(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG64(dst_vreg), &KEFIR_ASMCMP_MAKE_VREG32(src_vreg), NULL));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_VREG32(dst_vreg), &KEFIR_ASMCMP_MAKE_VREG32(src_vreg), NULL));
            }
            break;

        case KEFIR_IR_TYPE_INT64:
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), dst_vreg, src_vreg,
                NULL));
            *ulong = !signed_type;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected overflow arithmetics argument type");
    }

    return KEFIR_OK;
}

static kefir_result_t save_result(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                  kefir_ir_typecode_t result_typecode, kefir_bool_t result_signed,
                                  kefir_asmcmp_virtual_register_index_t tmp_vreg,
                                  kefir_asmcmp_virtual_register_index_t result_vreg,
                                  kefir_asmcmp_virtual_register_index_t result_ptr_vreg,
                                  kefir_asmcmp_virtual_register_index_t overflow_flag_vreg,
                                  kefir_asmcmp_virtual_register_index_t tmp_flag_vreg) {
    kefir_bool_t result_64bit = false;
    switch (result_typecode) {
        case KEFIR_IR_TYPE_INT8:
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
    return KEFIR_OK;
}

#define EXTRACT_SIGNEDNESS(_signedness, _sign1, _sign2, _sign3) \
    do {                                                        \
        *(_sign1) = (_signedness) & 1;                          \
        *(_sign2) = ((_signedness) >> 1) & 1;                   \
        *(_sign3) = ((_signedness) >> 2) & 1;                   \
    } while (0)

#define GET_TYPECODES(_function, _instruction, _arg1_type, _arg2_type, _result_type)                           \
    do {                                                                                                       \
        const struct kefir_ir_type *args_type = kefir_ir_module_get_named_type(                                \
            (_function)->module->ir_module, (_instruction)->operation.parameters.type.type_id);                \
        REQUIRE(args_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type"));            \
                                                                                                               \
        const struct kefir_ir_typeentry *args_typeentry =                                                      \
            kefir_ir_type_at(args_type, instruction->operation.parameters.type.type_index);                    \
        REQUIRE(args_typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type entry")); \
        *(_arg1_type) = args_typeentry->typecode;                                                              \
        args_typeentry = kefir_ir_type_at(args_type, instruction->operation.parameters.type.type_index + 1);   \
        REQUIRE(args_typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type entry")); \
        *(_arg2_type) = args_typeentry->typecode;                                                              \
        args_typeentry = kefir_ir_type_at(args_type, instruction->operation.parameters.type.type_index + 2);   \
        REQUIRE(args_typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type entry")); \
        *(_result_type) = args_typeentry->typecode;                                                            \
    } while (0)

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(add_overflow)(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64_function *function,
                                                                  const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t overflow_flag_vreg, arg1_vreg, arg1_expanded_vreg, result_vreg, arg2_vreg,
        arg2_expanded_vreg, result_ptr_vreg, tmp_vreg, tmp_flag_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &overflow_flag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg1_expanded_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg2_expanded_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_flag_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[2], &result_ptr_vreg));

    kefir_ir_typecode_t arg1_type, arg2_type, result_type;
    GET_TYPECODES(function, instruction, &arg1_type, &arg2_type, &result_type);

    kefir_bool_t arg1_signed, arg2_signed, result_signed;
    EXTRACT_SIGNEDNESS(instruction->operation.parameters.overflow_arith.signedness, &arg1_signed, &arg2_signed,
                       &result_signed);

    kefir_bool_t left_ulong = false;
    kefir_bool_t right_ulong = false;

    REQUIRE_OK(expand_arg(mem, function, arg1_type, arg1_signed, arg1_expanded_vreg, arg1_vreg, &left_ulong));
    REQUIRE_OK(expand_arg(mem, function, arg2_type, arg2_signed, arg2_expanded_vreg, arg2_vreg, &right_ulong));

    const kefir_bool_t result_64bit = result_type == KEFIR_IR_TYPE_INT64;
    if (result_signed || !result_64bit) {
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             result_vreg, arg1_expanded_vreg, NULL));
        REQUIRE_OK(kefir_asmcmp_amd64_add(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
        if (left_ulong && right_ulong) {
            REQUIRE_OK(kefir_asmcmp_amd64_sets(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_cmp(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
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
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), NULL));
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
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
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
                arg1_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setc(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
        } else if (right_ulong) {
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                arg1_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_xor(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(0x8000000000000000ull), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seto(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
        } else if (left_ulong) {
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                arg2_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_xor(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(0x8000000000000000ull), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seto(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                arg1_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg,
                arg1_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_sar(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(63), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), overflow_flag_vreg,
                arg2_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_sar(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_UINT(63), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_adc(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
        }
    }

    REQUIRE_OK(save_result(mem, function, result_type, result_signed, tmp_vreg, result_vreg, result_ptr_vreg,
                           overflow_flag_vreg, tmp_flag_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_UINT(1), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, overflow_flag_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(sub_overflow)(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64_function *function,
                                                                  const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t overflow_flag_vreg, arg1_vreg, arg1_expanded_vreg, result_vreg, arg2_vreg,
        arg2_expanded_vreg, result_ptr_vreg, tmp_vreg, tmp_flag_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &overflow_flag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg1_expanded_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg2_expanded_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_flag_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[2], &result_ptr_vreg));

    kefir_ir_typecode_t arg1_type, arg2_type, result_type;
    GET_TYPECODES(function, instruction, &arg1_type, &arg2_type, &result_type);

    kefir_bool_t arg1_signed, arg2_signed, result_signed;
    EXTRACT_SIGNEDNESS(instruction->operation.parameters.overflow_arith.signedness, &arg1_signed, &arg2_signed,
                       &result_signed);

    kefir_bool_t left_ulong = false;
    kefir_bool_t right_ulong = false;

    REQUIRE_OK(expand_arg(mem, function, arg1_type, arg1_signed, arg1_expanded_vreg, arg1_vreg, &left_ulong));
    REQUIRE_OK(expand_arg(mem, function, arg2_type, arg2_signed, arg2_expanded_vreg, arg2_vreg, &right_ulong));

    const kefir_bool_t result_64bit = result_type == KEFIR_IR_TYPE_INT64;
    if (result_signed || !result_64bit) {
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             result_vreg, arg1_expanded_vreg, NULL));
        if (left_ulong && right_ulong) {
            REQUIRE_OK(kefir_asmcmp_amd64_xor(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_sub(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_sbb(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_VREG64(overflow_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg, result_vreg,
                NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_shr(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(63), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_xor(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg), NULL));
        } else if (left_ulong) {
            REQUIRE_OK(kefir_asmcmp_amd64_sub(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(0x8000000000000000ull), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_cmp(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setnb(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
        } else if (right_ulong) {
            REQUIRE_OK(kefir_asmcmp_amd64_sub(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(0x8000000000000000ull), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_cmp(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setb(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_sub(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seto(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
        }
    } else {
        if (left_ulong && right_ulong) {
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                arg1_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_sub(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setb(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
        } else if (left_ulong) {
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                arg1_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_xor(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(0x8000000000000000ull), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_sub(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seto(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
        } else if (right_ulong) {
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                arg1_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_sub(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_sets(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_cmp(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setb(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(tmp_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_or(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp_flag_vreg), NULL));

        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                arg1_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), overflow_flag_vreg,
                arg1_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_sar(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_UINT(63), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg,
                arg2_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_sar(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_UINT(63), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_sub(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_sbb(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp_vreg), NULL));
        }
    }

    REQUIRE_OK(save_result(mem, function, result_type, result_signed, tmp_vreg, result_vreg, result_ptr_vreg,
                           overflow_flag_vreg, tmp_flag_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_UINT(1), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, overflow_flag_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(mul_overflow)(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64_function *function,
                                                                  const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t overflow_flag_vreg, arg1_vreg, arg1_expanded_vreg, result_vreg, arg2_vreg,
        arg2_expanded_vreg, result_ptr_vreg, tmp_vreg, tmp_flag_vreg;

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &overflow_flag_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg1_expanded_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg2_expanded_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_flag_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg1_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &arg2_vreg));
    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[2], &result_ptr_vreg));

    kefir_ir_typecode_t arg1_type, arg2_type, result_type;
    GET_TYPECODES(function, instruction, &arg1_type, &arg2_type, &result_type);

    kefir_bool_t arg1_signed, arg2_signed, result_signed;
    EXTRACT_SIGNEDNESS(instruction->operation.parameters.overflow_arith.signedness, &arg1_signed, &arg2_signed,
                       &result_signed);

    kefir_bool_t left_ulong = false;
    kefir_bool_t right_ulong = false;

    REQUIRE_OK(expand_arg(mem, function, arg1_type, arg1_signed, arg1_expanded_vreg, arg1_vreg, &left_ulong));
    REQUIRE_OK(expand_arg(mem, function, arg2_type, arg2_signed, arg2_expanded_vreg, arg2_vreg, &right_ulong));

    const kefir_bool_t result_64bit = result_type == KEFIR_IR_TYPE_INT64;
    if (result_signed || !result_64bit) {
        if (left_ulong && right_ulong) {
            kefir_asmcmp_virtual_register_index_t overflow_flag_placement_vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE,
                                                         &overflow_flag_placement_vreg));

            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, tmp_vreg,
                                                                          KEFIR_AMD64_XASMGEN_REGISTER_RAX));
            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
                mem, &function->code, overflow_flag_placement_vreg, KEFIR_AMD64_XASMGEN_REGISTER_RDX));

            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg,
                arg1_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                overflow_flag_placement_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mul(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seto(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_sets(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_placement_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_or(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_placement_vreg),
                                             &KEFIR_ASMCMP_MAKE_VREG8(arg2_expanded_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), overflow_flag_vreg,
                overflow_flag_placement_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg, tmp_vreg,
                NULL));
        } else if (left_ulong) {
            kefir_asmcmp_virtual_register_index_t overflow_flag_placement_vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE,
                                                         &overflow_flag_placement_vreg));

            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, tmp_vreg,
                                                                          KEFIR_AMD64_XASMGEN_REGISTER_RAX));
            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
                mem, &function->code, overflow_flag_placement_vreg, KEFIR_AMD64_XASMGEN_REGISTER_RDX));

            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg,
                arg2_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_neg(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_cmovs(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                overflow_flag_placement_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mul(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seto(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(arg1_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                overflow_flag_placement_vreg, arg2_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_shr(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(overflow_flag_placement_vreg), &KEFIR_ASMCMP_MAKE_UINT(63), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_flag_vreg), &KEFIR_ASMCMP_MAKE_UINT(0x7fffffffffffffffull), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_flag_vreg),
                                              &KEFIR_ASMCMP_MAKE_VREG64(overflow_flag_placement_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_cmp(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seta(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_placement_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_or(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_placement_vreg),
                                             &KEFIR_ASMCMP_MAKE_VREG8(arg1_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg, tmp_vreg,
                NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_neg(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_cmovns(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), overflow_flag_vreg,
                overflow_flag_placement_vreg, NULL));
        } else if (right_ulong) {
            kefir_asmcmp_virtual_register_index_t overflow_flag_placement_vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE,
                                                         &overflow_flag_placement_vreg));

            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, tmp_vreg,
                                                                          KEFIR_AMD64_XASMGEN_REGISTER_RAX));
            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
                mem, &function->code, overflow_flag_placement_vreg, KEFIR_AMD64_XASMGEN_REGISTER_RDX));

            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg,
                arg1_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_neg(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_cmovs(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                overflow_flag_placement_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mul(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seto(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                overflow_flag_placement_vreg, arg1_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_shr(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(overflow_flag_placement_vreg), &KEFIR_ASMCMP_MAKE_UINT(63), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_flag_vreg), &KEFIR_ASMCMP_MAKE_UINT(0x7fffffffffffffffull), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_flag_vreg),
                                              &KEFIR_ASMCMP_MAKE_VREG64(overflow_flag_placement_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_cmp(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seta(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_placement_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_or(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_placement_vreg),
                                             &KEFIR_ASMCMP_MAKE_VREG8(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg, tmp_vreg,
                NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_neg(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_cmovns(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), overflow_flag_vreg,
                overflow_flag_placement_vreg, NULL));
        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                arg1_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_imul(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seto(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
        }
    } else {
        if (left_ulong && right_ulong) {
            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, result_vreg,
                                                                          KEFIR_AMD64_XASMGEN_REGISTER_RAX));
            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, tmp_vreg,
                                                                          KEFIR_AMD64_XASMGEN_REGISTER_RDX));

            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                arg1_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mul(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seto(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
        } else if (left_ulong) {
            kefir_asmcmp_virtual_register_index_t tmp2_vreg;
            kefir_asmcmp_virtual_register_index_t overflow_flag_placement_vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE,
                                                         &overflow_flag_placement_vreg));

            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, tmp_vreg,
                                                                          KEFIR_AMD64_XASMGEN_REGISTER_RAX));
            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
                mem, &function->code, overflow_flag_placement_vreg, KEFIR_AMD64_XASMGEN_REGISTER_RDX));

            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg,
                arg2_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_neg(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_cmovs(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                overflow_flag_placement_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mul(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seto(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(arg1_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg, tmp_vreg,
                NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_neg(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setb(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(tmp2_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_sets(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_placement_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_cmovns(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_and(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_placement_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp2_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_or(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_placement_vreg),
                                             &KEFIR_ASMCMP_MAKE_VREG8(arg1_expanded_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), overflow_flag_vreg,
                overflow_flag_placement_vreg, NULL));

        } else if (right_ulong) {
            kefir_asmcmp_virtual_register_index_t tmp2_vreg;
            kefir_asmcmp_virtual_register_index_t overflow_flag_placement_vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp2_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE,
                                                         &overflow_flag_placement_vreg));

            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, tmp_vreg,
                                                                          KEFIR_AMD64_XASMGEN_REGISTER_RAX));
            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
                mem, &function->code, overflow_flag_placement_vreg, KEFIR_AMD64_XASMGEN_REGISTER_RDX));

            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg,
                arg1_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_neg(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_cmovs(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                overflow_flag_placement_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mul(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_seto(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg, tmp_vreg,
                NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_neg(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setb(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(tmp2_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_sets(mem, &function->code,
                                               kefir_asmcmp_context_instr_tail(&function->code.context),
                                               &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_placement_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_cmovns(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_and(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_placement_vreg), &KEFIR_ASMCMP_MAKE_VREG8(tmp2_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_or(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_placement_vreg),
                                             &KEFIR_ASMCMP_MAKE_VREG8(arg2_expanded_vreg), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), overflow_flag_vreg,
                overflow_flag_placement_vreg, NULL));
        } else {
            kefir_asmcmp_label_index_t label_both_sign, label_single_sign, label_multiply, label_set_sign, label_end;

            REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE,
                                                      &label_both_sign));
            REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE,
                                                      &label_single_sign));
            REQUIRE_OK(
                kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label_multiply));
            REQUIRE_OK(
                kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label_set_sign));
            REQUIRE_OK(
                kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &label_end));

            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, result_vreg,
                                                                          KEFIR_AMD64_XASMGEN_REGISTER_RAX));
            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, tmp_vreg,
                                                                          KEFIR_AMD64_XASMGEN_REGISTER_RDX));

            REQUIRE_OK(kefir_asmcmp_amd64_xor(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), result_vreg,
                arg1_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_js(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(label_both_sign), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg,
                arg1_expanded_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_xor(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_js(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(label_single_sign), NULL));

            REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, label_multiply));
            REQUIRE_OK(kefir_asmcmp_amd64_mul(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_jo(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(label_set_sign), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(label_end), NULL));

            REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, label_both_sign));
            REQUIRE_OK(kefir_asmcmp_amd64_neg(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_neg(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(label_multiply), NULL));

            REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, label_set_sign));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_UINT(1), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(label_end), NULL));

            REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, label_single_sign));
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg1_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_je(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(label_multiply), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_xor(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_test(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), &KEFIR_ASMCMP_MAKE_VREG64(arg2_expanded_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_setne(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG8(overflow_flag_vreg), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(label_multiply), NULL));

            REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, label_end));
        }
    }

    REQUIRE_OK(save_result(mem, function, result_type, result_signed, tmp_vreg, result_vreg, result_ptr_vreg,
                           overflow_flag_vreg, tmp_flag_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_and(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG32(overflow_flag_vreg), &KEFIR_ASMCMP_MAKE_UINT(1), NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, overflow_flag_vreg));
    return KEFIR_OK;
}
