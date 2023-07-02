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
#include "kefir/codegen/opt-system-v-amd64/runtime.h"
#include "kefir/target/abi/util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t visitor_not_supported(const struct kefir_ir_type *type, kefir_size_t index,
                                            const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    UNUSED(payload);
    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED, "Encountered not supported type code while traversing type");
}

struct vararg_getter_arg {
    struct kefir_mem *mem;
    struct kefir_codegen_opt_amd64 *codegen;
    const struct kefir_opt_function *function;
    struct kefir_opt_sysv_amd64_function *codegen_func;
    const struct kefir_opt_instruction *instr;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg_allocation;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation;
};

static kefir_result_t vararg_visit_integer(const struct kefir_ir_type *type, kefir_size_t index,
                                           const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct vararg_getter_arg *, arg, payload);
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid vararg type traversal argument"));

    struct kefir_codegen_opt_sysv_amd64_storage_register param_reg, result_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
        arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, arg->result_allocation,
        KEFIR_AMD64_XASMGEN_REGISTER_RAX, &result_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
        arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, arg->arg_allocation,
        KEFIR_AMD64_XASMGEN_REGISTER_RDI, &param_reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(arg->codegen, &arg->codegen_func->stack_frame_map,
                                                                arg->arg_allocation, param_reg.reg));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
        &arg->codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_label(&arg->codegen->xasmgen_helpers.operands[0],
                                              KEFIR_OPT_AMD64_SYSTEM_V_RUNTIME_LOAD_INT_VARARG)));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                     &arg->codegen_func->storage, &param_reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(arg->codegen, &arg->codegen_func->stack_frame_map,
                                                                 arg->result_allocation, result_reg.reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                     &arg->codegen_func->storage, &result_reg));

    return KEFIR_OK;
}

static kefir_result_t vararg_visit_sse(const struct kefir_ir_type *type, kefir_size_t index,
                                       const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    UNUSED(typeentry);
    ASSIGN_DECL_CAST(struct vararg_getter_arg *, arg, payload);
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid vararg type traversal argument"));

    struct kefir_codegen_opt_sysv_amd64_storage_register param_reg, tmp_reg, result_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
        arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, arg->result_allocation,
        KEFIR_AMD64_XASMGEN_REGISTER_XMM0, &result_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
        arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, arg->result_allocation,
        KEFIR_AMD64_XASMGEN_REGISTER_RAX, &tmp_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
        arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, arg->arg_allocation,
        KEFIR_AMD64_XASMGEN_REGISTER_RDI, &param_reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(arg->codegen, &arg->codegen_func->stack_frame_map,
                                                                arg->arg_allocation, param_reg.reg));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
        &arg->codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_label(&arg->codegen->xasmgen_helpers.operands[0],
                                              KEFIR_OPT_AMD64_SYSTEM_V_RUNTIME_LOAD_SSE_VARARG)));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                     &arg->codegen_func->storage, &param_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                     &arg->codegen_func->storage, &tmp_reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(arg->codegen, &arg->codegen_func->stack_frame_map,
                                                                 arg->result_allocation, result_reg.reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                     &arg->codegen_func->storage, &result_reg));

    return KEFIR_OK;
}

static kefir_result_t vararg_visit_memory_aggregate_impl(
    struct vararg_getter_arg *arg, const struct kefir_abi_sysv_amd64_typeentry_layout *param_layout,
    kefir_asm_amd64_xasmgen_register_t vararg_reg, kefir_asm_amd64_xasmgen_register_t result_reg,
    kefir_asm_amd64_xasmgen_register_t tmp_reg) {
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg),
        kefir_asm_amd64_xasmgen_operand_indirect(&arg->codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(vararg_reg),
                                                 KEFIR_AMD64_SYSV_ABI_QWORD)));
    if (param_layout->alignment > KEFIR_AMD64_SYSV_ABI_QWORD) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg),
            kefir_asm_amd64_xasmgen_operand_immu(&arg->codegen->xasmgen_helpers.operands[0],
                                                 param_layout->alignment - 1)));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_AND(
            &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg),
            kefir_asm_amd64_xasmgen_operand_imm(&arg->codegen->xasmgen_helpers.operands[0],
                                                -((kefir_int64_t) param_layout->alignment))));
    }

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
        &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_reg),
        kefir_asm_amd64_xasmgen_operand_indirect(
            &arg->codegen->xasmgen_helpers.operands[0], kefir_asm_amd64_xasmgen_operand_reg(result_reg),
            kefir_target_abi_pad_aligned(param_layout->size, KEFIR_AMD64_SYSV_ABI_QWORD))));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &arg->codegen->xasmgen,
        kefir_asm_amd64_xasmgen_operand_indirect(&arg->codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(vararg_reg),
                                                 KEFIR_AMD64_SYSV_ABI_QWORD),
        kefir_asm_amd64_xasmgen_operand_reg(tmp_reg)));

    return KEFIR_OK;
}

