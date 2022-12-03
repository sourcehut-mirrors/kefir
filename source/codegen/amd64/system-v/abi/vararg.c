/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#include "kefir/codegen/amd64/system-v/abi/vararg.h"
#include "kefir/target/abi/system-v-amd64/qwords.h"
#include "kefir/codegen/amd64/labels.h"
#include "kefir/codegen/amd64/system-v/runtime.h"
#include "kefir/target/abi/util.h"
#include "kefir/core/error.h"
#include "kefir/codegen/amd64/system-v/abi/builtins.h"

static kefir_result_t vararg_start(struct kefir_codegen_amd64 *codegen,
                                   struct kefir_codegen_amd64_sysv_module *sysv_module,
                                   const struct kefir_amd64_sysv_function *sysv_func, const char *identifier,
                                   void *payload) {
    UNUSED(sysv_module);
    UNUSED(payload);
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s", identifier));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&codegen->xasmgen,
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_amd64_xasmgen_operand_pointer(
            &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_QWORD,
            kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                 kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG), 0)),
        kefir_amd64_xasmgen_operand_immu(
            &codegen->xasmgen_helpers.operands[2],
            KEFIR_AMD64_SYSV_ABI_QWORD * sysv_func->decl.parameters.location.integer_register)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_amd64_xasmgen_operand_pointer(
            &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_DWORD,
            kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                 kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                                                 KEFIR_AMD64_SYSV_ABI_QWORD / 2)),
        kefir_amd64_xasmgen_operand_immu(
            &codegen->xasmgen_helpers.operands[2],
            KEFIR_AMD64_SYSV_ABI_QWORD * KEFIR_AMD64_SYSV_INTEGER_REGISTER_COUNT +
                2 * KEFIR_AMD64_SYSV_ABI_QWORD * sysv_func->decl.parameters.location.sse_register)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG),
        kefir_amd64_xasmgen_operand_indirect(
            &codegen->xasmgen_helpers.operands[0], kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
            2 * KEFIR_AMD64_SYSV_ABI_QWORD + sysv_func->decl.parameters.location.stack_offset)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                                             KEFIR_AMD64_SYSV_ABI_QWORD),
        kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG),
        kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_STACK_BASE_REG),
                                             sysv_func->frame.base.register_save_area)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                                             2 * KEFIR_AMD64_SYSV_ABI_QWORD),
        kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX),
        kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
        &codegen->xasmgen,
        kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX), 0)));
    return KEFIR_OK;
}

static kefir_result_t vararg_end(struct kefir_codegen_amd64 *codegen,
                                 struct kefir_codegen_amd64_sysv_module *sysv_module,
                                 const struct kefir_amd64_sysv_function *sysv_func, const char *identifier,
                                 void *payload) {
    UNUSED(sysv_module);
    UNUSED(sysv_func);
    UNUSED(payload);
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s", identifier));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSP),
        kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX),
        kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
        &codegen->xasmgen,
        kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX), 0)));
    return KEFIR_OK;
}

struct vararg_getter {
    struct kefir_mem *mem;
    struct kefir_codegen_amd64 *codegen;
    struct kefir_codegen_amd64_sysv_module *sysv_module;
    struct kefir_amd64_sysv_function *sysv_func;
    const char *identifier;
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

struct vararg_scalar_type {
    struct kefir_ir_typeentry typeentry;
};

static kefir_result_t free_vararg_scalar_type(struct kefir_mem *mem, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct vararg_scalar_type *, type, payload);
    KEFIR_FREE(mem, type);
    return KEFIR_OK;
}

static kefir_result_t mask_argument(struct kefir_codegen_amd64 *codegen, const struct kefir_ir_typeentry *typeentry,
                                    kefir_amd64_xasmgen_register_t reg) {
    switch (typeentry->typecode) {
        case KEFIR_IR_TYPE_BOOL:
        case KEFIR_IR_TYPE_CHAR:
        case KEFIR_IR_TYPE_INT8:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_EAX),
                kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 0xffull)));

            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_AND(&codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(reg),
                                              kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX)));
            break;

        case KEFIR_IR_TYPE_SHORT:
        case KEFIR_IR_TYPE_INT16:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_EAX),
                kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 0xffffull)));

            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_AND(&codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(reg),
                                              kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX)));
            break;

        case KEFIR_IR_TYPE_INT:
        case KEFIR_IR_TYPE_FLOAT32:
        case KEFIR_IR_TYPE_INT32:
            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_EAX),
                kefir_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0], -1)));

            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_INSTR_AND(&codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(reg),
                                              kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX)));
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
                    &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(reg),
                    kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 64 - bits)));

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_SHR(
                    &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(reg),
                    kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 64 - bits)));
            }
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected argument type");
    }
    return KEFIR_OK;
}

