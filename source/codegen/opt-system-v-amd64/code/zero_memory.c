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

DEFINE_TRANSLATOR(zero_memory) {
    DEFINE_TRANSLATOR_PROLOGUE;

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(kefir_opt_code_container_instr(&function->code, instr_ref, &instr));

    const struct kefir_codegen_opt_sysv_amd64_register_allocation *destination_allocation = NULL;

    REQUIRE_OK(kefir_codegen_opt_sysv_amd64_register_allocation_of(
        &codegen_func->register_allocator, instr->operation.parameters.typed_refs.ref[0], &destination_allocation));

    struct kefir_codegen_opt_amd64_sysv_storage_handle source_handle;
    struct kefir_codegen_opt_amd64_sysv_storage_handle destination_handle;
    struct kefir_codegen_opt_amd64_sysv_storage_handle count_handle;

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_SPECIFIC_REGISTER(KEFIR_AMD64_XASMGEN_REGISTER_RAX), NULL,
        &source_handle, NULL, NULL));
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_SPECIFIC_REGISTER(KEFIR_AMD64_XASMGEN_REGISTER_RDI),
        destination_allocation, &destination_handle, NULL, NULL));

    struct kefir_codegen_opt_amd64_sysv_storage_location destination_location_origin;

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_location_from_reg_allocation(
        &destination_location_origin, &codegen_func->stack_frame_map, destination_allocation));

    struct kefir_codegen_opt_amd64_sysv_storage_transform transform;
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_init(&transform));

    kefir_result_t res = kefir_codegen_opt_amd64_sysv_storage_transform_insert(
        mem, &transform, &destination_handle.location, &destination_location_origin);
    REQUIRE_CHAIN(&res, kefir_codegen_opt_amd64_sysv_storage_transform_perform(
                            mem, codegen, &codegen_func->storage, &codegen_func->stack_frame_map, &transform));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_codegen_opt_amd64_sysv_storage_transform_free(mem, &transform);
        return res;
    });
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_transform_free(mem, &transform));

    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_acquire(
        mem, &codegen->xasmgen, &codegen_func->storage, &codegen_func->stack_frame_map,
        KEFIR_CODEGEN_OPT_AMD64_SYSV_STORAGE_ACQUIRE_SPECIFIC_REGISTER(KEFIR_AMD64_XASMGEN_REGISTER_RCX),
        destination_allocation, &count_handle, NULL, NULL));

    const struct kefir_ir_type *ir_type =
        kefir_ir_module_get_named_type(module->ir_module, instr->operation.parameters.typed_refs.type_id);
    REQUIRE(ir_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type"));

    struct kefir_abi_amd64_type_layout type_layout;
    REQUIRE_OK(kefir_abi_amd64_type_layout(mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, ir_type, &type_layout));

    const struct kefir_abi_amd64_typeentry_layout *typeentry_layout = NULL;
    res = kefir_abi_amd64_type_layout_at(&type_layout, instr->operation.parameters.typed_refs.type_index,
                                         &typeentry_layout);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_type_layout_free(mem, &type_layout);
        return res;
    });

    kefir_size_t total_size = typeentry_layout->size;
    REQUIRE_OK(kefir_abi_amd64_type_layout_free(mem, &type_layout));

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_XOR(&codegen->xasmgen,
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX),
                                             kefir_asm_amd64_xasmgen_operand_reg(KEFIR_AMD64_XASMGEN_REGISTER_RAX)));
    if (total_size > KEFIR_INT32_MAX) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOVABS(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(count_handle.location.reg),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], total_size)));
    } else {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_MOV(
            &codegen->xasmgen, kefir_asm_amd64_xasmgen_operand_reg(count_handle.location.reg),
            kefir_asm_amd64_xasmgen_operand_immu(&codegen->xasmgen_helpers.operands[0], total_size)));
    }
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_INSTR_STOSB(&codegen->xasmgen, true));

    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &count_handle));
    REQUIRE_OK(kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage,
                                                            &destination_handle));
    REQUIRE_OK(
        kefir_codegen_opt_amd64_sysv_storage_release(mem, &codegen->xasmgen, &codegen_func->storage, &source_handle));
    return KEFIR_OK;
}
