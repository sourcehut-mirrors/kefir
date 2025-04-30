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
#include "kefir/target/abi/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t prepare_stack(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                    struct kefir_abi_amd64_function_decl *abi_func_decl,
                                    kefir_size_t *stack_increment) {
    struct kefir_abi_amd64_function_parameter_requirements reqs;
    const struct kefir_abi_amd64_function_parameters *parameters;
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(abi_func_decl, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_requirements(parameters, &reqs));

    *stack_increment = kefir_target_abi_pad_aligned(reqs.stack, 2 * KEFIR_AMD64_ABI_QWORD);

    if (*stack_increment > 0) {
        REQUIRE_OK(kefir_asmcmp_amd64_sub(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                          &KEFIR_ASMCMP_MAKE_UINT(*stack_increment), NULL));
    }
    return KEFIR_OK;
}

static kefir_result_t preserve_regs(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                    const struct kefir_opt_call_node *call_node, kefir_asmcmp_stash_index_t *stash_idx,
                                    kefir_asmcmp_virtual_register_index_t *return_space_vreg) {
    const kefir_size_t num_of_preserved_gp_regs =
        kefir_abi_amd64_num_of_caller_preserved_general_purpose_registers(function->codegen->abi_variant);
    const kefir_size_t num_of_preserved_sse_regs =
        kefir_abi_amd64_num_of_caller_preserved_sse_registers(function->codegen->abi_variant);

    if (call_node->return_space != KEFIR_ID_NONE) {
        kefir_asmcmp_virtual_register_index_t return_space;
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->return_space, &return_space));
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, return_space_vreg));
        kefir_asm_amd64_xasmgen_register_t return_space_placement;
        REQUIRE_OK(kefir_abi_amd64_get_callee_preserved_general_purpose_register(function->codegen->abi_variant, 0,
                                                                                 &return_space_placement));
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, *return_space_vreg,
                                                                      return_space_placement));
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             *return_space_vreg, return_space, NULL));
    } else {
        *return_space_vreg = KEFIR_ASMCMP_INDEX_NONE;
    }

    REQUIRE_OK(kefir_asmcmp_register_stash_new(mem, &function->code.context, stash_idx));

    for (kefir_size_t i = 0; i < num_of_preserved_gp_regs; i++) {
        kefir_asm_amd64_xasmgen_register_t reg;
        REQUIRE_OK(
            kefir_abi_amd64_get_caller_preserved_general_purpose_register(function->codegen->abi_variant, i, &reg));

        REQUIRE_OK(kefir_asmcmp_register_stash_add(mem, &function->code.context, *stash_idx,
                                                   (kefir_asmcmp_physical_register_index_t) reg));
    }

    for (kefir_size_t i = 0; i < num_of_preserved_sse_regs; i++) {
        kefir_asm_amd64_xasmgen_register_t reg;
        REQUIRE_OK(kefir_abi_amd64_get_caller_preserved_sse_register(function->codegen->abi_variant, i, &reg));

        REQUIRE_OK(kefir_asmcmp_register_stash_add(mem, &function->code.context, *stash_idx,
                                                   (kefir_asmcmp_physical_register_index_t) reg));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_activate_stash(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), *stash_idx, NULL));

    return KEFIR_OK;
}

static kefir_result_t restore_regs(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                   kefir_asmcmp_stash_index_t stash_idx) {
    REQUIRE_OK(kefir_asmcmp_amd64_deactivate_stash(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), stash_idx, NULL));

    return KEFIR_OK;
}

