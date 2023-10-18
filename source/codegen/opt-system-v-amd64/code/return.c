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

#include "kefir/codegen/opt-system-v-amd64/code_impl.h"
#include "kefir/target/abi/amd64/return.h"
#include "kefir/ir/builtins.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct result_return {
    struct kefir_codegen_opt_amd64 *codegen;
    struct kefir_opt_sysv_amd64_function *codegen_func;
    const struct kefir_opt_instruction *instr;
};

static kefir_result_t visitor_not_supported(const struct kefir_ir_type *type, kefir_size_t index,
                                            const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    UNUSED(payload);
    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Unsupported function return type");
}

static kefir_result_t return_integer(const struct kefir_ir_type *type, kefir_size_t index,
                                     const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    struct result_return *param = (struct result_return *) payload;
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen return parameter"));

    if (param->instr->operation.parameters.refs[0] != KEFIR_ID_NONE) {
        const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation = NULL;
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
            &param->codegen_func->register_allocator, param->instr->operation.parameters.refs[0], &reg_allocation));
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(
            param->codegen, &param->codegen_func->stack_frame_map, reg_allocation, KEFIR_AMD64_XASMGEN_REGISTER_RAX));
    } else {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_XOR(
            &param->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX),
            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX)));
    }
    return KEFIR_OK;
}

static kefir_result_t return_sse(const struct kefir_ir_type *type, kefir_size_t index,
                                 const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    struct result_return *param = (struct result_return *) payload;
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen return parameter"));

    if (param->instr->operation.parameters.refs[0] != KEFIR_ID_NONE) {
        const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation = NULL;
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
            &param->codegen_func->register_allocator, param->instr->operation.parameters.refs[0], &reg_allocation));
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(
            param->codegen, &param->codegen_func->stack_frame_map, reg_allocation, KEFIR_AMD64_XASMGEN_REGISTER_XMM0));
    } else {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PXOR(
            &param->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_XMM0),
            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_XMM0)));
    }
    return KEFIR_OK;
}

static kefir_result_t return_memory_aggregate(struct kefir_codegen_opt_amd64 *codegen,
                                              const struct kefir_abi_amd64_typeentry_layout *layout,
                                              struct kefir_opt_sysv_amd64_function *codegen_func,
                                              const struct kefir_opt_instruction *instr) {
    if (instr->operation.parameters.refs[0] != KEFIR_ID_NONE) {
        const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation = NULL;
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
            &codegen_func->register_allocator, instr->operation.parameters.refs[0], &reg_allocation));
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map,
                                                                    reg_allocation, KEFIR_AMD64_XASMGEN_REGISTER_RSI));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDI),
            kefir_asm_amd64_xasmgen_operand_indirect(
                &codegen->xasmgen_helpers.operands[0],
                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                codegen_func->stack_frame_map.offset.implicit_parameter)));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX),
            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDI)));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RCX),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], layout->size)));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CLD(&codegen->xasmgen));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVSB(&codegen->xasmgen, true));
    } else {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX),
            kefir_asm_amd64_xasmgen_operand_indirect(
                &codegen->xasmgen_helpers.operands[0],
                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                codegen_func->stack_frame_map.offset.implicit_parameter)));
    }
    return KEFIR_OK;
}

static kefir_result_t return_register_aggregate(struct kefir_codegen_opt_amd64 *codegen,
                                                const struct kefir_abi_amd64_function_parameter *return_param,
                                                struct kefir_opt_sysv_amd64_function *codegen_func,
                                                const struct kefir_opt_instruction *instr) {
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &reg_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map, reg_allocation,
                                                                KEFIR_AMD64_XASMGEN_REGISTER_RSI));

    kefir_size_t length;
    REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_length(return_param, &length));
    for (kefir_size_t i = 0; i < length; i++) {
        struct kefir_abi_amd64_function_parameter subparam;
        REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_at(return_param, i, &subparam));
        // kefir_asm_amd64_xasmgen_register_t reg;
        switch (subparam.location) {
            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_GENERAL_PURPOSE_REGISTER:
                REQUIRE(subparam.direct_reg != KEFIR_AMD64_XASMGEN_REGISTER_RSI,
                        KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Conflict in return register allocation"));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(subparam.direct_reg),
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSI),
                        i * KEFIR_AMD64_ABI_QWORD)));
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_SSE_REGISTER:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(subparam.direct_reg),
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSI),
                        i * KEFIR_AMD64_ABI_QWORD)));
                break;

            case KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87:
                REQUIRE(i + 1 < length,
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected X87 qword to be directly followed by X87UP"));
                REQUIRE_OK(kefir_abi_amd64_function_parameter_multireg_at(return_param, ++i, &subparam));
                REQUIRE(subparam.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_X87UP,
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected X87 qword to be directly followed by X87UP"));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FLD(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_pointer(
                                           &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_TBYTE,
                                           kefir_asm_amd64_xasmgen_operand_indirect(
                                               &codegen->xasmgen_helpers.operands[1],
                                               kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSI),
                                               (i - 1) * KEFIR_AMD64_ABI_QWORD))));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                       "Return of non-integer,sse aggregate members is not supported");
        }
    }
    return KEFIR_OK;
}

