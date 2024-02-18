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
#include "kefir/target/abi/amd64/return.h"
#include "kefir/target/abi/amd64/type_layout.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

#define LIBATOMIC_SEQ_CST 5

#define LIBATOMIC_LOAD_N(_n) "__atomic_load_" #_n
#define LIBATOMIC_STORE_N(_n) "__atomic_store_" #_n
#define LIBATOMIC_LOAD "__atomic_load"
#define LIBATOMIC_STORE "__atomic_store"
#define LIBATOMIC_CMPXCHG_N(_n) "__atomic_compare_exchange_" #_n
#define LIBATOMIC_CMPXCHG "__atomic_compare_exchange"

static kefir_result_t get_memorder(kefir_opt_memory_order_t model, kefir_int64_t *memorder) {
    *memorder = LIBATOMIC_SEQ_CST;
    switch (model) {
        case KEFIR_OPT_MEMORY_ORDER_SEQ_CST:
            // Intentionally left blank
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected atomic memory order");
    }
    return KEFIR_OK;
}

static kefir_result_t preserve_regs(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                    kefir_asmcmp_stash_index_t *stash_idx) {
    const kefir_size_t num_of_preserved_gp_regs =
        kefir_abi_amd64_num_of_caller_preserved_general_purpose_registers(function->codegen->abi_variant);
    const kefir_size_t num_of_preserved_sse_regs =
        kefir_abi_amd64_num_of_caller_preserved_sse_registers(function->codegen->abi_variant);

    REQUIRE_OK(kefir_asmcmp_register_stash_new(mem, &function->code.context, stash_idx));

    for (kefir_size_t i = 0; i < num_of_preserved_gp_regs; i++) {
        kefir_asm_amd64_xasmgen_register_t reg;
        REQUIRE_OK(
            kefir_abi_amd64_get_caller_preserved_general_purpose_register(function->codegen->abi_variant, i, &reg));

        REQUIRE_OK(kefir_asmcmp_register_stash_add(mem, &function->code.context, *stash_idx,
                                                   (kefir_asmcmp_physical_register_index_t) reg));
    }

    for (kefir_size_t i = 0; i < num_of_preserved_sse_regs; i++) {
        kefir_asm_amd64_xasmgen_register_t reg;
        REQUIRE_OK(kefir_abi_amd64_get_caller_preserved_sse_register(function->codegen->abi_variant, i, &reg));

        REQUIRE_OK(kefir_asmcmp_register_stash_add(mem, &function->code.context, *stash_idx,
                                                   (kefir_asmcmp_physical_register_index_t) reg));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_activate_stash(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), *stash_idx, NULL));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(atomic_load)(struct kefir_mem *mem,
                                                                 struct kefir_codegen_amd64_function *function,
                                                                 const struct kefir_opt_instruction *instruction) {
    kefir_asmcmp_stash_index_t stash_idx;
    REQUIRE_OK(preserve_regs(mem, function, &stash_idx));

    kefir_asmcmp_virtual_register_index_t ptr_vreg, ptr_placement_vreg, memorder_placement_vreg;
    kefir_asm_amd64_xasmgen_register_t ptr_phreg, memorder_phreg;
    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 0, &ptr_phreg));
    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 1, &memorder_phreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &ptr_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &memorder_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, ptr_placement_vreg, ptr_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, memorder_placement_vreg,
                                                                  memorder_phreg));

    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.atomic_op.ref[0], &ptr_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         ptr_placement_vreg, ptr_vreg, NULL));

    kefir_int64_t memorder;
    REQUIRE_OK(get_memorder(instruction->operation.parameters.atomic_op.model, &memorder));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(memorder_placement_vreg),
                                      &KEFIR_ASMCMP_MAKE_INT(memorder), NULL));

    const char *atomic_load_fn_label;
    const char *atomic_load_fn_name;
    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD8:
            atomic_load_fn_name = LIBATOMIC_LOAD_N(1);
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_LOAD16:
            atomic_load_fn_name = LIBATOMIC_LOAD_N(2);
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_LOAD32:
            atomic_load_fn_name = LIBATOMIC_LOAD_N(4);
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_LOAD64:
            atomic_load_fn_name = LIBATOMIC_LOAD_N(8);
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unepected optimizer instruction opcode");
    }
    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_format(mem, &function->code.context, &atomic_load_fn_label, KEFIR_AMD64_PLT,
                                       atomic_load_fn_name));
    } else {
        atomic_load_fn_label = atomic_load_fn_name;
    }

    kefir_asmcmp_instruction_index_t call_idx;
    REQUIRE_OK(kefir_asmcmp_amd64_call(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(atomic_load_fn_label, 0), &call_idx));
    REQUIRE_OK(kefir_asmcmp_register_stash_set_liveness_index(&function->code.context, stash_idx, call_idx));

    kefir_asmcmp_virtual_register_index_t result_vreg, result_placement_vreg;
    kefir_asm_amd64_xasmgen_register_t result_phreg;
    REQUIRE_OK(kefir_abi_amd64_general_purpose_return_register(function->codegen->abi_variant, 0, &result_phreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_placement_vreg));
    REQUIRE_OK(
        kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, result_placement_vreg, result_phreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_vreg, result_placement_vreg, NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_deactivate_stash(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), stash_idx, NULL));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(atomic_store)(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64_function *function,
                                                                  const struct kefir_opt_instruction *instruction) {
    kefir_asmcmp_stash_index_t stash_idx;
    REQUIRE_OK(preserve_regs(mem, function, &stash_idx));

    kefir_asmcmp_virtual_register_index_t ptr_vreg, ptr_placement_vreg, value_vreg, value_placement_vreg,
        memorder_placement_vreg;
    kefir_asm_amd64_xasmgen_register_t ptr_phreg, value_phreg, memorder_phreg;
    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 0, &ptr_phreg));
    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 1, &value_phreg));
    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 2, &memorder_phreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &ptr_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &value_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &memorder_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, ptr_placement_vreg, ptr_phreg));
    REQUIRE_OK(
        kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, value_placement_vreg, value_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, memorder_placement_vreg,
                                                                  memorder_phreg));

    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.atomic_op.ref[0], &ptr_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.atomic_op.ref[1],
                                                    &value_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         ptr_placement_vreg, ptr_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         value_placement_vreg, value_vreg, NULL));

    kefir_int64_t memorder;
    REQUIRE_OK(get_memorder(instruction->operation.parameters.atomic_op.model, &memorder));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(memorder_placement_vreg),
                                      &KEFIR_ASMCMP_MAKE_INT(memorder), NULL));

    const char *atomic_store_fn_label;
    const char *atomic_store_fn_name;
    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_ATOMIC_STORE8:
            atomic_store_fn_name = LIBATOMIC_STORE_N(1);
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_STORE16:
            atomic_store_fn_name = LIBATOMIC_STORE_N(2);
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_STORE32:
            atomic_store_fn_name = LIBATOMIC_STORE_N(4);
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_STORE64:
            atomic_store_fn_name = LIBATOMIC_STORE_N(8);
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unepected optimizer instruction opcode");
    }
    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_format(mem, &function->code.context, &atomic_store_fn_label, KEFIR_AMD64_PLT,
                                       atomic_store_fn_name));
    } else {
        atomic_store_fn_label = atomic_store_fn_name;
    }

    kefir_asmcmp_instruction_index_t call_idx;
    REQUIRE_OK(kefir_asmcmp_amd64_call(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(atomic_store_fn_label, 0), &call_idx));
    REQUIRE_OK(kefir_asmcmp_register_stash_set_liveness_index(&function->code.context, stash_idx, call_idx));

    REQUIRE_OK(kefir_asmcmp_amd64_deactivate_stash(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), stash_idx, NULL));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(atomic_copy_memory)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    const char *atomic_copy_fn_label, *atomic_memory_copy_fn_name;
    kefir_asmcmp_stash_index_t stash_idx;
    REQUIRE_OK(preserve_regs(mem, function, &stash_idx));

    const struct kefir_ir_type *ir_type = kefir_ir_module_get_named_type(
        function->module->ir_module, instruction->operation.parameters.atomic_op.type_id);
    REQUIRE(ir_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type"));

    struct kefir_abi_amd64_type_layout type_layout;
    REQUIRE_OK(kefir_abi_amd64_type_layout(mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, ir_type, &type_layout));

    const struct kefir_abi_amd64_typeentry_layout *typeentry_layout = NULL;
    kefir_result_t res = kefir_abi_amd64_type_layout_at(
        &type_layout, instruction->operation.parameters.atomic_op.type_index, &typeentry_layout);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_type_layout_free(mem, &type_layout);
        return res;
    });

    kefir_size_t total_size = typeentry_layout->size;
    REQUIRE_OK(kefir_abi_amd64_type_layout_free(mem, &type_layout));

    kefir_asmcmp_virtual_register_index_t size_placement_vreg, target_ptr_vreg, target_ptr_placement_vreg,
        source_ptr_vreg, source_ptr_placement_vreg, memorder_placement_vreg;
    kefir_asm_amd64_xasmgen_register_t size_phreg, target_ptr_phreg, source_ptr_phreg, memorder_phreg;
    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 0, &size_phreg));
    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_ATOMIC_COPY_MEMORY_FROM:
            REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 1,
                                                                          &source_ptr_phreg));
            REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 2,
                                                                          &target_ptr_phreg));
            atomic_memory_copy_fn_name = LIBATOMIC_LOAD;
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_COPY_MEMORY_TO:
            REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 1,
                                                                          &target_ptr_phreg));
            REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 2,
                                                                          &source_ptr_phreg));
            atomic_memory_copy_fn_name = LIBATOMIC_STORE;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unepected optimizer instruction opcode");
    }
    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_format(mem, &function->code.context, &atomic_copy_fn_label, KEFIR_AMD64_PLT,
                                       atomic_memory_copy_fn_name));
    } else {
        atomic_copy_fn_label = atomic_memory_copy_fn_name;
    }

    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 3, &memorder_phreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &size_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &target_ptr_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &source_ptr_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &memorder_placement_vreg));
    REQUIRE_OK(
        kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, size_placement_vreg, size_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, target_ptr_placement_vreg,
                                                                  target_ptr_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, source_ptr_placement_vreg,
                                                                  source_ptr_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, memorder_placement_vreg,
                                                                  memorder_phreg));

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.atomic_op.ref[0],
                                                    &target_ptr_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.atomic_op.ref[1],
                                                    &source_ptr_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         target_ptr_placement_vreg, target_ptr_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         source_ptr_placement_vreg, source_ptr_vreg, NULL));

    kefir_int64_t memorder;
    REQUIRE_OK(get_memorder(instruction->operation.parameters.atomic_op.model, &memorder));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(size_placement_vreg), &KEFIR_ASMCMP_MAKE_UINT(total_size),
                                      NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(memorder_placement_vreg),
                                      &KEFIR_ASMCMP_MAKE_INT(memorder), NULL));

    kefir_asmcmp_instruction_index_t call_idx;
    REQUIRE_OK(kefir_asmcmp_amd64_call(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(atomic_copy_fn_label, 0), &call_idx));
    REQUIRE_OK(kefir_asmcmp_register_stash_set_liveness_index(&function->code.context, stash_idx, call_idx));

    REQUIRE_OK(kefir_asmcmp_amd64_deactivate_stash(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), stash_idx, NULL));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(atomic_load_long_double)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    kefir_asmcmp_stash_index_t stash_idx;
    REQUIRE_OK(preserve_regs(mem, function, &stash_idx));

    kefir_asmcmp_virtual_register_index_t size_placement_vreg, result_vreg, target_ptr_placement_vreg, source_ptr_vreg,
        source_ptr_placement_vreg, memorder_placement_vreg;
    kefir_asm_amd64_xasmgen_register_t size_phreg, target_ptr_phreg, source_ptr_phreg, memorder_phreg;
    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 0, &size_phreg));
    REQUIRE_OK(
        kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 1, &source_ptr_phreg));
    REQUIRE_OK(
        kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 2, &target_ptr_phreg));

    const char *atomic_load_long_double_fn_label;
    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_format(mem, &function->code.context, &atomic_load_long_double_fn_label, KEFIR_AMD64_PLT,
                                       LIBATOMIC_LOAD));
    } else {
        atomic_load_long_double_fn_label = LIBATOMIC_LOAD;
    }
    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 3, &memorder_phreg));

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &size_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &target_ptr_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &source_ptr_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &memorder_placement_vreg));
    REQUIRE_OK(
        kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, size_placement_vreg, size_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, target_ptr_placement_vreg,
                                                                  target_ptr_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, source_ptr_placement_vreg,
                                                                  source_ptr_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, memorder_placement_vreg,
                                                                  memorder_phreg));

    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_long_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_long_double_qword_alignment(function->codegen->abi_variant), &result_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.atomic_op.ref[0],
                                                    &source_ptr_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_lea(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(target_ptr_placement_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(result_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         source_ptr_placement_vreg, source_ptr_vreg, NULL));

    kefir_int64_t memorder;
    REQUIRE_OK(get_memorder(instruction->operation.parameters.atomic_op.model, &memorder));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(size_placement_vreg),
        &KEFIR_ASMCMP_MAKE_UINT(kefir_abi_amd64_long_double_qword_size(function->codegen->abi_variant) *
                                KEFIR_AMD64_ABI_QWORD),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(memorder_placement_vreg),
                                      &KEFIR_ASMCMP_MAKE_INT(memorder), NULL));

    kefir_asmcmp_instruction_index_t call_idx;
    REQUIRE_OK(kefir_asmcmp_amd64_call(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(atomic_load_long_double_fn_label, 0),
                                       &call_idx));
    REQUIRE_OK(kefir_asmcmp_register_stash_set_liveness_index(&function->code.context, stash_idx, call_idx));

    REQUIRE_OK(kefir_asmcmp_amd64_deactivate_stash(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), stash_idx, NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(atomic_store_long_double)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    kefir_asmcmp_stash_index_t stash_idx;
    REQUIRE_OK(preserve_regs(mem, function, &stash_idx));

    kefir_asmcmp_virtual_register_index_t size_placement_vreg, target_ptr_vreg, target_ptr_placement_vreg,
        source_ptr_vreg, source_ptr_placement_vreg, memorder_placement_vreg;
    kefir_asm_amd64_xasmgen_register_t size_phreg, target_ptr_phreg, source_ptr_phreg, memorder_phreg;
    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 0, &size_phreg));
    REQUIRE_OK(
        kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 1, &target_ptr_phreg));
    REQUIRE_OK(
        kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 2, &source_ptr_phreg));

    const char *atomic_store_long_double_fn_label;
    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_format(mem, &function->code.context, &atomic_store_long_double_fn_label,
                                       KEFIR_AMD64_PLT, LIBATOMIC_STORE));
    } else {
        atomic_store_long_double_fn_label = LIBATOMIC_STORE;
    }
    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 3, &memorder_phreg));

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &size_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &target_ptr_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &source_ptr_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &memorder_placement_vreg));
    REQUIRE_OK(
        kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, size_placement_vreg, size_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, target_ptr_placement_vreg,
                                                                  target_ptr_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, source_ptr_placement_vreg,
                                                                  source_ptr_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, memorder_placement_vreg,
                                                                  memorder_phreg));

    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.atomic_op.ref[0],
                                                    &target_ptr_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.atomic_op.ref[1],
                                                    &source_ptr_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         target_ptr_placement_vreg, target_ptr_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_lea(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(source_ptr_placement_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(source_ptr_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT), NULL));

    kefir_int64_t memorder;
    REQUIRE_OK(get_memorder(instruction->operation.parameters.atomic_op.model, &memorder));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(size_placement_vreg),
        &KEFIR_ASMCMP_MAKE_UINT(kefir_abi_amd64_long_double_qword_size(function->codegen->abi_variant) *
                                KEFIR_AMD64_ABI_QWORD),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(memorder_placement_vreg),
                                      &KEFIR_ASMCMP_MAKE_INT(memorder), NULL));

    kefir_asmcmp_instruction_index_t call_idx;
    REQUIRE_OK(kefir_asmcmp_amd64_call(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(atomic_store_long_double_fn_label, 0),
                                       &call_idx));
    REQUIRE_OK(kefir_asmcmp_register_stash_set_liveness_index(&function->code.context, stash_idx, call_idx));

    REQUIRE_OK(kefir_asmcmp_amd64_deactivate_stash(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), stash_idx, NULL));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(atomic_compare_exchange)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    kefir_asmcmp_stash_index_t stash_idx;
    REQUIRE_OK(preserve_regs(mem, function, &stash_idx));

    kefir_asmcmp_virtual_register_index_t ptr_vreg, ptr_placement_vreg, expected_value_vreg,
        expected_value_location_vreg, expected_value_location_placement_vreg, desired_value_vreg,
        desired_value_placement_vreg, success_memorder_placement_vreg, fail_memorder_placement_vreg, result_vreg,
        result_placement_vreg;
    kefir_asm_amd64_xasmgen_register_t ptr_phreg, expected_value_location_phreg, desired_value_phreg,
        success_memorder_phreg, fail_memorder_phreg, result_phreg;

    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 0, &ptr_phreg));
    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 1,
                                                                  &expected_value_location_phreg));
    REQUIRE_OK(
        kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 2, &desired_value_phreg));
    REQUIRE_OK(
        kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 3, &success_memorder_phreg));
    REQUIRE_OK(
        kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 4, &fail_memorder_phreg));
    REQUIRE_OK(kefir_abi_amd64_general_purpose_return_register(function->codegen->abi_variant, 0, &result_phreg));

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &ptr_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(mem, &function->code.context, 1, 1,
                                                                                 &expected_value_location_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE,
                                                 &expected_value_location_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &desired_value_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &success_memorder_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &fail_memorder_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_placement_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, ptr_placement_vreg, ptr_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
        mem, &function->code, expected_value_location_placement_vreg, expected_value_location_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, desired_value_placement_vreg,
                                                                  desired_value_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, success_memorder_placement_vreg,
                                                                  success_memorder_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, fail_memorder_placement_vreg,
                                                                  fail_memorder_phreg));
    REQUIRE_OK(
        kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, result_placement_vreg, result_phreg));

    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.atomic_op.ref[0], &ptr_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.atomic_op.ref[1],
                                                    &expected_value_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.atomic_op.ref[2],
                                                    &desired_value_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         ptr_placement_vreg, ptr_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         desired_value_placement_vreg, desired_value_vreg, NULL));

    kefir_int64_t memorder;
    REQUIRE_OK(get_memorder(instruction->operation.parameters.atomic_op.model, &memorder));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(success_memorder_placement_vreg),
                                      &KEFIR_ASMCMP_MAKE_INT(memorder), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(fail_memorder_placement_vreg),
                                      &KEFIR_ASMCMP_MAKE_INT(memorder), NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(expected_value_location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        &KEFIR_ASMCMP_MAKE_VREG(expected_value_vreg), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_lea(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(expected_value_location_placement_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(expected_value_location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));

    const char *atomic_cmpxchg_fn_label;
    const char *atomic_cmpxchg_fn_name;
    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG8:
            atomic_cmpxchg_fn_name = LIBATOMIC_CMPXCHG_N(1);
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG16:
            atomic_cmpxchg_fn_name = LIBATOMIC_CMPXCHG_N(2);
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG32:
            atomic_cmpxchg_fn_name = LIBATOMIC_CMPXCHG_N(4);
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_CMPXCHG64:
            atomic_cmpxchg_fn_name = LIBATOMIC_CMPXCHG_N(8);
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unepected optimizer instruction opcode");
    }
    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_format(mem, &function->code.context, &atomic_cmpxchg_fn_label, KEFIR_AMD64_PLT,
                                       atomic_cmpxchg_fn_name));
    } else {
        atomic_cmpxchg_fn_label = atomic_cmpxchg_fn_name;
    }

    kefir_asmcmp_instruction_index_t call_idx;
    REQUIRE_OK(kefir_asmcmp_amd64_call(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(atomic_cmpxchg_fn_label, 0), &call_idx));
    REQUIRE_OK(kefir_asmcmp_register_stash_set_liveness_index(&function->code.context, stash_idx, call_idx));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_vreg, result_placement_vreg, NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_deactivate_stash(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), stash_idx, NULL));

    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(atomic_compare_exchange_long_double)(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
    const struct kefir_opt_instruction *instruction) {
    kefir_asmcmp_stash_index_t stash_idx;
    REQUIRE_OK(preserve_regs(mem, function, &stash_idx));

    kefir_asmcmp_virtual_register_index_t size_placement_vreg, ptr_vreg, ptr_placement_vreg, expected_value_vreg,
        expected_value_location_vreg, expected_value_location_placement_vreg, desired_value_vreg,
        desired_value_location_vreg, desired_value_location_placement_vreg, success_memorder_placement_vreg,
        fail_memorder_placement_vreg, result_vreg, result_placement_vreg;
    kefir_asm_amd64_xasmgen_register_t size_phreg, ptr_phreg, expected_value_location_phreg, desired_value_phreg,
        success_memorder_phreg, fail_memorder_phreg, result_phreg;

    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 0, &size_phreg));
    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 1, &ptr_phreg));
    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 2,
                                                                  &expected_value_location_phreg));
    REQUIRE_OK(
        kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 3, &desired_value_phreg));
    REQUIRE_OK(
        kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 4, &success_memorder_phreg));
    REQUIRE_OK(
        kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 5, &fail_memorder_phreg));
    REQUIRE_OK(kefir_abi_amd64_general_purpose_return_register(function->codegen->abi_variant, 0, &result_phreg));

    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &size_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &ptr_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_long_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_long_double_qword_alignment(function->codegen->abi_variant), &expected_value_location_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new_indirect_spill_space_allocation(
        mem, &function->code.context, kefir_abi_amd64_long_double_qword_size(function->codegen->abi_variant),
        kefir_abi_amd64_long_double_qword_alignment(function->codegen->abi_variant), &desired_value_location_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE,
                                                 &expected_value_location_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE,
                                                 &desired_value_location_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &success_memorder_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &fail_memorder_placement_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(
        mem, &function->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_placement_vreg));

    REQUIRE_OK(
        kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, size_placement_vreg, size_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, ptr_placement_vreg, ptr_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
        mem, &function->code, expected_value_location_placement_vreg, expected_value_location_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(
        mem, &function->code, desired_value_location_placement_vreg, desired_value_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, success_memorder_placement_vreg,
                                                                  success_memorder_phreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, fail_memorder_placement_vreg,
                                                                  fail_memorder_phreg));
    REQUIRE_OK(
        kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, result_placement_vreg, result_phreg));

    REQUIRE_OK(
        kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.atomic_op.ref[0], &ptr_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.atomic_op.ref[1],
                                                    &expected_value_vreg));
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instruction->operation.parameters.atomic_op.ref[2],
                                                    &desired_value_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         ptr_placement_vreg, ptr_vreg, NULL));

    kefir_int64_t memorder;
    REQUIRE_OK(get_memorder(instruction->operation.parameters.atomic_op.model, &memorder));

    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(success_memorder_placement_vreg),
                                      &KEFIR_ASMCMP_MAKE_INT(memorder), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_VREG(fail_memorder_placement_vreg),
                                      &KEFIR_ASMCMP_MAKE_INT(memorder), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_mov(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(size_placement_vreg),
        &KEFIR_ASMCMP_MAKE_INT(kefir_abi_amd64_long_double_qword_size(function->codegen->abi_variant) *
                               KEFIR_AMD64_ABI_QWORD),
        NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(expected_value_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                               &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(expected_value_location_vreg, KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                               &KEFIR_ASMCMP_MAKE_INT(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(expected_value_location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
        NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_lea(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(expected_value_location_placement_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(expected_value_location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_fld(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(desired_value_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_mov(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                               &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(desired_value_location_vreg, KEFIR_AMD64_ABI_QWORD,
                                                                   KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
                               &KEFIR_ASMCMP_MAKE_INT(0), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(desired_value_location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_lea(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_VREG(desired_value_location_placement_vreg),
        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(desired_value_location_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
        NULL));

    REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         expected_value_location_vreg, NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         desired_value_location_vreg, NULL));

    const char *atomic_cmpxchg_fn_label;
    if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_format(mem, &function->code.context, &atomic_cmpxchg_fn_label, KEFIR_AMD64_PLT,
                                       LIBATOMIC_CMPXCHG));
    } else {
        atomic_cmpxchg_fn_label = LIBATOMIC_CMPXCHG;
    }

    kefir_asmcmp_instruction_index_t call_idx;
    REQUIRE_OK(kefir_asmcmp_amd64_call(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(atomic_cmpxchg_fn_label, 0), &call_idx));
    REQUIRE_OK(kefir_asmcmp_register_stash_set_liveness_index(&function->code.context, stash_idx, call_idx));

    REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &function->code,
                                                         kefir_asmcmp_context_instr_tail(&function->code.context),
                                                         result_vreg, result_placement_vreg, NULL));

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_deactivate_stash(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), stash_idx, NULL));

    return KEFIR_OK;
}
