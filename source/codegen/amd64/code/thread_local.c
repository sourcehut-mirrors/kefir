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

static kefir_result_t emutls_preserve_regs(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
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

static kefir_result_t emulated_tls(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                   const struct kefir_opt_instruction *instruction) {
    kefir_asmcmp_stash_index_t stash_idx;
    REQUIRE_OK(emutls_preserve_regs(mem, function, &stash_idx));

    const char *identifier = kefir_ir_module_get_named_symbol(function->module->ir_module,
                                                              instruction->operation.parameters.variable.global_ref);
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find named IR symbol"));

    kefir_asmcmp_virtual_register_index_t param_vreg;
    kefir_asm_amd64_xasmgen_register_t param_phreg;
    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 0, &param_phreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &param_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, param_vreg, param_phreg));

    if (!kefir_ir_module_has_external(function->module->ir_module, identifier) &&
        !function->codegen->config->position_independent_code) {
        const char *emutls_v_label;
        REQUIRE_OK(
            kefir_asmcmp_format(mem, &function->code.context, &emutls_v_label, KEFIR_AMD64_EMUTLS_V, identifier));

        REQUIRE_OK(kefir_asmcmp_amd64_lea(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(param_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_ABSOLUTE, emutls_v_label, 0,
                                                       KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
            NULL));
    } else {
        const char *emutls_got_label;
        REQUIRE_OK(
            kefir_asmcmp_format(mem, &function->code.context, &emutls_got_label, KEFIR_AMD64_EMUTLS_V, identifier));

        REQUIRE_OK(kefir_asmcmp_amd64_mov(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(param_vreg),
            &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_ASMCMP_EXTERNAL_LABEL_GOTPCREL, emutls_got_label,
                                                     KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
            NULL));
    }

    kefir_asmcmp_instruction_index_t call_idx;
    REQUIRE_OK(kefir_asmcmp_amd64_call(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_PLT, KEFIR_AMD64_EMUTLS_GET_ADDR, 0), &call_idx));
    REQUIRE_OK(kefir_asmcmp_register_stash_set_liveness_index(&function->code.context, stash_idx, call_idx));

    kefir_asmcmp_virtual_register_index_t result_vreg, result_placement_vreg, tmp_vreg;
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
    const kefir_int64_t offset = instruction->operation.parameters.variable.offset;
    if (offset < KEFIR_INT32_MIN && offset > KEFIR_INT32_MAX) {
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
        REQUIRE_OK(kefir_asmcmp_amd64_movabs(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
        REQUIRE_OK(
            kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                   &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
    } else if (offset != 0) {
        REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_deactivate_stash(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), stash_idx, NULL));

    return KEFIR_OK;
}

static kefir_result_t general_dynamic_tls(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                          const struct kefir_opt_instruction *instruction) {
    kefir_asmcmp_stash_index_t stash_idx;
    REQUIRE_OK(emutls_preserve_regs(mem, function, &stash_idx));

    const char *identifier = kefir_ir_module_get_named_symbol(function->module->ir_module,
                                                              instruction->operation.parameters.variable.global_ref);
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find named IR symbol"));

    kefir_asmcmp_virtual_register_index_t param_vreg;
    kefir_asm_amd64_xasmgen_register_t param_phreg;
    REQUIRE_OK(kefir_abi_amd64_general_purpose_parameter_register(function->codegen->abi_variant, 0, &param_phreg));
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &param_vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &function->code, param_vreg, param_phreg));

    REQUIRE_OK(kefir_asmcmp_amd64_data16(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                         NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_lea(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                               &KEFIR_ASMCMP_MAKE_VREG(param_vreg),
                               &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_ASMCMP_EXTERNAL_LABEL_TLSGD, identifier,
                                                                        KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                               NULL));
    REQUIRE_OK(kefir_asmcmp_amd64_data_word(mem, &function->code,
                                            kefir_asmcmp_context_instr_tail(&function->code.context), 0x6666, NULL));
    REQUIRE_OK(
        kefir_asmcmp_amd64_rexW(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), NULL));

    kefir_asmcmp_instruction_index_t call_idx;
    REQUIRE_OK(kefir_asmcmp_amd64_call(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
        &KEFIR_ASMCMP_MAKE_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_PLT, KEFIR_AMD64_TLS_GET_ADDR, 0), &call_idx));
    REQUIRE_OK(kefir_asmcmp_register_stash_set_liveness_index(&function->code.context, stash_idx, call_idx));

    kefir_asmcmp_virtual_register_index_t result_vreg, result_placement_vreg, tmp_vreg;
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

    const kefir_int64_t offset = instruction->operation.parameters.variable.offset;
    if (offset < KEFIR_INT32_MIN && offset > KEFIR_INT32_MAX) {
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                     KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
        REQUIRE_OK(kefir_asmcmp_amd64_movabs(mem, &function->code,
                                             kefir_asmcmp_context_instr_tail(&function->code.context),
                                             &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
        REQUIRE_OK(
            kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                   &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
    } else if (offset != 0) {
        REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));

    REQUIRE_OK(kefir_asmcmp_amd64_deactivate_stash(
        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context), stash_idx, NULL));

    return KEFIR_OK;
}