static kefir_result_t prepare_parameters(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                         const struct kefir_opt_call_node *call_node,
                                         const struct kefir_ir_function_decl *ir_func_decl,
                                         struct kefir_abi_amd64_function_decl *abi_func_decl,
                                         kefir_asmcmp_virtual_register_index_t return_space_vreg,
                                         struct kefir_hashtree *argument_placement, kefir_bool_t tail_call) {
    const struct kefir_abi_amd64_function_parameters *parameters;
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(abi_func_decl, &parameters));

    kefir_size_t subarg_count = 0;
    for (kefir_size_t i = 0; i < call_node->argument_count; i++, subarg_count++) {
        kefir_asmcmp_virtual_register_index_t argument_vreg, argument_placement_vreg;
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[i], &argument_vreg));

        kefir_size_t parameter_type_index = kefir_ir_type_child_index(ir_func_decl->params, i);
        kefir_size_t parameter_slot_index;
        REQUIRE_OK(kefir_ir_type_slot_of(ir_func_decl->params, parameter_type_index, &parameter_slot_index));

        struct kefir_abi_amd64_function_parameter parameter;
        REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, parameter_slot_index, &parameter));

        switch (parameter.location) {
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE:
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER:
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS:
                // Intentionally left blank
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY: {
                kefir_asm_amd64_xasmgen_register_t base_reg;
                kefir_int64_t offset;
                REQUIRE_OK(kefir_abi_amd64_function_parameter_memory_location(
                    &parameter, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLER, &base_reg, &offset));
                REQUIRE_OK(kefir_asmcmp_virtual_register_new_memory_pointer(
                    mem, &function->code.context, (kefir_asmcmp_physical_register_index_t) base_reg, offset,
                    &argument_placement_vreg));
                REQUIRE_OK(kefir_codegen_amd64_stack_frame_require_frame_pointer(&function->stack_frame));

                const struct kefir_ir_typeentry *typeentry =
                    kefir_ir_type_at(ir_func_decl->params, parameter_type_index);
                REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to fetch IR type entry"));
                switch (typeentry->typecode) {
                    case KEFIR_IR_TYPE_STRUCT:
                    case KEFIR_IR_TYPE_ARRAY:
                    case KEFIR_IR_TYPE_UNION: {
                        kefir_asmcmp_virtual_register_index_t tmp_vreg;
                        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
                            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));

                        const struct kefir_abi_amd64_type_layout *parameters_layout;
                        const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
                        REQUIRE_OK(kefir_abi_amd64_function_decl_parameters_layout(abi_func_decl, &parameters_layout));
                        REQUIRE_OK(kefir_abi_amd64_type_layout_at(parameters_layout, parameter_type_index, &layout));

                        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), tmp_vreg,
                            argument_placement_vreg, NULL));

                        REQUIRE_OK(
                            kefir_codegen_amd64_copy_memory(mem, function, tmp_vreg, argument_vreg, layout->size));
                    } break;

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
                    case KEFIR_IR_TYPE_LONG_DOUBLE:
                    case KEFIR_IR_TYPE_COMPLEX_FLOAT32:
                    case KEFIR_IR_TYPE_COMPLEX_FLOAT64:
                    case KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE:
                        // Intentionally left blank
                        break;

                    case KEFIR_IR_TYPE_NONE:
                    case KEFIR_IR_TYPE_COUNT:
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR type code");
                }
            } break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87:
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP:
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_COMPLEX_X87:
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NESTED:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 function parameter location");
        }
    }

    const struct kefir_abi_amd64_type_layout *parameters_layout;
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters_layout(abi_func_decl, &parameters_layout));
    for (kefir_size_t i = 0; i < call_node->argument_count; i++, subarg_count++) {
        kefir_asmcmp_virtual_register_index_t argument_vreg, argument_placement_vreg;
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, call_node->arguments[i], &argument_vreg));

        kefir_size_t parameter_type_index = kefir_ir_type_child_index(ir_func_decl->params, i);
        kefir_size_t parameter_slot_index;
        REQUIRE_OK(kefir_ir_type_slot_of(ir_func_decl->params, parameter_type_index, &parameter_slot_index));

        const struct kefir_abi_amd64_typeentry_layout *parameter_layout = NULL;
        REQUIRE_OK(kefir_abi_amd64_type_layout_at(parameters_layout, parameter_type_index, &parameter_layout));

        struct kefir_abi_amd64_function_parameter parameter;
        REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, parameter_slot_index, &parameter));

        switch (parameter.location) {
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE:
                // Intentionally left blank
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER:
                REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                             KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE,
                                                             &argument_placement_vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
                    mem, &function->code, argument_placement_vreg, parameter.direct_reg));
                REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    argument_placement_vreg, argument_vreg, NULL));
                REQUIRE_OK(kefir_hashtree_insert(mem, argument_placement, (kefir_hashtree_key_t) subarg_count,
                                                 (kefir_hashtree_value_t) argument_placement_vreg));
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
                REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                             KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT,
                                                             &argument_placement_vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
                    mem, &function->code, argument_placement_vreg, parameter.direct_reg));
                REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    argument_placement_vreg, argument_vreg, NULL));
                REQUIRE_OK(kefir_hashtree_insert(mem, argument_placement, (kefir_hashtree_key_t) subarg_count,
                                                 (kefir_hashtree_value_t) argument_placement_vreg));
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS: {
                kefir_size_t multireg_length;
                REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_length(&parameter, &multireg_length));
                for (kefir_size_t i = 0; i < multireg_length; i++, subarg_count++) {
                    struct kefir_abi_amd64_function_parameter subparam;
                    REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_at(&parameter, i, &subparam));

                    switch (subparam.location) {
                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE:
                            // Intentionally left blank
                            break;

                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER: {
                            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE,
                                                                         &argument_placement_vreg));
                            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
                                mem, &function->code, argument_placement_vreg, subparam.direct_reg));
                            if (i + 1 == multireg_length) {
                                kefir_size_t load_size = parameter_layout->size % KEFIR_AMD64_ABI_QWORD;
                                if (load_size == 0) {
                                    load_size = KEFIR_AMD64_ABI_QWORD;
                                }
                                REQUIRE_OK(kefir_codegen_amd64_load_general_purpose_register(
                                    mem, function, argument_placement_vreg, argument_vreg, load_size,
                                    i * KEFIR_AMD64_ABI_QWORD));
                            } else {
                                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG64(argument_placement_vreg),
                                    &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_vreg, i * KEFIR_AMD64_ABI_QWORD,
                                                                        KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                                    NULL));
                            }
                            REQUIRE_OK(kefir_hashtree_insert(mem, argument_placement,
                                                             (kefir_hashtree_key_t) subarg_count,
                                                             (kefir_hashtree_value_t) argument_placement_vreg));
                        } break;

                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
                            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT,
                                                                         &argument_placement_vreg));
                            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
                                mem, &function->code, argument_placement_vreg, subparam.direct_reg));
                            if (i + 1 == multireg_length) {
                                kefir_size_t load_bytes = parameter_layout->size % KEFIR_AMD64_ABI_QWORD;
                                if (load_bytes == 0) {
                                    load_bytes = 8;
                                }
                                REQUIRE_OK(kefir_codegen_amd64_load_floating_point_register(
                                    mem, function, argument_placement_vreg, argument_vreg, load_bytes,
                                    i * KEFIR_AMD64_ABI_QWORD));
                            } else {
                                REQUIRE_OK(kefir_asmcmp_amd64_movq(
                                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                    &KEFIR_ASMCMP_MAKE_VREG(argument_placement_vreg),
                                    &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_vreg, i * KEFIR_AMD64_ABI_QWORD,
                                                                        KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                    NULL));
                            }
                            REQUIRE_OK(kefir_hashtree_insert(mem, argument_placement,
                                                             (kefir_hashtree_key_t) subarg_count,
                                                             (kefir_hashtree_value_t) argument_placement_vreg));
                            break;

                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87:
                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP:
                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_COMPLEX_X87:
                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS:
                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY:
                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NESTED:
                            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 function parameter location");
                    }
                }
            } break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY: {
                kefir_asm_amd64_xasmgen_register_t base_reg;
                kefir_int64_t offset;
                REQUIRE_OK(kefir_abi_amd64_function_parameter_memory_location(
                    &parameter, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLER, &base_reg, &offset));
                REQUIRE_OK(kefir_asmcmp_virtual_register_new_memory_pointer(
                    mem, &function->code.context, (kefir_asmcmp_physical_register_index_t) base_reg, offset,
                    &argument_placement_vreg));
                REQUIRE_OK(kefir_codegen_amd64_stack_frame_require_frame_pointer(&function->stack_frame));

                const struct kefir_ir_typeentry *typeentry =
                    kefir_ir_type_at(ir_func_decl->params, parameter_type_index);
                REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to fetch IR type entry"));
                switch (typeentry->typecode) {
                    case KEFIR_IR_TYPE_STRUCT:
                    case KEFIR_IR_TYPE_ARRAY:
                    case KEFIR_IR_TYPE_UNION:
                        // Intentionally left blank
                        break;

                    case KEFIR_IR_TYPE_LONG_DOUBLE:
                        REQUIRE_OK(kefir_asmcmp_amd64_fld(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
                            NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_fstp(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(base_reg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
                            NULL));
                        break;

                    case KEFIR_IR_TYPE_BOOL:
                    case KEFIR_IR_TYPE_CHAR:
                    case KEFIR_IR_TYPE_INT8:
                        REQUIRE_OK(kefir_asmcmp_amd64_mov(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_placement_vreg, 0,
                                                                KEFIR_ASMCMP_OPERAND_VARIANT_8BIT),
                            &KEFIR_ASMCMP_MAKE_VREG8(argument_vreg), NULL));
                        break;

                    case KEFIR_IR_TYPE_SHORT:
                    case KEFIR_IR_TYPE_INT16:
                        REQUIRE_OK(kefir_asmcmp_amd64_mov(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_placement_vreg, 0,
                                                                KEFIR_ASMCMP_OPERAND_VARIANT_16BIT),
                            &KEFIR_ASMCMP_MAKE_VREG16(argument_vreg), NULL));
                        break;

                    case KEFIR_IR_TYPE_INT:
                    case KEFIR_IR_TYPE_INT32:
                        REQUIRE_OK(kefir_asmcmp_amd64_mov(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_placement_vreg, 0,
                                                                KEFIR_ASMCMP_OPERAND_VARIANT_32BIT),
                            &KEFIR_ASMCMP_MAKE_VREG32(argument_vreg), NULL));
                        break;

                    case KEFIR_IR_TYPE_FLOAT32:
                        REQUIRE_OK(kefir_asmcmp_amd64_movd(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_placement_vreg, 0,
                                                                KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                            &KEFIR_ASMCMP_MAKE_VREG(argument_vreg), NULL));
                        break;

                    case KEFIR_IR_TYPE_INT64:
                    case KEFIR_IR_TYPE_LONG:
                    case KEFIR_IR_TYPE_WORD:
                    case KEFIR_IR_TYPE_BITS:
                        REQUIRE_OK(kefir_asmcmp_amd64_mov(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_placement_vreg, 0,
                                                                KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                            &KEFIR_ASMCMP_MAKE_VREG64(argument_vreg), NULL));
                        break;

                    case KEFIR_IR_TYPE_FLOAT64:
                        REQUIRE_OK(kefir_asmcmp_amd64_movq(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_placement_vreg, 0,
                                                                KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                            &KEFIR_ASMCMP_MAKE_VREG(argument_vreg), NULL));
                        break;

                    case KEFIR_IR_TYPE_COMPLEX_FLOAT32: {
                        kefir_asmcmp_virtual_register_index_t tmp_vreg;
                        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
                            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));

                        REQUIRE_OK(kefir_asmcmp_amd64_mov(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                            NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_movq(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_placement_vreg, 0,
                                                                KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                            &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
                    } break;

                    case KEFIR_IR_TYPE_COMPLEX_FLOAT64: {
                        kefir_asmcmp_virtual_register_index_t tmp_vreg;
                        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
                            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &tmp_vreg));

                        REQUIRE_OK(kefir_asmcmp_amd64_movdqu(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_128BIT),
                            NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_movdqu(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_placement_vreg, 0,
                                                                KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                            &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
                    } break;

                    case KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE:
                        REQUIRE_OK(kefir_asmcmp_amd64_fld(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
                            NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_fstp(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(base_reg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
                            NULL));

                        REQUIRE_OK(kefir_asmcmp_amd64_fld(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_vreg, KEFIR_AMD64_ABI_QWORD * 2,
                                                                KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
                            NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_fstp(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(base_reg, offset + KEFIR_AMD64_ABI_QWORD * 2,
                                                                 KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
                            NULL));
                        break;

                    case KEFIR_IR_TYPE_NONE:
                    case KEFIR_IR_TYPE_COUNT:
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR type code");
                }
            } break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87:
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP:
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_COMPLEX_X87:
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NESTED:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 function parameter location");
        }
    }

    kefir_bool_t implicit_parameter_present;
    kefir_asm_amd64_xasmgen_register_t implicit_parameter_reg;
    REQUIRE_OK(kefir_abi_amd64_function_decl_returns_implicit_parameter(abi_func_decl, &implicit_parameter_present,
                                                                        &implicit_parameter_reg));
    if (!tail_call && implicit_parameter_present) {
        kefir_asmcmp_virtual_register_index_t implicit_vreg;
        REQUIRE(return_space_vreg != KEFIR_ID_NONE,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                "Expected valid return space to be defined for call with implicit parameter"));

        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &implicit_vreg));
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, implicit_vreg,
                                                                      implicit_parameter_reg));
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             implicit_vreg, return_space_vreg, NULL));
        REQUIRE_OK(kefir_hashtree_insert(mem, argument_placement, (kefir_hashtree_key_t) subarg_count++,
                                         (kefir_hashtree_value_t) implicit_vreg));
    }

    kefir_bool_t sse_reqs_parameter;
    kefir_asm_amd64_xasmgen_register_t sse_reqs_parameter_reg;
    REQUIRE_OK(
        kefir_abi_amd64_function_decl_parameters_sse_reqs(abi_func_decl, &sse_reqs_parameter, &sse_reqs_parameter_reg));
    if (sse_reqs_parameter) {
        struct kefir_abi_amd64_function_parameter_requirements reqs;
        REQUIRE_OK(kefir_abi_amd64_function_parameters_requirements(parameters, &reqs));

        kefir_asmcmp_virtual_register_index_t sse_reqs_vreg;
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &sse_reqs_vreg));
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, sse_reqs_vreg,
                                                                      sse_reqs_parameter_reg));
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(sse_reqs_vreg), &KEFIR_ASMCMP_MAKE_UINT(reqs.sse_regs), NULL));
        REQUIRE_OK(kefir_hashtree_insert(mem, argument_placement, (kefir_hashtree_key_t) subarg_count++,
                                         (kefir_hashtree_value_t) sse_reqs_vreg));
    }

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(argument_placement, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg_idx, node->value);
        REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), vreg_idx, NULL));
    }

    return KEFIR_OK;
}