static kefir_result_t vararg_get_int(struct kefir_codegen_amd64 *codegen,
                                     struct kefir_codegen_amd64_sysv_module *sysv_module,
                                     const struct kefir_amd64_sysv_function *sysv_func, const char *identifier,
                                     void *payload) {
    UNUSED(sysv_module);
    UNUSED(sysv_func);
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));

    ASSIGN_DECL_CAST(struct vararg_scalar_type *, scalar_type, payload);
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s", identifier));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&codegen->xasmgen,
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0],
                                                             KEFIR_AMD64_SYSTEM_V_RUNTIME_VARARG_INT)));

    REQUIRE_OK(mask_argument(codegen, &scalar_type->typeentry, KEFIR_AMD64_SYSV_ABI_DATA2_REG));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen,
                                              kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX),
        kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
        &codegen->xasmgen,
        kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX), 0)));
    return KEFIR_OK;
}

static kefir_result_t vararg_visit_integer(const struct kefir_ir_type *type, kefir_size_t index,
                                           const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    char buffer[256];
    const kefir_size_t BUF_SIZE = sizeof(buffer);

    ASSIGN_DECL_CAST(struct vararg_getter *, param, payload);
    snprintf(buffer, BUF_SIZE - 1, KEFIR_AMD64_SYSV_FUNCTION_VARARG_ARG_SCALAR_LABEL, param->sysv_func->func->name,
             (kefir_int_t) typeentry->typecode);

    struct vararg_scalar_type *scalar_type = KEFIR_MALLOC(param->mem, sizeof(struct vararg_scalar_type));
    REQUIRE(scalar_type != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate vararg scalar type"));
    scalar_type->typeentry = *typeentry;

    kefir_result_t res = kefir_amd64_sysv_function_insert_appendix(param->mem, param->sysv_func, vararg_get_int,
                                                                   free_vararg_scalar_type, scalar_type, buffer);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(param->mem, scalar_type);
        REQUIRE(res == KEFIR_ALREADY_EXISTS, res);
    });
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
        kefir_amd64_xasmgen_operand_label(&param->codegen->xasmgen_helpers.operands[0], buffer),
        kefir_amd64_xasmgen_operand_immu(&param->codegen->xasmgen_helpers.operands[1], 0)));
    return KEFIR_OK;
}

static kefir_result_t vararg_get_sse(struct kefir_codegen_amd64 *codegen,
                                     struct kefir_codegen_amd64_sysv_module *sysv_module,
                                     const struct kefir_amd64_sysv_function *sysv_func, const char *identifier,
                                     void *payload) {
    UNUSED(sysv_module);
    UNUSED(sysv_func);
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));

    ASSIGN_DECL_CAST(struct vararg_scalar_type *, scalar_type, payload);
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s", identifier));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&codegen->xasmgen,
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0],
                                                             KEFIR_AMD64_SYSTEM_V_RUNTIME_VARARG_SSE)));

    if (scalar_type->typeentry.typecode == KEFIR_IR_TYPE_FLOAT64) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVQ(&codegen->xasmgen,
                                                  kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG),
                                                  kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_XMM0)));
    } else {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVD(&codegen->xasmgen,
                                                  kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG),
                                                  kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_XMM0)));
    }

    REQUIRE_OK(mask_argument(codegen, &scalar_type->typeentry, KEFIR_AMD64_SYSV_ABI_DATA2_REG));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen,
                                              kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX),
        kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
        &codegen->xasmgen,
        kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX), 0)));
    return KEFIR_OK;
}

