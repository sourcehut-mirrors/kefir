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

#include "kefir/codegen/system-v-amd64/abi.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/codegen/system-v-amd64/symbolic_labels.h"
#include "kefir/target/abi/amd64/type_layout.h"
#include "kefir/codegen/system-v-amd64/registers.h"
#include "kefir/codegen/system-v-amd64/builtins.h"
#include "kefir/target/abi/util.h"
#include <stdio.h>

static kefir_result_t preserve_state(struct kefir_amd64_xasmgen *xasmgen) {
    struct kefir_asm_amd64_xasmgen_operand op;
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
        xasmgen, kefir_asm_amd64_xasmgen_operand_label(&op, KEFIR_AMD64_SYSTEM_V_RUNTIME_PRESERVE_STATE)));
    return KEFIR_OK;
}

struct argument_load {
    const struct kefir_amd64_sysv_function *sysv_func;
    struct kefir_codegen_amd64 *codegen;
    kefir_size_t frame_offset;
};

static kefir_result_t visitor_not_supported(const struct kefir_ir_type *type, kefir_size_t index,
                                            const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    UNUSED(payload);
    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Unsupported function parameter type");
}

static kefir_result_t mask_argument(struct kefir_codegen_amd64 *codegen, const struct kefir_ir_typeentry *typeentry,
                                    kefir_asm_amd64_xasmgen_register_t reg) {
    switch (typeentry->typecode) {
        case KEFIR_IR_TYPE_BOOL:
        case KEFIR_IR_TYPE_CHAR:
        case KEFIR_IR_TYPE_INT8:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_EAX),
                kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 0xffull)));

            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_AND(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg),
                                              kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX)));
            break;

        case KEFIR_IR_TYPE_SHORT:
        case KEFIR_IR_TYPE_INT16:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_EAX),
                kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 0xffffull)));

            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_AND(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg),
                                              kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX)));
            break;

        case KEFIR_IR_TYPE_INT:
        case KEFIR_IR_TYPE_FLOAT32:
        case KEFIR_IR_TYPE_INT32:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_EAX),
                kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0], -1)));

            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_AND(&codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg),
                                              kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX)));
            break;

        case KEFIR_IR_TYPE_INT64:
        case KEFIR_IR_TYPE_FLOAT64:
        case KEFIR_IR_TYPE_LONG:
        case KEFIR_IR_TYPE_WORD:
            // Do nothing
            break;

        case KEFIR_IR_TYPE_BITS: {
            kefir_size_t bits = typeentry->param;
            if (bits > 0) {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SHL(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg),
                    kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 64 - bits)));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SHR(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(reg),
                    kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 64 - bits)));
            }
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected argument type");
    }
    return KEFIR_OK;
}

static kefir_result_t load_integer_argument(const struct kefir_ir_type *type, kefir_size_t index,
                                            const struct kefir_ir_typeentry *typeentry, void *payload) {
    struct argument_load *param = (struct argument_load *) payload;
    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter parameter;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(&param->sysv_func->decl, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, slot, &parameter));
    if (parameter.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&param->codegen->xasmgen,
                                                  kefir_asm_amd64_xasmgen_operand_reg(parameter.direct_reg)));
    } else {
        kefir_asm_amd64_xasmgen_register_t memory_base_reg;
        kefir_int64_t memory_offset;
        REQUIRE_OK(kefir_abi_amd64_function_parameter_memory_location(
            &parameter, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLEE, &memory_base_reg, &memory_offset));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &param->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
            kefir_asm_amd64_xasmgen_operand_indirect(&param->codegen->xasmgen_helpers.operands[0],
                                                     kefir_asm_amd64_xasmgen_operand_reg(memory_base_reg),
                                                     memory_offset)));
        REQUIRE_OK(mask_argument(param->codegen, typeentry, KEFIR_AMD64_SYSV_ABI_DATA_REG));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&param->codegen->xasmgen,
                                                  kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));
    }
    return KEFIR_OK;
}

static kefir_result_t load_sse_argument(const struct kefir_ir_type *type, kefir_size_t index,
                                        const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(typeentry);
    struct argument_load *param = (struct argument_load *) payload;
    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter parameter;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(&param->sysv_func->decl, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, slot, &parameter));
    if (parameter.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PEXTRQ(
            &param->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
            kefir_asm_amd64_xasmgen_operand_reg(parameter.direct_reg),
            kefir_asm_amd64_xasmgen_operand_imm(&param->codegen->xasmgen_helpers.operands[0], 0)));
    } else {
        kefir_asm_amd64_xasmgen_register_t memory_base_reg;
        kefir_int64_t memory_offset;
        REQUIRE_OK(kefir_abi_amd64_function_parameter_memory_location(
            &parameter, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLEE, &memory_base_reg, &memory_offset));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &param->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
            kefir_asm_amd64_xasmgen_operand_indirect(&param->codegen->xasmgen_helpers.operands[0],
                                                     kefir_asm_amd64_xasmgen_operand_reg(memory_base_reg),
                                                     memory_offset)));
        REQUIRE_OK(mask_argument(param->codegen, typeentry, KEFIR_AMD64_SYSV_ABI_DATA_REG));
    }
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&param->codegen->xasmgen,
                                              kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));
    return KEFIR_OK;
}

