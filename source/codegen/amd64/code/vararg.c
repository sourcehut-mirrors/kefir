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
#include "kefir/target/abi/amd64/parameters.h"
#include "kefir/target/abi/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t vararg_start_impl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                        const struct kefir_opt_instruction *instruction) {
    kefir_asmcmp_virtual_register_index_t arg_vreg, tmp_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &arg_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));

    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter_requirements reqs;
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(&function->abi_function_declaration, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_requirements(parameters, &reqs));

    REQUIRE_OK(
        kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                               &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT),
                               &KEFIR_ASMCMP_MAKE_UINT(KEFIR_AMD64_ABI_QWORD * reqs.general_purpose_regs), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg_vreg, 4, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT),
        &KEFIR_ASMCMP_MAKE_UINT(KEFIR_AMD64_ABI_QWORD * kefir_abi_amd64_num_of_general_purpose_parameter_registers(
                                                            KEFIR_ABI_AMD64_VARIANT_SYSTEM_V) +
                                2 * KEFIR_AMD64_ABI_QWORD * reqs.sse_regs),
        NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_lea(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_PHYSICAL(KEFIR_AMD64_XASMGEN_REGISTER_RBP, 2 * KEFIR_AMD64_ABI_QWORD + reqs.stack,
                                             KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                               &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg_vreg, 8, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                               &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_lea(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
                                      &KEFIR_ASMCMP_MAKE_INDIRECT_VARARG(0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                      NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                               &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(arg_vreg, 16, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                               &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(vararg_start)(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64_function *function,
                                                                  const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    switch (function->codegen->abi_variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            REQUIRE_OK(vararg_start_impl(mem, function, instruction));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(vararg_end)(struct kefir_mem *mem,
                                                                struct kefir_codegen_amd64_function *function,
                                                                const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    switch (function->codegen->abi_variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}

static kefir_result_t vararg_copy_impl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                       const struct kefir_opt_instruction *instruction) {
    kefir_asmcmp_virtual_register_index_t source_vreg, target_vreg, tmp_vreg;

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &source_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[1], &target_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(source_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                               &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                               &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(source_vreg, 8, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                               &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, 8, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                               &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(source_vreg, 16, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                               &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(target_vreg, 16, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                               &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(vararg_copy)(struct kefir_mem *mem,
                                                                 struct kefir_codegen_amd64_function *function,
                                                                 const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    switch (function->codegen->abi_variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            REQUIRE_OK(vararg_copy_impl(mem, function, instruction));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}

static kefir_result_t visitor_not_supported(const struct kefir_ir_type *type, kefir_size_t index,
                                            const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    UNUSED(payload);
    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Encountered not supported type code while traversing type");
}

struct vararg_get_param {
    struct kefir_mem *mem;
    struct kefir_codegen_amd64_function *function;
    const struct kefir_opt_instruction *instruction;
};

static kefir_result_t vararg_visit_integer(const struct kefir_ir_type *type, kefir_size_t index,
                                           const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct vararg_get_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid vararg visitor payload"));

    kefir_asmcmp_virtual_register_index_t valist_vreg, valist_placement_vreg, result_vreg, result_placement_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(param->mem, &param->function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE,
                                                 &valist_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(param->mem, &param->function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(param->mem, &param->function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE,
                                                 &result_placement_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(param->function, param->instruction->operation.parameters.refs[0],
                                                    &valist_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
        param->mem, &param->function->code, valist_placement_vreg, KEFIR_AMD64_XASMGEN_REGISTER_RDI));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
        param->mem, &param->function->code, result_placement_vreg, KEFIR_AMD64_XASMGEN_REGISTER_RAX));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        param->mem, &param->function->code, kefir_asmcmp_context_instr_tail(&param->function->code.context),
        valist_placement_vreg, valist_vreg, NULL));

    const char *symbolic_label = KEFIR_AMD64_SYSTEM_V_RUNTIME_LOAD_INT_VARARG;
    if (param->function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_format(param->mem, &param->function->code.context, &symbolic_label, KEFIR_AMD64_PLT,
                                       KEFIR_AMD64_SYSTEM_V_RUNTIME_LOAD_INT_VARARG));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_call(param->mem, &param->function->code,
                                       kefir_asmcmp_context_instr_tail(&param->function->code.context),
                                       &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(symbolic_label, 0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
        param->mem, &param->function->code, kefir_asmcmp_context_instr_tail(&param->function->code.context),
        result_placement_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        param->mem, &param->function->code, kefir_asmcmp_context_instr_tail(&param->function->code.context),
        result_vreg, result_placement_vreg, NULL));

    REQUIRE_OK(
        kefir_codegen_amd64_function_assign_vreg(param->mem, param->function, param->instruction->id, result_vreg));

    return KEFIR_OK;
}

static kefir_result_t vararg_visit_sse(const struct kefir_ir_type *type, kefir_size_t index,
                                       const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct vararg_get_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid vararg visitor payload"));

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_mxcsr(&param->function->stack_frame));

    kefir_asmcmp_virtual_register_index_t valist_vreg, valist_placement_vreg, result_vreg, result_placement_vreg,
        tmp_placement_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(param->mem, &param->function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE,
                                                 &valist_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(param->mem, &param->function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(param->mem, &param->function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(param->mem, &param->function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT, &result_placement_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(param->function, param->instruction->operation.parameters.refs[0],
                                                    &valist_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
        param->mem, &param->function->code, valist_placement_vreg, KEFIR_AMD64_XASMGEN_REGISTER_RDI));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
        param->mem, &param->function->code, tmp_placement_vreg, KEFIR_AMD64_XASMGEN_REGISTER_RAX));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
        param->mem, &param->function->code, result_placement_vreg, KEFIR_AMD64_XASMGEN_REGISTER_XMM0));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        param->mem, &param->function->code, kefir_asmcmp_context_instr_tail(&param->function->code.context),
        valist_placement_vreg, valist_vreg, NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
        param->mem, &param->function->code, kefir_asmcmp_context_instr_tail(&param->function->code.context),
        tmp_placement_vreg, NULL));

    const char *symbolic_label = KEFIR_AMD64_SYSTEM_V_RUNTIME_LOAD_SSE_VARARG;
    if (param->function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_format(param->mem, &param->function->code.context, &symbolic_label, KEFIR_AMD64_PLT,
                                       KEFIR_AMD64_SYSTEM_V_RUNTIME_LOAD_SSE_VARARG));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_call(param->mem, &param->function->code,
                                       kefir_asmcmp_context_instr_tail(&param->function->code.context),
                                       &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(symbolic_label, 0), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
        param->mem, &param->function->code, kefir_asmcmp_context_instr_tail(&param->function->code.context),
        result_placement_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
        param->mem, &param->function->code, kefir_asmcmp_context_instr_tail(&param->function->code.context),
        result_vreg, result_placement_vreg, NULL));

    REQUIRE_OK(
        kefir_codegen_amd64_function_assign_vreg(param->mem, param->function, param->instruction->id, result_vreg));

    return KEFIR_OK;
}

static kefir_result_t vararg_visit_memory_aggregate_impl(struct kefir_mem *mem,
                                                         struct kefir_codegen_amd64_function *function,
                                                         const struct kefir_abi_amd64_typeentry_layout *param_layout,
                                                         kefir_asmcmp_virtual_register_index_t valist_vreg,
                                                         kefir_asmcmp_virtual_register_index_t result_vreg,
                                                         kefir_asmcmp_virtual_register_index_t tmp_vreg) {
    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(valist_vreg, 8, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), NULL));

    if (param_layout->alignment > KEFIR_AMD64_ABI_QWORD) {
        REQUIRE_OK(kefir_asmcmp_amd64_add(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_INT(param_layout->alignment - 1), NULL));

        REQUIRE_OK(kefir_asmcmp_amd64_and(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(result_vreg), &KEFIR_ASMCMP_MAKE_INT(-param_layout->alignment), NULL));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_lea(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg,
                                            kefir_target_abi_pad_aligned(param_layout->size, KEFIR_AMD64_ABI_QWORD),
                                            KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));

    REQUIRE_OK(
        kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                               &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(valist_vreg, 8, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                               &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), NULL));
    return KEFIR_OK;
}

static kefir_result_t vararg_visit_memory_aggregate(struct kefir_mem *mem,
                                                    struct kefir_codegen_amd64_function *function,
                                                    const struct kefir_opt_instruction *instruction,
                                                    const struct kefir_abi_amd64_typeentry_layout *param_layout) {
    kefir_asmcmp_virtual_register_index_t valist_vreg, result_vreg, tmp_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &valist_vreg));

    REQUIRE_OK(vararg_visit_memory_aggregate_impl(mem, function, param_layout, valist_vreg, result_vreg, tmp_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t vararg_register_aggregate_check(struct kefir_mem *mem,
                                                      struct kefir_codegen_amd64_function *function,
                                                      struct kefir_abi_amd64_function_parameter *parameter,
                                                      kefir_asmcmp_virtual_register_index_t valist_vreg,
                                                      kefir_asmcmp_virtual_register_index_t tmp_vreg,
                                                      kefir_asmcmp_label_index_t overflow_area_label,
                                                      kefir_size_t *integer_qwords, kefir_size_t *sse_qwords) {
    kefir_size_t required_integers = 0;
    kefir_size_t required_sse = 0;
    kefir_size_t length;
    REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_length(parameter, &length));
    for (kefir_size_t i = 0; i < length; i++) {
        struct kefir_abi_amd64_function_parameter subparam;
        REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_at(parameter, i, &subparam));
        switch (subparam.location) {
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER:
                required_integers++;
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
                required_sse++;
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                       "Non-integer,sse vararg aggregate members are not supported");
        }
    }

    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(valist_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
                                      &KEFIR_ASMCMP_MAKE_UINT(required_integers * KEFIR_AMD64_ABI_QWORD), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_cmp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_UINT(
            kefir_abi_amd64_num_of_general_purpose_parameter_registers(function->codegen->abi_variant) *
            KEFIR_AMD64_ABI_QWORD),
        NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_ja(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(overflow_area_label), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG32(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(valist_vreg, 4, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
                                      &KEFIR_ASMCMP_MAKE_UINT(required_sse * 2 * KEFIR_AMD64_ABI_QWORD), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_cmp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
        &KEFIR_ASMCMP_MAKE_UINT(
            kefir_abi_amd64_num_of_general_purpose_parameter_registers(function->codegen->abi_variant) *
                KEFIR_AMD64_ABI_QWORD +
            2 * kefir_abi_amd64_num_of_sse_parameter_registers(function->codegen->abi_variant) * KEFIR_AMD64_ABI_QWORD),
        NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_ja(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                     &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(overflow_area_label), NULL));

    *integer_qwords = required_integers;
    *sse_qwords = required_sse;
    return KEFIR_OK;
}

static kefir_result_t vararg_register_aggregate_load(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    struct kefir_abi_amd64_function_parameter *parameter,
    const struct kefir_abi_amd64_typeentry_layout *parameter_layout, kefir_asmcmp_virtual_register_index_t valist_vreg,
    kefir_asmcmp_virtual_register_index_t result_vreg, kefir_asmcmp_virtual_register_index_t tmp_vreg,
    kefir_asmcmp_virtual_register_index_t tmp2_vreg, kefir_asmcmp_label_index_t overflow_area_end_label,
    kefir_size_t integer_qwords, kefir_size_t sse_qwords) {

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_ensure_temporary_area(
        &function->stack_frame, kefir_target_abi_pad_aligned(parameter_layout->size, KEFIR_AMD64_ABI_QWORD),
        MAX(parameter_layout->alignment, KEFIR_AMD64_ABI_QWORD)));

    REQUIRE_OK(kefir_asmcmp_amd64_lea(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(result_vreg),
                                      &KEFIR_ASMCMP_MAKE_INDIRECT_TEMPORARY(0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                                      NULL));

    if (integer_qwords > 0) {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(valist_vreg, 16, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), NULL));

        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG32(tmp2_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(valist_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT), NULL));

        REQUIRE_OK(
            kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                   &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp2_vreg), NULL));

        kefir_size_t integer_offset = 0;
        kefir_size_t length;
        REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_length(parameter, &length));
        for (kefir_size_t i = 0; i < length; i++) {
            struct kefir_abi_amd64_function_parameter subparam;
            REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_at(parameter, i, &subparam));
            switch (subparam.location) {
                case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER:
                    REQUIRE_OK(kefir_asmcmp_amd64_mov(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG64(tmp2_vreg),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp_vreg, integer_offset * KEFIR_AMD64_ABI_QWORD,
                                                            KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                        NULL));

                    REQUIRE_OK(kefir_asmcmp_amd64_mov(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, i * KEFIR_AMD64_ABI_QWORD,
                                                            KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                        &KEFIR_ASMCMP_MAKE_VREG64(tmp2_vreg), NULL));

                    integer_offset++;
                    break;

                case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
                    // Intentionally left blank
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                           "Non-integer,sse vararg aggregate members are not supported");
            }
        }

        REQUIRE_OK(kefir_asmcmp_amd64_add(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(valist_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT),
            &KEFIR_ASMCMP_MAKE_UINT(integer_offset * KEFIR_AMD64_ABI_QWORD), NULL));
    }

    if (sse_qwords > 0) {
        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(valist_vreg, 16, KEFIR_ASMCMP_OPERAND_VARIANT_64BIT), NULL));

        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG32(tmp2_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(valist_vreg, 4, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT), NULL));

        REQUIRE_OK(
            kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                   &KEFIR_ASMCMP_MAKE_VREG64(tmp_vreg), &KEFIR_ASMCMP_MAKE_VREG64(tmp2_vreg), NULL));

        kefir_size_t sse_offset = 0;
        kefir_size_t length;
        REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_length(parameter, &length));
        for (kefir_size_t i = 0; i < length; i++) {
            struct kefir_abi_amd64_function_parameter subparam;
            REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_at(parameter, i, &subparam));
            switch (subparam.location) {
                case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER:
                    // Intentionally left blank
                    break;

                case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
                    REQUIRE_OK(kefir_asmcmp_amd64_mov(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_VREG64(tmp2_vreg),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp_vreg, sse_offset * 2 * KEFIR_AMD64_ABI_QWORD,
                                                            KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                        NULL));

                    REQUIRE_OK(kefir_asmcmp_amd64_mov(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, i * KEFIR_AMD64_ABI_QWORD,
                                                            KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                        &KEFIR_ASMCMP_MAKE_VREG64(tmp2_vreg), NULL));

                    sse_offset++;
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                           "Non-integer,sse vararg aggregate members are not supported");
            }
        }

        REQUIRE_OK(kefir_asmcmp_amd64_add(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(valist_vreg, 4, KEFIR_ASMCMP_OPERAND_VARIANT_32BIT),
            &KEFIR_ASMCMP_MAKE_UINT(sse_offset * 2 * KEFIR_AMD64_ABI_QWORD), NULL));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_jmp(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(overflow_area_end_label), NULL));
    return KEFIR_OK;
}

