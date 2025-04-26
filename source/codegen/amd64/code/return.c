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

#include "kefir/codegen/amd64/asmcmp.h"
#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/function.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_amd64_return_from_function(struct kefir_mem *mem,
                                                        struct kefir_codegen_amd64_function *function,
                                                        kefir_asmcmp_virtual_register_index_t return_vreg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));

    REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));

    kefir_asmcmp_virtual_register_index_t vreg = KEFIR_ASMCMP_INDEX_NONE;
    if (kefir_ir_type_length(function->function->ir_func->declaration->result) > 0) {
        const struct kefir_abi_amd64_function_parameters *function_returns;
        REQUIRE_OK(kefir_abi_amd64_function_decl_returns(&function->abi_function_declaration, &function_returns));

        struct kefir_abi_amd64_function_parameter function_return;
        REQUIRE_OK(kefir_abi_amd64_function_parameters_at(function_returns, 0, &function_return));

        switch (function_return.location) {
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE:
                // Intentionally left blank
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER:
                REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                             KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, vreg,
                                                                              function_return.direct_reg));
                if (return_vreg != KEFIR_ID_NONE) {
                    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), vreg,
                        return_vreg, NULL));
                } else {
                    REQUIRE_OK(kefir_asmcmp_amd64_xor(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG64(vreg), &KEFIR_ASMCMP_MAKE_VREG64(vreg), NULL));
                }
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
                REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                             KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, vreg,
                                                                              function_return.direct_reg));
                if (return_vreg != KEFIR_ID_NONE) {
                    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), vreg,
                        return_vreg, NULL));
                } else {
                    REQUIRE_OK(kefir_asmcmp_amd64_pxor(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG(vreg), &KEFIR_ASMCMP_MAKE_VREG(vreg), NULL));
                }
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS: {
                kefir_size_t length;
                REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_length(&function_return, &length));
                for (kefir_size_t i = 0; i < length; i++) {
                    struct kefir_abi_amd64_function_parameter subparam;
                    REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_at(&function_return, i, &subparam));

                    switch (subparam.location) {
                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE:
                            // Intentionally left blank
                            break;

                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER:
                            REQUIRE_OK(kefir_asmcmp_virtual_register_new(
                                mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &vreg));
                            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, vreg,
                                                                                          subparam.direct_reg));
                            if (return_vreg != KEFIR_ID_NONE) {
                                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG64(vreg),
                                    &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(return_vreg, i * KEFIR_AMD64_ABI_QWORD,
                                                                        KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                                    NULL));
                            } else {
                                REQUIRE_OK(kefir_asmcmp_amd64_xor(
                                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG64(vreg), &KEFIR_ASMCMP_MAKE_VREG64(vreg), NULL));
                            }
                            break;

                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
                            REQUIRE_OK(kefir_asmcmp_virtual_register_new(
                                mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &vreg));
                            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, vreg,
                                                                                          subparam.direct_reg));
                            if (return_vreg != KEFIR_ID_NONE) {
                                REQUIRE_OK(kefir_asmcmp_amd64_movq(
                                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(vreg),
                                    &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(return_vreg, i * KEFIR_AMD64_ABI_QWORD,
                                                                        KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                    NULL));
                            } else {
                                REQUIRE_OK(kefir_asmcmp_amd64_pxor(
                                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(vreg), &KEFIR_ASMCMP_MAKE_VREG(vreg), NULL));
                            }
                            break;

                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87:
                            REQUIRE(i + 1 < length,
                                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                                    "Expected X87 qword to be directly followed by X87UP"));
                            REQUIRE_OK(
                                kefir_abi_amd64_function_parameter_multireg_at(&function_return, ++i, &subparam));
                            REQUIRE(subparam.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP,
                                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                                    "Expected X87 qword to be directly followed by X87UP"));
                            if (return_vreg != KEFIR_ID_NONE) {
                                REQUIRE_OK(kefir_asmcmp_amd64_fld(
                                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(return_vreg, (i - 1) * KEFIR_AMD64_ABI_QWORD,
                                                                        KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
                                    NULL));
                            } else {
                                REQUIRE_OK(kefir_asmcmp_amd64_fldz(
                                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    NULL));
                            }
                            break;

                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_COMPLEX_X87:
                            if (return_vreg != KEFIR_ID_NONE) {
                                REQUIRE_OK(kefir_asmcmp_amd64_fld(
                                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(return_vreg, (i + 2) * KEFIR_AMD64_ABI_QWORD,
                                                                        KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
                                    NULL));
                                REQUIRE_OK(kefir_asmcmp_amd64_fld(
                                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(return_vreg, i * KEFIR_AMD64_ABI_QWORD,
                                                                        KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
                                    NULL));
                            } else {
                                REQUIRE_OK(kefir_asmcmp_amd64_fldz(
                                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    NULL));
                                REQUIRE_OK(kefir_asmcmp_amd64_fldz(
                                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    NULL));
                            }
                            break;

                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP:
                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY:
                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS:
                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NESTED:
                            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected return location");
                    }
                }
            } break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY:
                REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                             KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, vreg,
                                                                              KEFIR_AMD64_XASMGEN_REGISTER_RAX));
                REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), vreg,
                    function->return_address_vreg, NULL));

                if (return_vreg != KEFIR_ID_NONE) {
                    const struct kefir_abi_amd64_type_layout *return_layout;
                    const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
                    REQUIRE_OK(kefir_abi_amd64_function_decl_returns_layout(&function->abi_function_declaration,
                                                                            &return_layout));
                    REQUIRE_OK(kefir_abi_amd64_type_layout_at(return_layout, 0, &layout));

                    REQUIRE_OK(kefir_codegen_amd64_copy_memory(mem, function, vreg, return_vreg, layout->size));
                }
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87:
                if (return_vreg != KEFIR_ID_NONE) {
                    REQUIRE_OK(kefir_asmcmp_amd64_fld(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(return_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
                } else {
                    REQUIRE_OK(kefir_asmcmp_amd64_fldz(mem, &function->code,
                                                       kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
                }
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_COMPLEX_X87:
                if (return_vreg != KEFIR_ID_NONE) {
                    REQUIRE_OK(kefir_asmcmp_amd64_fld(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(return_vreg, 2 * KEFIR_AMD64_ABI_QWORD,
                                                            KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
                        NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_fld(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(return_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
                } else {
                    REQUIRE_OK(kefir_asmcmp_amd64_fldz(mem, &function->code,
                                                       kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_fldz(mem, &function->code,
                                                       kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
                }
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP:
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NESTED:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected return location");
        }
    }

    REQUIRE_OK(kefir_asmcmp_amd64_function_epilogue(mem, &function->code,
                                                    kefir_asmcmp_context_instr_tail(&function->code.context), NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_ret(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));

    if (vreg != KEFIR_ASMCMP_INDEX_NONE) {
        REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), vreg, NULL));
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(return)(struct kefir_mem *mem,
                                                            struct kefir_codegen_amd64_function *function,
                                                            const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t arg_vreg = KEFIR_ID_NONE;
    if (instruction->operation.parameters.refs[0] != KEFIR_ID_NONE) {
        REQUIRE_OK(
            kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));
    }
    REQUIRE_OK(kefir_codegen_amd64_return_from_function(mem, function, arg_vreg));
    return KEFIR_OK;
}