static kefir_result_t load_long_double_argument(const struct kefir_ir_type *type, kefir_size_t index,
                                                const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(typeentry);
    struct argument_load *param = (struct argument_load *) payload;
    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter parameter;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(&param->sysv_func->decl, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, slot, &parameter));
    REQUIRE(parameter.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected long double argument to be allocated in memory"));

    kefir_asm_amd64_xasmgen_register_t memory_base_reg;
    kefir_int64_t memory_offset;
    REQUIRE_OK(kefir_abi_amd64_function_parameter_memory_location(
        &parameter, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLEE, &memory_base_reg, &memory_offset));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
        &param->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
        kefir_asm_amd64_xasmgen_operand_indirect(&param->codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(memory_base_reg), memory_offset)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&param->codegen->xasmgen,
                                              kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));
    return KEFIR_OK;
}

static kefir_result_t load_reg_aggregate(struct argument_load *param,
                                         const struct kefir_abi_amd64_typeentry_layout *layout,
                                         const struct kefir_abi_amd64_function_parameter *parameter) {
    kefir_size_t length;
    REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_length(parameter, &length));
    param->frame_offset = kefir_target_abi_pad_aligned(param->frame_offset, layout->alignment);
    for (kefir_size_t i = 0; i < length; i++) {
        struct kefir_abi_amd64_function_parameter subparam;
        REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_at(parameter, i, &subparam));
        switch (subparam.location) {
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &param->codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &param->codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_STACK_BASE_REG),
                        param->frame_offset + KEFIR_AMD64_ABI_QWORD * i),
                    kefir_asm_amd64_xasmgen_operand_reg(subparam.direct_reg)));
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PEXTRQ(
                    &param->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                    kefir_asm_amd64_xasmgen_operand_reg(subparam.direct_reg),
                    kefir_asm_amd64_xasmgen_operand_imm(&param->codegen->xasmgen_helpers.operands[0], 0)));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &param->codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &param->codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_STACK_BASE_REG),
                        param->frame_offset + KEFIR_AMD64_ABI_QWORD * i),
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                       "Aggregates with non-INTEGER and non-SSE members are not supported yet");
        }
    }
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
        &param->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
        kefir_asm_amd64_xasmgen_operand_indirect(
            &param->codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_STACK_BASE_REG), param->frame_offset)));
    param->frame_offset += layout->size;
    return KEFIR_OK;
}

static kefir_result_t load_aggregate_argument(const struct kefir_ir_type *type, kefir_size_t index,
                                              const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(typeentry);
    struct argument_load *param = (struct argument_load *) payload;
    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter parameter;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(&param->sysv_func->decl, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, slot, &parameter));
    if (parameter.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY) {
        kefir_asm_amd64_xasmgen_register_t memory_base_reg;
        kefir_int64_t memory_offset;
        REQUIRE_OK(kefir_abi_amd64_function_parameter_memory_location(
            &parameter, KEFIR_ABI_AMD64_FUNCTION_PARAMETER_ADDRESS_CALLEE, &memory_base_reg, &memory_offset));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
            &param->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
            kefir_asm_amd64_xasmgen_operand_indirect(&param->codegen->xasmgen_helpers.operands[0],
                                                     kefir_asm_amd64_xasmgen_operand_reg(memory_base_reg),
                                                     memory_offset)));
    } else {
        const struct kefir_abi_amd64_type_layout *parameters_layout;
        const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
        REQUIRE_OK(kefir_abi_amd64_function_decl_parameters_layout(&param->sysv_func->decl, &parameters_layout));
        REQUIRE_OK(kefir_abi_amd64_type_layout_at(parameters_layout, index, &layout));
        REQUIRE_OK(load_reg_aggregate(param, layout, &parameter));
    }
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&param->codegen->xasmgen,
                                              kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));
    return KEFIR_OK;
}

