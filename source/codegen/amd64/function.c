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
#include "kefir/codegen/amd64/devirtualize.h"
#include "kefir/codegen/asmcmp/format.h"
#include "kefir/optimizer/code.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t translate_instruction(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                            const struct kefir_opt_instruction *instruction) {
    REQUIRE(kefir_hashtreeset_has(&function->translated_instructions, (kefir_hashtreeset_entry_t) instruction->id),
            KEFIR_OK);
    switch (instruction->operation.opcode) {
#define CASE_INSTR(_id, _opcode)                                                           \
    case _opcode:                                                                          \
        REQUIRE_OK(KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(_id)(mem, function, instruction)); \
        break
        KEFIR_CODEGEN_AMD64_INSTRUCTIONS(CASE_INSTR, ;);
#undef CASE_INSTR
    }
    return KEFIR_OK;
}

struct translate_instruction_collector_param {
    struct kefir_mem *mem;
    struct kefir_codegen_amd64_function *func;
    struct kefir_list *queue;
};

static kefir_result_t translate_instruction_collector_callback(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct translate_instruction_collector_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid collector callback parameter"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&param->func->function->code, instr_ref, &instr));

    if (param->func->function_analysis->blocks[instr->block_id].reachable) {
        REQUIRE_OK(kefir_list_insert_after(param->mem, param->queue, kefir_list_tail(param->queue), (void *) instr));
    }
    return KEFIR_OK;
}