static kefir_result_t vararg_visit_memory_aggregate(struct vararg_getter_arg *arg,
                                                    const struct kefir_abi_sysv_amd64_typeentry_layout *param_layout) {
    struct kefir_codegen_opt_sysv_amd64_storage_register param_reg, result_reg, tmp_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, arg->result_allocation, &result_reg, NULL,
        NULL));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, arg->arg_allocation, &param_reg, NULL, NULL));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_any_general_purpose_register(
        arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, &tmp_reg, NULL, NULL));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(arg->codegen, &arg->codegen_func->stack_frame_map,
                                                                arg->arg_allocation, param_reg.reg));

    REQUIRE_OK(vararg_visit_memory_aggregate_impl(arg, param_layout, param_reg.reg, result_reg.reg, tmp_reg.reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                     &arg->codegen_func->storage, &tmp_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                     &arg->codegen_func->storage, &param_reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(arg->codegen, &arg->codegen_func->stack_frame_map,
                                                                 arg->result_allocation, result_reg.reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                     &arg->codegen_func->storage, &result_reg));
    return KEFIR_OK;
}

static kefir_result_t vararg_register_aggregate_check(
    struct vararg_getter_arg *arg, struct kefir_abi_sysv_amd64_parameter_allocation *param_allocation,
    kefir_asm_amd64_xasmgen_register_t vararg_reg, kefir_asm_amd64_xasmgen_register_t tmp_reg,
    kefir_id_t overflow_area_label, kefir_size_t *integer_qwords, kefir_size_t *sse_qwords) {
    kefir_size_t required_integers = 0;
    kefir_size_t required_sse = 0;
    for (kefir_size_t i = 0; i < kefir_vector_length(&param_allocation->container.qwords); i++) {
        ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_qword *, qword,
                         kefir_vector_at(&param_allocation->container.qwords, i));
        switch (qword->klass) {
            case KEFIR_AMD64_SYSV_PARAM_INTEGER:
                required_integers++;
                break;

            case KEFIR_AMD64_SYSV_PARAM_SSE:
                required_sse++;
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                       "Non-integer,sse vararg aggregate members are not supported");
        }
    }

    kefir_asm_amd64_xasmgen_register_t tmp_reg_variant;
    REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(tmp_reg, &tmp_reg_variant));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_reg_variant),
        kefir_asm_amd64_xasmgen_operand_pointer(
            &arg->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_DWORD,
            kefir_asm_amd64_xasmgen_operand_indirect(&arg->codegen->xasmgen_helpers.operands[1],
                                                     kefir_asm_amd64_xasmgen_operand_reg(vararg_reg), 0))));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
        &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_reg),
        kefir_asm_amd64_xasmgen_operand_immu(&arg->codegen->xasmgen_helpers.operands[0],
                                             required_integers * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CMP(
        &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_reg),
        kefir_asm_amd64_xasmgen_operand_immu(
            &arg->codegen->xasmgen_helpers.operands[0],
            KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTER_COUNT * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JA(
        &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_label(
                                    &arg->codegen->xasmgen_helpers.operands[0],
                                    kefir_asm_amd64_xasmgen_helpers_format(
                                        &arg->codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL,
                                        arg->function->ir_func->name, arg->instr->block_id, overflow_area_label))));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
        &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_reg_variant),
        kefir_asm_amd64_xasmgen_operand_pointer(
            &arg->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_DWORD,
            kefir_asm_amd64_xasmgen_operand_indirect(&arg->codegen->xasmgen_helpers.operands[1],
                                                     kefir_asm_amd64_xasmgen_operand_reg(vararg_reg),
                                                     KEFIR_AMD64_SYSV_ABI_QWORD / 2))));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
        &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_reg),
        kefir_asm_amd64_xasmgen_operand_immu(&arg->codegen->xasmgen_helpers.operands[0],
                                             required_sse * 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CMP(
        &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_reg),
        kefir_asm_amd64_xasmgen_operand_immu(
            &arg->codegen->xasmgen_helpers.operands[0],
            KEFIR_ABI_SYSV_AMD64_PARAMETER_INTEGER_REGISTER_COUNT * KEFIR_AMD64_SYSV_ABI_QWORD +
                2 * KEFIR_ABI_SYSV_AMD64_PARAMETER_SSE_REGISTER_COUNT * KEFIR_AMD64_SYSV_ABI_QWORD)));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JA(
        &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_label(
                                    &arg->codegen->xasmgen_helpers.operands[0],
                                    kefir_asm_amd64_xasmgen_helpers_format(
                                        &arg->codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL,
                                        arg->function->ir_func->name, arg->instr->block_id, overflow_area_label))));

    *integer_qwords = required_integers;
    *sse_qwords = required_sse;
    return KEFIR_OK;
}

