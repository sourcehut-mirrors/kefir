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

#include <string.h>
#include "kefir/codegen/system-v-amd64/abi.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/codegen/system-v-amd64/symbolic_labels.h"
#include "kefir/codegen/system-v-amd64/builtins.h"

struct invoke_info {
    struct kefir_codegen_amd64 *codegen;
    const struct kefir_abi_amd64_function_decl *decl;
    const kefir_size_t total_arguments;
    kefir_size_t argument;
};

struct invoke_returns {
    struct kefir_codegen_amd64 *codegen;
    const struct kefir_abi_amd64_function_decl *decl;
};

static kefir_result_t visitor_not_supported(const struct kefir_ir_type *type, kefir_size_t index,
                                            const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    UNUSED(payload);
    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, KEFIR_AMD64_SYSV_ABI_ERROR_PREFIX
                           "Encountered not supported type code while traversing type");
}

static kefir_result_t scalar_argument(const struct kefir_ir_type *type, kefir_size_t index,
                                      const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct invoke_info *, info, payload);
    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter parameter;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(info->decl, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, slot, &parameter));
    kefir_asm_amd64_xasmgen_register_t memory_base_reg;
    kefir_int64_t memory_offset;
    switch (parameter.location) {
        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(parameter.direct_reg),
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &info->codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                    (info->total_arguments - info->argument - 1) * KEFIR_AMD64_ABI_QWORD)));
            break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PINSRQ(
                &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(parameter.direct_reg),
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &info->codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                    (info->total_arguments - info->argument - 1) * KEFIR_AMD64_ABI_QWORD),
                kefir_asm_amd64_xasmgen_operand_imm(&info->codegen->xasmgen_helpers.operands[1], 0)));
            break;

        case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG),
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &info->codegen->xasmgen_helpers.operands[0],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                    (info->total_arguments - info->argument - 1) * KEFIR_AMD64_ABI_QWORD)));
            REQUIRE_OK(kefir_abi_amd64_function_parameter_memory_location(
                &parameter, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLER, &memory_base_reg, &memory_offset));
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &info->codegen->xasmgen,
                kefir_asm_amd64_xasmgen_operand_indirect(&info->codegen->xasmgen_helpers.operands[0],
                                                         kefir_asm_amd64_xasmgen_operand_reg(memory_base_reg),
                                                         memory_offset),
                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG)));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR,
                                   "Integer function argument cannot have non-INTEGER and non-MEMORY class");
    }
    info->argument++;
    return KEFIR_OK;
}

static kefir_result_t long_double_argument(const struct kefir_ir_type *type, kefir_size_t index,
                                           const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct invoke_info *, info, payload);
    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter parameter;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(info->decl, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, slot, &parameter));
    REQUIRE(parameter.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected long double argument to have memory allocation class"));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG),
        kefir_asm_amd64_xasmgen_operand_indirect(
            &info->codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
            (info->total_arguments - info->argument - 1) * KEFIR_AMD64_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG),
        kefir_asm_amd64_xasmgen_operand_indirect(&info->codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG),
                                                 0)));
    kefir_asm_amd64_xasmgen_register_t memory_base_reg;
    kefir_int64_t memory_offset;
    REQUIRE_OK(kefir_abi_amd64_function_parameter_memory_location(
        &parameter, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLER, &memory_base_reg, &memory_offset));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &info->codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_indirect(&info->codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(memory_base_reg), memory_offset),
        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG),
        kefir_asm_amd64_xasmgen_operand_indirect(&info->codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG),
                                                 KEFIR_AMD64_ABI_QWORD)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &info->codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_indirect(&info->codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(memory_base_reg),
                                                 memory_offset + KEFIR_AMD64_ABI_QWORD),
        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG)));

    info->argument++;
    return KEFIR_OK;
}

static kefir_result_t memory_aggregate_argument(struct invoke_info *info,
                                                const struct kefir_abi_amd64_typeentry_layout *layout,
                                                const struct kefir_abi_amd64_function_parameter *parameter) {
    if (layout->size > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(
            &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDI)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(
            &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSI)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(
            &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RCX)));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RCX),
            kefir_asm_amd64_xasmgen_operand_immu(&info->codegen->xasmgen_helpers.operands[0], layout->size)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSI),
            kefir_asm_amd64_xasmgen_operand_indirect(
                &info->codegen->xasmgen_helpers.operands[0],
                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                (info->total_arguments - info->argument - 1) * KEFIR_AMD64_ABI_QWORD)));
        kefir_asm_amd64_xasmgen_register_t memory_base_reg;
        kefir_int64_t memory_offset;
        REQUIRE_OK(kefir_abi_amd64_function_parameter_memory_location(
            parameter, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLER, &memory_base_reg, &memory_offset));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
            &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDI),
            kefir_asm_amd64_xasmgen_operand_indirect(&info->codegen->xasmgen_helpers.operands[0],
                                                     kefir_asm_amd64_xasmgen_operand_reg(memory_base_reg),
                                                     3 * KEFIR_AMD64_ABI_QWORD + memory_offset)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CLD(&info->codegen->xasmgen));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVSB(&info->codegen->xasmgen, true));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(
            &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RCX)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(
            &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSI)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(
            &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDI)));
    }
    return KEFIR_OK;
}

