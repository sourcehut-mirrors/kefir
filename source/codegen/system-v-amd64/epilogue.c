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
#include "kefir/target/abi/system-v-amd64/data_layout.h"
#include "kefir/codegen/system-v-amd64/registers.h"
#include "kefir/codegen/system-v-amd64/builtins.h"
#include "kefir/target/abi/util.h"
#include <stdio.h>

struct result_return {
    struct kefir_codegen_amd64 *codegen;
    const struct kefir_amd64_sysv_function *func;
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
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&param->codegen->xasmgen,
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX)));
    return KEFIR_OK;
}

static kefir_result_t return_float(const struct kefir_ir_type *type, kefir_size_t index,
                                   const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    struct result_return *param = (struct result_return *) payload;
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&param->codegen->xasmgen,
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PINSRQ(
        &param->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_XMM0),
        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
        kefir_asm_amd64_xasmgen_operand_imm(&param->codegen->xasmgen_helpers.operands[0], 0)));
    return KEFIR_OK;
}

static kefir_result_t return_long_double(const struct kefir_ir_type *type, kefir_size_t index,
                                         const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    struct result_return *param = (struct result_return *) payload;
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&param->codegen->xasmgen,
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FLD(
        &param->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_pointer(
                                      &param->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_TBYTE,
                                      kefir_asm_amd64_xasmgen_operand_indirect(
                                          &param->codegen->xasmgen_helpers.operands[1],
                                          kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG), 0))));
    return KEFIR_OK;
}

static kefir_result_t return_memory_aggregate(struct kefir_codegen_amd64 *codegen,
                                              const struct kefir_abi_sysv_amd64_typeentry_layout *layout) {
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDI),
        kefir_asm_amd64_xasmgen_operand_indirect(
            &codegen->xasmgen_helpers.operands[0],
            kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_STACK_BASE_REG),
            KEFIR_AMD64_SYSV_INTERNAL_RETURN_ADDRESS * KEFIR_AMD64_SYSV_ABI_QWORD)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX),
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDI)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&codegen->xasmgen,
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSI)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RCX),
        kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], layout->size)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CLD(&codegen->xasmgen));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVSB(&codegen->xasmgen, true));
    return KEFIR_OK;
}

static kefir_result_t return_register_aggregate(struct kefir_codegen_amd64 *codegen,
                                                struct kefir_abi_sysv_amd64_parameter_allocation *alloc) {
    kefir_size_t integer_register = 0;
    kefir_size_t sse_register = 0;
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&codegen->xasmgen,
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));
    for (kefir_size_t i = 0; i < kefir_vector_length(&alloc->container.qwords); i++) {
        ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_qword *, qword, kefir_vector_at(&alloc->container.qwords, i));
        switch (qword->klass) {
            case KEFIR_AMD64_SYSV_PARAM_INTEGER:
                if (integer_register >= KEFIR_ABI_SYSV_AMD64_RETURN_INTEGER_REGISTER_COUNT) {
                    return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR,
                                           "Unable to return aggregate which exceeds available registers");
                }
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_reg(
                        KEFIR_ABI_SYSV_AMD64_RETURN_INTEGER_REGISTERS[integer_register++]),
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                        i * KEFIR_AMD64_SYSV_ABI_QWORD)));
                break;

            case KEFIR_AMD64_SYSV_PARAM_SSE:
                if (sse_register >= KEFIR_ABI_SYSV_AMD64_RETURN_SSE_REGISTER_COUNT) {
                    return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR,
                                           "Unable to return aggregate which exceeds available registers");
                }
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG),
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                        i * KEFIR_AMD64_SYSV_ABI_QWORD)));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PINSRQ(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_ABI_SYSV_AMD64_RETURN_SSE_REGISTERS[sse_register++]),
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG),
                    kefir_asm_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0], 0)));
                break;

            case KEFIR_AMD64_SYSV_PARAM_X87:
                REQUIRE(i + 1 < kefir_vector_length(&alloc->container.qwords),
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected X87 qword to be directly followed by X87UP"));
                ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_qword *, next_qword,
                                 kefir_vector_at(&alloc->container.qwords, ++i));
                REQUIRE(next_qword->klass == KEFIR_AMD64_SYSV_PARAM_X87UP,
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected X87 qword to be directly followed by X87UP"));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FLD(
                    &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_pointer(
                                           &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_TBYTE,
                                           kefir_asm_amd64_xasmgen_operand_indirect(
                                               &codegen->xasmgen_helpers.operands[1],
                                               kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                                               (i - 1) * KEFIR_AMD64_SYSV_ABI_QWORD))));
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
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, alloc,
                     kefir_vector_at(&param->func->decl.returns.allocation, slot));
    const struct kefir_abi_sysv_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_at(&param->func->decl.returns.layout, index, &layout));
    if (alloc->klass == KEFIR_AMD64_SYSV_PARAM_MEMORY) {
        REQUIRE_OK(return_memory_aggregate(param->codegen, layout));
    } else {
        REQUIRE_OK(return_register_aggregate(param->codegen, alloc));
    }
    return KEFIR_OK;
}