static kefir_result_t vararg_register_aggregate_load(
    struct vararg_getter_arg *arg, struct kefir_abi_sysv_amd64_parameter_allocation *param_allocation,
    kefir_asm_amd64_xasmgen_register_t vararg_reg, kefir_asm_amd64_xasmgen_register_t result_reg,
    kefir_asm_amd64_xasmgen_register_t tmp_reg, kefir_asm_amd64_xasmgen_register_t tmp2_reg,
    kefir_id_t overflow_area_end_label, kefir_size_t integer_qwords, kefir_size_t sse_qwords) {
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_LEA(
        &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(result_reg),
        kefir_asm_amd64_xasmgen_operand_indirect(&arg->codegen->xasmgen_helpers.operands[0],
                                                 kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RBP),
                                                 arg->codegen_func->stack_frame_map.offset.temporary_area)));

    if (integer_qwords > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_reg),
            kefir_asm_amd64_xasmgen_operand_indirect(&arg->codegen->xasmgen_helpers.operands[0],
                                                     kefir_asm_amd64_xasmgen_operand_reg(vararg_reg),
                                                     2 * KEFIR_AMD64_SYSV_ABI_QWORD)));

        kefir_asm_amd64_xasmgen_register_t tmp2_reg_variant;
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(tmp2_reg, &tmp2_reg_variant));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp2_reg_variant),
            kefir_asm_amd64_xasmgen_operand_pointer(
                &arg->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_DWORD,
                kefir_asm_amd64_xasmgen_operand_indirect(&arg->codegen->xasmgen_helpers.operands[1],
                                                         kefir_asm_amd64_xasmgen_operand_reg(vararg_reg), 0))));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(&arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_reg),
                                                 kefir_asm_amd64_xasmgen_operand_reg(tmp2_reg)));

        kefir_size_t integer_offset = 0;
        for (kefir_size_t i = 0; i < kefir_vector_length(&param_allocation->container.qwords); i++) {
            ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_qword *, qword,
                             kefir_vector_at(&param_allocation->container.qwords, i));
            switch (qword->klass) {
                case KEFIR_AMD64_SYSV_PARAM_INTEGER:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                        &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp2_reg),
                        kefir_asm_amd64_xasmgen_operand_indirect(&arg->codegen->xasmgen_helpers.operands[0],
                                                                 kefir_asm_amd64_xasmgen_operand_reg(tmp_reg),
                                                                 integer_offset * KEFIR_AMD64_SYSV_ABI_QWORD)));

                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                        &arg->codegen->xasmgen,
                        kefir_asm_amd64_xasmgen_operand_indirect(&arg->codegen->xasmgen_helpers.operands[0],
                                                                 kefir_asm_amd64_xasmgen_operand_reg(result_reg),
                                                                 i * KEFIR_AMD64_SYSV_ABI_QWORD),
                        kefir_asm_amd64_xasmgen_operand_reg(tmp2_reg)));

                    integer_offset++;
                    break;

                case KEFIR_AMD64_SYSV_PARAM_SSE:
                    // Intentionally left blank
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                           "Non-integer,sse vararg aggregate members are not supported");
            }
        }

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &arg->codegen->xasmgen,
            kefir_asm_amd64_xasmgen_operand_pointer(
                &arg->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_DWORD,
                kefir_asm_amd64_xasmgen_operand_indirect(&arg->codegen->xasmgen_helpers.operands[1],
                                                         kefir_asm_amd64_xasmgen_operand_reg(vararg_reg), 0)),
            kefir_asm_amd64_xasmgen_operand_imm(&arg->codegen->xasmgen_helpers.operands[2],
                                                integer_offset * KEFIR_AMD64_SYSV_ABI_QWORD)));
    }

    if (sse_qwords > 0) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_reg),
            kefir_asm_amd64_xasmgen_operand_indirect(&arg->codegen->xasmgen_helpers.operands[0],
                                                     kefir_asm_amd64_xasmgen_operand_reg(vararg_reg),
                                                     2 * KEFIR_AMD64_SYSV_ABI_QWORD)));

        kefir_asm_amd64_xasmgen_register_t tmp2_reg_variant;
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register32(tmp2_reg, &tmp2_reg_variant));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp2_reg_variant),
            kefir_asm_amd64_xasmgen_operand_pointer(
                &arg->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_DWORD,
                kefir_asm_amd64_xasmgen_operand_indirect(&arg->codegen->xasmgen_helpers.operands[1],
                                                         kefir_asm_amd64_xasmgen_operand_reg(vararg_reg),
                                                         KEFIR_AMD64_SYSV_ABI_QWORD / 2))));

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(&arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp_reg),
                                                 kefir_asm_amd64_xasmgen_operand_reg(tmp2_reg)));

        kefir_size_t sse_offset = 0;
        for (kefir_size_t i = 0; i < kefir_vector_length(&param_allocation->container.qwords); i++) {
            ASSIGN_DECL_CAST(struct kefir_abi_sysv_amd64_qword *, qword,
                             kefir_vector_at(&param_allocation->container.qwords, i));
            switch (qword->klass) {
                case KEFIR_AMD64_SYSV_PARAM_INTEGER:
                    // Intentionally left blank
                    break;

                case KEFIR_AMD64_SYSV_PARAM_SSE:
                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                        &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(tmp2_reg),
                        kefir_asm_amd64_xasmgen_operand_indirect(&arg->codegen->xasmgen_helpers.operands[0],
                                                                 kefir_asm_amd64_xasmgen_operand_reg(tmp_reg),
                                                                 sse_offset * 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));

                    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
                        &arg->codegen->xasmgen,
                        kefir_asm_amd64_xasmgen_operand_indirect(&arg->codegen->xasmgen_helpers.operands[0],
                                                                 kefir_asm_amd64_xasmgen_operand_reg(result_reg),
                                                                 i * KEFIR_AMD64_SYSV_ABI_QWORD),
                        kefir_asm_amd64_xasmgen_operand_reg(tmp2_reg)));

                    sse_offset++;
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_NOT_SUPPORTED,
                                           "Non-integer,sse vararg aggregate members are not supported");
            }
        }

        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_ADD(
            &arg->codegen->xasmgen,
            kefir_asm_amd64_xasmgen_operand_pointer(
                &arg->codegen->xasmgen_helpers.operands[0], KEFIR_AMD64_XASMGEN_POINTER_DWORD,
                kefir_asm_amd64_xasmgen_operand_indirect(&arg->codegen->xasmgen_helpers.operands[1],
                                                         kefir_asm_amd64_xasmgen_operand_reg(vararg_reg),
                                                         KEFIR_AMD64_SYSV_ABI_QWORD / 2)),
            kefir_asm_amd64_xasmgen_operand_imm(&arg->codegen->xasmgen_helpers.operands[2],
                                                sse_offset * 2 * KEFIR_AMD64_SYSV_ABI_QWORD)));
    }

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_JMP(
        &arg->codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_label(
                                    &arg->codegen->xasmgen_helpers.operands[0],
                                    kefir_asm_amd64_xasmgen_helpers_format(
                                        &arg->codegen->xasmgen_helpers, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL,
                                        arg->function->ir_func->name, arg->instr->block_id, overflow_area_end_label))));
    return KEFIR_OK;
}