static kefir_result_t vararg_visit_sse(const struct kefir_ir_type *type, kefir_size_t index,
                                       const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    char buffer[256];
    const kefir_size_t BUF_SIZE = sizeof(buffer);

    ASSIGN_DECL_CAST(struct vararg_getter *, param, payload);
    snprintf(buffer, BUF_SIZE - 1, KEFIR_AMD64_SYSV_FUNCTION_VARARG_ARG_SCALAR_LABEL, param->sysv_func->func->name,
             (kefir_int_t) typeentry->typecode);

    struct vararg_scalar_type *scalar_type = KEFIR_MALLOC(param->mem, sizeof(struct vararg_scalar_type));
    REQUIRE(scalar_type != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate vararg scalar type"));
    scalar_type->typeentry = *typeentry;

    kefir_result_t res = kefir_amd64_sysv_function_insert_appendix(param->mem, param->sysv_func, vararg_get_sse,
                                                                   free_vararg_scalar_type, scalar_type, buffer);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(param->mem, scalar_type);
        REQUIRE(res == KEFIR_ALREADY_EXISTS, res);
    });

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
        kefir_amd64_xasmgen_operand_label(&param->codegen->xasmgen_helpers.operands[0], buffer),
        kefir_amd64_xasmgen_operand_immu(&param->codegen->xasmgen_helpers.operands[1], 0)));
    return KEFIR_OK;
}

static kefir_result_t vararg_visit_long_double(const struct kefir_ir_type *type, kefir_size_t index,
                                               const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct vararg_getter *, param, payload);

    REQUIRE_OK(
        KEFIR_AMD64_XASMGEN_DATA(&param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                                 kefir_amd64_xasmgen_operand_label(&param->codegen->xasmgen_helpers.operands[0],
                                                                   KEFIR_AMD64_SYSTEM_V_RUNTIME_VARARG_LONG_DOUBLE),
                                 kefir_amd64_xasmgen_operand_immu(&param->codegen->xasmgen_helpers.operands[1], 0)));
    return KEFIR_OK;
}

struct vararg_aggregate_info {
    const struct kefir_abi_sysv_amd64_typeentry_layout *arg_layout;
    struct kefir_amd64_sysv_parameter_allocation *arg_alloc;

    struct kefir_abi_sysv_amd64_type_layout layout;
    struct kefir_vector allocation;
    struct kefir_amd64_sysv_parameter_location location;
};

static kefir_result_t vararg_load_memory_aggregate(struct kefir_codegen_amd64 *codegen,
                                                   struct vararg_aggregate_info *info) {
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG),
        kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                                             KEFIR_AMD64_SYSV_ABI_QWORD)));
    if (info->arg_layout->alignment > KEFIR_AMD64_SYSV_ABI_QWORD) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG),
            kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], info->arg_layout->alignment - 1)));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_AND(
            &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG),
            kefir_amd64_xasmgen_operand_imm(&codegen->xasmgen_helpers.operands[0],
                                            -((kefir_int64_t) info->arg_layout->alignment))));
    }
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen,
                                              kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG),
        kefir_amd64_xasmgen_operand_immu(
            &codegen->xasmgen_helpers.operands[0],
            kefir_target_abi_pad_aligned(info->arg_layout->size, KEFIR_AMD64_SYSV_ABI_QWORD))));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                                             KEFIR_AMD64_SYSV_ABI_QWORD),
        kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG)));
    return KEFIR_OK;
}

static kefir_result_t vararg_get_memory_aggregate(struct kefir_codegen_amd64 *codegen,
                                                  struct kefir_codegen_amd64_sysv_module *sysv_module,
                                                  const struct kefir_amd64_sysv_function *sysv_func,
                                                  const char *identifier, void *payload) {
    UNUSED(sysv_module);
    UNUSED(sysv_func);
    ASSIGN_DECL_CAST(struct vararg_aggregate_info *, info, payload);
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s", identifier));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&codegen->xasmgen,
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));

    REQUIRE_OK(vararg_load_memory_aggregate(codegen, info));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX),
        kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
        &codegen->xasmgen,
        kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX), 0)));
    return KEFIR_OK;
}

struct aggregate_requirements {
    kefir_size_t integers;
    kefir_size_t sse;
};

static kefir_result_t register_aggregate_requirements(struct vararg_aggregate_info *info,
                                                      struct aggregate_requirements *req) {
    req->integers = 0;
    req->sse = 0;
    for (kefir_size_t i = 0; i < kefir_vector_length(&info->arg_alloc->container.qwords); i++) {
        ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_qword *, qword,
                         kefir_vector_at(&info->arg_alloc->container.qwords, i));
        switch (qword->klass) {
            case KEFIR_AMD64_SYSV_PARAM_INTEGER:
                req->integers++;
                break;

            case KEFIR_AMD64_SYSV_PARAM_SSE:
                req->sse++;
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                       "Non-integer,sse vararg aggregate members are not supported");
        }
    }
    return KEFIR_OK;
}

