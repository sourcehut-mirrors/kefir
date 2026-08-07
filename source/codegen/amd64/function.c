/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#include "kefir/optimizer/function.h"
#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/codegen/amd64/stack_frame.h"
#include "kefir/codegen/amd64/translator.h"
#include "kefir/codegen/amd64/constants.h"
#include "kefir/codegen/asmcmp/context.h"
#include "kefir/codegen/asmcmp/type_defs.h"
#include "kefir/codegen/function.h"
#include "kefir/codegen/target-ir/regalloc.h"
#include "kefir/codegen/target-ir/format.h"
#include "kefir/core/basic-types.h"
#include "kefir/core/hashset.h"
#include "kefir/core/hashtree.h"
#include "kefir/core/mem.h"
#include "kefir/ir/module.h"
#include "kefir/optimizer/module.h"
#include "kefir/optimizer/schedule.h"
#include "kefir/target/abi/amd64/base.h"
#include "kefir/codegen/amd64/function.h"
#include "kefir/codegen/amd64/scheduler.h"
#include "kefir/codegen/amd64/instructions.h"
#include "kefir/codegen/amd64/module.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/codegen/asmcmp/transform.h"
#include "kefir/codegen/asmcmp/format.h"
#include "kefir/optimizer/code.h"
#include "kefir/optimizer/topological_schedule.h"
#include "kefir/codegen/target-ir/code.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t alias_return_space_allocation(struct kefir_codegen_amd64_function *func) {
    REQUIRE(func->generic.variable_allocator.return_space_variable_ref == KEFIR_ID_NONE, KEFIR_OK);
    kefir_size_t num_of_blocks = kefir_opt_code_container_block_count(&func->generic.function->code);
    for (kefir_opt_block_id_t block_id = 0; block_id < num_of_blocks; block_id++) {
        if (block_id != func->generic.function->code.entry_point &&
            func->generic.control_flow.blocks[block_id].immediate_dominator == KEFIR_ID_NONE) {
            continue;
        }

        const struct kefir_opt_code_block *block;
        REQUIRE_OK(kefir_opt_code_container_block(&func->generic.function->code, block_id, &block));

        kefir_opt_instruction_ref_t tail_ref;
        REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&func->generic.function->code, block_id, &tail_ref));
        if (tail_ref == KEFIR_ID_NONE) {
            continue;
        }

        const struct kefir_opt_instruction *tail_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->generic.function->code, tail_ref, &tail_instr));
        if (tail_instr->operation.opcode != KEFIR_OPT_OPCODE_RETURN ||
            tail_instr->operation.parameters.refs[0] == KEFIR_ID_NONE) {
            continue;
        }

        const struct kefir_opt_instruction *returned_instr, *return_space_instr, *alloc_instr = NULL;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->generic.function->code, tail_instr->operation.parameters.refs[0],
                                                  &returned_instr));
        if (returned_instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) {
            alloc_instr = returned_instr;
        } else if (returned_instr->operation.opcode == KEFIR_OPT_OPCODE_INVOKE ||
                   returned_instr->operation.opcode == KEFIR_OPT_OPCODE_INVOKE_VIRTUAL ||
                   returned_instr->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE ||
                   returned_instr->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL) {
            const struct kefir_opt_call_node *call;
            REQUIRE_OK(kefir_opt_code_container_call(
                &func->generic.function->code, returned_instr->operation.parameters.function_call.call_ref, &call));
            if (call->return_space != KEFIR_ID_NONE) {
                REQUIRE_OK(
                    kefir_opt_code_container_instr(&func->generic.function->code, call->return_space, &return_space_instr));
                if (return_space_instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) {
                    alloc_instr = return_space_instr;
                }
            }
        }

        if (alloc_instr != NULL) {
            const struct kefir_ir_type *alloc_ir_type =
                kefir_ir_module_get_named_type(func->generic.module->ir_module, alloc_instr->operation.parameters.type.type_id);
            REQUIRE(alloc_ir_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type"));
            kefir_bool_t same_type;
            REQUIRE_OK(kefir_ir_type_same(func->generic.function->ir_func->declaration->result, 0, alloc_ir_type,
                                          alloc_instr->operation.parameters.type.type_index, &same_type));
            if (same_type && func->stack_frame.return_space_vreg != KEFIR_ASMCMP_INDEX_NONE) {
                REQUIRE_OK(kefir_codegen_local_variable_allocator_mark_return_space(&func->generic.variable_allocator,
                                                                                    alloc_instr->id));
                break;
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t generate_vararg_prologue(struct kefir_mem *mem, struct kefir_codegen_amd64_function *func, kefir_asmcmp_instruction_index_t *after_prologue) {
    REQUIRE_OK(kefir_asmcmp_amd64_produce_virtual_register(mem, &func->code, *after_prologue, func->translator.vararg_area,
                                                            after_prologue));
    kefir_asmcmp_label_index_t save_int_label;
    switch (func->codegen->abi_variant) {
        case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V: {
            kefir_asmcmp_virtual_register_index_t vreg;
            REQUIRE_OK(
                kefir_asmcmp_context_new_label(mem, &func->code.context, KEFIR_ASMCMP_INDEX_NONE, &save_int_label));
            REQUIRE_OK(
                kefir_codegen_amd64_function_translator_get_argument_register(mem, &func->translator, &func->code, KEFIR_AMD64_XASMGEN_REGISTER_RAX, &vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_test(mem, &func->code, *after_prologue, &KEFIR_ASMCMP_MAKE_VREG8(vreg),
                                                &KEFIR_ASMCMP_MAKE_VREG8(vreg), after_prologue));
            REQUIRE_OK(kefir_asmcmp_amd64_je(mem, &func->code, *after_prologue,
                                                &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(save_int_label), after_prologue));
            for (kefir_size_t i = 0; i < 8; i++) {
                REQUIRE_OK(kefir_codegen_amd64_function_translator_get_argument_register(mem, &func->translator, &func->code,
                                                                        KEFIR_AMD64_XASMGEN_REGISTER_XMM0 + i, &vreg));
                REQUIRE_OK(kefir_asmcmp_amd64_movdqu(
                    mem, &func->code, *after_prologue,
                    &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(func->translator.vararg_area, 48 + i * 16,
                                                        KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                    &KEFIR_ASMCMP_MAKE_VREG(vreg), after_prologue));
            }
            REQUIRE_OK(
                kefir_codegen_amd64_function_translator_get_argument_register(mem, &func->translator, &func->code, KEFIR_AMD64_XASMGEN_REGISTER_RDI, &vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &func->code, *after_prologue,
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(func->translator.vararg_area, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                &KEFIR_ASMCMP_MAKE_VREG(vreg), after_prologue));
            REQUIRE_OK(kefir_asmcmp_context_bind_label(mem, &func->code.context, *after_prologue, save_int_label));
            REQUIRE_OK(
                kefir_codegen_amd64_function_translator_get_argument_register(mem, &func->translator, &func->code, KEFIR_AMD64_XASMGEN_REGISTER_RSI, &vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &func->code, *after_prologue,
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(func->translator.vararg_area, 8, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                &KEFIR_ASMCMP_MAKE_VREG(vreg), after_prologue));
            REQUIRE_OK(
                kefir_codegen_amd64_function_translator_get_argument_register(mem, &func->translator, &func->code, KEFIR_AMD64_XASMGEN_REGISTER_RDX, &vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &func->code, *after_prologue,
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(func->translator.vararg_area, 16, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                &KEFIR_ASMCMP_MAKE_VREG(vreg), after_prologue));
            REQUIRE_OK(
                kefir_codegen_amd64_function_translator_get_argument_register(mem, &func->translator, &func->code, KEFIR_AMD64_XASMGEN_REGISTER_RCX, &vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &func->code, *after_prologue,
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(func->translator.vararg_area, 24, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                &KEFIR_ASMCMP_MAKE_VREG(vreg), after_prologue));
            REQUIRE_OK(
                kefir_codegen_amd64_function_translator_get_argument_register(mem, &func->translator, &func->code, KEFIR_AMD64_XASMGEN_REGISTER_R8, &vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &func->code, *after_prologue,
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(func->translator.vararg_area, 32, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                &KEFIR_ASMCMP_MAKE_VREG(vreg), after_prologue));
            REQUIRE_OK(
                kefir_codegen_amd64_function_translator_get_argument_register(mem, &func->translator, &func->code, KEFIR_AMD64_XASMGEN_REGISTER_R9, &vreg));
            REQUIRE_OK(kefir_asmcmp_amd64_mov(
                mem, &func->code, *after_prologue,
                &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(func->translator.vararg_area, 40, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                &KEFIR_ASMCMP_MAKE_VREG(vreg), after_prologue));
        } break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 abi variant");
    }

    return KEFIR_OK;
}

static kefir_result_t instantiate_block_labels(struct kefir_mem *mem, struct kefir_codegen_amd64_function *func) {
    for (kefir_size_t block_linear_index = 0;
         block_linear_index < kefir_opt_code_schedule_num_of_blocks(&func->generic.schedule); block_linear_index++) {
        kefir_opt_block_id_t block_id;
        kefir_asmcmp_label_index_t asmlabel, end_asmlabel;
        REQUIRE_OK(kefir_opt_code_schedule_block_by_index(&func->generic.schedule, block_linear_index, &block_id));
        REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &func->code.context, KEFIR_ASMCMP_INDEX_NONE, &asmlabel));
        REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &func->code.context, KEFIR_ASMCMP_INDEX_NONE, &end_asmlabel));
        REQUIRE_OK(kefir_codegen_amd64_function_translator_new_block(mem, &func->translator, block_id, asmlabel, end_asmlabel));

        kefir_result_t res;
        struct kefir_opt_code_block_public_label_iterator iter;
        for (res = kefir_opt_code_container_block_public_labels_iter(&func->generic.function->code, block_id, &iter);
             res == KEFIR_OK; res = kefir_opt_code_container_block_public_labels_next(&iter)) {
            REQUIRE_OK(
                kefir_asmcmp_context_label_add_public_name(mem, &func->code.context, asmlabel, iter.public_label));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_block(struct kefir_mem *mem, struct kefir_codegen_amd64_function *func, kefir_opt_block_id_t block_id) {
    kefir_asmcmp_label_index_t asmlabel;
    REQUIRE_OK(kefir_codegen_amd64_function_translator_get_block_label(&func->translator, block_id, &asmlabel));
    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &func->code.context, asmlabel));

    if (func->codegen->config->debug_info) {
        REQUIRE_OK(kefir_asmcmp_context_label_mark_external_dependencies(mem, &func->code.context, asmlabel));
    }

    struct kefir_opt_code_block_schedule_iterator iter;
    kefir_result_t res;
    for (res = kefir_opt_code_block_schedule_iter(&func->generic.schedule, block_id, &iter); res == KEFIR_OK;
            res = kefir_opt_code_block_schedule_next(&iter)) {
        const struct kefir_opt_instruction *instr = NULL;
        REQUIRE_OK(kefir_opt_code_container_instr(&func->generic.function->code, iter.instr_ref, &instr));
        REQUIRE_OK(kefir_codegen_amd64_function_translate_instruction(mem, func, instr));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    kefir_asmcmp_instruction_index_t block_begin_idx;
    REQUIRE_OK(kefir_asmcmp_context_label_at(&func->code.context, asmlabel, &block_begin_idx));
    if (block_begin_idx != KEFIR_ASMCMP_INDEX_NONE) {
        block_begin_idx = kefir_asmcmp_context_instr_prev(&func->code.context, block_begin_idx);
    } else {
        block_begin_idx = kefir_asmcmp_context_instr_tail(&func->code.context);
    }

    kefir_asmcmp_label_index_t end_asmlabel;
    REQUIRE_OK(kefir_codegen_amd64_function_translator_get_block_end_label(&func->translator, block_id, &end_asmlabel));
    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &func->code.context, end_asmlabel));
    if (func->codegen->config->debug_info) {
        REQUIRE_OK(kefir_asmcmp_context_label_mark_external_dependencies(mem, &func->code.context, end_asmlabel));
    }
    return KEFIR_OK;
}

static kefir_result_t extend_vreg_lifetimes(struct kefir_mem *mem, struct kefir_codegen_amd64_function *func) {
    struct kefir_hashset_iterator exit_point_iter;
    kefir_hashset_key_t exit_point;
    kefir_result_t res;
    for (res = kefir_hashset_iter(&func->translator.function_exit_points, &exit_point_iter, &exit_point);
        res == KEFIR_OK;
        res = kefir_hashset_next(&exit_point_iter, &exit_point)) {
        ASSIGN_DECL_CAST(kefir_asmcmp_instruction_index_t, idx, exit_point);
        kefir_asmcmp_virtual_register_index_t next_instr_idx =
            kefir_asmcmp_context_instr_next(&func->code.context, idx);

        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t alive_vreg;
        for (res = kefir_hashset_iter(&func->translator.virtual_registers_alive_at_exit, &iter, &alive_vreg);
            res == KEFIR_OK;
            res = kefir_hashset_next(&iter, &alive_vreg)) {
            ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg, alive_vreg);
            REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(mem, &func->code, idx, vreg, &idx));
            if (next_instr_idx != KEFIR_ASMCMP_INDEX_NONE) {
                REQUIRE_OK(kefir_asmcmp_context_move_labels(mem, &func->code.context, idx, next_instr_idx));
                next_instr_idx = KEFIR_ASMCMP_INDEX_NONE;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t translate_code(struct kefir_mem *mem, struct kefir_codegen_amd64_function *func) {
    kefir_bool_t implicit_parameter_present;
    kefir_asm_amd64_xasmgen_register_t implicit_parameter_reg;
    REQUIRE_OK(kefir_abi_amd64_function_decl_returns_implicit_parameter(
        &func->abi_function_declaration, &implicit_parameter_present, &implicit_parameter_reg));
    if (implicit_parameter_present) {
        kefir_asmcmp_virtual_register_index_t implicit_param_vreg;
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &func->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &implicit_param_vreg));
        func->stack_frame.return_space_vreg = implicit_param_vreg;
    }

    // Schedule code
    struct kefir_codegen_amd64_function_schedule_instruction_parameters scheduler_param = {.mem = mem, .func = func};
    struct kefir_opt_code_topological_scheduler scheduler;
    REQUIRE_OK(kefir_opt_code_topological_scheduler_init(&scheduler, kefir_codegen_amd64_function_schedule_instruction, &scheduler_param));
    REQUIRE_OK(kefir_opt_code_schedule_run(mem, &func->generic.schedule, &func->generic.function->code, &func->generic.control_flow,
                                           &func->generic.liveness, &scheduler.scheduler));
    REQUIRE_OK(kefir_opt_code_linear_liveness_build(mem, &func->generic.linear_liveness, &func->generic.function->code,
                                                    &func->generic.control_flow, &func->generic.schedule));

    // Initialize block labels
    REQUIRE_OK(instantiate_block_labels(mem, func));

    REQUIRE_OK(kefir_asmcmp_amd64_function_prologue(
        mem, &func->code, kefir_asmcmp_context_instr_tail(&func->code.context), &func->translator.prologue_tail));
    kefir_asmcmp_instruction_index_t after_prologue = func->translator.prologue_tail;
    if (implicit_parameter_present) {
        kefir_asmcmp_virtual_register_index_t implicit_param_placement_vreg;
        REQUIRE_OK(kefir_codegen_amd64_function_translator_get_argument_register(mem, &func->translator, &func->code, implicit_parameter_reg,
                                                             &implicit_param_placement_vreg));
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &func->code, after_prologue,
                                                             func->stack_frame.return_space_vreg,
                                                             implicit_param_placement_vreg, &after_prologue));
        REQUIRE_OK(kefir_codegen_amd64_function_translator_add_virtual_register_alive_at_exit_point(mem, &func->translator, func->stack_frame.return_space_vreg));
        REQUIRE_OK(alias_return_space_allocation(func));
    }

    // Translate blocks
    for (kefir_size_t block_linear_index = 0;
         block_linear_index < kefir_opt_code_schedule_num_of_blocks(&func->generic.schedule); block_linear_index++) {
        kefir_opt_block_id_t block_id;
        REQUIRE_OK(kefir_opt_code_schedule_block_by_index(&func->generic.schedule, block_linear_index, &block_id));
        REQUIRE_OK(translate_block(mem, func, block_id));        
    }

    REQUIRE_OK(kefir_codegen_amd64_function_translator_add_exit_point(mem, &func->translator, kefir_asmcmp_context_instr_tail(&func->code.context)));
    REQUIRE_OK(extend_vreg_lifetimes(mem, func));

    if (func->translator.vararg_area != KEFIR_ASMCMP_INDEX_NONE) {
        REQUIRE_OK(generate_vararg_prologue(mem, func, &after_prologue));
    }

    REQUIRE_OK(kefir_asmcmp_amd64_noop(mem, &func->code, kefir_asmcmp_context_instr_tail(&func->code.context), NULL));
    return KEFIR_OK;
}

static kefir_result_t output_asmcmp(struct kefir_codegen_amd64 *codegen, const struct kefir_asmcmp_context *code,
                                 kefir_bool_t debug_info) {
    const char *comment_prefix;
    REQUIRE_OK(kefir_asm_amd64_xasmgen_line_comment_prefix(&codegen->xasmgen, &comment_prefix));
    FILE *output = kefir_asm_amd64_xasmgen_get_output(&codegen->xasmgen);

    struct kefir_json_output json;
    REQUIRE_OK(kefir_json_output_init(&json, output, 4));
    REQUIRE_OK(kefir_json_set_line_prefix(&json, comment_prefix));
    REQUIRE_OK(kefir_json_output_object_begin(&json));
    REQUIRE_OK(kefir_json_output_object_key(&json, "function"));
    REQUIRE_OK(kefir_asmcmp_context_format(&json, code, debug_info));
    REQUIRE_OK(kefir_json_output_object_end(&json));
    REQUIRE_OK(kefir_json_output_finalize(&json));
    fprintf(output, "\n");
    return KEFIR_OK;
}

static kefir_result_t output_target_ir(struct kefir_mem *mem, struct kefir_codegen_amd64 *codegen, const struct kefir_codegen_target_ir_code *code, const struct kefir_codegen_target_ir_regalloc *regalloc) {
    const char *comment_prefix;
    REQUIRE_OK(kefir_asm_amd64_xasmgen_line_comment_prefix(&codegen->xasmgen, &comment_prefix));
    FILE *output = kefir_asm_amd64_xasmgen_get_output(&codegen->xasmgen);

    struct kefir_json_output json;
    REQUIRE_OK(kefir_json_output_init(&json, output, 4));
    REQUIRE_OK(kefir_json_set_line_prefix(&json, comment_prefix));
    REQUIRE_OK(kefir_codegen_target_ir_code_format(mem, code, regalloc, &json));
    REQUIRE_OK(kefir_json_output_finalize(&json));
    fprintf(output, "\n");
    return KEFIR_OK;
}

struct variable_allocator_type_layout_param {
    struct kefir_mem *mem;
    struct kefir_codegen_amd64_function *func;
};

static kefir_result_t variable_allocator_type_layout(kefir_id_t type_id, kefir_size_t type_index,
                                                     kefir_size_t *size_ptr, kefir_size_t *alignment_ptr,
                                                     void *payload) {
    ASSIGN_DECL_CAST(struct variable_allocator_type_layout_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen variable allocator hook parameter"));

    const struct kefir_abi_amd64_type_layout *type_layout = NULL;
    const struct kefir_abi_amd64_typeentry_layout *typeentry_layout = NULL;
    REQUIRE_OK(kefir_codegen_amd64_function_translator_get_local_variable_type_layout(param->mem, &param->func->translator, param->func->generic.module->ir_module, param->func->codegen->abi_variant, type_id, &type_layout));
    REQUIRE_OK(kefir_abi_amd64_type_layout_at(type_layout, type_index, &typeentry_layout));

    ASSIGN_PTR(size_ptr, typeentry_layout->size);
    ASSIGN_PTR(alignment_ptr, typeentry_layout->alignment);
    return KEFIR_OK;
}

static kefir_result_t detect_extra_alignment(struct kefir_mem *mem, struct kefir_codegen_amd64_function *func) {
    struct kefir_hashtree_node_iterator scopes_iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&func->generic.variable_scopes.scope_variables, &scopes_iter);
         node != NULL; node = kefir_hashtree_next(&scopes_iter)) {
        ASSIGN_DECL_CAST(struct kefir_opt_code_scope_variables *, scope_vars, node->value);

        kefir_result_t res;
        kefir_hashset_key_t entry;
        struct kefir_hashset_iterator iter;
        for (res = kefir_hashset_iter(&scope_vars->allocations, &iter, &entry); res == KEFIR_OK;
             res = kefir_hashset_next(&iter, &entry)) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, entry);
            const struct kefir_opt_instruction *instr;
            REQUIRE_OK(kefir_opt_code_container_instr(&func->generic.function->code, instr_ref, &instr));
            REQUIRE(instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected local variable allocation instruction"));

            const struct kefir_abi_amd64_type_layout *type_layout = NULL;
            const struct kefir_abi_amd64_typeentry_layout *typeentry_layout = NULL;
            REQUIRE_OK(
                kefir_codegen_amd64_function_translator_get_local_variable_type_layout(mem, &func->translator, func->generic.module->ir_module, func->codegen->abi_variant, instr->operation.parameters.type.type_id, &type_layout));
            REQUIRE_OK(kefir_abi_amd64_type_layout_at(type_layout, instr->operation.parameters.type.type_index,
                                                      &typeentry_layout));

            REQUIRE_OK(
                kefir_codegen_amd64_stack_frame_require_alignment(&func->stack_frame, typeentry_layout->alignment));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translate(struct kefir_mem *mem,
                                                      struct kefir_codegen_amd64_function *func) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen function"));

    const struct kefir_ir_identifier *ir_identifier;
    REQUIRE_OK(kefir_ir_module_get_identifier(func->generic.module->ir_module, func->generic.function->ir_func->name, &ir_identifier));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&func->codegen->xasmgen, "%s", ir_identifier->symbol));

    const struct kefir_source_location *function_source_location =
        kefir_ir_function_debug_info_source_location(&func->generic.function->ir_func->debug_info);
    if (function_source_location != NULL && func->codegen->debug_info_tracker != NULL) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DEBUG_INFO_SOURCE_LOCATION(mem, &func->codegen->xasmgen, func->codegen->debug_info_tracker,
                                                                  function_source_location));
    }

    if (!func->codegen->config->omit_frame_pointer) {
        REQUIRE_OK(kefir_codegen_amd64_stack_frame_require_frame_pointer(&func->stack_frame));
    }
    REQUIRE_OK(kefir_opt_code_control_flow_build(mem, &func->generic.control_flow, &func->generic.function->code));
    REQUIRE_OK(kefir_opt_code_liveness_build(mem, &func->generic.liveness, &func->generic.control_flow));
    REQUIRE_OK(kefir_opt_code_variable_scopes_build(mem, &func->generic.variable_scopes, &func->generic.liveness));
    REQUIRE_OK(detect_extra_alignment(mem, func));
    REQUIRE_OK(translate_code(mem, func));
    REQUIRE_OK(kefir_codegen_local_variable_allocator_run(
        mem, &func->generic.variable_allocator, &func->generic.function->code,
        &(struct kefir_codegen_local_variable_allocator_hooks) {
            .type_layout = variable_allocator_type_layout,
            .payload = &(struct variable_allocator_type_layout_param) {.mem = mem, .func = func}},
        &func->generic.variable_scopes));
    REQUIRE_OK(kefir_opt_code_liveness_reset(mem, &func->generic.liveness));
    REQUIRE_OK(kefir_opt_code_control_flow_reset(mem, &func->generic.control_flow));

    if (func->codegen->config->print_details != NULL && strcmp(func->codegen->config->print_details, "vasm") == 0) {
        REQUIRE_OK(output_asmcmp(func->codegen, &func->code.context, func->codegen->config->debug_info));
    }

    REQUIRE_OK(kefir_codegen_amd64_function_target_ir_apply(mem, func->codegen, &func->translator, &func->target_ir, &func->target_ir.target_ir_metadata, &func->code, &func->stack_frame, &func->generic.function->debug_info));

    if (func->codegen->config->print_details != NULL && strcmp(func->codegen->config->print_details, "target_ir") == 0) {
        REQUIRE_OK(output_target_ir(mem, func->codegen, &func->target_ir.code, &func->target_ir.regalloc));
    }

    REQUIRE_OK(kefir_asmcmp_drop_virtual_instructions(mem, &func->code.context));
    REQUIRE_OK(kefir_asmcmp_compact_labels(mem, &func->code.context));
    REQUIRE_OK(kefir_asmcmp_code_map_coalesce(mem, &func->code.context.debug_info.code_map));

    if (func->codegen->config->print_details != NULL && strcmp(func->codegen->config->print_details, "devasm") == 0) {
        REQUIRE_OK(output_asmcmp(func->codegen, &func->code.context, func->codegen->config->debug_info));
    }

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_calculate(func->codegen->abi_variant, &func->stack_frame));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&func->codegen->xasmgen, KEFIR_AMD64_FUNCTION_BEGIN, func->codegen->symbol_prefix,
                                         ir_identifier->symbol));
    REQUIRE_OK(kefir_asmcmp_amd64_generate_code(mem, &func->codegen->xasmgen, func->codegen->debug_info_tracker, &func->code,
                                                &func->stack_frame, func->codegen->symbol_prefix));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&func->codegen->xasmgen, KEFIR_AMD64_FUNCTION_END, func->codegen->symbol_prefix,
                                         ir_identifier->symbol));
    REQUIRE_OK(kefir_codegen_amd64_generate_local_constants(&func->translator, func->generic.module, func->generic.function, func->codegen));
    if (func->codegen->config->consume_optimizer_code) {
        REQUIRE_OK(kefir_opt_code_container_clear(mem, &func->generic.function->code));
    }

    REQUIRE_OK(kefir_asmcmp_context_instr_drop_code(mem, &func->code.context));
    REQUIRE_OK(kefir_opt_code_schedule_clear(mem, &func->generic.schedule));
    REQUIRE_OK(kefir_opt_code_linear_liveness_clear(mem, &func->generic.linear_liveness));

    if (!func->codegen->config->debug_info) {
        REQUIRE_OK(kefir_codegen_target_ir_regalloc_reset(mem, &func->target_ir.regalloc));
        REQUIRE_OK(kefir_codegen_target_ir_code_reset(mem, &func->target_ir.code));
    }
    return KEFIR_OK;
}

