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
#include "kefir/codegen/opt-system-v-amd64/storage_transform.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

DEFINE_TRANSLATOR(memory) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *source_allocation = NULL;
    const struct kefir_codegen_opt_sysv_amd64_register_allocation *destination_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.typed_refs.ref[1], &source_allocation));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.typed_refs.ref[0], &destination_allocation));

    struct kefir_codegen_opt_sysv_amd64_translate_temporary_register source_reg;
    struct kefir_codegen_opt_sysv_amd64_translate_temporary_register destination_reg;
    struct kefir_codegen_opt_sysv_amd64_translate_temporary_register count_reg;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain_specific(
        mem, codegen, source_allocation, KEFIR_AMD64_XASMGEN_REGISTER_RSI, codegen_func, &source_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain_specific(
        mem, codegen, destination_allocation, KEFIR_AMD64_XASMGEN_REGISTER_RDI, codegen_func, &destination_reg));

    struct kefir_codegen_opt_amd64_sysv_storage_transform_location source_location_origin;
    struct kefir_codegen_opt_amd64_sysv_storage_transform_location destination_location_origin;
    struct kefir_codegen_opt_amd64_sysv_storage_transform_location source_location_intended = {
        .type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_REGISTER, .reg = source_reg.reg};
    struct kefir_codegen_opt_amd64_sysv_storage_transform_location destination_location_intended = {
        .type = KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_TRANSFORM_REGISTER, .reg = destination_reg.reg};

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_location_from_reg_allocation(
        &source_location_origin, &codegen_func->stack_frame_map, source_allocation));
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_location_from_reg_allocation(
        &destination_location_origin, &codegen_func->stack_frame_map, destination_allocation));

    struct kefir_codegen_opt_amd64_sysv_storage_transform transform;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_init(&transform));

    kefir_result_t res = kefir_codegen_opt_amd64_sysv_storage_transform_insert(
        mem, &transform, &source_location_intended, &source_location_origin);
    REQUIRE_CHAIN(&res, kefir_codegen_opt_amd64_sysv_storage_transform_insert(
                            mem, &transform, &destination_location_intended, &destination_location_origin));
    REQUIRE_CHAIN(&res, kefir_codegen_opt_amd64_sysv_storage_transform_perform(mem, codegen, codegen_func, &transform));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_opt_amd64_sysv_storage_transform_free(mem, &transform);
        return res;
    });
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_free(mem, &transform));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_general_purpose_register_obtain_specific(
        mem, codegen, destination_allocation, KEFIR_AMD64_XASMGEN_REGISTER_RCX, codegen_func, &count_reg));

    const struct kefir_ir_type *ir_type =
        kefir_ir_module_get_named_type(module->ir_module, instr->operation.parameters.typed_refs.type_id);
    REQUIRE(ir_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type"));

    struct kefir_abi_sysv_amd64_type_layout type_layout;
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout(ir_type, mem, &type_layout));

    const struct kefir_abi_sysv_amd64_typeentry_layout *typeentry_layout = NULL;
    res = kefir_abi_sysv_amd64_type_layout_at(&type_layout, instr->operation.parameters.typed_refs.type_index,
                                              &typeentry_layout);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_sysv_amd64_type_layout_free(mem, &type_layout);
        return res;
    });

    kefir_size_t total_size = typeentry_layout->size;
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_free(mem, &type_layout));

    if (total_size > KEFIR_INT32_MAX) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVABS(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(count_reg.reg),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], total_size)));
    } else {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(count_reg.reg),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], total_size)));
    }
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVSB(&codegen->xasmgen, true));

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &count_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &destination_reg));
    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_temporary_register_free(mem, codegen, codegen_func, &source_reg));
    return KEFIR_OK;
}