static kefir_result_t vararg_get_register_aggregate_intro(struct kefir_codegen_amd64 *codegen,
                                                          const struct kefir_amd64_sysv_function *sysv_func,
                                                          const char *identifier, struct vararg_aggregate_info *info,
                                                          struct aggregate_requirements *requirements) {
    UNUSED(sysv_func);
    REQUIRE_OK(register_aggregate_requirements(info, requirements));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_EAX),
        kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG), 0)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX),
        kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                         requirements->integers * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CMP(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX),
        kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                         KEFIR_AMD64_SYSV_INTEGER_REGISTER_COUNT * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JA(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_label(
                               &codegen->xasmgen_helpers.operands[0],
                               kefir_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, "%s_stack", identifier))));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_EAX),
        kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                                             KEFIR_AMD64_SYSV_ABI_QWORD / 2)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX),
        kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                         requirements->sse * 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CMP(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX),
        kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                         KEFIR_AMD64_SYSV_INTEGER_REGISTER_COUNT * KEFIR_AMD64_SYSV_ABI_QWORD +
                                             2 * KEFIR_AMD64_SYSV_SSE_REGISTER_COUNT * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JA(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_label(
                               &codegen->xasmgen_helpers.operands[0],
                               kefir_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, "%s_stack", identifier))));
    return KEFIR_OK;
}
static kefir_result_t vararg_get_register_aggregate_load(struct kefir_codegen_amd64 *codegen,
                                                         const struct kefir_amd64_sysv_function *sysv_func,
                                                         const char *identifier, struct vararg_aggregate_info *info,
                                                         const struct aggregate_requirements *requirements) {
    UNUSED(sysv_func);
    UNUSED(info);
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG),
        kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_STACK_BASE_REG),
                                             KEFIR_AMD64_SYSV_INTERNAL_BOUND)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_PUSH(&codegen->xasmgen,
                                              kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSI),
        kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                                             2 * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_EAX),
        kefir_amd64_xasmgen_operand_pointer(
            &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_DWORD,
            kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                 kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG), 0))));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDX),
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(&codegen->xasmgen,
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDX),
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSI)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX),
        kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                         requirements->integers * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_amd64_xasmgen_operand_pointer(
            &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_DWORD,
            kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                 kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG), 0)),
        kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_EAX)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_EAX),
        kefir_amd64_xasmgen_operand_pointer(
            &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_DWORD,
            kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                 kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                                                 KEFIR_AMD64_SYSV_ABI_QWORD / 2))));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RCX),
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(&codegen->xasmgen,
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RCX),
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RSI)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX),
        kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0],
                                         requirements->sse * 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &codegen->xasmgen,
        kefir_amd64_xasmgen_operand_pointer(
            &codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_DWORD,
            kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[1],
                                                 kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG),
                                                 KEFIR_AMD64_SYSV_ABI_QWORD / 2)),
        kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_EAX)));

    kefir_size_t integer_offset = 0, sse_offset = 0;
    for (kefir_size_t i = 0; i < kefir_vector_length(&info->arg_alloc->container.qwords); i++) {
        ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_qword *, qword,
                         kefir_vector_at(&info->arg_alloc->container.qwords, i));
        switch (qword->klass) {
            case KEFIR_AMD64_SYSV_PARAM_INTEGER:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG),
                    kefir_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RDX),
                        integer_offset * KEFIR_AMD64_SYSV_ABI_QWORD)));

                REQUIRE_OK(
                    KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
                                                  kefir_amd64_xasmgen_operand_indirect(
                                                      &codegen->xasmgen_helpers.operands[0],
                                                      kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG),
                                                      i * KEFIR_AMD64_SYSV_ABI_QWORD),
                                                  kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG)));

                integer_offset++;
                break;

            case KEFIR_AMD64_SYSV_PARAM_SSE:
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                    &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG),
                    kefir_amd64_xasmgen_operand_indirect(
                        &codegen->xasmgen_helpers.operands[0],
                        kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RCX),
                        sse_offset * 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));

                REQUIRE_OK(
                    KEFIR_AMD64_XASMGEN_INSTR_MOV(&codegen->xasmgen,
                                                  kefir_amd64_xasmgen_operand_indirect(
                                                      &codegen->xasmgen_helpers.operands[0],
                                                      kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA2_REG),
                                                      i * KEFIR_AMD64_SYSV_ABI_QWORD),
                                                  kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_TMP_REG)));

                sse_offset++;
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                       "Non-integer,sse vararg aggregate members are not supported");
        }
    }

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_label(
                               &codegen->xasmgen_helpers.operands[0],
                               kefir_amd64_xasmgen_helpers_format(&codegen->xasmgen_helpers, "%s_end", identifier))));
    return KEFIR_OK;
}