static kefir_result_t register_aggregate_argument(struct invoke_info *info,
                                                  const struct kefir_abi_amd64_function_parameter *parameter) {
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG),
        kefir_asm_amd64_xasmgen_operand_indirect(
            &info->codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
            (info->total_arguments - info->argument - 1) * KEFIR_AMD64_ABI_QWORD)));
    kefir_size_t length;
    REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_length(parameter, &length));
    for (kefir_size_t i = 0; i < length; i++) {
        struct kefir_abi_amd64_function_parameter subparam;
        REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_at(parameter, i, &subparam));
        switch (subparam.location) {
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(subparam.direct_reg),
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &info->codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG), i * KEFIR_AMD64_ABI_QWORD)));
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PINSRQ(
                    &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(subparam.direct_reg),
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &info->codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG), i * KEFIR_AMD64_ABI_QWORD),
                    kefir_asm_amd64_xasmgen_operand_imm(&info->codegen->xasmgen_helpers.operands[1], 0)));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Non-INTEGER & non-SSE arguments are not supported");
        }
    }
    return KEFIR_OK;
}

static kefir_result_t aggregate_argument(const struct kefir_ir_type *type, kefir_size_t index,
                                         const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct invoke_info *, info, payload);
    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter parameter;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(info->decl, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, slot, &parameter));
    if (parameter.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY) {
        const struct kefir_abi_amd64_type_layout *type_layout;
        const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
        REQUIRE_OK(kefir_abi_amd64_function_decl_parameters_layout(info->decl, &type_layout));
        REQUIRE_OK(kefir_abi_amd64_type_layout_at(type_layout, index, &layout));
        REQUIRE_OK(memory_aggregate_argument(info, layout, &parameter));
    } else {
        REQUIRE_OK(register_aggregate_argument(info, &parameter));
    }
    info->argument++;
    return KEFIR_OK;
}

static kefir_result_t builtin_argument(const struct kefir_ir_type *type, kefir_size_t index,
                                       const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct invoke_info *, info, payload);
    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter parameter;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(info->decl, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, slot, &parameter));
    kefir_ir_builtin_type_t builtin = (kefir_ir_builtin_type_t) typeentry->param;
    REQUIRE(builtin < KEFIR_IR_TYPE_BUILTIN_COUNT, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unknown built-in type"));
    const struct kefir_codegen_amd64_sysv_builtin_type *builtin_type =
        KEFIR_CODEGEN_SYSTEM_V_AMD64_SYSV_BUILTIN_TYPES[builtin];
    REQUIRE_OK(builtin_type->store_function_argument(builtin_type, typeentry, info->codegen, &parameter,
                                                     info->total_arguments - info->argument - 1));
    info->argument++;
    return KEFIR_OK;
}

kefir_result_t invoke_prologue(struct kefir_codegen_amd64 *codegen, const struct kefir_abi_amd64_function_decl *decl,
                               struct invoke_info *info) {
    struct kefir_ir_type_visitor visitor;
    REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, visitor_not_supported));
    KEFIR_IR_TYPE_VISITOR_INIT_INTEGERS(&visitor, scalar_argument);
    KEFIR_IR_TYPE_VISITOR_INIT_FIXED_FP(&visitor, scalar_argument);
    visitor.visit[KEFIR_IR_TYPE_LONG_DOUBLE] = long_double_argument;
    visitor.visit[KEFIR_IR_TYPE_STRUCT] = aggregate_argument;
    visitor.visit[KEFIR_IR_TYPE_ARRAY] = aggregate_argument;
    visitor.visit[KEFIR_IR_TYPE_UNION] = aggregate_argument;
    visitor.visit[KEFIR_IR_TYPE_BUILTIN] = builtin_argument;

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP)));
    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter_requirements reqs;
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(decl, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_requirements(parameters, &reqs));
    if (reqs.stack > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], reqs.stack)));
    }
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_AND(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
        kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0], ~0xfll)));
    const struct kefir_ir_function_decl *ir_func_decl;
    REQUIRE_OK(kefir_abi_amd64_function_decl_ir(decl, &ir_func_decl));
    REQUIRE_OK(
        kefir_ir_type_visitor_list_nodes(ir_func_decl->params, &visitor, (void *) info, 0, info->total_arguments));
    kefir_bool_t implicit_parameter;
    kefir_asm_amd64_xasmgen_register_t implicit_parameter_reg;
    REQUIRE_OK(
        kefir_abi_amd64_function_decl_returns_implicit_parameter(decl, &implicit_parameter, &implicit_parameter_reg));
    if (implicit_parameter) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(implicit_parameter_reg),
            kefir_asm_amd64_xasmgen_operand_indirect(
                &codegen->xasmgen_helpers.operands[0],
                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_STACK_BASE_REG),
                KEFIR_AMD64_SYSV_INTERNAL_BOUND)));
    }
    if (ir_func_decl->vararg) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], reqs.sse_regs)));
    }
    return KEFIR_OK;
}

