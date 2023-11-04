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

#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/function.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/codegen/amd64/devirtualize.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t translate_instruction(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                            const struct kefir_opt_instruction *instruction) {
    switch (instruction->operation.opcode) {
#define CASE_INSTR(_id, _opcode)                                                           \
    case _opcode:                                                                          \
        REQUIRE_OK(KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(_id)(mem, function, instruction)); \
        break
        KEFIR_CODEGEN_AMD64_INSTRUCTIONS(CASE_INSTR, ;);
#undef CASE_INSTR

        default:
            REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &function->code,
                                              kefir_asmcmp_context_instr_tail(&function->code.context),
                                              &KEFIR_ASMCMP_MAKE_INT(1), &KEFIR_ASMCMP_MAKE_INT(2), NULL));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t translate_code(struct kefir_mem *mem, struct kefir_codegen_amd64_function *func) {
    UNUSED(mem);
    UNUSED(func);

    // Initialize block labels
    for (kefir_size_t block_idx = 0; block_idx < func->function_analysis->block_linearization_length; block_idx++) {
        const struct kefir_opt_code_analysis_block_properties *block_props =
            func->function_analysis->block_linearization[block_idx];
        kefir_asmcmp_label_index_t asmlabel;
        REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &func->code.context, KEFIR_ASMCMP_INDEX_NONE, &asmlabel));
        REQUIRE_OK(kefir_hashtree_insert(mem, &func->labels, (kefir_hashtree_key_t) block_props->block_id,
                                         (kefir_hashtree_value_t) asmlabel));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_function_prologue(
        mem, &func->code, kefir_asmcmp_context_instr_tail(&func->code.context), &func->prologue_tail));
    kefir_bool_t implicit_parameter_present;
    kefir_asm_amd64_xasmgen_register_t implicit_parameter_reg;
    REQUIRE_OK(kefir_abi_amd64_function_decl_returns_implicit_parameter(
        &func->abi_function_declaration, &implicit_parameter_present, &implicit_parameter_reg));
    if (implicit_parameter_present) {
        kefir_asmcmp_virtual_register_index_t implicit_param_vreg, implicit_param_placement_vreg;
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &func->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_SPILL_SPACE_SLOT, &implicit_param_vreg));
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &func->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &implicit_param_placement_vreg));
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &func->code, implicit_param_placement_vreg,
                                                                      implicit_parameter_reg));
        REQUIRE_OK(kefir_asmcmp_amd64_mov(mem, &func->code, kefir_asmcmp_context_instr_tail(&func->code.context),
                                          &KEFIR_ASMCMP_MAKE_VREG64(implicit_param_vreg),
                                          &KEFIR_ASMCMP_MAKE_VREG64(implicit_param_placement_vreg), NULL));
        func->return_address_vreg = implicit_param_vreg;
    }

    // Translate blocks
    for (kefir_size_t block_idx = 0; block_idx < func->function_analysis->block_linearization_length; block_idx++) {
        const struct kefir_opt_code_analysis_block_properties *block_props =
            func->function_analysis->block_linearization[block_idx];

        struct kefir_hashtree_node *asmlabel_node;
        REQUIRE_OK(kefir_hashtree_at(&func->labels, (kefir_hashtree_key_t) block_props->block_id, &asmlabel_node));
        ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, asmlabel, asmlabel_node->value);
        REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &func->code.context, asmlabel));

        for (kefir_size_t instr_idx = block_props->linear_range.begin_index;
             instr_idx < block_props->linear_range.end_index; instr_idx++) {
            struct kefir_opt_instruction *instr = NULL;
            REQUIRE_OK(kefir_opt_code_container_instr(
                &func->function->code, func->function_analysis->linearization[instr_idx]->instr_ref, &instr));
            REQUIRE_OK(translate_instruction(mem, func, instr));
        }
    }

    if (func->return_address_vreg != KEFIR_ASMCMP_INDEX_NONE) {
        REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
            mem, &func->code, kefir_asmcmp_context_instr_tail(&func->code.context), func->return_address_vreg, NULL));
    }
    if (func->dynamic_scope_vreg != KEFIR_ASMCMP_INDEX_NONE) {
        REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
            mem, &func->code, kefir_asmcmp_context_instr_tail(&func->code.context), func->dynamic_scope_vreg, NULL));
    }
    return KEFIR_OK;
}