static kefir_result_t vararg_get_register_aggregate(struct kefir_codegen_amd64 *codegen,
                                                    struct kefir_codegen_amd64_sysv_module *sysv_module,
                                                    const struct kefir_amd64_sysv_function *sysv_func,
                                                    const char *identifier, void *payload) {
    UNUSED(sysv_module);
    UNUSED(sysv_func);
    ASSIGN_DECL_CAST(struct vararg_aggregate_info *, info, payload);
    struct aggregate_requirements requirements;

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s", identifier));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_POP(&codegen->xasmgen,
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_SYSV_ABI_DATA_REG)));

    REQUIRE_OK(vararg_get_register_aggregate_intro(codegen, sysv_func, identifier, info, &requirements));
    REQUIRE_OK(vararg_get_register_aggregate_load(codegen, sysv_func, identifier, info, &requirements));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s_stack", identifier));
    REQUIRE_OK(vararg_load_memory_aggregate(codegen, info));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&codegen->xasmgen, "%s_end", identifier));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
        &codegen->xasmgen, kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX),
        kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
        &codegen->xasmgen,
        kefir_amd64_xasmgen_operand_indirect(&codegen->xasmgen_helpers.operands[0],
                                             kefir_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBX), 0)));
    return KEFIR_OK;
}

static kefir_result_t free_vararg_aggregate_info(struct kefir_mem *mem, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid memory allocator"));
    REQUIRE(payload != NULL, KEFIR_OK);
    ASSIGN_DECL_CAST(struct vararg_aggregate_info *, info, payload);
    REQUIRE_OK(kefir_amd64_sysv_parameter_free(mem, &info->allocation));
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_free(mem, &info->layout));
    KEFIR_FREE(mem, payload);
    return KEFIR_OK;
}

static kefir_result_t vararg_visit_memory_aggregate(struct vararg_aggregate_info *info, struct vararg_getter *param) {
    kefir_result_t res = kefir_amd64_sysv_function_insert_appendix(
        param->mem, param->sysv_func, vararg_get_memory_aggregate, free_vararg_aggregate_info, info, param->identifier);
    REQUIRE(res == KEFIR_OK || res == KEFIR_ALREADY_EXISTS, res);
    kefir_result_t result = res;
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
        kefir_amd64_xasmgen_operand_label(&param->codegen->xasmgen_helpers.operands[0], param->identifier),
        kefir_amd64_xasmgen_operand_immu(&param->codegen->xasmgen_helpers.operands[1], 0)));
    return result;
}

static kefir_result_t vararg_visit_register_aggregate(struct vararg_aggregate_info *info, struct vararg_getter *param) {
    kefir_result_t res =
        kefir_amd64_sysv_function_insert_appendix(param->mem, param->sysv_func, vararg_get_register_aggregate,
                                                  free_vararg_aggregate_info, info, param->identifier);
    REQUIRE(res == KEFIR_OK || res == KEFIR_ALREADY_EXISTS, res);
    kefir_result_t result = res;
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
        &param->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
        kefir_amd64_xasmgen_operand_label(&param->codegen->xasmgen_helpers.operands[0], param->identifier),
        kefir_amd64_xasmgen_operand_immu(&param->codegen->xasmgen_helpers.operands[1], 0)));
    return result;
}