static kefir_result_t save_returns(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                   const struct kefir_opt_instruction *instruction,
                                   struct kefir_abi_amd64_function_decl *abi_func_decl,
                                   kefir_asmcmp_stash_index_t stash_idx,
                                   kefir_asmcmp_virtual_register_index_t return_space_vreg,
                                   kefir_asmcmp_virtual_register_index_t *result_vreg) {
    const struct kefir_abi_amd64_function_parameters *return_parameters;
    REQUIRE_OK(kefir_abi_amd64_function_decl_returns(abi_func_decl, &return_parameters));

    struct kefir_abi_amd64_function_parameter return_parameter;
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(return_parameters, 0, &return_parameter));

    const struct kefir_ir_function_decl *ir_func_decl;
    REQUIRE_OK(kefir_abi_amd64_function_decl_ir(abi_func_decl, &ir_func_decl));

    *result_vreg = KEFIR_ID_NONE;
    kefir_asmcmp_virtual_register_index_t return_vreg, return_placement_vreg;
    switch (return_parameter.location) {
        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE:
            // Intentionally left blank
            break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &return_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(
                mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &return_placement_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, return_placement_vreg,
                                                                          return_parameter.direct_reg));
            REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), return_placement_vreg,
                NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), return_vreg,
                return_placement_vreg, NULL));
            *result_vreg = return_vreg;
            REQUIRE_OK(kefir_asmcmp_register_stash_exclude(mem, &function->code.context, stash_idx, return_vreg));
            break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &return_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(
                mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &return_placement_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, return_placement_vreg,
                                                                          return_parameter.direct_reg));
            REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), return_placement_vreg,
                NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), return_vreg,
                return_placement_vreg, NULL));
            *result_vreg = return_vreg;
            REQUIRE_OK(kefir_asmcmp_register_stash_exclude(mem, &function->code.context, stash_idx, return_vreg));
            break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS: {
            const struct kefir_ir_typeentry *return_typeentry = kefir_ir_type_at(ir_func_decl->result, 0);
            REQUIRE(return_typeentry != NULL,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected valid function return IR type entry"));

            const struct kefir_abi_amd64_type_layout *layout;
            const struct kefir_abi_amd64_typeentry_layout *return_layout;
            REQUIRE_OK(kefir_abi_amd64_function_decl_returns_layout(abi_func_decl, &layout));
            REQUIRE_OK(kefir_abi_amd64_type_layout_at(layout, 0, &return_layout));

            const kefir_size_t result_size_padded =
                kefir_target_abi_pad_aligned(return_layout->size, KEFIR_AMD64_ABI_QWORD);
            const kefir_size_t result_alignment_padded =
                kefir_target_abi_pad_aligned(return_layout->alignment, KEFIR_AMD64_ABI_QWORD);

            switch (return_typeentry->typecode) {
                case KEFIR_IR_TYPE_STRUCT:
                case KEFIR_IR_TYPE_ARRAY:
                case KEFIR_IR_TYPE_UNION:
                    REQUIRE(
                        return_space_vreg != KEFIR_ID_NONE,
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                        "Expected return space to be defined for call with multiple register return"));
                    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
                        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &return_vreg));
                    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), return_vreg,
                        return_space_vreg, NULL));
                    break;

                case KEFIR_IR_TYPE_COMPLEX_FLOAT32:
                case KEFIR_IR_TYPE_COMPLEX_FLOAT64:
                    REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(
                        mem, &function->code.context, result_size_padded / KEFIR_AMD64_ABI_QWORD,
                        result_alignment_padded / KEFIR_AMD64_ABI_QWORD, &return_vreg));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected function return IR type");
            }

            kefir_size_t multireg_length;
            REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_length(&return_parameter, &multireg_length));
            kefir_size_t result_offset = 0;
            for (kefir_size_t i = 0; i < multireg_length; i++) {
                struct kefir_abi_amd64_function_parameter subparam;
                REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_at(&return_parameter, i, &subparam));

                switch (subparam.location) {
                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NONE:
                        // Intentionally left blank
                        break;

                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER:
                        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE,
                                                                     &return_placement_vreg));
                        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
                            mem, &function->code, return_placement_vreg, subparam.direct_reg));
                        if (i + 1 == multireg_length) {
                            kefir_size_t store_bytes = return_layout->size % KEFIR_AMD64_ABI_QWORD;
                            if (store_bytes == 0) {
                                store_bytes = KEFIR_AMD64_ABI_QWORD;
                            }
                            REQUIRE_OK(kefir_codegen_amd64_store_general_purpose_register(
                                mem, function, return_vreg, return_placement_vreg, store_bytes, result_offset));
                            result_offset += store_bytes;
                        } else {
                            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(return_vreg, result_offset,
                                                                    KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                                &KEFIR_ASMCMP_MAKE_VREG64(return_placement_vreg), NULL));
                            result_offset += KEFIR_AMD64_ABI_QWORD;
                        }
                        break;

                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
                        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT,
                                                                     &return_placement_vreg));
                        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
                            mem, &function->code, return_placement_vreg, subparam.direct_reg));
                        if (i + 1 == multireg_length) {
                            kefir_size_t store_bytes = return_layout->size % KEFIR_AMD64_ABI_QWORD;
                            if (store_bytes == 0) {
                                store_bytes = KEFIR_AMD64_ABI_QWORD;
                            }
                            REQUIRE_OK(kefir_codegen_amd64_store_floating_point_register(
                                mem, function, return_vreg, return_placement_vreg, store_bytes, result_offset));
                            result_offset += store_bytes;
                        } else {
                            REQUIRE_OK(kefir_asmcmp_amd64_movq(
                                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(return_vreg, result_offset,
                                                                    KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                &KEFIR_ASMCMP_MAKE_VREG(return_placement_vreg), NULL));
                            result_offset += KEFIR_AMD64_ABI_QWORD;
                        }
                        break;

                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87:
                        REQUIRE(i + 1 < multireg_length,
                                KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                                "Expected X87 qword to be directly followed by X87UP"));
                        REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_at(&return_parameter, ++i, &subparam));
                        REQUIRE(subparam.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP,
                                KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                                "Expected X87 qword to be directly followed by X87UP"));

                        REQUIRE_OK(kefir_asmcmp_amd64_fstp(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(return_vreg, result_offset,
                                                                KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
                            NULL));
                        result_offset += 2 * KEFIR_AMD64_ABI_QWORD;
                        break;

                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS:
                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY:
                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP:
                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_COMPLEX_X87:
                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NESTED:
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 function parameter location");
                }
            }

            *result_vreg = return_vreg;
            REQUIRE_OK(kefir_asmcmp_register_stash_exclude(mem, &function->code.context, stash_idx, return_vreg));
        } break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY:
            REQUIRE(return_space_vreg != KEFIR_ID_NONE,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                    "Expected return space to be defined for call with multiple register return"));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &return_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), return_vreg,
                return_space_vreg, NULL));
            *result_vreg = return_vreg;
            break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(
                mem, &function->code.context, kefir_abi_amd64_long_double_qword_size(function->codegen->abi_variant),
                kefir_abi_amd64_long_double_qword_alignment(function->codegen->abi_variant), &return_vreg));

            if (instruction->operation.opcode == KEFIR_OPT_OPCODE_INVOKE ||
                instruction->operation.opcode == KEFIR_OPT_OPCODE_INVOKE_VIRTUAL) {
                REQUIRE_OK(kefir_codegen_amd64_function_x87_push(mem, function, instruction->id));
            } else {
                REQUIRE_OK(kefir_asmcmp_amd64_fstp(
                    mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                    &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(return_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
            }
            *result_vreg = return_vreg;
            REQUIRE_OK(kefir_asmcmp_register_stash_exclude(mem, &function->code.context, stash_idx, return_vreg));
            break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_COMPLEX_X87:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(
                mem, &function->code.context,
                kefir_abi_amd64_complex_long_double_qword_size(function->codegen->abi_variant),
                kefir_abi_amd64_complex_long_double_qword_alignment(function->codegen->abi_variant), &return_vreg));

            REQUIRE_OK(kefir_asmcmp_amd64_fstp(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(return_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
            REQUIRE_OK(
                kefir_asmcmp_amd64_fstp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(return_vreg, KEFIR_AMD64_ABI_QWORD * 2,
                                                                            KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
                                        NULL));

            *result_vreg = return_vreg;
            REQUIRE_OK(kefir_asmcmp_register_stash_exclude(mem, &function->code.context, stash_idx, return_vreg));
            break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NESTED:
        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 function parameter location");
    }
    return KEFIR_OK;
}

static kefir_result_t tail_invoke_impl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                       const struct kefir_opt_instruction *instruction,
                                       const struct kefir_opt_call_node *call_node,
                                       struct kefir_abi_amd64_function_decl *abi_func_decl,
                                       struct kefir_hashtree *argument_placement) {

    const struct kefir_ir_function_decl *ir_func_decl =
        kefir_ir_module_get_declaration(function->module->ir_module, call_node->function_declaration_id);
    REQUIRE(ir_func_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve IR function declaration"));
    REQUIRE_OK(prepare_parameters(mem, function, call_node, ir_func_decl, abi_func_decl, KEFIR_ASMCMP_INDEX_NONE,
                                  argument_placement, true));

    kefir_asmcmp_virtual_register_index_t indirect_call_idx;
    if (instruction->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL) {
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &indirect_call_idx));
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, indirect_call_idx,
                                                                      KEFIR_AMD64_XASMGEN_REGISTER_RAX));

        kefir_asmcmp_virtual_register_index_t func_vreg;
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(
            function, instruction->operation.parameters.function_call.indirect_ref, &func_vreg));

        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                                             indirect_call_idx, func_vreg, NULL));
    }

    if (instruction->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE) {
        const struct kefir_ir_identifier *ir_identifier;
        REQUIRE_OK(kefir_ir_module_get_identifier(function->module->ir_module, ir_func_decl->name, &ir_identifier));
        kefir_asmcmp_external_label_relocation_t fn_location =
            function->codegen->config->position_independent_code &&
                    ir_identifier->scope == KEFIR_IR_IDENTIFIER_SCOPE_IMPORT
                ? KEFIR_ASMCMP_EXTERNAL_LABEL_PLT
                : KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE;
        REQUIRE_OK(kefir_asmcmp_amd64_tail_call(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(fn_location, ir_identifier->symbol, 0), NULL));
    } else {
        REQUIRE_OK(kefir_asmcmp_amd64_tail_call(mem, &function->code,
                                                kefir_asmcmp_context_instr_tail(&function->code.context),
                                                &KEFIR_ASMCMP_MAKE_VREG(indirect_call_idx), NULL));
    }
    return KEFIR_OK;
}