static kefir_result_t vararg_visit_register_aggregate(struct kefir_mem *mem,
                                                      struct kefir_codegen_amd64_function *function,
                                                      const struct kefir_opt_instruction *instruction,
                                                      struct kefir_abi_amd64_function_parameter *parameter,
                                                      const struct kefir_abi_amd64_typeentry_layout *parameter_layout) {
    kefir_asmcmp_label_index_t overflow_area_label, overflow_area_end_label;
    REQUIRE_OK(
        kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &overflow_area_label));
    REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE,
                                              &overflow_area_end_label));

    kefir_asmcmp_virtual_register_index_t valist_vreg, tmp_vreg, tmp2_vreg, result_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp2_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.refs[0], &valist_vreg));

    kefir_size_t required_integers = 0, required_sse = 0;
    REQUIRE_OK(vararg_register_aggregate_check(mem, function, parameter, valist_vreg, tmp_vreg, overflow_area_label,
                                               &required_integers, &required_sse));
    REQUIRE_OK(vararg_register_aggregate_load(mem, function, parameter, parameter_layout, valist_vreg, result_vreg,
                                              tmp_vreg, tmp2_vreg, overflow_area_end_label, required_integers,
                                              required_sse));

    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, overflow_area_label));
    REQUIRE_OK(vararg_visit_memory_aggregate_impl(mem, function, parameter_layout, valist_vreg, result_vreg, tmp_vreg));
    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, overflow_area_end_label));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