static kefir_result_t initial_exec_tls(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                       const struct kefir_opt_instruction *instruction) {
    const char *identifier = kefir_ir_module_get_named_symbol(function->module->ir_module,
                                                              instruction->operation.parameters.variable.global_ref);
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find named IR symbol"));

    kefir_asmcmp_virtual_register_index_t result_vreg, tmp_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                 KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &result_vreg));

    const kefir_int64_t offset = instruction->operation.parameters.variable.offset;
    if (!kefir_ir_module_has_external(function->module->ir_module, identifier) &&
        !function->codegen->config->position_independent_code) {
        REQUIRE_OK(kefir_asmcmp_amd64_lea(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
            &KEFIR_ASMCMP_MAKE_INDIRECT_EXTERNAL_LABEL(KEFIR_ASMCMP_EXTERNAL_LABEL_TPOFF, identifier, 0,
                                                       KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
            NULL));

        if (offset >= KEFIR_INT32_MIN && offset <= KEFIR_INT32_MAX) {
            REQUIRE_OK(
                kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
        } else {
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
            REQUIRE_OK(
                kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
        }

        struct kefir_asmcmp_value segment_base = KEFIR_ASMCMP_MAKE_UINT(0);
        KEFIR_ASMCMP_SET_SEGMENT(&segment_base, KEFIR_AMD64_XASMGEN_SEGMENT_FS);
        REQUIRE_OK(kefir_asmcmp_amd64_add(mem, &function->code,
                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                          &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &segment_base, NULL));
    } else {
        if (offset >= KEFIR_INT32_MIN && offset <= KEFIR_INT32_MAX) {
            struct kefir_asmcmp_value segment_base = KEFIR_ASMCMP_MAKE_INT(offset);
            KEFIR_ASMCMP_SET_SEGMENT(&segment_base, KEFIR_AMD64_XASMGEN_SEGMENT_FS);

            REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &segment_base, NULL));
        } else {
            REQUIRE_OK(kefir_asmcmp_virtual_register_new(mem, &function->code.context,
                                                         KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &tmp_vreg));

            struct kefir_asmcmp_value segment_base = KEFIR_ASMCMP_MAKE_INT(0);
            KEFIR_ASMCMP_SET_SEGMENT(&segment_base, KEFIR_AMD64_XASMGEN_SEGMENT_FS);

            REQUIRE_OK(kefir_asmcmp_amd64_movabs(
                mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_INT(offset), NULL));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), &segment_base, NULL));
            REQUIRE_OK(
                kefir_asmcmp_amd64_add(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                       &KEFIR_ASMCMP_MAKE_VREG(result_vreg), &KEFIR_ASMCMP_MAKE_VREG(tmp_vreg), NULL));
        }

        REQUIRE_OK(kefir_asmcmp_amd64_add(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_VREG(result_vreg),
            &KEFIR_ASMCMP_MAKE_RIP_INDIRECT_EXTERNAL(KEFIR_ASMCMP_EXTERNAL_LABEL_GOTTPOFF, identifier,
                                                     KEFIR_ASMCMP_OPERAND_VARIANT_64BIT),
            NULL));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_assign_vreg(mem, function, instruction->id, result_vreg));
    return KEFIR_OK;
}

kefir_result_t KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(thread_local)(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64_function *function,
                                                                  const struct kefir_opt_instruction *instruction) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(instruction != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction"));

    if (function->codegen->config->emulated_tls) {
        REQUIRE_OK(emulated_tls(mem, function, instruction));
    } else if (function->codegen->config->position_independent_code) {
        REQUIRE_OK(general_dynamic_tls(mem, function, instruction));
    } else {
        REQUIRE_OK(initial_exec_tls(mem, function, instruction));
    }
    return KEFIR_OK;
}