static kefir_result_t invoke_impl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                  const struct kefir_opt_instruction *instruction,
                                  const struct kefir_opt_call_node *call_node,
                                  struct kefir_abi_amd64_function_decl *abi_func_decl,
                                  struct kefir_hashtree *argument_placement, kefir_bool_t tail_call,
                                  kefir_asmcmp_virtual_register_index_t *result_vreg, kefir_bool_t *tail_call_done) {
    const struct kefir_ir_function_decl *ir_func_decl =
        kefir_ir_module_get_declaration(function->module->ir_module, call_node->function_declaration_id);
    REQUIRE(ir_func_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve IR function declaration"));

    REQUIRE_OK(kefir_codegen_amd64_function_x87_flush(mem, function));

    if (tail_call) {
        kefir_bool_t tail_call_possible;
        REQUIRE_OK(kefir_codegen_amd64_tail_call_possible(
            mem, function, instruction->operation.parameters.function_call.call_ref, &tail_call_possible));
        if (tail_call_possible) {
            REQUIRE_OK(tail_invoke_impl(mem, function, instruction, call_node, abi_func_decl, argument_placement));
            *result_vreg = KEFIR_ID_NONE;
            *tail_call_done = true;
            return KEFIR_OK;
        }
    }

    kefir_size_t stack_increment;
    REQUIRE_OK(prepare_stack(mem, function, abi_func_decl, &stack_increment));

    kefir_asmcmp_stash_index_t stash_idx;
    kefir_asmcmp_virtual_register_index_t return_space_vreg;
    REQUIRE_OK(preserve_regs(mem, function, call_node, &stash_idx, &return_space_vreg));

    REQUIRE_OK(prepare_parameters(mem, function, call_node, ir_func_decl, abi_func_decl, return_space_vreg,
                                  argument_placement, false));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_require_frame_pointer(&function->stack_frame));

    kefir_asmcmp_instruction_index_t call_idx;
    if (instruction->operation.opcode == KEFIR_OPT_OPCODE_INVOKE ||
        instruction->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE) {
        const struct kefir_ir_identifier *ir_identifier;
        REQUIRE_OK(kefir_ir_module_get_identifier(function->module->ir_module, ir_func_decl->name, &ir_identifier));
        kefir_asmcmp_external_label_relocation_t fn_location =
            function->codegen->config->position_independent_code &&
                    ir_identifier->scope == KEFIR_IR_IDENTIFIER_SCOPE_IMPORT
                ? KEFIR_ASMCMP_EXTERNAL_LABEL_PLT
                : KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE;
        REQUIRE_OK(kefir_asmcmp_amd64_call(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(fn_location, ir_identifier->symbol, 0), &call_idx));
    } else {
        kefir_asmcmp_virtual_register_index_t func_vreg;
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(
            function, instruction->operation.parameters.function_call.indirect_ref, &func_vreg));
        REQUIRE_OK(kefir_asmcmp_amd64_call(mem, &function->code,
                                           kefir_asmcmp_context_instr_tail(&function->code.context),
                                           &KEFIR_ASMCMP_MAKE_VREG(func_vreg), &call_idx));
    }
    REQUIRE_OK(kefir_asmcmp_register_stash_set_liveness_index(&function->code.context, stash_idx, call_idx));

    REQUIRE_OK(kefir_abi_amd64_function_decl_ir(abi_func_decl, &ir_func_decl));
    if (ir_func_decl->returns_twice) {
        REQUIRE_OK(kefir_asmcmp_amd64_preserve_active_virtual_registers(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
        REQUIRE_OK(kefir_codegen_local_variable_allocator_mark_all_global(&function->variable_allocator));
    }

    if (kefir_ir_type_length(ir_func_decl->result) > 0) {
        REQUIRE_OK(save_returns(mem, function, instruction, abi_func_decl, stash_idx, return_space_vreg, result_vreg));
    }
    REQUIRE_OK(restore_regs(mem, function, stash_idx));
    if (stack_increment > 0) {
        REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                          &KEFIR_ASMCMP_MAKE_UINT(stack_increment), NULL));
    }
    return KEFIR_OK;
}