static kefir_result_t vararg_visit_aggregate_impl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                                  const struct kefir_opt_instruction *instruction,
                                                  const struct kefir_ir_type *type, kefir_size_t index,
                                                  struct kefir_abi_amd64_type_layout *type_layout,
                                                  struct kefir_abi_amd64_function_parameters *parameters) {
    const struct kefir_abi_amd64_typeentry_layout *param_layout = NULL;
    REQUIRE_OK(kefir_abi_amd64_type_layout_at(type_layout, index, &param_layout));

    struct kefir_abi_amd64_function_parameter parameter;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, slot, &parameter));

    if (parameter.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY) {
        REQUIRE_OK(vararg_visit_memory_aggregate(mem, function, instruction, param_layout));
    } else {
        REQUIRE_OK(vararg_visit_register_aggregate(mem, function, instruction, &parameter, param_layout));
    }
    return KEFIR_OK;
}

static kefir_result_t vararg_visit_aggregate(const struct kefir_ir_type *type, kefir_size_t index,
                                             const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(index);
    UNUSED(typeentry);
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));
    ASSIGN_DECL_CAST(struct vararg_get_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid vararg visitor payload"));

    struct kefir_abi_amd64_type_layout type_layout;
    struct kefir_abi_amd64_function_parameters parameters;

    REQUIRE_OK(kefir_abi_amd64_type_layout(param->mem, param->function->codegen->abi_variant, type, &type_layout));

    kefir_result_t res = kefir_abi_amd64_function_parameters_classify(param->mem, param->function->codegen->abi_variant,
                                                                      type, &type_layout, &parameters);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_type_layout_free(param->mem, &type_layout);
        return res;
    });

    res = kefir_abi_amd64_function_parameters_allocate(param->mem, &parameters);
    REQUIRE_CHAIN(&res, vararg_visit_aggregate_impl(param->mem, param->function, param->instruction, type, index,
                                                    &type_layout, &parameters));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_function_parameters_free(param->mem, &parameters);
        kefir_abi_amd64_type_layout_free(param->mem, &type_layout);
        return res;
    });

    res = kefir_abi_amd64_function_parameters_free(param->mem, &parameters);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_type_layout_free(param->mem, &type_layout);
        return res;
    });
    REQUIRE_OK(kefir_abi_amd64_type_layout_free(param->mem, &type_layout));

    return KEFIR_OK;
}