static kefir_result_t kefir_codegen_amd64_function_translate_impl(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64 *codegen,
                                                                  struct kefir_codegen_amd64_function *func) {
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&func->codegen->xasmgen, "%s", func->function->ir_func->name));

    REQUIRE_OK(translate_code(mem, func));
    REQUIRE_OK(
        kefir_codegen_amd64_register_allocator_run(mem, &func->code, &func->stack_frame, &func->register_allocator));
    REQUIRE_OK(kefir_codegen_amd64_devirtualize(mem, &func->code, &func->register_allocator, &func->stack_frame));
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_calculate(codegen->abi_variant, func->function->ir_func->locals,
                                                         &func->locals_layout, &func->stack_frame));
    REQUIRE_OK(kefir_asmcmp_amd64_generate_code(&codegen->xasmgen, &func->code, &func->stack_frame));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translate(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen,
                                                      const struct kefir_opt_module *module,
                                                      const struct kefir_opt_function *function,
                                                      const struct kefir_opt_code_analysis *function_analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(codegen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 code generator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));
    REQUIRE(function_analysis != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function analysis"));

    struct kefir_codegen_amd64_function func = {.codegen = codegen,
                                                .module = module,
                                                .function = function,
                                                .function_analysis = function_analysis,
                                                .argument_touch_instr = KEFIR_ASMCMP_INDEX_NONE,
                                                .prologue_tail = KEFIR_ASMCMP_INDEX_NONE,
                                                .return_address_vreg = KEFIR_ASMCMP_INDEX_NONE,
                                                .dynamic_scope_vreg = KEFIR_ASMCMP_INDEX_NONE};
    REQUIRE_OK(kefir_asmcmp_amd64_init(function->ir_func->name, codegen->abi_variant, &func.code));
    REQUIRE_OK(kefir_hashtree_init(&func.instructions, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&func.labels, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&func.virtual_registers, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_init(&func.stack_frame));
    REQUIRE_OK(kefir_codegen_amd64_register_allocator_init(&func.register_allocator));
    REQUIRE_OK(kefir_abi_amd64_function_decl_alloc(mem, codegen->abi_variant, function->ir_func->declaration,
                                                   &func.abi_function_declaration));
    kefir_result_t res =
        kefir_abi_amd64_type_layout(mem, codegen->abi_variant, function->ir_func->locals, &func.locals_layout);
    REQUIRE_ELSE(res == KEFIR_OK, { goto on_error1; });

    res = kefir_codegen_amd64_function_translate_impl(mem, codegen, &func);

    kefir_abi_amd64_type_layout_free(mem, &func.locals_layout);
on_error1:
    kefir_abi_amd64_function_decl_free(mem, &func.abi_function_declaration);
    kefir_codegen_amd64_register_allocator_free(mem, &func.register_allocator);
    kefir_codegen_amd64_stack_frame_free(mem, &func.stack_frame);
    kefir_hashtree_free(mem, &func.instructions);
    kefir_hashtree_free(mem, &func.virtual_registers);
    kefir_hashtree_free(mem, &func.labels);
    kefir_asmcmp_amd64_free(mem, &func.code);
    return res;
}

kefir_result_t kefir_codegen_amd64_function_assign_vreg(struct kefir_mem *mem,
                                                        struct kefir_codegen_amd64_function *function,
                                                        kefir_opt_instruction_ref_t instr_ref,
                                                        kefir_asmcmp_virtual_register_index_t vreg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));

    kefir_result_t res = kefir_hashtree_insert(mem, &function->virtual_registers, (kefir_hashtree_key_t) instr_ref,
                                               (kefir_hashtree_value_t) vreg);
    if (res == KEFIR_ALREADY_EXISTS) {
        res = KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Virtual register has already been assigned");
    }
    REQUIRE_OK(res);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_vreg_of(struct kefir_codegen_amd64_function *function,
                                                    kefir_opt_instruction_ref_t instr_ref,
                                                    kefir_asmcmp_virtual_register_index_t *vreg) {
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(vreg != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 virtual register index"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&function->virtual_registers, (kefir_hashtree_key_t) instr_ref, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find associated virtual register");
    }
    REQUIRE_OK(res);

    *vreg = node->value;
    return KEFIR_OK;
}
