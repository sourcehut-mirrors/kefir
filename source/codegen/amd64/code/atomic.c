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
#include "kefir/core/error.h"
#include "kefir/core/util.h"

#define LIBATOMIC_SEQ_CST 5

#define LIBATOMIC_LOAD_N(_n) "__atomic_load_" #_n
#define LIBATOMIC_STORE_N(_n) "__atomic_store_" #_n

static kefir_result_t get_memorder(kefir_opt_atomic_model_t model, kefir_int64_t *memorder) {
    *memorder = LIBATOMIC_SEQ_CST;
    switch (model) {
        case KEFIR_OPT_ATOMIC_MODEL_SEQ_CST:
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

    const char *load_got_label;
    const char *load_fn;
    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_ATOMIC_LOAD8:
            load_fn = LIBATOMIC_LOAD_N(1);
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_LOAD16:
            load_fn = LIBATOMIC_LOAD_N(2);
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_LOAD32:
            load_fn = LIBATOMIC_LOAD_N(4);
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_LOAD64:
            load_fn = LIBATOMIC_LOAD_N(8);
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unepected optimizer instruction opcode");
    }
    REQUIRE_OK(kefir_asmcmp_format(mem, &function->code.context, &load_got_label, KEFIR_AMD64_PLT, load_fn));

    kefir_asmcmp_instruction_index_t call_idx;
    REQUIRE_OK(kefir_asmcmp_amd64_call(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(load_got_label, 0), &call_idx));
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

    const char *store_got_label;
    const char *store_fn;
    switch (instruction->operation.opcode) {
        case KEFIR_OPT_OPCODE_ATOMIC_STORE8:
            store_fn = LIBATOMIC_STORE_N(1);
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_STORE16:
            store_fn = LIBATOMIC_STORE_N(2);
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_STORE32:
            store_fn = LIBATOMIC_STORE_N(4);
            break;

        case KEFIR_OPT_OPCODE_ATOMIC_STORE64:
            store_fn = LIBATOMIC_STORE_N(8);
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unepected optimizer instruction opcode");
    }
    REQUIRE_OK(kefir_asmcmp_format(mem, &function->code.context, &store_got_label, KEFIR_AMD64_PLT, store_fn));

    kefir_asmcmp_instruction_index_t call_idx;
    REQUIRE_OK(kefir_asmcmp_amd64_call(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(store_got_label, 0), &call_idx));
    REQUIRE_OK(kefir_asmcmp_register_stash_set_liveness_index(&function->code.context, stash_idx, call_idx));

    REQUIRE_OK(kefir_asmcmp_amd64_deactivate_stash(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), stash_idx, NULL));

    return KEFIR_OK;
}
