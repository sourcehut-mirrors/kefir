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

#include "kefir/codegen/system-v-amd64/builtins.h"
#include "kefir/codegen/system-v-amd64/symbolic_labels.h"
#include "kefir/target/abi/util.h"
#include "kefir/core/error.h"

static kefir_result_t vararg_load_function_argument(const struct kefir_codegen_amd64_sysv_builtin_type *builtin_type,
                                                    const struct kefir_ir_typeentry *typeentry,
                                                    struct kefir_codegen_amd64 *codegen,
                                                    struct kefir_abi_sysv_amd64_parameter_allocation *allocation) {
    UNUSED(builtin_type);
    UNUSED(typeentry);
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid code generator"));
    REQUIRE(allocation != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid built-in data allocation"));

    if (allocation->klass == KEFIR_AMD64_SYSV_PARAM_INTEGER) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(
            &codegen->xasmgen,
            kefir_asm_amd64_xasmgen_operand_reg(
                KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTERS[allocation->location.integer_register])));
    } else {
        REQUIRE(allocation->klass == KEFIR_AMD64_SYSV_PARAM_MEMORY,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected va_list to be either integer or memory parameter"));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
            kefir_asm_amd64_xasmgen_operand_indirect(
                &codegen->xasmgen_helpers.operands[0],
                kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                allocation->location.stack_offset + 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen,
                                                  kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));
    }
    return KEFIR_OK;
}

static kefir_result_t vararg_store_function_return(const struct kefir_codegen_amd64_sysv_builtin_type *builtin_type,
                                                   const struct kefir_ir_typeentry *typeentry,
                                                   struct kefir_codegen_amd64 *codegen,
                                                   struct kefir_abi_sysv_amd64_parameter_allocation *allocation) {
    UNUSED(builtin_type);
    UNUSED(typeentry);
    UNUSED(codegen);
    UNUSED(allocation);
    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Returning va_list from function is not supported");
}

static kefir_result_t vararg_store_function_argument(const struct kefir_codegen_amd64_sysv_builtin_type *builtin_type,
                                                     const struct kefir_ir_typeentry *typeentry,
                                                     struct kefir_codegen_amd64 *codegen,
                                                     struct kefir_abi_sysv_amd64_parameter_allocation *allocation,
                                                     kefir_size_t argument_index) {
    UNUSED(builtin_type);
    UNUSED(typeentry);
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid code generator"));
    REQUIRE(allocation != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid built-in data allocation"));

    if (allocation->klass == KEFIR_AMD64_SYSV_PARAM_INTEGER) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen,
            kefir_asm_amd64_xasmgen_operand_reg(
                KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTERS[allocation->location.integer_register]),
            kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                     kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                                                     argument_index * KEFIR_AMD64_SYSV_ABI_QWORD)));
    } else {
        REQUIRE(allocation->klass == KEFIR_AMD64_SYSV_PARAM_MEMORY,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected va_list to be either integer or memory parameter"));
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG),
            kefir_asm_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                                     kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                                                     argument_index * KEFIR_AMD64_SYSV_ABI_QWORD)));
        REQUIRE_OK(
            KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
                                          kefir_asm_amd64_xasmgen_operand_indirect(
                                              &codegen->xasmgen_helpers.operands[0],
                                              kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
                                              allocation->location.stack_offset),
                                          kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG)));
    }
    return KEFIR_OK;
}

static kefir_result_t vararg_load_function_return(const struct kefir_codegen_amd64_sysv_builtin_type *builtin_type,
                                                  const struct kefir_ir_typeentry *typeentry,
                                                  struct kefir_codegen_amd64 *codegen,
                                                  struct kefir_abi_sysv_amd64_parameter_allocation *allocation) {
    UNUSED(builtin_type);
    UNUSED(typeentry);
    UNUSED(codegen);
    UNUSED(allocation);
    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Returning va_list from function is not supported");
}

static kefir_result_t vararg_load_vararg_impl(struct kefir_codegen_amd64 *codegen) {
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0],
                                                                 KEFIR_AMD64_SYSTEM_V_RUNTIME_VARARG_INT)));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen,
                                              kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG)));
    return KEFIR_OK;
}

static kefir_result_t vararg_load_vararg_appendix(struct kefir_codegen_amd64 *codegen,
                                                  struct kefir_codegen_amd64_sysv_module *sysv_module,
                                                  const struct kefir_amd64_sysv_function *sysv_func,
                                                  const char *identifier, void *payload) {
    UNUSED(sysv_module);
    UNUSED(sysv_func);
    UNUSED(payload);
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s", identifier));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&codegen->xasmgen,
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));

    REQUIRE_OK(vararg_load_vararg_impl(codegen));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX),
        kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
        &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_indirect(
                               &codegen->xasmgen_helpers.operands[0],
                               kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX), 0)));
    return KEFIR_OK;
}

static kefir_result_t vararg_load_vararg(struct kefir_mem *mem,
                                         const struct kefir_codegen_amd64_sysv_builtin_type *builtin_type,
                                         const struct kefir_ir_typeentry *typeentry,
                                         struct kefir_codegen_amd64 *codegen,
                                         struct kefir_amd64_sysv_function *sysv_func, const char *identifier,
                                         struct kefir_abi_sysv_amd64_parameter_allocation *allocation) {
    UNUSED(builtin_type);
    UNUSED(typeentry);
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid code generator"));
    REQUIRE(sysv_func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 System-V function"));
    REQUIRE(allocation != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid built-in data allocation"));
    kefir_result_t res =
        kefir_amd64_sysv_function_insert_appendix(mem, sysv_func, vararg_load_vararg_appendix, NULL, NULL, identifier);
    REQUIRE(res == KEFIR_OK || res == KEFIR_ALREADY_EXISTS, res);
    kefir_result_t result = res;
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
        kefir_asm_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], identifier),
        kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], 0)));
    return result;
    return KEFIR_OK;
}

const struct kefir_codegen_amd64_sysv_builtin_type KEFIR_CODEGEN_SYSTEM_V_AMD64_SYSV_BUILIN_VARARG_TYPE = {
    .load_function_argument = vararg_load_function_argument,
    .store_function_return = vararg_store_function_return,
    .store_function_argument = vararg_store_function_argument,
    .load_function_return = vararg_load_function_return,
    .load_vararg = vararg_load_vararg};
