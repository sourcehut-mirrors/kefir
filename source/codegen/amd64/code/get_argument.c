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
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(get_argument)(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64_function *function,
                                                                  const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    const kefir_size_t param_type_index = kefir_ir_type_child_index(function->function->ir_func->declaration->params,
                                                                    instruction->operation.parameters.index);
    kefir_size_t param_type_slot;
    REQUIRE_OK(
        kefir_ir_type_slot_of(function->function->ir_func->declaration->params, param_type_index, &param_type_slot));

    const struct kefir_abi_amd64_function_parameters *function_parameters;
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(&function->abi_function_declaration, &function_parameters));

    struct kefir_abi_amd64_function_parameter function_parameter;
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(function_parameters, param_type_slot, &function_parameter));

    kefir_asmcmp_virtual_register_index_t arg_vreg, vreg, vreg2;
    switch (function_parameter.location) {
        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE:
            // Intentionally left blank
            break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, arg_vreg,
                                                                          function_parameter.direct_reg));
            REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(mem, &function->code, function->argument_touch_instr,
                                                                 arg_vreg, &function->argument_touch_instr));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), vreg, arg_vreg, NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, vreg));
            break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, arg_vreg,
                                                                          function_parameter.direct_reg));
            REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(mem, &function->code, function->argument_touch_instr,
                                                                 arg_vreg, &function->argument_touch_instr));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), vreg, arg_vreg, NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, vreg));
            break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS: {
            kefir_size_t qwords;
            REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_length(&function_parameter, &qwords));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(mem, &function->code.context,
                                                                                         qwords, &vreg));

            for (kefir_size_t i = 0; i < qwords; i++) {
                struct kefir_abi_amd64_function_parameter subparam;
                REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_at(&function_parameter, i, &subparam));

                switch (subparam.location) {
                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE:
                        // Intentionally left blank
                        break;

                    case KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE:
                        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
                            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &arg_vreg));
                        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, arg_vreg,
                                                                                      subparam.direct_reg));
                        REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(mem, &function->code,
                                                                             function->argument_touch_instr, arg_vreg,
                                                                             &function->argument_touch_instr));

                        REQUIRE_OK(kefir_asmcmp_amd64_mov(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(vreg, i * KEFIR_AMD64_ABI_QWORD,
                                                                KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                            &KEFIR_ASMCMP_MAKE_VREG64(arg_vreg), NULL));
                        break;

                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
                        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
                            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &arg_vreg));
                        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, arg_vreg,
                                                                                      subparam.direct_reg));
                        REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(mem, &function->code,
                                                                             function->argument_touch_instr, arg_vreg,
                                                                             &function->argument_touch_instr));

                        REQUIRE_OK(kefir_asmcmp_amd64_movq(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(vreg, i * KEFIR_AMD64_ABI_QWORD,
                                                                KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                            &KEFIR_ASMCMP_MAKE_VREG(arg_vreg), NULL));
                        break;

                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87:
                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP:
                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS:
                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY:
                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NESTED:
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected multi-register parameter location");
                }
            }

            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, vreg));
        } break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY: {
            kefir_asm_amd64_xasmgen_register_t base_reg;
            kefir_int64_t offset;
            REQUIRE_OK(kefir_abi_amd64_function_parameter_memory_location(
                &function_parameter, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLEE, &base_reg, &offset));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new_memory_pointer(
                mem, &function->code.context, (kefir_asmcmp_physical_register_index_t) base_reg, offset, &vreg));

            const struct kefir_ir_typeentry *typeentry =
                kefir_ir_type_at(function->function->ir_func->declaration->params, param_type_index);
            REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to fetch IR type entry"));
            switch (typeentry->typecode) {
                case KEFIR_IR_TYPE_STRUCT:
                case KEFIR_IR_TYPE_ARRAY:
                case KEFIR_IR_TYPE_UNION:
                case KEFIR_IR_TYPE_LONG_DOUBLE:
                    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
                        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &vreg2));
                    REQUIRE_OK(kefir_asmcmp_amd64_lea(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG64(vreg2), &KEFIR_ASMCMP_MAKE_VREG(vreg), NULL));
                    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, vreg2));
                    break;

                case KEFIR_IR_TYPE_INT8:
                case KEFIR_IR_TYPE_INT16:
                case KEFIR_IR_TYPE_INT32:
                case KEFIR_IR_TYPE_INT64:
                case KEFIR_IR_TYPE_FLOAT32:
                case KEFIR_IR_TYPE_FLOAT64:
                case KEFIR_IR_TYPE_BOOL:
                case KEFIR_IR_TYPE_CHAR:
                case KEFIR_IR_TYPE_SHORT:
                case KEFIR_IR_TYPE_INT:
                case KEFIR_IR_TYPE_LONG:
                case KEFIR_IR_TYPE_WORD:
                case KEFIR_IR_TYPE_BITS:
                case KEFIR_IR_TYPE_BUILTIN:
                    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, vreg));
                    break;

                case KEFIR_IR_TYPE_NONE:
                case KEFIR_IR_TYPE_COUNT:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR type code");
            }
        } break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87:
        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP:
        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NESTED:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected function parameter location");
    }

    return KEFIR_OK;
}