static kefir_result_t vararg_visit_register_aggregate(
    struct vararg_getter_arg *arg, const struct kefir_abi_sysv_amd64_typeentry_layout *param_layout,
    struct kefir_abi_sysv_amd64_parameter_allocation *param_allocation) {
    struct kefir_codegen_opt_sysv_amd64_storage_register param_reg, result_reg, tmp_reg, tmp2_reg;
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, arg->result_allocation, &result_reg, NULL,
        NULL));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_allocated_register(
        arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, arg->arg_allocation, &param_reg, NULL, NULL));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_any_general_purpose_register(
        arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, &tmp_reg, NULL, NULL));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_any_general_purpose_register(
        arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, &tmp2_reg, NULL, NULL));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(arg->codegen, &arg->codegen_func->stack_frame_map,
                                                                arg->arg_allocation, param_reg.reg));

    kefir_id_t overflow_area_label = arg->codegen_func->nonblock_labels++;
    kefir_id_t overflow_area_end_label = arg->codegen_func->nonblock_labels++;
    kefir_size_t integer_qwords, sse_qwords;
    REQUIRE_OK(vararg_register_aggregate_check(arg, param_allocation, param_reg.reg, tmp_reg.reg, overflow_area_label,
                                               &integer_qwords, &sse_qwords));
    REQUIRE_OK(vararg_register_aggregate_load(arg, param_allocation, param_reg.reg, result_reg.reg, tmp_reg.reg,
                                              tmp2_reg.reg, overflow_area_end_label, integer_qwords, sse_qwords));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&arg->codegen->xasmgen, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL,
                                         arg->function->ir_func->name, arg->instr->block_id, overflow_area_label));
    REQUIRE_OK(vararg_visit_memory_aggregate_impl(arg, param_layout, param_reg.reg, result_reg.reg, tmp_reg.reg));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&arg->codegen->xasmgen, KEFIR_OPT_AMD64_SYSTEM_V_FUNCTION_BLOCK_LABEL,
                                         arg->function->ir_func->name, arg->instr->block_id, overflow_area_end_label));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                     &arg->codegen_func->storage, &tmp2_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                     &arg->codegen_func->storage, &tmp_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                     &arg->codegen_func->storage, &param_reg));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(arg->codegen, &arg->codegen_func->stack_frame_map,
                                                                 arg->result_allocation, result_reg.reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                     &arg->codegen_func->storage, &result_reg));
    return KEFIR_OK;
}