static kefir_result_t integer_return(const struct kefir_ir_type *type, kefir_size_t index,
                                     const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct invoke_info *, info, payload);
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&info->codegen->xasmgen,
                                              kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX)));
    return KEFIR_OK;
}

static kefir_result_t sse_return(const struct kefir_ir_type *type, kefir_size_t index,
                                 const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct invoke_info *, info, payload);
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PEXTRQ(
        &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG),
        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_XMM0),
        kefir_asm_amd64_xasmgen_operand_imm(&info->codegen->xasmgen_helpers.operands[0], 0)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&info->codegen->xasmgen,
                                              kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG)));
    return KEFIR_OK;
}

static kefir_result_t long_double_return(const struct kefir_ir_type *type, kefir_size_t index,
                                         const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct invoke_info *, info, payload);

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FSTP(
        &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_pointer(
                                     &info->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_TBYTE,
                                     kefir_asm_amd64_xasmgen_operand_indirect(
                                         &info->codegen->xasmgen_helpers.operands[1],
                                         kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_STACK_BASE_REG),
                                         KEFIR_AMD64_SYSV_INTERNAL_BOUND))));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
        &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG),
        kefir_asm_amd64_xasmgen_operand_indirect(
            &info->codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_STACK_BASE_REG),
            KEFIR_AMD64_SYSV_INTERNAL_BOUND)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&info->codegen->xasmgen,
                                              kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG)));
    return KEFIR_OK;
}

static kefir_result_t register_aggregate_return(struct invoke_info *info,
                                                const struct kefir_abi_amd64_function_parameter *parameter) {
    kefir_size_t length;
    REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_length(parameter, &length));
    for (kefir_size_t i = 0; i < length; i++) {
        struct kefir_abi_amd64_function_parameter subparam;
        REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_at(parameter, i, &subparam));
        switch (subparam.location) {
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &info->codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &info->codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_STACK_BASE_REG),
                        KEFIR_AMD64_SYSV_INTERNAL_BOUND + i * KEFIR_AMD64_ABI_QWORD),
                    kefir_asm_amd64_xasmgen_operand_reg(subparam.direct_reg)));
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PEXTRQ(
                    &info->codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &info->codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_STACK_BASE_REG),
                        KEFIR_AMD64_SYSV_INTERNAL_BOUND + i * KEFIR_AMD64_ABI_QWORD),
                    kefir_asm_amd64_xasmgen_operand_reg(subparam.direct_reg),
                    kefir_asm_amd64_xasmgen_operand_imm(&info->codegen->xasmgen_helpers.operands[1], 0)));
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87:
                REQUIRE(i + 1 < length,
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected X87 qword to be directly followed by X87UP"));
                REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_at(parameter, ++i, &subparam));
                REQUIRE(subparam.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP,
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected X87 qword to be directly followed by X87UP"));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FSTP(
                    &info->codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_pointer(
                        &info->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_TBYTE,
                        kefir_asm_amd64_xasmgen_operand_indirect(
                            &info->codegen->xasmgen_helpers.operands[1],
                            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_STACK_BASE_REG),
                            KEFIR_AMD64_SYSV_INTERNAL_BOUND + (i - 1) * KEFIR_AMD64_ABI_QWORD))));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Non-INTEGER & non-SSE arguments are not supported");
        }
    }
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
        &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG),
        kefir_asm_amd64_xasmgen_operand_indirect(
            &info->codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_STACK_BASE_REG),
            KEFIR_AMD64_SYSV_INTERNAL_BOUND)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&info->codegen->xasmgen,
                                              kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG)));
    return KEFIR_OK;
}