static kefir_result_t load_builtin_argument(const struct kefir_ir_type *type, kefir_size_t index,
                                            const struct kefir_ir_typeentry *typeentry, void *payload) {
    struct argument_load *param = (struct argument_load *) payload;
    const struct kefir_abi_amd64_function_parameters *parameters;
    struct kefir_abi_amd64_function_parameter parameter;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    REQUIRE_OK(kefir_abi_amd64_function_decl_parameters(&param->sysv_func->decl, &parameters));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(parameters, slot, &parameter));
    kefir_ir_builtin_type_t builtin = (kefir_ir_builtin_type_t) typeentry->param;
    REQUIRE(builtin < KEFIR_IR_TYPE_BUILTIN_COUNT, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unknown built-in type"));
    const struct kefir_codegen_amd64_sysv_builtin_type *builtin_type =
        KEFIR_CODEGEN_SYSTEM_V_AMD64_SYSV_BUILTIN_TYPES[builtin];
    REQUIRE_OK(builtin_type->load_function_argument(builtin_type, typeentry, param->codegen, &parameter));
    return KEFIR_OK;
}

static kefir_result_t load_arguments(struct kefir_codegen_amd64 *codegen,
                                     const struct kefir_amd64_sysv_function *sysv_func) {
    if (sysv_func->frame.size > 0 && sysv_func->frame.size <= KEFIR_INT32_MAX) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], sysv_func->frame.size)));
    } else if (sysv_func->frame.size > KEFIR_INT32_MAX) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVABS(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], sysv_func->frame.size)));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SUB(&codegen->xasmgen,
                                                 kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                                 kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));
    }
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0],
                                                                 KEFIR_AMD64_SYSTEM_V_RUNTIME_GENERIC_PROLOGUE)));
    const struct kefir_ir_function_decl *func = sysv_func->func->declaration;
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "Load parameters of %s", func->name));

    kefir_bool_t implicit_param;
    kefir_asm_amd64_xasmgen_register_t implicit_param_reg;
    REQUIRE_OK(kefir_abi_amd64_function_decl_returns_implicit_parameter(&sysv_func->decl, &implicit_param,
                                                                        &implicit_param_reg));
    if (implicit_param) {
        REQUIRE_OK(
            KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
                                          kefir_asm_amd64_xasmgen_operand_indirect(
                                              &codegen->xasmgen_helpers.operands[0],
                                              kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_STACK_BASE_REG),
                                              KEFIR_AMD64_SYSV_INTERNAL_RETURN_ADDRESS * KEFIR_AMD64_ABI_QWORD),
                                          kefir_asm_amd64_xasmgen_operand_reg(implicit_param_reg)));
    }
    struct argument_load param = {
        .sysv_func = sysv_func, .codegen = codegen, .frame_offset = sysv_func->frame.base.parameters};
    struct kefir_ir_type_visitor visitor;
    REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, visitor_not_supported));
    KEFIR_IR_TYPE_VISITOR_INIT_INTEGERS(&visitor, load_integer_argument);
    KEFIR_IR_TYPE_VISITOR_INIT_FIXED_FP(&visitor, load_sse_argument);
    visitor.visit[KEFIR_IR_TYPE_LONG_DOUBLE] = load_long_double_argument;
    visitor.visit[KEFIR_IR_TYPE_STRUCT] = load_aggregate_argument;
    visitor.visit[KEFIR_IR_TYPE_UNION] = load_aggregate_argument;
    visitor.visit[KEFIR_IR_TYPE_ARRAY] = load_aggregate_argument;
    visitor.visit[KEFIR_IR_TYPE_BUILTIN] = load_builtin_argument;
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(func->params, &visitor, (void *) &param, 0,
                                                kefir_ir_type_children(func->params)));
    return KEFIR_OK;
}

static kefir_result_t save_registers(struct kefir_codegen_amd64 *codegen,
                                     const struct kefir_amd64_sysv_function *sysv_func) {
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
        kefir_asm_amd64_xasmgen_operand_indirect(
            &codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_STACK_BASE_REG),
            sysv_func->frame.base.register_save_area)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0],
                                                                 KEFIR_AMD64_SYSTEM_V_RUNTIME_SAVE_REGISTERS)));
    return KEFIR_OK;
}

kefir_result_t kefir_amd64_sysv_function_prologue(struct kefir_codegen_amd64 *codegen,
                                                  const struct kefir_amd64_sysv_function *func) {
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 code generator"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR function declaration"));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "Begin prologue of %s", func->func->name));
    REQUIRE_OK(preserve_state(&codegen->xasmgen));
    REQUIRE_OK(load_arguments(codegen, func));
    if (func->func->declaration->vararg) {
        REQUIRE_OK(save_registers(codegen, func));
    }
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "End prologue of %s", func->func->name));
    return KEFIR_OK;
}
