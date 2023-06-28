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
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_codegen_opt_sysv_amd64_filter_regs_allocation(kefir_asm_amd64_xasmgen_register_t reg,
                                                                   kefir_bool_t *success, void *payload) {
    REQUIRE(success != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to payload"));

    ASSIGN_DECL_CAST(const struct kefir_codegen_opt_sysv_amd64_register_allocation **, allocation_iter, payload);
    for (; *allocation_iter != NULL; ++allocation_iter) {
        if (((*allocation_iter)->result.type ==
                 KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER ||
             (*allocation_iter)->result.type ==
                 KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_FLOATING_POINT_REGISTER) &&
            (*allocation_iter)->result.reg == reg) {
            *success = false;
            return KEFIR_OK;
        }
    }
    *success = true;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_floating_point_operand_init(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
    struct kefir_codegen_opt_sysv_amd64_storage *storage,
    struct kefir_codegen_opt_sysv_amd64_stack_frame_map *stack_frame_map,
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *reg_allocation,
    struct kefir_codegen_opt_sysv_amd64_floating_point_operand *fp_operand,
    kefir_result_t (*filter_callback)(kefir_asm_amd64_xasmgen_register_t, kefir_bool_t *, void *),
    void *filter_payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(stack_frame_map != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen stack frame map"));
    REQUIRE(fp_operand != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to the floating point operand"));

    fp_operand->reg_allocation = reg_allocation;
    if (reg_allocation == NULL ||
        reg_allocation->result.type == KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER) {
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_try_acquire_exclusive_floating_point_allocated_register(
            mem, &codegen->xasmgen, storage, reg_allocation, &fp_operand->storage_reg, filter_callback,
            filter_payload));

        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_load_reg_allocation(codegen, stack_frame_map, reg_allocation,
                                                                    fp_operand->storage_reg.reg));

        fp_operand->operand = kefir_asm_amd64_xasmgen_operand_reg(fp_operand->storage_reg.reg);
    } else {
        fp_operand->operand = kefir_codegen_opt_sysv_amd64_reg_allocation_operand(&fp_operand->value_operand,
                                                                                  stack_frame_map, reg_allocation);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_opt_sysv_amd64_floating_point_operand_free(
    struct kefir_mem *mem, struct kefir_codegen_opt_amd64 *codegen,
    struct kefir_codegen_opt_sysv_amd64_storage *storage,
    struct kefir_codegen_opt_sysv_amd64_floating_point_operand *fp_operand) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen"));
    REQUIRE(storage != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer codegen storage"));
    REQUIRE(fp_operand != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to the floating point operand"));

    if (fp_operand->reg_allocation == NULL ||
        fp_operand->reg_allocation->result.type ==
            KEFIR_CODEGEN_OPT_SYSV_AMD64_REGISTER_ALLOCATION_GENERAL_PURPOSE_REGISTER) {
        REQUIRE_OK(kefir_codegen_opt_sysv_amd64_storage_release_register(mem, &codegen->xasmgen, storage,
                                                                         &fp_operand->storage_reg));
    }
    return KEFIR_OK;
}