static kefir_result_t return_aggregate(const struct kefir_ir_type *type, kefir_size_t index,
                                       const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(typeentry);
    struct result_return *param = (struct result_return *) payload;
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen return parameter"));

    const struct kefir_abi_amd64_function_parameters *returns_param;
    struct kefir_abi_amd64_function_parameter return_param;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    REQUIRE_OK(kefir_abi_amd64_function_decl_returns(&param->codegen_func->declaration, &returns_param));
    REQUIRE_OK(kefir_abi_amd64_function_parameters_at(returns_param, slot, &return_param));

    const struct kefir_abi_amd64_type_layout *return_layout;
    const struct kefir_abi_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_amd64_function_decl_returns_layout(&param->codegen_func->declaration, &return_layout));
    REQUIRE_OK(kefir_abi_amd64_type_layout_at(return_layout, index, &layout));
    if (return_param.location == KEFIR_ABI_AMD64_FUNCTION_PARAMETER_LOCATION_MEMORY) {
        REQUIRE_OK(return_memory_aggregate(param->codegen, layout, param->codegen_func, param->instr));
    } else if (param->instr->operation.parameters.refs[0] != KEFIR_ID_NONE) {
        REQUIRE_OK(return_register_aggregate(param->codegen, &return_param, param->codegen_func, param->instr));
    }
    return KEFIR_OK;
}

static kefir_result_t return_builtin(const struct kefir_ir_type *type, kefir_size_t index,
                                     const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(payload);
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type entry"));

    switch (typeentry->param) {
        case KEFIR_IR_TYPE_BUILTIN_VARARG:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Returning va_list from function is not supported");

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown IR builtin type");
    }
    return KEFIR_OK;
}

static kefir_result_t return_long_double(const struct kefir_ir_type *type, kefir_size_t index,
                                         const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    struct result_return *param = (struct result_return *) payload;
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen return parameter"));

    if (param->instr->operation.parameters.refs[0] != KEFIR_ID_NONE) {
        const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation = NULL;
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
            &param->codegen_func->register_allocator, param->instr->operation.parameters.refs[0], &reg_allocation));
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(
            param->codegen, &param->codegen_func->stack_frame_map, reg_allocation, KEFIR_AMD64_XASMGEN_REGISTER_RAX));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FLD(
            &param->codegen->xasmgen,
            kefir_asm_amd64_xasmgen_operand_pointer(
                &param->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_TBYTE,
                kefir_asm_amd64_xasmgen_operand_indirect(
                    &param->codegen->xasmgen_helpers.operands[1],
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX), 0))));
    } else {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FLDZ(&param->codegen->xasmgen));
    }
    return KEFIR_OK;
}

DEFINE_TRANSLATOR(return) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    struct kefir_ir_type_visitor visitor;
    REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, visitor_not_supported));
    KEFIR_IR_TYPE_VISITOR_INIT_INTEGERS(&visitor, return_integer);
    KEFIR_IR_TYPE_VISITOR_INIT_FIXED_FP(&visitor, return_sse);
    visitor.visit[KEFIR_IR_TYPE_LONG_DOUBLE] = return_long_double;
    visitor.visit[KEFIR_IR_TYPE_STRUCT] = return_aggregate;
    visitor.visit[KEFIR_IR_TYPE_UNION] = return_aggregate;
    visitor.visit[KEFIR_IR_TYPE_ARRAY] = return_aggregate;
    visitor.visit[KEFIR_IR_TYPE_BUILTIN] = return_builtin;
    struct result_return param = {.codegen = codegen, .codegen_func = codegen_func, .instr = instr};
    REQUIRE_OK(
        kefir_ir_type_visitor_list_nodes(function->ir_func->declaration->result, &visitor, (void *) &param, 0, 1));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_stack_frame_epilogue(&codegen_func->stack_frame, &codegen->xasmgen));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_RET(&codegen->xasmgen));
    return KEFIR_OK;
}