static kefir_result_t vararg_visit_builtin(const struct kefir_ir_type *type, kefir_size_t index,
                                           const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type entry"));
    ASSIGN_DECL_CAST(struct vararg_get_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid vararg visitor payload"));

    switch (typeentry->param) {
        case KEFIR_IR_TYPE_BUILTIN: {
            kefir_asmcmp_virtual_register_index_t valist_vreg, valist_placement_vreg, result_vreg,
                result_placement_vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(param->mem, &param->function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE,
                                                         &valist_placement_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(param->mem, &param->function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(param->mem, &param->function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE,
                                                         &result_placement_vreg));
            REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(
                param->function, param->instruction->operation.parameters.refs[0], &valist_vreg));

            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
                param->mem, &param->function->code, valist_placement_vreg, KEFIR_AMD64_XASMGEN_REGISTER_RDI));
            REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
                param->mem, &param->function->code, result_placement_vreg, KEFIR_AMD64_XASMGEN_REGISTER_RAX));

            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                param->mem, &param->function->code, kefir_asmcmp_context_instr_tail(&param->function->code.context),
                valist_placement_vreg, valist_vreg, NULL));

            const char *symbolic_label = KEFIR_AMD64_SYSTEM_V_RUNTIME_LOAD_INT_VARARG;
            if (param->function->codegen->config->position_independent_code) {
                REQUIRE_OK(kefir_asmcmp_format(param->mem, &param->function->code.context, &symbolic_label,
                                               KEFIR_AMD64_PLT, KEFIR_AMD64_SYSTEM_V_RUNTIME_LOAD_INT_VARARG));
            }

            REQUIRE_OK(kefir_asmcmp_amd64_call(param->mem, &param->function->code,
                                               kefir_asmcmp_context_instr_tail(&param->function->code.context),
                                               &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(symbolic_label, 0), NULL));

            REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
                param->mem, &param->function->code, kefir_asmcmp_context_instr_tail(&param->function->code.context),
                result_placement_vreg, NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(
                param->mem, &param->function->code, kefir_asmcmp_context_instr_tail(&param->function->code.context),
                result_vreg, result_placement_vreg, NULL));

            REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(param->mem, param->function, param->instruction->id,
                                                                result_vreg));
        } break;
    }

    return KEFIR_OK;
}