static kefir_result_t vararg_visit_aggregate_impl(struct vararg_getter_arg *arg, const struct kefir_ir_type *type,
                                                  kefir_size_t index,
                                                  struct kefir_abi_sysv_amd64_type_layout *type_layout,
                                                  struct kefir_vector *allocation) {
    kefir_size_t slot;
    REQUIRE_OK(kefir_ir_type_slot_of(type, index, &slot));

    const struct kefir_abi_sysv_amd64_typeentry_layout *param_layout = NULL;
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_at(type_layout, index, &param_layout));

    struct kefir_abi_sysv_amd64_parameter_allocation *param_allocation =
        (struct kefir_abi_sysv_amd64_parameter_allocation *) kefir_vector_at(allocation, slot);
    REQUIRE(param_allocation != NULL,
            KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected to have layout and allocation info for slot"));

    if (param_allocation->klass == KEFIR_AMD64_SYSV_PARAM_MEMORY) {
        REQUIRE_OK(vararg_visit_memory_aggregate(arg, param_layout));
    } else {
        REQUIRE_OK(vararg_visit_register_aggregate(arg, param_layout, param_allocation));
    }
    return KEFIR_OK;
}

static kefir_result_t vararg_visit_aggregate(const struct kefir_ir_type *type, kefir_size_t index,
                                             const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(index);
    UNUSED(typeentry);
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));
    ASSIGN_DECL_CAST(struct vararg_getter_arg *, arg, payload);
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid vararg type traversal argument"));

    struct kefir_abi_sysv_amd64_type_layout type_layout;
    struct kefir_vector allocation;
    struct kefir_abi_sysv_amd64_parameter_location param_location = {0};

    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout(type, arg->mem, &type_layout));

    kefir_result_t res = kefir_abi_sysv_amd64_parameter_classify(arg->mem, type, &type_layout, &allocation);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_sysv_amd64_type_layout_free(arg->mem, &type_layout);
        return res;
    });

    res = kefir_abi_sysv_amd64_parameter_allocate(arg->mem, type, &type_layout, &allocation, &param_location);
    REQUIRE_CHAIN(&res, vararg_visit_aggregate_impl(arg, type, index, &type_layout, &allocation));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_sysv_amd64_parameter_free(arg->mem, &allocation);
        kefir_abi_sysv_amd64_type_layout_free(arg->mem, &type_layout);
        return res;
    });

    res = kefir_abi_sysv_amd64_parameter_free(arg->mem, &allocation);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_sysv_amd64_type_layout_free(arg->mem, &type_layout);
        return res;
    });
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_free(arg->mem, &type_layout));

    return KEFIR_OK;
}