static kefir_result_t do_invoke(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                const struct kefir_opt_instruction *instruction, kefir_bool_t tail_call,
                                kefir_asmcmp_virtual_register_index_t *result_vreg_ptr, kefir_bool_t *tail_call_done) {
    *tail_call_done = false;
    const struct kefir_opt_call_node *call_node = NULL;
    REQUIRE_OK(kefir_opt_code_container_call(&function->function->code,
                                             instruction->operation.parameters.function_call.call_ref, &call_node));

    const struct kefir_ir_function_decl *ir_func_decl =
        kefir_ir_module_get_declaration(function->module->ir_module, call_node->function_declaration_id);
    REQUIRE(ir_func_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR function declaration"));

    const char BUILTIN_PREFIX[] = "__kefir_builtin_";
    if (ir_func_decl->name != NULL && strncmp(BUILTIN_PREFIX, ir_func_decl->name, sizeof(BUILTIN_PREFIX) - 1) == 0) {
        kefir_bool_t found_builtin = false;
        REQUIRE_OK(kefir_codegen_amd64_translate_builtin(mem, function, instruction, &found_builtin, result_vreg_ptr));
        REQUIRE(!found_builtin, KEFIR_OK);
    }

    struct kefir_abi_amd64_function_decl abi_func_decl;
    REQUIRE_OK(kefir_abi_amd64_function_decl_alloc(mem, function->codegen->abi_variant, ir_func_decl, &abi_func_decl));

    struct kefir_hashtree argument_placement;
    REQUIRE_OK(kefir_hashtree_init(&argument_placement, &kefir_hashtree_uint_ops));

    kefir_result_t res = invoke_impl(mem, function, instruction, call_node, &abi_func_decl, &argument_placement,
                                     tail_call, result_vreg_ptr, tail_call_done);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &argument_placement);
        kefir_abi_amd64_function_decl_free(mem, &abi_func_decl);
        return res;
    });
    res = kefir_hashtree_free(mem, &argument_placement);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_function_decl_free(mem, &abi_func_decl);
        return res;
    });

    REQUIRE_OK(kefir_abi_amd64_function_decl_free(mem, &abi_func_decl));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(invoke)(struct kefir_mem *mem,
                                                            struct kefir_codegen_amd64_function *function,
                                                            const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg = KEFIR_ID_NONE;
    kefir_bool_t tail_call_done;
    REQUIRE_OK(do_invoke(mem, function, instruction, false, &result_vreg, &tail_call_done));
    if (result_vreg != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    }

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(tail_invoke)(struct kefir_mem *mem,
                                                                 struct kefir_codegen_amd64_function *function,
                                                                 const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t result_vreg = KEFIR_ID_NONE;
    kefir_bool_t tail_call_done;
    REQUIRE_OK(do_invoke(mem, function, instruction, true, &result_vreg, &tail_call_done));
    if (!tail_call_done) {
        REQUIRE_OK(kefir_codegen_amd64_return_from_function(mem, function, KEFIR_ID_NONE, result_vreg));
    }

    return KEFIR_OK;
}