static kefir_result_t collect_translated_instructions_impl(struct kefir_mem *mem,
                                                           struct kefir_codegen_amd64_function *func,
                                                           struct kefir_list *queue) {
    for (kefir_size_t block_idx = 0; block_idx < func->function_analysis->block_linearization_length; block_idx++) {
        const struct kefir_opt_code_analysis_block_properties *block_props =
            func->function_analysis->block_linearization[block_idx];

        const struct kefir_opt_code_block *block;
        REQUIRE_OK(kefir_opt_code_container_block(&func->function->code, block_props->block_id, &block));

        kefir_opt_instruction_ref_t instr_ref;
        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_block_instr_control_head(&func->function->code, block, &instr_ref));
        for (; instr_ref != KEFIR_ID_NONE;) {
            REQUIRE_OK(kefir_opt_code_container_instr(&func->function->code, instr_ref, &instr));
            REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue), (void *) instr));
            REQUIRE_OK(kefir_opt_instruction_next_control(&func->function->code, instr_ref, &instr_ref));
        }
    }

    struct translate_instruction_collector_param param = {.mem = mem, .func = func, .queue = queue};
    for (struct kefir_list_entry *head = kefir_list_head(queue); head != NULL; head = kefir_list_head(queue)) {
        ASSIGN_DECL_CAST(struct kefir_opt_instruction *, instr, head->value);
        REQUIRE_OK(kefir_list_pop(mem, queue, head));

        if (!kefir_hashtreeset_has(&func->translated_instructions, (kefir_hashtreeset_entry_t) instr->id)) {
            REQUIRE_OK(
                kefir_hashtreeset_add(mem, &func->translated_instructions, (kefir_hashtreeset_entry_t) instr->id));

            switch (instr->operation.opcode) {
#define CASE_INSTR(_id, _opcode)                                                  \
    case _opcode:                                                                 \
        REQUIRE_OK(KEFIR_CODEGEN_AMD64_INSTRUCTION_FUSION_IMPL(_id)(              \
            mem, func, instr, translate_instruction_collector_callback, &param)); \
        break
                KEFIR_CODEGEN_AMD64_INSTRUCTION_FUSION(CASE_INSTR, ;);
#undef CASE_INSTR

                default:
                    REQUIRE_OK(kefir_opt_instruction_extract_inputs(&func->function->code, instr, false,
                                                                    translate_instruction_collector_callback, &param));
                    break;
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t collect_translated_instructions(struct kefir_mem *mem,
                                                      struct kefir_codegen_amd64_function *func) {
    struct kefir_list queue;
    REQUIRE_OK(kefir_list_init(&queue));
    kefir_result_t res = collect_translated_instructions_impl(mem, func, &queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &queue));
    return KEFIR_OK;
}

static kefir_result_t translate_code(struct kefir_mem *mem, struct kefir_codegen_amd64_function *func) {
    REQUIRE_OK(collect_translated_instructions(mem, func));
    // Initialize block labels
    for (kefir_size_t block_idx = 0; block_idx < func->function_analysis->block_linearization_length; block_idx++) {
        const struct kefir_opt_code_analysis_block_properties *block_props =
            func->function_analysis->block_linearization[block_idx];
        kefir_asmcmp_label_index_t asmlabel;
        REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &func->code.context, KEFIR_ASMCMP_INDEX_NONE, &asmlabel));
        REQUIRE_OK(kefir_hashtree_insert(mem, &func->labels, (kefir_hashtree_key_t) block_props->block_id,
                                         (kefir_hashtree_value_t) asmlabel));

        kefir_result_t res;
        struct kefir_opt_code_block_public_label_iterator iter;
        for (res =
                 kefir_opt_code_container_block_public_labels_iter(&func->function->code, block_props->block_id, &iter);
             res == KEFIR_OK; res = kefir_opt_code_container_block_public_labels_next(&iter)) {
            REQUIRE_OK(
                kefir_asmcmp_context_label_add_public_name(mem, &func->code.context, asmlabel, iter.public_label));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    REQUIRE_OK(kefir_asmcmp_amd64_function_prologue(
        mem, &func->code, kefir_asmcmp_context_instr_tail(&func->code.context), &func->prologue_tail));
    kefir_bool_t implicit_parameter_present;
    kefir_asm_amd64_xasmgen_register_t implicit_parameter_reg;
    REQUIRE_OK(kefir_abi_amd64_function_decl_returns_implicit_parameter(
        &func->abi_function_declaration, &implicit_parameter_present, &implicit_parameter_reg));
    if (implicit_parameter_present) {
        kefir_asmcmp_virtual_register_index_t implicit_param_vreg, implicit_param_placement_vreg;
        REQUIRE_OK(kefir_asmcmp_virtual_register_new_direct_spill_space_allocation(mem, &func->code.context, 1, 1,
                                                                                   &implicit_param_vreg));
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
            const struct kefir_opt_instruction *instr = NULL;
            REQUIRE_OK(kefir_opt_code_container_instr(
                &func->function->code, func->function_analysis->linearization[instr_idx]->instr_ref, &instr));
            REQUIRE_OK(translate_instruction(mem, func, instr));
        }

        kefir_asmcmp_instruction_index_t block_begin_idx;
        REQUIRE_OK(kefir_asmcmp_context_label_at(&func->code.context, asmlabel, &block_begin_idx));
        if (block_begin_idx != KEFIR_ASMCMP_INDEX_NONE) {
            block_begin_idx = kefir_asmcmp_context_instr_prev(&func->code.context, block_begin_idx);
        } else {
            block_begin_idx = kefir_asmcmp_context_instr_tail(&func->code.context);
        }

        struct kefir_hashtreeset lifetime_vregs;
        REQUIRE_OK(kefir_hashtreeset_init(&lifetime_vregs, &kefir_hashtree_uint_ops));

        kefir_result_t res = KEFIR_OK;
        for (kefir_size_t instr_idx = block_props->linear_range.begin_index;
             res == KEFIR_OK && instr_idx < block_props->linear_range.end_index; instr_idx++) {
            kefir_asmcmp_virtual_register_index_t vreg = 0;
            res = kefir_codegen_amd64_function_vreg_of(
                func, func->function_analysis->linearization[instr_idx]->instr_ref, &vreg);
            if (res == KEFIR_NOT_FOUND) {
                res = KEFIR_OK;
                continue;
            }
            if (kefir_hashtreeset_has(&lifetime_vregs, (kefir_hashtreeset_entry_t) vreg)) {
                continue;
            }
            REQUIRE_CHAIN(&res, kefir_asmcmp_amd64_vreg_lifetime_range_begin(mem, &func->code, block_begin_idx, vreg,
                                                                             &block_begin_idx));
            REQUIRE_CHAIN(&res, kefir_hashtreeset_add(mem, &lifetime_vregs, (kefir_hashtreeset_entry_t) vreg));
        }

        for (kefir_size_t instr_idx = block_props->linear_range.begin_index;
             res == KEFIR_OK && instr_idx < block_props->linear_range.end_index; instr_idx++) {
            kefir_asmcmp_virtual_register_index_t vreg = 0;
            kefir_result_t res = kefir_codegen_amd64_function_vreg_of(
                func, func->function_analysis->linearization[instr_idx]->instr_ref, &vreg);
            if (res == KEFIR_NOT_FOUND) {
                res = KEFIR_OK;
                continue;
            }
            if (!kefir_hashtreeset_has(&lifetime_vregs, (kefir_hashtreeset_entry_t) vreg)) {
                continue;
            }
            REQUIRE_CHAIN(&res,
                          kefir_asmcmp_amd64_vreg_lifetime_range_end(
                              mem, &func->code, kefir_asmcmp_context_instr_tail(&func->code.context), vreg, NULL));
            REQUIRE_CHAIN(&res, kefir_hashtreeset_delete(mem, &lifetime_vregs, (kefir_hashtreeset_entry_t) vreg));
        }
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_hashtreeset_free(mem, &lifetime_vregs);
            return res;
        });
        REQUIRE_OK(kefir_hashtreeset_free(mem, &lifetime_vregs));
    }

    if (func->return_address_vreg != KEFIR_ASMCMP_INDEX_NONE) {
        REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
            mem, &func->code, kefir_asmcmp_context_instr_tail(&func->code.context), func->return_address_vreg, NULL));
    }
    if (func->dynamic_scope_vreg != KEFIR_ASMCMP_INDEX_NONE) {
        REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
            mem, &func->code, kefir_asmcmp_context_instr_tail(&func->code.context), func->dynamic_scope_vreg, NULL));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_noop(mem, &func->code, kefir_asmcmp_context_instr_tail(&func->code.context), NULL));
    return KEFIR_OK;
}