static kefir_result_t codegen_function_virtual_register_resolve(kefir_opt_instruction_ref_t instr_ref, kefir_asmcmp_instruction_index_t *vreg_idx, void *payload) {
    ASSIGN_DECL_CAST(struct kefir_codegen_amd64_function *, function, payload);
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expevted valid codegen function payload"));

    REQUIRE_OK(kefir_codegen_amd64_function_translator_vreg_of(&function->translator, instr_ref, vreg_idx));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_init(struct kefir_mem *mem, struct kefir_codegen_amd64_function *func,
                                                 struct kefir_codegen_amd64_module *codegen_module,
                                                 const struct kefir_opt_module *module,
                                                 struct kefir_opt_function *function) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AMD64 codegen function"));
    REQUIRE(codegen_module != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 code generator module"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    *func = (struct kefir_codegen_amd64_function) {.codegen = codegen_module->codegen,
                                                   .codegen_module = codegen_module};

    const struct kefir_ir_identifier *ir_identifier;
    REQUIRE_OK(kefir_ir_module_get_identifier(module->ir_module, function->ir_func->name, &ir_identifier));

    REQUIRE_OK(kefir_codegen_function_init(module, function, &func->generic));
    REQUIRE_OK(kefir_asmcmp_amd64_init(ir_identifier->symbol, codegen_module->codegen->abi_variant,
                                       codegen_module->codegen->config->position_independent_code, &func->code));
    REQUIRE_OK(kefir_codegen_amd64_function_target_ir_init(mem, &func->target_ir, codegen_module->codegen->abi_variant));
    REQUIRE_OK(kefir_codegen_amd64_function_translator_state_init(&func->translator, &func->target_ir.target_ir_metadata));
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_init(&func->stack_frame, &func->generic.variable_allocator));
    REQUIRE_OK(kefir_abi_amd64_function_decl_alloc(mem, codegen_module->codegen->abi_variant,
                                                   function->ir_func->declaration, &func->abi_function_declaration));

    func->generic.payload = func;
    func->generic.resolve_virtual_register = codegen_function_virtual_register_resolve;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_free(struct kefir_mem *mem, struct kefir_codegen_amd64_function *func) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen function"));

    REQUIRE_OK(kefir_abi_amd64_function_decl_free(mem, &func->abi_function_declaration));
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_free(mem, &func->stack_frame));
    REQUIRE_OK(kefir_codegen_amd64_function_target_ir_free(mem, &func->target_ir));
    REQUIRE_OK(kefir_codegen_amd64_function_translator_state_free(mem, &func->translator));
    REQUIRE_OK(kefir_asmcmp_amd64_free(mem, &func->code));
    REQUIRE_OK(kefir_codegen_function_free(mem, &func->generic));
    return KEFIR_OK;
}