static kefir_result_t return_builtin(const struct kefir_ir_type *type, kefir_size_t index,
                                     const struct kefir_ir_typeentry *typeentry, void *payload) {
    struct result_return *param = (struct result_return *) payload;
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, alloc,
                     kefir_vector_at(&param->func->decl.returns.allocation, slot));
    kefir_ir_builtin_type_t builtin = (kefir_ir_builtin_type_t) typeentry->param;
    REQUIRE(builtin < KEFIR_IR_TYPE_BUILTIN_COUNT, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unknown built-in type"));
    const struct kefir_codegen_amd64_sysv_builtin_type *builtin_type =
        KEFIR_CODEGEN_SYSTEM_V_AMD64_SYSV_BUILTIN_TYPES[builtin];
    REQUIRE_OK(builtin_type->store_function_return(builtin_type, typeentry, param->codegen, alloc));
    return KEFIR_OK;
}

static kefir_result_t return_values(struct kefir_codegen_amd64 *codegen, const struct kefir_amd64_sysv_function *func) {
    struct kefir_ir_type_visitor visitor;
    REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, visitor_not_supported));
    KEFIR_IR_TYPE_VISITOR_INIT_INTEGERS(&visitor, return_integer);
    KEFIR_IR_TYPE_VISITOR_INIT_FIXED_FP(&visitor, return_float);
    visitor.visit[KEFIR_IR_TYPE_LONG_DOUBLE] = return_long_double;
    visitor.visit[KEFIR_IR_TYPE_STRUCT] = return_aggregate;
    visitor.visit[KEFIR_IR_TYPE_UNION] = return_aggregate;
    visitor.visit[KEFIR_IR_TYPE_ARRAY] = return_aggregate;
    visitor.visit[KEFIR_IR_TYPE_BUILTIN] = return_builtin;
    struct result_return param = {.codegen = codegen, .func = func};
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(func->func->declaration->result, &visitor, (void *) &param, 0, 1));
    return KEFIR_OK;
}

static kefir_result_t restore_state(struct kefir_codegen_amd64 *codegen, const struct kefir_amd64_sysv_function *func) {
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_STACK_BASE_REG)));
    if (func->frame.size > 0 && func->frame.size <= KEFIR_INT32_MAX) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], func->frame.size)));
    } else if (func->frame.size > KEFIR_INT32_MAX) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVABS(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], func->frame.size)));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(&codegen->xasmgen,
                                                 kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                                 kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));
    }
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0],
                                                                 KEFIR_AMD64_SYSTEM_V_RUNTIME_RESTORE_STATE)));
    return KEFIR_OK;
}

kefir_result_t kefir_amd64_sysv_function_epilogue(struct kefir_codegen_amd64 *codegen,
                                                  const struct kefir_amd64_sysv_function *func) {
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 code generator"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR function declaration"));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "Begin epilogue of %s", func->func->name));
    REQUIRE_OK(return_values(codegen, func));
    REQUIRE_OK(restore_state(codegen, func));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_COMMENT(&codegen->xasmgen, "End of %s", func->func->name));
    return KEFIR_OK;
}