static kefir_result_t generate_constants(struct kefir_mem *mem, struct kefir_codegen_amd64_function *func) {
    UNUSED(mem);
    struct kefir_hashtree_node_iterator iter;
    const struct kefir_hashtree_node *node = kefir_hashtree_iter(&func->constants, &iter);
    REQUIRE(node != NULL, KEFIR_OK);

    REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&func->codegen->xasmgen, ".rodata", KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
    for (; node != NULL; node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, constant_label, node->key);
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, node->value);

        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->function->code, instr_ref, &instr));
        switch (instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_FLOAT32_CONST: {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&func->codegen->xasmgen, 4));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&func->codegen->xasmgen, KEFIR_AMD64_LABEL,
                                                     func->function->ir_func->name, constant_label));
                union {
                    kefir_uint32_t u32;
                    kefir_float32_t f32;
                } value = {.f32 = instr->operation.parameters.imm.float32};

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &func->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_DOUBLE, 1,
                    kefir_asm_amd64_xasmgen_operand_immu(&func->codegen->xasmgen_helpers.operands[0], value.u32)));
            } break;

            case KEFIR_OPT_OPCODE_FLOAT64_CONST: {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&func->codegen->xasmgen, 8));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&func->codegen->xasmgen, KEFIR_AMD64_LABEL,
                                                     func->function->ir_func->name, constant_label));
                union {
                    kefir_uint64_t u64;
                    kefir_float64_t f64;
                } value = {.f64 = instr->operation.parameters.imm.float64};

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &func->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                    kefir_asm_amd64_xasmgen_operand_immu(&func->codegen->xasmgen_helpers.operands[0], value.u64)));
            } break;

            case KEFIR_OPT_OPCODE_LONG_DOUBLE_CONST: {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&func->codegen->xasmgen, 8));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&func->codegen->xasmgen, KEFIR_AMD64_LABEL,
                                                     func->function->ir_func->name, constant_label));
                volatile union {
                    kefir_uint64_t u64[2];
                    kefir_long_double_t long_double;
                } value = {.u64 = {0, 0}};

                value.long_double = instr->operation.parameters.imm.long_double;

                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &func->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                    kefir_asm_amd64_xasmgen_operand_immu(&func->codegen->xasmgen_helpers.operands[0], value.u64[0])));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_DATA(
                    &func->codegen->xasmgen, KEFIR_AMD64_XASMGEN_DATA_QUAD, 1,
                    kefir_asm_amd64_xasmgen_operand_immu(&func->codegen->xasmgen_helpers.operands[0], value.u64[1])));
            } break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer instruction opcode");
        }
    }
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_SECTION(&func->codegen->xasmgen, ".text", KEFIR_AMD64_XASMGEN_SECTION_NOATTR));
    return KEFIR_OK;
}

