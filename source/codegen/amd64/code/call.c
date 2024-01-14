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
#include "kefir/target/abi/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

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
                                    kefir_asmcmp_stash_index_t *stash_idx) {
    const kefir_size_t num_of_preserved_gp_regs =
        kefir_abi_amd64_num_of_caller_preserved_general_purpose_registers(function->codegen->abi_variant);
    const kefir_size_t num_of_preserved_sse_regs =
        kefir_abi_amd64_num_of_caller_preserved_sse_registers(function->codegen->abi_variant);

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
                                         struct kefir_opt_call_node *call_node,
                                         const struct kefir_ir_function_decl *ir_func_decl,
                                         struct kefir_abi_amd64_function_decl *abi_func_decl,
                                         struct kefir_hashtree *argument_placement) {
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

                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER:
                            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE,
                                                                         &argument_placement_vreg));
                            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
                                mem, &function->code, argument_placement_vreg, subparam.direct_reg));
                            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                &KEFIR_ASMCMP_MAKE_VREG64(argument_placement_vreg),
                                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_vreg, i * KEFIR_AMD64_ABI_QWORD,
                                                                    KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                                NULL));
                            REQUIRE_OK(kefir_hashtree_insert(mem, argument_placement,
                                                             (kefir_hashtree_key_t) subarg_count,
                                                             (kefir_hashtree_value_t) argument_placement_vreg));
                            break;

                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
                            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT,
                                                                         &argument_placement_vreg));
                            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
                                mem, &function->code, argument_placement_vreg, subparam.direct_reg));
                            REQUIRE_OK(kefir_asmcmp_amd64_movq(
                                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                &KEFIR_ASMCMP_MAKE_VREG(argument_placement_vreg),
                                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(argument_vreg, i * KEFIR_AMD64_ABI_QWORD,
                                                                    KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                NULL));
                            REQUIRE_OK(kefir_hashtree_insert(mem, argument_placement,
                                                             (kefir_hashtree_key_t) subarg_count,
                                                             (kefir_hashtree_value_t) argument_placement_vreg));
                            break;

                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87:
                        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP:
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
                    case KEFIR_IR_TYPE_UNION: {
                        kefir_asmcmp_virtual_register_index_t tmp_vreg;
                        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
                            mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));

                        const struct kefir_abi_amd64_type_layout *parameters_layout;
                        const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
                        REQUIRE_OK(kefir_abi_amd64_function_decl_parameters_layout(abi_func_decl, &parameters_layout));
                        REQUIRE_OK(kefir_abi_amd64_type_layout_at(parameters_layout, parameter_type_index, &layout));

                        REQUIRE_OK(kefir_asmcmp_amd64_push(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RDI), NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_push(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RSI), NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_push(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RCX), NULL));

                        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                                          &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RSI),
                                                          &KEFIR_ASMCMP_MAKE_VREG64(argument_vreg), NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_lea(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RDI),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(base_reg, offset + 3 * KEFIR_AMD64_ABI_QWORD,
                                                                 KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                            NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                                          &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RCX),
                                                          &KEFIR_ASMCMP_MAKE_UINT(layout->size), NULL));

                        REQUIRE_OK(kefir_asmcmp_amd64_cld(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_movsb_rep(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));

                        REQUIRE_OK(kefir_asmcmp_amd64_pop(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RCX), NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_pop(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RSI), NULL));
                        REQUIRE_OK(kefir_asmcmp_amd64_pop(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RDI), NULL));
                    } break;

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
                        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                                          &KEFIR_ASMCMP_MAKE_VREG(argument_placement_vreg),
                                                          &KEFIR_ASMCMP_MAKE_VREG(argument_vreg), NULL));
                        break;

                    case KEFIR_IR_TYPE_NONE:
                    case KEFIR_IR_TYPE_COUNT:
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected IR type code");
                }
            } break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87:
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP:
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NESTED:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 function parameter location");
        }
    }

    kefir_bool_t implicit_parameter_present;
    kefir_asm_amd64_xasmgen_register_t implicit_parameter_reg;
    REQUIRE_OK(kefir_abi_amd64_function_decl_returns_implicit_parameter(abi_func_decl, &implicit_parameter_present,
                                                                        &implicit_parameter_reg));
    if (implicit_parameter_present) {
        kefir_asmcmp_virtual_register_index_t implicit_vreg;
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &implicit_vreg));
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, implicit_vreg,
                                                                      implicit_parameter_reg));
        REQUIRE_OK(
            kefir_asmcmp_amd64_lea(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                   &KEFIR_ASMCMP_MAKE_VREG64(implicit_vreg),
                                   &KEFIR_ASMCMP_MAKE_INDIRECT_TEMPORARY(0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), NULL));
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
                                   kefir_opt_instruction_ref_t instr_ref,
                                   struct kefir_abi_amd64_function_decl *abi_func_decl) {
    const struct kefir_abi_amd64_function_parameters *return_parameters;
    REQUIRE_OK(kefir_abi_amd64_function_decl_returns(abi_func_decl, &return_parameters));

    struct kefir_abi_amd64_function_parameter return_parameter;
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(return_parameters, 0, &return_parameter));

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
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), return_vreg,
                return_placement_vreg, NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instr_ref, return_vreg));
            break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &return_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(
                mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &return_placement_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, return_placement_vreg,
                                                                          return_parameter.direct_reg));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), return_vreg,
                return_placement_vreg, NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instr_ref, return_vreg));
            break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS: {
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &return_vreg));
            kefir_size_t temporary_area_size = 0;

            kefir_size_t multireg_length;
            REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_length(&return_parameter, &multireg_length));
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
                        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                                          &KEFIR_ASMCMP_MAKE_INDIRECT_TEMPORARY(
                                                              temporary_area_size, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                                                          &KEFIR_ASMCMP_MAKE_VREG64(return_placement_vreg), NULL));
                        temporary_area_size += KEFIR_AMD64_ABI_QWORD;
                        break;

                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
                        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT,
                                                                     &return_placement_vreg));
                        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
                            mem, &function->code, return_placement_vreg, subparam.direct_reg));
                        REQUIRE_OK(kefir_asmcmp_amd64_movq(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_TEMPORARY(temporary_area_size,
                                                                  KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                            &KEFIR_ASMCMP_MAKE_VREG(return_placement_vreg), NULL));
                        temporary_area_size += KEFIR_AMD64_ABI_QWORD;
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
                            &KEFIR_ASMCMP_MAKE_INDIRECT_TEMPORARY((i - 1) * KEFIR_AMD64_ABI_QWORD,
                                                                  KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
                            NULL));
                        break;

                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MULTIPLE_REGISTERS:
                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY:
                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP:
                    case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NESTED:
                        return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 function parameter location");
                }
            }
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_ensure_temporary_area(
                &function->stack_frame, temporary_area_size, KEFIR_AMD64_ABI_QWORD));

            REQUIRE_OK(kefir_asmcmp_amd64_lea(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(return_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_TEMPORARY(0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instr_ref, return_vreg));
        } break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY: {
            kefir_asm_amd64_xasmgen_register_t base_reg;
            kefir_int64_t offset;
            REQUIRE_OK(kefir_abi_amd64_function_return_memory_location(
                &return_parameter, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLER, &base_reg, &offset));

            const struct kefir_abi_amd64_type_layout *layout;
            const struct kefir_abi_amd64_typeentry_layout *return_layout;
            REQUIRE_OK(kefir_abi_amd64_function_decl_returns_layout(abi_func_decl, &layout));
            REQUIRE_OK(kefir_abi_amd64_type_layout_at(layout, 0, &return_layout));
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_ensure_temporary_area(
                &function->stack_frame, return_layout->size, return_layout->alignment));

            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &return_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_lea(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(return_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(base_reg, offset, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instr_ref, return_vreg));
        } break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87: {
            const struct kefir_abi_amd64_type_layout *layout;
            const struct kefir_abi_amd64_typeentry_layout *return_layout;
            REQUIRE_OK(kefir_abi_amd64_function_decl_returns_layout(abi_func_decl, &layout));
            REQUIRE_OK(kefir_abi_amd64_type_layout_at(layout, 0, &return_layout));
            REQUIRE_OK(kefir_codegen_amd64_stack_frame_ensure_temporary_area(
                &function->stack_frame, return_layout->size, return_layout->alignment));

            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &return_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_fstp(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_INDIRECT_TEMPORARY(0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_lea(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG64(return_vreg),
                &KEFIR_ASMCMP_MAKE_INDIRECT_TEMPORARY(0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instr_ref, return_vreg));
        } break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_NESTED:
        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 function parameter location");
    }
    return KEFIR_OK;
}

static kefir_result_t invoke_impl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                  const struct kefir_opt_instruction *instruction,
                                  struct kefir_opt_call_node *call_node,
                                  struct kefir_abi_amd64_function_decl *abi_func_decl,
                                  struct kefir_hashtree *argument_placement) {
    kefir_size_t stack_increment;
    REQUIRE_OK(prepare_stack(mem, function, abi_func_decl, &stack_increment));

    kefir_asmcmp_stash_index_t stash_idx;
    REQUIRE_OK(preserve_regs(mem, function, &stash_idx));

    const struct kefir_ir_function_decl *ir_func_decl =
        kefir_ir_module_get_declaration(function->module->ir_module, call_node->function_declaration_id);
    REQUIRE(ir_func_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrieve IR function declaration"));
    REQUIRE_OK(prepare_parameters(mem, function, call_node, ir_func_decl, abi_func_decl, argument_placement));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_require_frame_pointer(&function->stack_frame));

    kefir_asmcmp_instruction_index_t call_idx;
    if (instruction->operation.opcode == KEFIR_OPT_OPCODE_INVOKE) {
        const char *symbol = ir_func_decl->name;
        if (function->codegen->config->position_independent_code) {
            REQUIRE_OK(kefir_asmcmp_format(mem, &function->code.context, &symbol, KEFIR_AMD64_PLT, ir_func_decl->name));
        }
        REQUIRE_OK(kefir_asmcmp_amd64_call(mem, &function->code,
                                           kefir_asmcmp_context_instr_tail(&function->code.context),
                                           &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(symbol, 0), &call_idx));
    } else {
        kefir_asmcmp_virtual_register_index_t func_vreg;
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(
            function, instruction->operation.parameters.function_call.indirect_ref, &func_vreg));
        REQUIRE_OK(kefir_asmcmp_amd64_call(mem, &function->code,
                                           kefir_asmcmp_context_instr_tail(&function->code.context),
                                           &KEFIR_ASMCMP_MAKE_VREG(func_vreg), &call_idx));
    }
    REQUIRE_OK(kefir_asmcmp_register_stash_set_liveness_index(&function->code.context, stash_idx, call_idx));

    if (kefir_ir_type_length(ir_func_decl->result) > 0) {
        REQUIRE_OK(save_returns(mem, function, instruction->id, abi_func_decl));
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

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(invoke)(struct kefir_mem *mem,
                                                            struct kefir_codegen_amd64_function *function,
                                                            const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    struct kefir_opt_call_node *call_node = NULL;
    REQUIRE_OK(kefir_opt_code_container_call(&function->function->code,
                                             instruction->operation.parameters.function_call.call_ref, &call_node));

    const struct kefir_ir_function_decl *ir_func_decl =
        kefir_ir_module_get_declaration(function->module->ir_module, call_node->function_declaration_id);
    REQUIRE(ir_func_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR function declaration"));

    struct kefir_abi_amd64_function_decl abi_func_decl;
    REQUIRE_OK(kefir_abi_amd64_function_decl_alloc(mem, function->codegen->abi_variant, ir_func_decl, &abi_func_decl));

    struct kefir_hashtree argument_placement;
    REQUIRE_OK(kefir_hashtree_init(&argument_placement, &kefir_hashtree_uint_ops));

    kefir_result_t res = invoke_impl(mem, function, instruction, call_node, &abi_func_decl, &argument_placement);
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