static kefir_result_t vararg_get_impl(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                      const struct kefir_opt_instruction *instruction) {
    const kefir_id_t type_id = (kefir_id_t) instruction->operation.parameters.type.type_id;
    const kefir_size_t type_index = (kefir_size_t) instruction->operation.parameters.type.type_index;
    struct kefir_ir_type *type = kefir_ir_module_get_named_type(function->module->ir_module, type_id);
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown named IR type"));

    struct kefir_ir_type_visitor visitor;
    REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, visitor_not_supported));
    KEFIR_IR_TYPE_VISITOR_INIT_INTEGERS(&visitor, vararg_visit_integer);
    KEFIR_IR_TYPE_VISITOR_INIT_FIXED_FP(&visitor, vararg_visit_sse);
    KEFIR_IR_TYPE_VISITOR_INIT_COMPLEX(&visitor, vararg_visit_aggregate);
    visitor.visit[KEFIR_IR_TYPE_STRUCT] = vararg_visit_aggregate;
    visitor.visit[KEFIR_IR_TYPE_UNION] = vararg_visit_aggregate;
    visitor.visit[KEFIR_IR_TYPE_ARRAY] = vararg_visit_aggregate;
    visitor.visit[KEFIR_IR_TYPE_BUILTIN] = vararg_visit_builtin;
    visitor.visit[KEFIR_IR_TYPE_LONG_DOUBLE] = vararg_visit_aggregate;

    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(
        type, &visitor,
        (void *) &(struct vararg_get_param){.mem = mem, .function = function, .instruction = instruction}, type_index,
        1));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(vararg_get)(struct kefir_mem *mem,
                                                                struct kefir_codegen_amd64_function *function,
                                                                const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    switch (function->codegen->abi_variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
            REQUIRE_OK(vararg_get_impl(mem, function, instruction));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 abi variant");
    }
    return KEFIR_OK;
}