static kefir_result_t output_asm(struct kefir_codegen_amd64 *codegen, struct kefir_codegen_amd64_function *func,
                                 kefir_bool_t reg_alloc) {
    const char *comment_prefix;
    REQUIRE_OK(kefir_asm_amd64_xasmgen_line_comment_prefix(&codegen->xasmgen, &comment_prefix));
    FILE *output = kefir_asm_amd64_xasmgen_get_output(&codegen->xasmgen);

    struct kefir_json_output json;
    REQUIRE_OK(kefir_json_output_init(&json, output, 4));
    REQUIRE_OK(kefir_json_set_line_prefix(&json, comment_prefix));
    REQUIRE_OK(kefir_json_output_object_begin(&json));
    REQUIRE_OK(kefir_json_output_object_key(&json, "function"));
    REQUIRE_OK(kefir_asmcmp_context_format(&json, &func->code.context));
    if (reg_alloc) {
        REQUIRE_OK(kefir_json_output_object_key(&json, "register_allocation"));
        REQUIRE_OK(kefir_json_output_array_begin(&json));
        for (kefir_size_t i = 0; i < func->register_allocator.num_of_vregs; i++) {
            const struct kefir_codegen_amd64_register_allocation *ra = &func->register_allocator.allocations[i];
            REQUIRE_OK(kefir_json_output_object_begin(&json));
            REQUIRE_OK(kefir_json_output_object_key(&json, "id"));
            REQUIRE_OK(kefir_json_output_uinteger(&json, i));
            const char *mnemonic;
            switch (ra->type) {
                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED:
                    REQUIRE_OK(kefir_json_output_object_key(&json, "type"));
                    REQUIRE_OK(kefir_json_output_string(&json, "unallocated"));
                    break;

                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER:
                    REQUIRE_OK(kefir_json_output_object_key(&json, "type"));
                    REQUIRE_OK(kefir_json_output_string(&json, "physical_register"));
                    mnemonic = kefir_asm_amd64_xasmgen_register_symbolic_name(ra->direct_reg);
                    REQUIRE(mnemonic != NULL,
                            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to determine amd64 register symbolic name"));
                    REQUIRE_OK(kefir_json_output_object_key(&json, "reg"));
                    REQUIRE_OK(kefir_json_output_string(&json, mnemonic));
                    break;

                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_DIRECT:
                    REQUIRE_OK(kefir_json_output_object_key(&json, "type"));
                    REQUIRE_OK(kefir_json_output_string(&json, "spill_area_direct"));
                    REQUIRE_OK(kefir_json_output_object_key(&json, "index"));
                    REQUIRE_OK(kefir_json_output_uinteger(&json, ra->spill_area.index));
                    REQUIRE_OK(kefir_json_output_object_key(&json, "length"));
                    REQUIRE_OK(kefir_json_output_uinteger(&json, ra->spill_area.length));
                    break;

                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_INDIRECT:
                    REQUIRE_OK(kefir_json_output_object_key(&json, "type"));
                    REQUIRE_OK(kefir_json_output_string(&json, "spill_area_indirect"));
                    REQUIRE_OK(kefir_json_output_object_key(&json, "index"));
                    REQUIRE_OK(kefir_json_output_uinteger(&json, ra->spill_area.index));
                    REQUIRE_OK(kefir_json_output_object_key(&json, "length"));
                    REQUIRE_OK(kefir_json_output_uinteger(&json, ra->spill_area.length));
                    break;

                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_MEMORY_POINTER:
                    REQUIRE_OK(kefir_json_output_object_key(&json, "type"));
                    REQUIRE_OK(kefir_json_output_string(&json, "memory_pointer"));
                    break;
            }
            REQUIRE_OK(kefir_json_output_object_end(&json));
        }
        REQUIRE_OK(kefir_json_output_array_end(&json));
    }
    REQUIRE_OK(kefir_json_output_object_end(&json));
    REQUIRE_OK(kefir_json_output_finalize(&json));
    fprintf(output, "\n");
    return KEFIR_OK;
}

static kefir_result_t kefir_codegen_amd64_function_translate_impl(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64 *codegen,
                                                                  struct kefir_codegen_amd64_function *func) {
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&func->codegen->xasmgen, "%s", func->function->ir_func->name));

    if (func->function->ir_func->declaration->vararg) {
        REQUIRE_OK(kefir_codegen_amd64_stack_frame_vararg(&func->stack_frame));
    }
    if (!codegen->config->omit_frame_pointer) {
        REQUIRE_OK(kefir_codegen_amd64_stack_frame_require_frame_pointer(&func->stack_frame));
    }
    REQUIRE_OK(translate_code(mem, func));
    REQUIRE_OK(
        kefir_asmcmp_pipeline_apply(mem, &codegen->pipeline, KEFIR_ASMCMP_PIPELINE_PASS_VIRTUAL, &func->code.context));
    if (codegen->config->print_details != NULL && strcmp(codegen->config->print_details, "vasm") == 0) {
        REQUIRE_OK(output_asm(codegen, func, false));
    }

    REQUIRE_OK(
        kefir_codegen_amd64_register_allocator_run(mem, &func->code, &func->stack_frame, &func->register_allocator));
    if (codegen->config->print_details != NULL && strcmp(codegen->config->print_details, "vasm+regs") == 0) {
        REQUIRE_OK(output_asm(codegen, func, true));
    }

    REQUIRE_OK(kefir_codegen_amd64_devirtualize(mem, &func->code, &func->register_allocator, &func->stack_frame));
    REQUIRE_OK(kefir_asmcmp_pipeline_apply(mem, &codegen->pipeline, KEFIR_ASMCMP_PIPELINE_PASS_DEVIRTUAL,
                                           &func->code.context));

    if (codegen->config->print_details != NULL && strcmp(codegen->config->print_details, "devasm") == 0) {
        REQUIRE_OK(output_asm(codegen, func, false));
    }

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_calculate(codegen->abi_variant, func->function->locals.type,
                                                         &func->locals_layout, &func->stack_frame));
    REQUIRE_OK(kefir_asmcmp_amd64_generate_code(mem, &codegen->xasmgen, &func->code, &func->stack_frame));
    REQUIRE_OK(generate_constants(mem, func));
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
    REQUIRE_OK(kefir_asmcmp_amd64_init(function->ir_func->name, codegen->abi_variant,
                                       codegen->config->position_independent_code, &func.code));
    REQUIRE_OK(kefir_hashtree_init(&func.instructions, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&func.labels, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&func.virtual_registers, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&func.constants, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&func.translated_instructions, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_init(&func.stack_frame));
    REQUIRE_OK(kefir_codegen_amd64_register_allocator_init(&func.register_allocator));
    REQUIRE_OK(kefir_abi_amd64_function_decl_alloc(mem, codegen->abi_variant, function->ir_func->declaration,
                                                   &func.abi_function_declaration));
    kefir_result_t res = KEFIR_OK;
    if (function->locals.type != NULL) {
        res = kefir_abi_amd64_type_layout(mem, codegen->abi_variant, function->locals.type, &func.locals_layout);
        REQUIRE_ELSE(res == KEFIR_OK, { goto on_error1; });
    }

    res = kefir_codegen_amd64_function_translate_impl(mem, codegen, &func);

    if (function->locals.type != NULL) {
        kefir_abi_amd64_type_layout_free(mem, &func.locals_layout);
    }
on_error1:
    kefir_abi_amd64_function_decl_free(mem, &func.abi_function_declaration);
    kefir_codegen_amd64_register_allocator_free(mem, &func.register_allocator);
    kefir_codegen_amd64_stack_frame_free(mem, &func.stack_frame);
    kefir_hashtreeset_free(mem, &func.translated_instructions);
    kefir_hashtree_free(mem, &func.constants);
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
