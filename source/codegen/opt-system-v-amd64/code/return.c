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
#include "kefir/target/abi/system-v-amd64/return.h"
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

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &param->codegen_func->register_allocator, param->instr->operation.parameters.refs[0], &reg_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(param->codegen, &param->codegen_func->stack_frame_map,
                                                                reg_allocation, KEFIR_AMD64_XASMGEN_REGISTER_RAX));
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

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &param->codegen_func->register_allocator, param->instr->operation.parameters.refs[0], &reg_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(param->codegen, &param->codegen_func->stack_frame_map,
                                                                reg_allocation, KEFIR_AMD64_XASMGEN_REGISTER_XMM0));
    return KEFIR_OK;
}

static kefir_result_t return_memory_aggregate(struct kefir_codegen_opt_amd64 *codegen,
                                              const struct kefir_abi_sysv_amd64_typeentry_layout *layout,
                                              struct kefir_opt_sysv_amd64_function *codegen_func,
                                              const struct kefir_opt_instruction *instr) {
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &reg_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map, reg_allocation,
                                                                KEFIR_AMD64_XASMGEN_REGISTER_RSI));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDI),
        kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                                                 codegen_func->stack_frame_map.offset.implicit_parameter)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX),
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDI)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RCX),
        kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], layout->size)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CLD(&codegen->xasmgen));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVSB(&codegen->xasmgen, true));
    return KEFIR_OK;
}

static kefir_result_t return_register_aggregate(struct kefir_codegen_opt_amd64 *codegen,
                                                struct kefir_abi_sysv_amd64_parameter_allocation *alloc,
                                                struct kefir_opt_sysv_amd64_function *codegen_func,
                                                const struct kefir_opt_instruction *instr) {
    kefir_size_t integer_register = 0;
    kefir_size_t sse_register = 0;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &reg_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, &codegen_func->stack_frame_map, reg_allocation,
                                                                KEFIR_AMD64_XASMGEN_REGISTER_RSI));

    for (kefir_size_t i = 0; i < kefir_vector_length(&alloc->container.qwords); i++) {
        ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_qword *, qword, kefir_vector_at(&alloc->container.qwords, i));
        switch (qword->klass) {
            case KEFIR_AMD64_SYSV_PARAM_INTEGER:
                REQUIRE(integer_register < KEFIR_ABI_SYSV_AMD64_RETURN_INTEGER_REGISTER_COUNT,
                        KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR,
                                        "Unable to return aggregate which exceeds available registers"));
                REQUIRE(
                    KEFIR_ABI_SYSV_AMD64_RETURN_INTEGER_REGISTERS[integer_register] != KEFIR_AMD64_XASMGEN_REGISTER_RSI,
                    KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Conflict in return register allocation"));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_reg(
                        KEFIR_ABI_SYSV_AMD64_RETURN_INTEGER_REGISTERS[integer_register++]),
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSI),
                        i * KEFIR_AMD64_SYSV_ABI_QWORD)));
                break;

            case KEFIR_AMD64_SYSV_PARAM_SSE:
                REQUIRE(sse_register < KEFIR_ABI_SYSV_AMD64_RETURN_SSE_REGISTER_COUNT,
                        KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR,
                                        "Unable to return aggregate which exceeds available registers"));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(
                    &codegen->xasmgen,
                    kefir_asm_amd64_xasmgen_operand_reg(KEFIR_ABI_SYSV_AMD64_RETURN_SSE_REGISTERS[sse_register++]),
                    kefir_asm_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSI),
                        i * KEFIR_AMD64_SYSV_ABI_QWORD)));
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
                                               kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSI),
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
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen return parameter"));

    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));
    ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_parameter_allocation *, alloc,
                     kefir_vector_at(&param->codegen_func->declaration.returns.allocation, slot));
    const struct kefir_abi_sysv_amd64_typeentry_layout *layout = NULL;
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_at(&param->codegen_func->declaration.returns.layout, index, &layout));
    if (alloc->klass == KEFIR_AMD64_SYSV_PARAM_MEMORY) {
        REQUIRE_OK(return_memory_aggregate(param->codegen, layout, param->codegen_func, param->instr));
    } else {
        REQUIRE_OK(return_register_aggregate(param->codegen, alloc, param->codegen_func, param->instr));
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

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation = NULL;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &param->codegen_func->register_allocator, param->instr->operation.parameters.refs[0], &reg_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(param->codegen, &param->codegen_func->stack_frame_map,
                                                                reg_allocation, KEFIR_AMD64_XASMGEN_REGISTER_RAX));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_FLD(
        &param->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_pointer(
                                      &param->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_TBYTE,
                                      kefir_asm_amd64_xasmgen_operand_indirect(
                                          &param->codegen->xasmgen_helpers.operands[1],
                                          kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX), 0))));
    return KEFIR_OK;
}

DEFINE_TRANSLATOR(return) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    if (instr->operation.parameters.refs[0] != KEFIR_ID_NONE) {
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
    }

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_stack_frame_epilogue(&codegen_func->stack_frame, &codegen->xasmgen));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_RET(&codegen->xasmgen));
    return KEFIR_OK;
}