static kefir_result_t vararg_visit_aggregate(const struct kefir_ir_type *type, kefir_size_t index,
                                             const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct vararg_getter *, param, payload);
    struct kefir_ir_type_iterator iter;
    REQUIRE_OK(kefir_ir_type_iterator_init(type, &iter));
    REQUIRE_OK(kefir_ir_type_iterator_goto(&iter, index));

    struct vararg_aggregate_info *info = KEFIR_MALLOC(param->mem, sizeof(struct vararg_aggregate_info));
    REQUIRE(info != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate memory aggregate vararg info"));
    *info = (struct vararg_aggregate_info){0};

    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout(type, param->mem, &info->layout));
    kefir_result_t res = kefir_amd64_sysv_parameter_classify(param->mem, type, &info->layout.layout, &info->allocation);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_sysv_amd64_type_layout_free(param->mem, &info->layout);
        KEFIR_FREE(param->mem, info);
        return res;
    });

    res =
        kefir_amd64_sysv_parameter_allocate(param->mem, type, &info->layout.layout, &info->allocation, &info->location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_vector_free(param->mem, &info->allocation);
        kefir_abi_sysv_amd64_type_layout_free(param->mem, &info->layout);
        KEFIR_FREE(param->mem, info);
        return res;
    });

    info->arg_layout = NULL;
    res = kefir_abi_sysv_amd64_type_layout_at(&info->layout, index, &info->arg_layout);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_amd64_sysv_parameter_free(param->mem, &info->allocation);
        kefir_abi_sysv_amd64_type_layout_free(param->mem, &info->layout);
        KEFIR_FREE(param->mem, info);
        return res;
    });
    info->arg_alloc = (struct kefir_amd64_sysv_parameter_allocation *) kefir_vector_at(&info->allocation, iter.slot);
    REQUIRE_ELSE(info->arg_alloc != NULL, {
        kefir_amd64_sysv_parameter_free(param->mem, &info->allocation);
        kefir_abi_sysv_amd64_type_layout_free(param->mem, &info->layout);
        KEFIR_FREE(param->mem, info);
        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected to have layout and allocation info for slot");
    });
    res = KEFIR_OK;
    if (info->arg_alloc->klass == KEFIR_AMD64_SYSV_PARAM_MEMORY) {
        res = vararg_visit_memory_aggregate(info, param);
    } else {
        res = vararg_visit_register_aggregate(info, param);
    }
    if (res == KEFIR_ALREADY_EXISTS) {
        return free_vararg_aggregate_info(param->mem, info);
    } else {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t vararg_visit_builtin_impl(struct kefir_mem *mem, struct kefir_vector *allocation,
                                                kefir_size_t slot, const struct kefir_ir_typeentry *typeentry,
                                                struct kefir_codegen_amd64 *codegen,
                                                struct kefir_amd64_sysv_function *sysv_func, const char *identifier) {
    ASSIGN_DECL_CAST(struct kefir_amd64_sysv_parameter_allocation *, alloc, kefir_vector_at(allocation, slot));
    REQUIRE(alloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unable to find parameter allocation information"));

    kefir_ir_builtin_type_t builtin = (kefir_ir_builtin_type_t) typeentry->param;
    REQUIRE(builtin < KEFIR_IR_TYPE_BUILTIN_COUNT, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unknown built-in type"));
    const struct kefir_codegen_amd64_sysv_builtin_type *builtin_type = KEFIR_CODEGEN_AMD64_SYSV_BUILTIN_TYPES[builtin];
    REQUIRE_OK(builtin_type->load_vararg(mem, builtin_type, typeentry, codegen, sysv_func, identifier, alloc));
    return KEFIR_OK;
}

static kefir_result_t vararg_visit_builtin(const struct kefir_ir_type *type, kefir_size_t index,
                                           const struct kefir_ir_typeentry *typeentry, void *payload) {
    ASSIGN_DECL_CAST(struct vararg_getter *, param, payload);
    struct kefir_ir_type_iterator iter;
    REQUIRE_OK(kefir_ir_type_iterator_init(type, &iter));
    REQUIRE_OK(kefir_ir_type_iterator_goto(&iter, index));

    struct kefir_abi_sysv_amd64_type_layout layout;
    struct kefir_vector allocation;

    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout(type, param->mem, &layout));
    kefir_result_t res = kefir_amd64_sysv_parameter_classify(param->mem, type, &layout.layout, &allocation);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_sysv_amd64_type_layout_free(param->mem, &layout);
        return res;
    });
    res = vararg_visit_builtin_impl(param->mem, &allocation, iter.slot, typeentry, param->codegen, param->sysv_func,
                                    param->identifier);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_amd64_sysv_parameter_free(param->mem, &allocation);
        kefir_abi_sysv_amd64_type_layout_free(param->mem, &layout);
        return res;
    });

    REQUIRE_OK(kefir_amd64_sysv_parameter_free(param->mem, &allocation));
    kefir_abi_sysv_amd64_type_layout_free(param->mem, &layout);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_sysv_vararg_instruction(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                                           struct kefir_codegen_amd64_sysv_module *sysv_module,
                                                           struct kefir_amd64_sysv_function *sysv_func,
                                                           const struct kefir_irinstr *instr) {
    UNUSED(codegen);
    UNUSED(sysv_module);
    UNUSED(sysv_func);

    char buffer[256];
    const kefir_size_t BUF_SIZE = sizeof(buffer);

    switch (instr->opcode) {
        case KEFIR_IROPCODE_VARARG_START: {
            snprintf(buffer, BUF_SIZE - 1, KEFIR_AMD64_SYSV_FUNCTION_VARARG_START_LABEL, sysv_func->func->name);
            kefir_result_t res =
                kefir_amd64_sysv_function_insert_appendix(mem, sysv_func, vararg_start, NULL, NULL, buffer);
            REQUIRE(res == KEFIR_OK || res == KEFIR_ALREADY_EXISTS, res);

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], buffer),
                kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], 0)));
        } break;

        case KEFIR_IROPCODE_VARARG_COPY:
            REQUIRE_OK(
                KEFIR_AMD64_XASMGEN_DATA(&codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                                         kefir_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0],
                                                                           KEFIR_AMD64_SYSTEM_V_RUNTIME_VARARG_COPY),
                                         kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], 0)));
            break;

        case KEFIR_IROPCODE_VARARG_GET: {
            snprintf(buffer, BUF_SIZE - 1, KEFIR_AMD64_SYSV_FUNCTION_VARARG_ARG_LABEL, sysv_func->func->name,
                     instr->arg.u32[0], instr->arg.u32[1]);

            const kefir_id_t type_id = (kefir_id_t) instr->arg.u32[0];
            const kefir_size_t type_index = (kefir_size_t) instr->arg.u32[1];
            struct kefir_ir_type *type = kefir_ir_module_get_named_type(sysv_module->module, type_id);
            REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown named IR type"));
            struct kefir_ir_type_visitor visitor;
            REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, visitor_not_supported));
            KEFIR_IR_TYPE_VISITOR_INIT_INTEGERS(&visitor, vararg_visit_integer);
            KEFIR_IR_TYPE_VISITOR_INIT_FIXED_FP(&visitor, vararg_visit_sse);
            visitor.visit[KEFIR_IR_TYPE_LONG_DOUBLE] = vararg_visit_long_double;
            visitor.visit[KEFIR_IR_TYPE_STRUCT] = vararg_visit_aggregate;
            visitor.visit[KEFIR_IR_TYPE_UNION] = vararg_visit_aggregate;
            visitor.visit[KEFIR_IR_TYPE_ARRAY] = vararg_visit_aggregate;
            visitor.visit[KEFIR_IR_TYPE_BUILTIN] = vararg_visit_builtin;
            struct vararg_getter param = {.codegen = codegen,
                                          .sysv_module = sysv_module,
                                          .sysv_func = sysv_func,
                                          .identifier = buffer,
                                          .mem = mem};
            REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, &visitor, (void *) &param, type_index, 1));
        } break;

        case KEFIR_IROPCODE_VARARG_END: {
            snprintf(buffer, BUF_SIZE - 1, KEFIR_AMD64_SYSV_FUNCTION_VARARG_END_LABEL, sysv_func->func->name);
            kefir_result_t res =
                kefir_amd64_sysv_function_insert_appendix(mem, sysv_func, vararg_end, NULL, NULL, buffer);
            REQUIRE(res == KEFIR_OK || res == KEFIR_ALREADY_EXISTS, res);

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                &codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 2,
                kefir_amd64_xasmgen_operand_label(&codegen->xasmgen_helpers.operands[0], buffer),
                kefir_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[1], 0)));
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected built-in opcode");
    }
    return KEFIR_OK;
}