static kefir_result_t aggregate_return(const struct kefir_ir_type *type, kefir_size_t index,
                                       const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct invoke_info *, info, payload);
    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter parameter;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    REQUIRE_OK(kefir_abi_amd64_function_decl_returns(info->decl, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, slot, &parameter));
    if (parameter.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(
            &info->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX)));
    } else {
        REQUIRE_OK(register_aggregate_return(info, &parameter));
    }
    return KEFIR_OK;
}

static kefir_result_t builtin_return(const struct kefir_ir_type *type, kefir_size_t index,
                                     const struct kefir_ir_typeentry *typeentry, void *payload) {
    ASSIGN_DECL_CAST(struct invoke_info *, info, payload);
    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter parameter;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    REQUIRE_OK(kefir_abi_amd64_function_decl_returns(info->decl, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, slot, &parameter));
    kefir_ir_builtin_type_t builtin = (kefir_ir_builtin_type_t) typeentry->param;
    REQUIRE(builtin < KEFIR_IR_TYPE_BUILTIN_COUNT, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unknown built-in type"));
    const struct kefir_codegen_amd64_sysv_builtin_type *builtin_type =
        KEFIR_CODEGEN_SYSTEM_V_AMD64_SYSV_BUILTIN_TYPES[builtin];
    REQUIRE_OK(builtin_type->load_function_return(builtin_type, typeentry, info->codegen, &parameter));
    return KEFIR_OK;
}

kefir_result_t invoke_epilogue(struct kefir_codegen_amd64 *codegen, const struct kefir_abi_amd64_function_decl *decl,
                               struct invoke_info *info, bool virtualDecl) {
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));
    if (info->total_arguments > 0 || virtualDecl) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_immu(
                &codegen->xasmgen_helpers.operands[0],
                (info->total_arguments + (virtualDecl ? 1 : 0)) * KEFIR_AMD64_ABI_QWORD)));
    }
    struct kefir_ir_type_visitor visitor;
    REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, visitor_not_supported));
    KEFIR_IR_TYPE_VISITOR_INIT_INTEGERS(&visitor, integer_return);
    KEFIR_IR_TYPE_VISITOR_INIT_FIXED_FP(&visitor, sse_return);
    visitor.visit[KEFIR_IR_TYPE_LONG_DOUBLE] = long_double_return;
    visitor.visit[KEFIR_IR_TYPE_STRUCT] = aggregate_return;
    visitor.visit[KEFIR_IR_TYPE_UNION] = aggregate_return;
    visitor.visit[KEFIR_IR_TYPE_ARRAY] = aggregate_return;
    visitor.visit[KEFIR_IR_TYPE_BUILTIN] = builtin_return;
    const struct kefir_ir_function_decl *ir_func_decl;
    REQUIRE_OK(kefir_abi_amd64_function_decl_ir(decl, &ir_func_decl));
    REQUIRE(kefir_ir_type_children(ir_func_decl->result) <= 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Function cannot return more than one value"));
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(ir_func_decl->result, &visitor, (void *) info, 0, 1));
    return KEFIR_OK;
}

static kefir_result_t argument_counter(const struct kefir_ir_type *type, kefir_size_t index,
                                       const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid typeentry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected payload"));

    ASSIGN_DECL_CAST(kefir_size_t *, counter, payload);
    (*counter)++;
    return KEFIR_OK;
}

kefir_result_t kefir_amd64_sysv_function_invoke(struct kefir_codegen_amd64 *codegen,
                                                const struct kefir_abi_amd64_function_decl *decl, bool virtualDecl) {
    struct invoke_info info = {.codegen = codegen, .decl = decl, .total_arguments = 0, .argument = 0};
    struct kefir_ir_type_visitor visitor;
    kefir_ir_type_visitor_init(&visitor, argument_counter);
    const struct kefir_ir_function_decl *ir_func_decl;
    REQUIRE_OK(kefir_abi_amd64_function_decl_ir(decl, &ir_func_decl));
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(ir_func_decl->params, &visitor, (void *) &info.total_arguments, 0,
                                                kefir_ir_type_length(ir_func_decl->params)));

    REQUIRE_OK(invoke_prologue(codegen, decl, &info));
    if (virtualDecl) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
            &codegen->xasmgen,
            kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                     kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                                                     info.total_arguments * KEFIR_AMD64_ABI_QWORD)));
    } else {
        REQUIRE(ir_func_decl->name != NULL && strlen(ir_func_decl->name) != 0,
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unable to translate invocation with no valid identifier"));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
            &codegen->xasmgen,
            kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], ir_func_decl->name)));
    }
    REQUIRE_OK(invoke_epilogue(codegen, decl, &info, virtualDecl));
    return KEFIR_OK;
}
