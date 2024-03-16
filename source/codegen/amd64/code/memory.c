/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/function.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(copy_memory)(struct kefir_mem *mem,
                                                                 struct kefir_codegen_amd64_function *function,
                                                                 const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t source_vreg, target_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.typed_refs.ref[0],
                                                    &target_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.typed_refs.ref[1],
                                                    &source_vreg));

    const struct kefir_ir_type *ir_type = kefir_ir_module_get_named_type(
        function->module->ir_module, instruction->operation.parameters.typed_refs.type_id);
    REQUIRE(ir_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type"));

    struct kefir_abi_amd64_type_layout type_layout;
    REQUIRE_OK(kefir_abi_amd64_type_layout(mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, ir_type, &type_layout));

    const struct kefir_abi_amd64_typeentry_layout *typeentry_layout = NULL;
    kefir_result_t res = kefir_abi_amd64_type_layout_at(
        &type_layout, instruction->operation.parameters.typed_refs.type_index, &typeentry_layout);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_type_layout_free(mem, &type_layout);
        return res;
    });

    kefir_size_t total_size = typeentry_layout->size;
    REQUIRE_OK(kefir_abi_amd64_type_layout_free(mem, &type_layout));

    REQUIRE_OK(kefir_codegen_amd64_copy_memory(mem, function, target_vreg, source_vreg, total_size));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(zero_memory)(struct kefir_mem *mem,
                                                                 struct kefir_codegen_amd64_function *function,
                                                                 const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    kefir_asmcmp_virtual_register_index_t target_vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.typed_refs.ref[0],
                                                    &target_vreg));

    const struct kefir_ir_type *ir_type = kefir_ir_module_get_named_type(
        function->module->ir_module, instruction->operation.parameters.typed_refs.type_id);
    REQUIRE(ir_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type"));

    struct kefir_abi_amd64_type_layout type_layout;
    REQUIRE_OK(kefir_abi_amd64_type_layout(mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, ir_type, &type_layout));

    const struct kefir_abi_amd64_typeentry_layout *typeentry_layout = NULL;
    kefir_result_t res = kefir_abi_amd64_type_layout_at(
        &type_layout, instruction->operation.parameters.typed_refs.type_index, &typeentry_layout);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_type_layout_free(mem, &type_layout);
        return res;
    });

    kefir_size_t total_size = typeentry_layout->size;
    REQUIRE_OK(kefir_abi_amd64_type_layout_free(mem, &type_layout));

    kefir_asmcmp_virtual_register_index_t destination_placement_vreg, source_placement_vreg, count_placement_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &destination_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &source_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &count_placement_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, destination_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_RDI));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, source_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_RAX));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, count_placement_vreg,
                                                                  KEFIR_AMD64_XASMGEN_REGISTER_RCX));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         destination_placement_vreg, target_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(source_placement_vreg), &KEFIR_ASMCMP_MAKE_UINT(0),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG64(count_placement_vreg),
                                      &KEFIR_ASMCMP_MAKE_UINT(total_size), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_stosb_rep(mem, &function->code,
                                            kefir_asmcmp_context_instr_tail(&function->code.context), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         destination_placement_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), source_placement_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), count_placement_vreg, NULL));

    return KEFIR_OK;
}