static kefir_result_t vararg_visit_builtin(const struct kefir_ir_type *type, kefir_size_t index,
                                           const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(type);
    UNUSED(index);
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type entry"));
    ASSIGN_DECL_CAST(struct vararg_getter_arg *, arg, payload);
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid vararg type traversal argument"));

    switch (typeentry->param) {
        case KEFIR_IR_TYPE_BUILTIN: {
            struct kefir_codegen_opt_sysv_amd64_storage_register param_reg, result_reg;
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
                arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, arg->result_allocation,
                KEFIR_AMD64_XASMGEN_REGISTER_RAX, &result_reg));
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_acquire_specific_register(
                arg->mem, &arg->codegen->xasmgen, &arg->codegen_func->storage, arg->arg_allocation,
                KEFIR_AMD64_XASMGEN_REGISTER_RDI, &param_reg));

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(
                arg->codegen, &arg->codegen_func->stack_frame_map, arg->arg_allocation, param_reg.reg));

            REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_CALL(
                &arg->codegen->xasmgen,
                kefir_asm_amd64_xasmgen_operand_label(&arg->codegen->xasmgen_helpers.operands[0],
                                                      KEFIR_OPT_AMD64_SYSTEM_V_RUNTIME_LOAD_INT_VARARG)));

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                             &arg->codegen_func->storage, &param_reg));

            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_store_reg_allocation(
                arg->codegen, &arg->codegen_func->stack_frame_map, arg->result_allocation, result_reg.reg));
            REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(arg->mem, &arg->codegen->xasmgen,
                                                                             &arg->codegen_func->storage, &result_reg));
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unknown IR builtin type");
    }

    return KEFIR_OK;
}

DEFINE_TRANSLATOR(vararg_get) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *arg_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *result_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.refs[0], &arg_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(&codegen_func->register_allocator, instr_ref,
                                                                   &result_allocation));

    const kefir_id_t type_id = (kefir_id_t) instr->operation.parameters.typed_refs.type_id;
    const kefir_size_t type_index = (kefir_size_t) instr->operation.parameters.typed_refs.type_index;
    struct kefir_ir_type *type = kefir_ir_module_get_named_type(module->ir_module, type_id);
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown named IR type"));

    struct kefir_ir_type_visitor visitor;
    REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, visitor_not_supported));
    KEFIR_IR_TYPE_VISITOR_INIT_INTEGERS(&visitor, vararg_visit_integer);
    KEFIR_IR_TYPE_VISITOR_INIT_FIXED_FP(&visitor, vararg_visit_sse);
    visitor.visit[KEFIR_IR_TYPE_STRUCT] = vararg_visit_aggregate;
    visitor.visit[KEFIR_IR_TYPE_UNION] = vararg_visit_aggregate;
    visitor.visit[KEFIR_IR_TYPE_ARRAY] = vararg_visit_aggregate;
    visitor.visit[KEFIR_IR_TYPE_BUILTIN] = vararg_visit_builtin;
    visitor.visit[KEFIR_IR_TYPE_LONG_DOUBLE] = vararg_visit_aggregate;
    struct vararg_getter_arg param = {.mem = mem,
                                      .codegen = codegen,
                                      .function = function,
                                      .codegen_func = codegen_func,
                                      .instr = instr,
                                      .arg_allocation = arg_allocation,
                                      .result_allocation = result_allocation};
    REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, &visitor, (void *) &param, type_index, 1));

    return KEFIR_OK;
}
