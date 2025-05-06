/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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

#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/codegen/asmcmp/context.h"
#define KEFIR_CODEGEN_AMD64_FUNCTION_INTERNAL
#include "kefir/codegen/amd64/function.h"
#include "kefir/codegen/amd64/module.h"
#include "kefir/codegen/amd64/symbolic_labels.h"
#include "kefir/codegen/amd64/devirtualize.h"
#include "kefir/codegen/asmcmp/format.h"
#include "kefir/optimizer/code.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/queue.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t translate_instruction(struct kefir_mem *mem, struct kefir_codegen_amd64_function *function,
                                            const struct kefir_opt_instruction *instruction) {
    REQUIRE_OK(kefir_codegen_amd64_function_generate_debug_instruction_locations(mem, function, instruction->id));
    REQUIRE(kefir_hashtreeset_has(&function->translated_instructions, (kefir_hashtreeset_entry_t) instruction->id),
            KEFIR_OK);
    const kefir_asmcmp_instruction_index_t begin_idx = kefir_asmcmp_context_instr_length(&function->code.context);
    switch (instruction->operation.opcode) {
#define CASE_INSTR(_id, _opcode)                                                           \
    case _opcode:                                                                          \
        REQUIRE_OK(KEFIR_CODEGEN_AMD64_INSTRUCTION_IMPL(_id)(mem, function, instruction)); \
        break
        KEFIR_CODEGEN_AMD64_INSTRUCTIONS(CASE_INSTR, ;);
#undef CASE_INSTR
    }

    kefir_size_t instruction_location;
    REQUIRE_OK(kefir_opt_code_debug_info_instruction_location(&function->function->debug_info, instruction->id,
                                                              &instruction_location));
    if (instruction_location != KEFIR_OPT_CODE_DEBUG_INSTRUCTION_LOCATION_NONE) {
        const struct kefir_ir_debug_source_location *source_location;
        kefir_result_t res = kefir_ir_debug_function_source_map_find(
            &function->function->ir_func->debug_info.source_map, instruction_location, &source_location);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            const kefir_asmcmp_instruction_index_t end_idx = kefir_asmcmp_context_instr_length(&function->code.context);
            REQUIRE_OK(kefir_asmcmp_source_map_add_location(mem, &function->code.context.debug_info.source_map,
                                                            &function->code.context.strings, begin_idx, end_idx,
                                                            &source_location->location));
        }
    }

    if (instruction->operation.opcode == KEFIR_OPT_OPCODE_GET_ARGUMENT) {
        kefir_result_t res = kefir_hashtree_insert(mem, &function->debug.function_parameters,
                                                   (kefir_hashtree_key_t) instruction->operation.parameters.index,
                                                   (kefir_hashtree_value_t) instruction->id);
        if (res != KEFIR_ALREADY_EXISTS) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

struct translate_instruction_collector_param {
    struct kefir_mem *mem;
    struct kefir_codegen_amd64_function *func;
    struct kefir_queue *queue;
};

static kefir_result_t translate_instruction_collector_callback(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct translate_instruction_collector_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid collector callback parameter"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&param->func->function->code, instr_ref, &instr));

    if (kefir_opt_code_schedule_has_block(&param->func->schedule, instr->block_id)) {
        REQUIRE_OK(kefir_queue_push(param->mem, param->queue, (kefir_queue_entry_t) instr));
    }
    return KEFIR_OK;
}

static kefir_result_t collect_translated_instructions_impl(struct kefir_mem *mem,
                                                           struct kefir_codegen_amd64_function *func,
                                                           struct kefir_queue *queue) {
    for (kefir_size_t block_linear_index = 0;
         block_linear_index < kefir_opt_code_schedule_num_of_blocks(&func->schedule); block_linear_index++) {
        kefir_opt_block_id_t block_id;
        const struct kefir_opt_code_block *block;
        REQUIRE_OK(kefir_opt_code_schedule_block_by_index(&func->schedule, block_linear_index, &block_id));
        REQUIRE_OK(kefir_opt_code_container_block(&func->function->code, block_id, &block));

        kefir_opt_instruction_ref_t instr_ref;
        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_block_instr_control_head(&func->function->code, block, &instr_ref));
        for (; instr_ref != KEFIR_ID_NONE;) {
            REQUIRE_OK(kefir_opt_code_container_instr(&func->function->code, instr_ref, &instr));
            REQUIRE_OK(kefir_queue_push(mem, queue, (kefir_queue_entry_t) instr));
            REQUIRE_OK(kefir_opt_instruction_next_control(&func->function->code, instr_ref, &instr_ref));
        }
    }

    struct translate_instruction_collector_param param = {.mem = mem, .func = func, .queue = queue};
    while (!kefir_queue_is_empty(queue)) {
        kefir_queue_entry_t entry;
        REQUIRE_OK(kefir_queue_pop_first(mem, queue, &entry));
        ASSIGN_DECL_CAST(struct kefir_opt_instruction *, instr, entry);

        if (!kefir_hashtreeset_has(&func->translated_instructions, (kefir_hashtreeset_entry_t) instr->id)) {
            REQUIRE_OK(
                kefir_hashtreeset_add(mem, &func->translated_instructions, (kefir_hashtreeset_entry_t) instr->id));
            REQUIRE_OK(kefir_opt_instruction_extract_inputs(&func->function->code, instr, false,
                                                            translate_instruction_collector_callback, &param));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t collect_translated_instructions(struct kefir_mem *mem,
                                                      struct kefir_codegen_amd64_function *func) {
    struct kefir_queue queue;
    REQUIRE_OK(kefir_queue_init(&queue));
    kefir_result_t res = collect_translated_instructions_impl(mem, func, &queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_queue_free(mem, &queue);
        return res;
    });
    REQUIRE_OK(kefir_queue_free(mem, &queue));
    return KEFIR_OK;
}

struct scheduler_schedule_param {
    struct kefir_mem *mem;
    struct kefir_codegen_amd64_function *func;
};

kefir_result_t kefir_codegen_amd64_tail_call_possible(struct kefir_mem *mem,
                                                      struct kefir_codegen_amd64_function *function,
                                                      kefir_opt_call_id_t call_ref,
                                                      kefir_bool_t *tail_call_possible_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen function"));
    REQUIRE(tail_call_possible_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    const struct kefir_opt_call_node *call_node;
    REQUIRE_OK(kefir_opt_code_container_call(&function->function->code, call_ref, &call_node));

    const struct kefir_ir_function_decl *ir_func_decl =
        kefir_ir_module_get_declaration(function->module->ir_module, call_node->function_declaration_id);
    REQUIRE(ir_func_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR function declaration"));

    struct kefir_abi_amd64_function_decl abi_func_decl;
    REQUIRE_OK(kefir_abi_amd64_function_decl_alloc(mem, function->codegen->abi_variant, ir_func_decl, &abi_func_decl));

    struct kefir_abi_amd64_function_parameter_requirements reqs, return_reqs;
    const struct kefir_abi_amd64_function_parameters *parameters;
    const struct kefir_abi_amd64_function_parameters *return_parameters;

    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(&res, kefir_abi_amd64_function_decl_parameters(&abi_func_decl, &parameters));
    REQUIRE_CHAIN(&res, kefir_abi_amd64_function_parameters_requirements(parameters, &reqs));
    REQUIRE_CHAIN(&res, kefir_abi_amd64_function_decl_returns(&abi_func_decl, &return_parameters));
    REQUIRE_CHAIN(&res, kefir_abi_amd64_function_parameters_requirements(return_parameters, &return_reqs));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_abi_amd64_function_decl_free(mem, &abi_func_decl);
        return KEFIR_OK;
    });
    REQUIRE_OK(kefir_abi_amd64_function_decl_free(mem, &abi_func_decl));

    *tail_call_possible_ptr =
        reqs.stack == 0 && return_reqs.stack == 0 && !ir_func_decl->returns_twice && !ir_func_decl->vararg;
    return KEFIR_OK;
}

static kefir_result_t scheduler_schedule(kefir_opt_instruction_ref_t instr_ref,
                                         kefir_opt_code_instruction_scheduler_dependency_callback_t dependency_callback,
                                         void *dependency_callback_payload, kefir_bool_t *schedule_instruction,
                                         void *payload) {
    REQUIRE(
        dependency_callback != NULL,
        KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction dependency scheduler callback"));
    REQUIRE(schedule_instruction != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction scheduler flag"));
    ASSIGN_DECL_CAST(struct scheduler_schedule_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer scheduler parameter"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&param->func->function->code, instr_ref, &instr));
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_LOCAL_LIFETIME_MARK) {
        return KEFIR_OK;
    }
    if ((instr->operation.opcode == KEFIR_OPT_OPCODE_BRANCH_COMPARE &&
         (instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT32_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT32_NOT_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT32_GREATER ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT32_GREATER_OR_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER ||
          instr->operation.parameters.branch.comparison.operation ==
              KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER_OR_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT32_LESSER ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT32_LESSER_OR_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER_OR_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT64_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT64_NOT_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT64_GREATER ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT64_GREATER_OR_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER ||
          instr->operation.parameters.branch.comparison.operation ==
              KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER_OR_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT64_LESSER ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT64_LESSER_OR_EQUAL ||
          instr->operation.parameters.branch.comparison.operation == KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER ||
          instr->operation.parameters.branch.comparison.operation ==
              KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER_OR_EQUAL)) ||
        (instr->operation.opcode == KEFIR_OPT_OPCODE_SCALAR_COMPARE &&
         (instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_EQUAL ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_NOT_EQUAL ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_GREATER ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_GREATER_OR_EQUAL ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER_OR_EQUAL ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_EQUAL ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_NOT_EQUAL ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_GREATER ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_GREATER_OR_EQUAL ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER ||
          instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER_OR_EQUAL))) {
        const struct kefir_opt_instruction *arg2_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(&param->func->function->code, instr->operation.parameters.refs[1],
                                                  &arg2_instr));
        if (arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_CONST ||
            arg2_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT64_CONST) {
            REQUIRE_OK(dependency_callback(instr->operation.parameters.refs[0], dependency_callback_payload));
            *schedule_instruction = true;
            return KEFIR_OK;
        }
    }
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_SCALAR_COMPARE &&
        (instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_LESSER ||
         instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_LESSER_OR_EQUAL ||
         instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER ||
         instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER_OR_EQUAL ||
         instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_LESSER ||
         instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_LESSER_OR_EQUAL ||
         instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER ||
         instr->operation.parameters.comparison == KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER_OR_EQUAL)) {
        const struct kefir_opt_instruction *arg1_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(&param->func->function->code, instr->operation.parameters.refs[0],
                                                  &arg1_instr));
        if (arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT32_CONST ||
            arg1_instr->operation.opcode == KEFIR_OPT_OPCODE_FLOAT64_CONST) {
            REQUIRE_OK(dependency_callback(instr->operation.parameters.refs[1], dependency_callback_payload));
            *schedule_instruction = true;
            return KEFIR_OK;
        }
    }
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE ||
        instr->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL) {
        const struct kefir_opt_call_node *call_node;
        REQUIRE_OK(kefir_opt_code_container_call(&param->func->function->code,
                                                 instr->operation.parameters.function_call.call_ref, &call_node));

        kefir_bool_t tail_call_possible;
        REQUIRE_OK(
            kefir_codegen_amd64_tail_call_possible(param->mem, param->func, call_node->node_id, &tail_call_possible));

        if (call_node->return_space != KEFIR_ID_NONE && !tail_call_possible) {
            REQUIRE_OK(dependency_callback(call_node->return_space, dependency_callback_payload));
        }

        if (instr->operation.parameters.function_call.indirect_ref != KEFIR_ID_NONE) {
            REQUIRE_OK(dependency_callback(instr->operation.parameters.function_call.indirect_ref,
                                           dependency_callback_payload));
        }
        for (kefir_size_t i = 0; i < call_node->argument_count; i++) {
            if (call_node->arguments[i] != KEFIR_ID_NONE) {
                REQUIRE_OK(dependency_callback(call_node->arguments[i], dependency_callback_payload));
            }
        }

        *schedule_instruction = true;
        return KEFIR_OK;
    }

    REQUIRE_OK(kefir_opt_instruction_extract_inputs(&param->func->function->code, instr, true, dependency_callback,
                                                    dependency_callback_payload));
    *schedule_instruction = true;
    return KEFIR_OK;
}

static kefir_result_t translate_code(struct kefir_mem *mem, struct kefir_codegen_amd64_function *func) {
    // Schedule code
    struct scheduler_schedule_param scheduler_param = {.mem = mem, .func = func};
    struct kefir_opt_code_instruction_scheduler scheduler = {.try_schedule = scheduler_schedule,
                                                             .payload = &scheduler_param};
    REQUIRE_OK(
        kefir_opt_code_schedule_run(mem, &func->schedule, &func->function->code, &func->function_analysis, &scheduler));

    REQUIRE_OK(collect_translated_instructions(mem, func));
    // Initialize block labels
    kefir_result_t res;
    for (kefir_size_t block_linear_index = 0;
         block_linear_index < kefir_opt_code_schedule_num_of_blocks(&func->schedule); block_linear_index++) {
        kefir_opt_block_id_t block_id;
        kefir_asmcmp_label_index_t asmlabel;
        REQUIRE_OK(kefir_opt_code_schedule_block_by_index(&func->schedule, block_linear_index, &block_id));
        REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &func->code.context, KEFIR_ASMCMP_INDEX_NONE, &asmlabel));
        REQUIRE_OK(kefir_hashtree_insert(mem, &func->labels, (kefir_hashtree_key_t) block_id,
                                         (kefir_hashtree_value_t) asmlabel));

        struct kefir_opt_code_block_public_label_iterator iter;
        for (res = kefir_opt_code_container_block_public_labels_iter(&func->function->code, block_id, &iter);
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
    kefir_asmcmp_instruction_index_t after_prologue = func->prologue_tail;
    kefir_bool_t implicit_parameter_present;
    kefir_asm_amd64_xasmgen_register_t implicit_parameter_reg;
    REQUIRE_OK(kefir_abi_amd64_function_decl_returns_implicit_parameter(
        &func->abi_function_declaration, &implicit_parameter_present, &implicit_parameter_reg));
    if (implicit_parameter_present) {
        kefir_asmcmp_virtual_register_index_t implicit_param_vreg, implicit_param_placement_vreg;
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &func->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &implicit_param_vreg));
        REQUIRE_OK(kefir_asmcmp_virtual_register_new(
            mem, &func->code.context, KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE, &implicit_param_placement_vreg));
        REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_requirement(mem, &func->code, implicit_param_placement_vreg,
                                                                      implicit_parameter_reg));
        REQUIRE_OK(kefir_asmcmp_amd64_link_virtual_registers(mem, &func->code,
                                                             kefir_asmcmp_context_instr_tail(&func->code.context),
                                                             implicit_param_vreg, implicit_param_placement_vreg, NULL));
        func->return_address_vreg = implicit_param_vreg;
        REQUIRE_OK(
            kefir_hashtreeset_add(mem, &func->preserve_vregs, (kefir_hashtreeset_entry_t) func->return_address_vreg));
    }

    // Translate blocks
    for (kefir_size_t block_linear_index = 0;
         block_linear_index < kefir_opt_code_schedule_num_of_blocks(&func->schedule); block_linear_index++) {
        kefir_opt_block_id_t block_id;
        REQUIRE_OK(kefir_opt_code_schedule_block_by_index(&func->schedule, block_linear_index, &block_id));

        struct kefir_hashtree_node *asmlabel_node;
        REQUIRE_OK(kefir_hashtree_at(&func->labels, (kefir_hashtree_key_t) block_id, &asmlabel_node));
        ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, asmlabel, asmlabel_node->value);
        REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &func->code.context, asmlabel));

        struct kefir_opt_code_block_schedule_iterator iter;
        kefir_result_t res;
        for (res = kefir_opt_code_block_schedule_iter(&func->schedule, block_id, &iter); res == KEFIR_OK;
             res = kefir_opt_code_block_schedule_next(&iter)) {
            const struct kefir_opt_instruction *instr = NULL;
            REQUIRE_OK(kefir_opt_code_container_instr(&func->function->code, iter.instr_ref, &instr));
            REQUIRE_OK(translate_instruction(mem, func, instr));
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

        struct kefir_bucketset_iterator alive_instr_iter;
        kefir_bucketset_entry_t alive_instr_entry;
        for (res = kefir_bucketset_iter(&func->function_analysis.liveness.blocks[block_id].alive_instr,
                                        &alive_instr_iter, &alive_instr_entry);
             res == KEFIR_OK; res = kefir_bucketset_next(&alive_instr_iter, &alive_instr_entry)) {
            kefir_asmcmp_virtual_register_index_t vreg = 0;
            res = kefir_codegen_amd64_function_vreg_of(func, (kefir_opt_instruction_ref_t) alive_instr_entry, &vreg);
            if (res == KEFIR_NOT_FOUND) {
                res = KEFIR_OK;
                continue;
            }
            REQUIRE_OK(res);

            const struct kefir_opt_instruction *instr;
            REQUIRE_OK(kefir_opt_code_container_instr(&func->function->code,
                                                      (kefir_opt_instruction_ref_t) alive_instr_entry, &instr));

            kefir_bool_t preserve_vreg = false;
            for (const struct kefir_list_entry *iter =
                     kefir_list_head(&func->function_analysis.structure.blocks[block_id].successors);
                 !preserve_vreg && iter != NULL; kefir_list_next(&iter)) {
                ASSIGN_DECL_CAST(kefir_opt_block_id_t, succ_block_id, (kefir_uptr_t) iter->value);
                if (instr->block_id != succ_block_id &&
                    kefir_bucketset_has(&func->function_analysis.liveness.blocks[succ_block_id].alive_instr,
                                        alive_instr_entry)) {
                    preserve_vreg = true;
                }
            }
            if (preserve_vreg) {
                REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
                    mem, &func->code, kefir_asmcmp_context_instr_tail(&func->code.context), vreg, NULL));
            }
        }

        REQUIRE_OK(
            kefir_asmcmp_amd64_virtual_block_begin(mem, &func->code, block_begin_idx, block_id, &block_begin_idx));
        REQUIRE_OK(kefir_asmcmp_amd64_virtual_block_end(
            mem, &func->code, kefir_asmcmp_context_instr_tail(&func->code.context), block_id, NULL));
    }

    struct kefir_hashtreeset_iterator iter;
    for (res = kefir_hashtreeset_iter(&func->preserve_vregs, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg, iter.entry);
        REQUIRE_OK(kefir_asmcmp_amd64_touch_virtual_register(
            mem, &func->code, kefir_asmcmp_context_instr_tail(&func->code.context), vreg, NULL));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    if (func->vararg_area != KEFIR_ASMCMP_INDEX_NONE) {
        kefir_asmcmp_label_index_t save_int_label;
        switch (func->codegen->abi_variant) {
            case KEFIR_ABI_AMD64_VARIANT_SYSTEM_V:
                REQUIRE_OK(
                    kefir_asmcmp_context_new_label(mem, &func->code.context, KEFIR_ASMCMP_INDEX_NONE, &save_int_label));
                REQUIRE_OK(kefir_asmcmp_amd64_test(
                    mem, &func->code, after_prologue, &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_AL),
                    &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_AL), &after_prologue));
                REQUIRE_OK(kefir_asmcmp_amd64_je(mem, &func->code, after_prologue,
                                                 &KEFIR_ASMCMP_MAKE_INTERNAL_LABEL(save_int_label), &after_prologue));
                for (kefir_size_t i = 0; i < 8; i++) {
                    REQUIRE_OK(kefir_asmcmp_amd64_movdqu(
                        mem, &func->code, after_prologue,
                        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(func->vararg_area, 48 + i * 16,
                                                            KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                        &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_XMM0 + i), &after_prologue));
                }
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &func->code, after_prologue,
                    &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(func->vararg_area, 0, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                    &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RDI), &after_prologue));
                REQUIRE_OK(kefir_asmcmp_context_bind_label(mem, &func->code.context, after_prologue, save_int_label));
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &func->code, after_prologue,
                    &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(func->vararg_area, 8, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                    &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RSI), &after_prologue));
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &func->code, after_prologue,
                    &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(func->vararg_area, 16, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                    &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RDX), &after_prologue));
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &func->code, after_prologue,
                    &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(func->vararg_area, 24, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                    &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_RCX), &after_prologue));
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &func->code, after_prologue,
                    &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(func->vararg_area, 32, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                    &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_R8), &after_prologue));
                REQUIRE_OK(kefir_asmcmp_amd64_mov(
                    mem, &func->code, after_prologue,
                    &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(func->vararg_area, 40, KEFIR_ASMCMP_OPERAND_VARIANT_DEFAULT),
                    &KEFIR_ASMCMP_MAKE_PHREG(KEFIR_AMD64_XASMGEN_REGISTER_R9), &after_prologue));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unknown amd64 abi variant");
        }
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

        const struct kefir_ir_identifier *ir_identifier;
        REQUIRE_OK(
            kefir_ir_module_get_identifier(func->module->ir_module, func->function->ir_func->name, &ir_identifier));
        switch (instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_FLOAT32_CONST: {
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_ALIGN(&func->codegen->xasmgen, 4));
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&func->codegen->xasmgen, KEFIR_AMD64_LABEL, ir_identifier->symbol,
                                                     constant_label));
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
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&func->codegen->xasmgen, KEFIR_AMD64_LABEL, ir_identifier->symbol,
                                                     constant_label));
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
                REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&func->codegen->xasmgen, KEFIR_AMD64_LABEL, ir_identifier->symbol,
                                                     constant_label));
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
                                 kefir_bool_t reg_alloc, kefir_bool_t debug_info) {
    const char *comment_prefix;
    REQUIRE_OK(kefir_asm_amd64_xasmgen_line_comment_prefix(&codegen->xasmgen, &comment_prefix));
    FILE *output = kefir_asm_amd64_xasmgen_get_output(&codegen->xasmgen);

    struct kefir_json_output json;
    REQUIRE_OK(kefir_json_output_init(&json, output, 4));
    REQUIRE_OK(kefir_json_set_line_prefix(&json, comment_prefix));
    REQUIRE_OK(kefir_json_output_object_begin(&json));
    REQUIRE_OK(kefir_json_output_object_key(&json, "function"));
    REQUIRE_OK(kefir_asmcmp_context_format(&json, &func->code.context, debug_info));
    if (reg_alloc) {
        REQUIRE_OK(kefir_json_output_object_key(&json, "register_allocation"));
        REQUIRE_OK(kefir_json_output_array_begin(&json));
        for (kefir_size_t i = 0; i < func->xregalloc.virtual_register_length; i++) {
            const struct kefir_codegen_amd64_register_allocation *ra = &func->xregalloc.virtual_registers[i].allocation;
            const struct kefir_asmcmp_virtual_register *vreg;
            REQUIRE_OK(kefir_asmcmp_virtual_register_get(&func->code.context, i, &vreg));
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

                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_LOCAL_VARIABLE:
                    REQUIRE_OK(kefir_json_output_object_key(&json, "type"));
                    REQUIRE_OK(kefir_json_output_string(&json, "local_variable"));
                    REQUIRE_OK(kefir_json_output_object_key(&json, "identifier"));
                    REQUIRE_OK(kefir_json_output_integer(&json, vreg->parameters.local_variable.identifier));
                    REQUIRE_OK(kefir_json_output_object_key(&json, "offset"));
                    REQUIRE_OK(kefir_json_output_integer(&json, vreg->parameters.local_variable.offset));
                    break;

                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_IMMEDIATE_VALUE:
                    REQUIRE_OK(kefir_json_output_object_key(&json, "type"));
                    REQUIRE_OK(kefir_json_output_string(&json, "immediate"));
                    REQUIRE_OK(kefir_json_output_object_key(&json, "value"));
                    REQUIRE_OK(
                        kefir_asmcmp_value_format(&json, &func->code.context, &vreg->parameters.immediate_value));
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

static kefir_result_t trace_preallocation_hints(struct kefir_mem *mem, struct kefir_codegen_amd64_function *func,
                                                kefir_asmcmp_virtual_register_index_t root,
                                                struct kefir_hashtreeset *visited,
                                                kefir_asm_amd64_xasmgen_register_t *phreg) {
    for (kefir_asmcmp_virtual_register_index_t current = root;
         !kefir_hashtreeset_has(visited, (kefir_hashtreeset_entry_t) current);) {
        const struct kefir_asmcmp_amd64_register_preallocation *preallocation;
        REQUIRE_OK(kefir_asmcmp_amd64_get_register_preallocation(&func->code, current, &preallocation));
        if (preallocation == NULL) {
            break;
        } else if (preallocation->type == KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_SAME_AS) {
            REQUIRE_OK(kefir_hashtreeset_add(mem, visited, (kefir_hashtreeset_entry_t) current));
            current = preallocation->vreg;
        } else if (preallocation->type == KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_REQUIREMENT ||
                   preallocation->type == KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_HINT) {
            *phreg = preallocation->reg;
            return KEFIR_OK;
        }
    }

    return KEFIR_NO_MATCH;
}

static kefir_result_t propagate_virtual_register_hints(struct kefir_mem *mem,
                                                       struct kefir_codegen_amd64_function *func) {
    kefir_size_t num_of_vregs;
    REQUIRE_OK(kefir_asmcmp_number_of_virtual_registers(&func->code.context, &num_of_vregs));

    for (kefir_asmcmp_virtual_register_index_t vreg_idx = 0; vreg_idx < num_of_vregs; vreg_idx++) {
        const struct kefir_asmcmp_amd64_register_preallocation *preallocation;
        REQUIRE_OK(kefir_asmcmp_amd64_get_register_preallocation(&func->code, vreg_idx, &preallocation));

        if (preallocation != NULL && preallocation->type == KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_SAME_AS) {
            struct kefir_hashtreeset visited;
            REQUIRE_OK(kefir_hashtreeset_init(&visited, &kefir_hashtree_uint_ops));
            kefir_asm_amd64_xasmgen_register_t phreg;
            kefir_result_t res = trace_preallocation_hints(mem, func, preallocation->vreg, &visited, &phreg);
            if (res != KEFIR_NO_MATCH) {
                REQUIRE_ELSE(res == KEFIR_OK, {
                    kefir_hashtreeset_free(mem, &visited);
                    return res;
                });
                REQUIRE_OK(kefir_hashtreeset_free(mem, &visited));
                REQUIRE_OK(kefir_asmcmp_amd64_register_allocation_hint(mem, &func->code, vreg_idx, phreg));
            } else {
                REQUIRE_OK(kefir_hashtreeset_free(mem, &visited));
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t get_local_variable_type_layout(struct kefir_mem *, struct kefir_codegen_amd64_function *,
                                                     kefir_id_t, const struct kefir_abi_amd64_type_layout **);

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
    REQUIRE_OK(get_local_variable_type_layout(param->mem, param->func, type_id, &type_layout));
    REQUIRE_OK(kefir_abi_amd64_type_layout_at(type_layout, type_index, &typeentry_layout));

    ASSIGN_PTR(size_ptr, typeentry_layout->size);
    ASSIGN_PTR(alignment_ptr, typeentry_layout->alignment);
    return KEFIR_OK;
}

static kefir_result_t kefir_codegen_amd64_function_translate_impl(struct kefir_mem *mem,
                                                                  struct kefir_codegen_amd64 *codegen,
                                                                  struct kefir_codegen_amd64_function *func) {
    const struct kefir_ir_identifier *ir_identifier;
    REQUIRE_OK(kefir_ir_module_get_identifier(func->module->ir_module, func->function->ir_func->name, &ir_identifier));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&func->codegen->xasmgen, "%s", ir_identifier->symbol));

    const struct kefir_source_location *function_source_location =
        kefir_ir_function_debug_info_source_location(&func->function->ir_func->debug_info);
    if (function_source_location != NULL && codegen->debug_info_tracker != NULL) {
        REQUIRE_OK(KEFIR_AMD64_XASMGEN_DEBUG_INFO_SOURCE_LOCATION(mem, &codegen->xasmgen, codegen->debug_info_tracker,
                                                                  function_source_location));
    }

    if (!codegen->config->omit_frame_pointer) {
        REQUIRE_OK(kefir_codegen_amd64_stack_frame_require_frame_pointer(&func->stack_frame));
    }
    REQUIRE_OK(kefir_opt_code_analyze(mem, &func->function->code, &func->function_analysis));
    REQUIRE_OK(translate_code(mem, func));
    REQUIRE_OK(kefir_codegen_local_variable_allocator_run(
        mem, &func->variable_allocator, &func->function->code,
        &(struct kefir_codegen_local_variable_allocator_hooks) {
            .type_layout = variable_allocator_type_layout,
            .payload = &(struct variable_allocator_type_layout_param) {.mem = mem, .func = func}},
        &func->function_analysis.variable_conflicts));
    REQUIRE_OK(kefir_opt_code_analysis_clear(mem, &func->function_analysis));
    REQUIRE_OK(propagate_virtual_register_hints(mem, func));
    REQUIRE_OK(
        kefir_asmcmp_pipeline_apply(mem, &codegen->pipeline, KEFIR_ASMCMP_PIPELINE_PASS_VIRTUAL, &func->code.context));

    if (codegen->config->print_details != NULL && strcmp(codegen->config->print_details, "vasm") == 0) {
        REQUIRE_OK(output_asm(codegen, func, false, codegen->config->debug_info));
    }

    REQUIRE_OK(kefir_codegen_amd64_xregalloc_run(mem, &func->code, &func->stack_frame, &func->xregalloc));
    if (codegen->config->print_details != NULL && strcmp(codegen->config->print_details, "vasm+regs") == 0) {
        REQUIRE_OK(output_asm(codegen, func, true, codegen->config->debug_info));
    }

    REQUIRE_OK(kefir_codegen_amd64_devirtualize(mem, &func->code, &func->xregalloc, &func->stack_frame));
    REQUIRE_OK(kefir_asmcmp_pipeline_apply(mem, &codegen->pipeline, KEFIR_ASMCMP_PIPELINE_PASS_DEVIRTUAL,
                                           &func->code.context));

    if (codegen->config->print_details != NULL && strcmp(codegen->config->print_details, "devasm") == 0) {
        REQUIRE_OK(output_asm(codegen, func, false, codegen->config->debug_info));
    }

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_calculate(codegen->abi_variant, &func->stack_frame));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&func->codegen->xasmgen, KEFIR_AMD64_FUNCTION_BEGIN, ir_identifier->symbol));
    REQUIRE_OK(kefir_asmcmp_amd64_generate_code(mem, &codegen->xasmgen, codegen->debug_info_tracker, &func->code,
                                                &func->stack_frame));
    REQUIRE_OK(KEFIR_AMD64_XASMGEN_LABEL(&func->codegen->xasmgen, KEFIR_AMD64_FUNCTION_END, ir_identifier->symbol));
    REQUIRE_OK(generate_constants(mem, func));
    return KEFIR_OK;
}

static kefir_result_t free_type_layout(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                       kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_abi_amd64_type_layout *, type_layout, value);
    REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 type layout"));

    REQUIRE_OK(kefir_abi_amd64_type_layout_free(mem, type_layout));
    KEFIR_FREE(mem, type_layout);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_init(struct kefir_mem *mem, struct kefir_codegen_amd64_function *func,
                                                 struct kefir_codegen_amd64_module *codegen_module,
                                                 const struct kefir_opt_module *module,
                                                 const struct kefir_opt_function *function) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AMD64 codegen function"));
    REQUIRE(codegen_module != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 code generator module"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    *func = (struct kefir_codegen_amd64_function) {.codegen = codegen_module->codegen,
                                                   .codegen_module = codegen_module,
                                                   .module = module,
                                                   .function = function,
                                                   .argument_touch_instr = KEFIR_ASMCMP_INDEX_NONE,
                                                   .prologue_tail = KEFIR_ASMCMP_INDEX_NONE,
                                                   .return_address_vreg = KEFIR_ASMCMP_INDEX_NONE,
                                                   .dynamic_scope_vreg = KEFIR_ASMCMP_INDEX_NONE,
                                                   .vararg_area = KEFIR_ASMCMP_INDEX_NONE};

    const struct kefir_ir_identifier *ir_identifier;
    REQUIRE_OK(kefir_ir_module_get_identifier(module->ir_module, function->ir_func->name, &ir_identifier));

    REQUIRE_OK(kefir_asmcmp_amd64_init(ir_identifier->symbol, codegen_module->codegen->abi_variant,
                                       codegen_module->codegen->config->position_independent_code, &func->code));
    REQUIRE_OK(kefir_hashtree_init(&func->labels, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&func->virtual_registers, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&func->constants, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&func->type_layouts, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&func->type_layouts, free_type_layout, NULL));
    REQUIRE_OK(kefir_hashtreeset_init(&func->translated_instructions, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&func->preserve_vregs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&func->debug.opt_instruction_location_labels, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&func->debug.ir_instructions, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&func->debug.function_parameters, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&func->debug.occupied_x87_stack_slots, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_list_init(&func->x87_stack));
    REQUIRE_OK(kefir_opt_code_analysis_init(&func->function_analysis));
    REQUIRE_OK(kefir_opt_code_schedule_init(&func->schedule));
    REQUIRE_OK(kefir_codegen_local_variable_allocator_init(&func->variable_allocator));
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_init(&func->stack_frame, &func->variable_allocator));
    REQUIRE_OK(kefir_codegen_amd64_xregalloc_init(&func->xregalloc));
    REQUIRE_OK(kefir_abi_amd64_function_decl_alloc(mem, codegen_module->codegen->abi_variant,
                                                   function->ir_func->declaration, &func->abi_function_declaration));

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_free(struct kefir_mem *mem, struct kefir_codegen_amd64_function *func) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen function"));

    REQUIRE_OK(kefir_abi_amd64_function_decl_free(mem, &func->abi_function_declaration));
    REQUIRE_OK(kefir_codegen_local_variable_allocator_free(mem, &func->variable_allocator));
    REQUIRE_OK(kefir_codegen_amd64_xregalloc_free(mem, &func->xregalloc));
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_free(mem, &func->stack_frame));
    REQUIRE_OK(kefir_opt_code_schedule_free(mem, &func->schedule));
    REQUIRE_OK(kefir_opt_code_analysis_free(mem, &func->function_analysis));
    REQUIRE_OK(kefir_list_free(mem, &func->x87_stack));
    REQUIRE_OK(kefir_hashtree_free(mem, &func->debug.function_parameters));
    REQUIRE_OK(kefir_hashtree_free(mem, &func->debug.ir_instructions));
    REQUIRE_OK(kefir_hashtree_free(mem, &func->debug.opt_instruction_location_labels));
    REQUIRE_OK(kefir_hashtree_free(mem, &func->debug.occupied_x87_stack_slots));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &func->preserve_vregs));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &func->translated_instructions));
    REQUIRE_OK(kefir_hashtree_free(mem, &func->type_layouts));
    REQUIRE_OK(kefir_hashtree_free(mem, &func->constants));
    REQUIRE_OK(kefir_hashtree_free(mem, &func->virtual_registers));
    REQUIRE_OK(kefir_hashtree_free(mem, &func->labels));
    REQUIRE_OK(kefir_asmcmp_amd64_free(mem, &func->code));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_translate(struct kefir_mem *mem,
                                                      struct kefir_codegen_amd64_function *func) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen function"));

    REQUIRE_OK(kefir_codegen_amd64_function_translate_impl(mem, func->codegen, func));
    return KEFIR_OK;
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

kefir_result_t kefir_codegen_amd64_function_generate_debug_instruction_locations(
    struct kefir_mem *mem, struct kefir_codegen_amd64_function *function, kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen amd64 function"));
    REQUIRE(function->codegen->config->debug_info, KEFIR_OK);

    kefir_size_t x87_stack_index = 0;
    for (const struct kefir_list_entry *iter = kefir_list_head(&function->x87_stack); iter != NULL;
         kefir_list_next(&iter), x87_stack_index++) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, stack_instr_ref, (kefir_uptr_t) iter->value);
        const kefir_hashtree_key_t key = (((kefir_uint64_t) stack_instr_ref) << 32) | ((kefir_uint32_t) instr_ref);
        REQUIRE_OK(kefir_hashtree_insert(mem, &function->debug.occupied_x87_stack_slots, key,
                                         (kefir_hashtree_value_t) x87_stack_index));
    }

    kefir_size_t instruction_location;
    REQUIRE_OK(kefir_opt_code_debug_info_instruction_location(&function->function->debug_info, instr_ref,
                                                              &instruction_location));

    if (instruction_location != KEFIR_OPT_CODE_DEBUG_INSTRUCTION_LOCATION_NONE &&
        !kefir_hashtree_has(&function->debug.ir_instructions, (kefir_hashtree_key_t) instruction_location)) {
        REQUIRE_OK(kefir_hashtree_insert(mem, &function->debug.ir_instructions,
                                         (kefir_hashtree_key_t) instruction_location,
                                         (kefir_hashtree_value_t) instr_ref));
    }

    const struct kefir_opt_code_instruction_schedule *instruction_schedule;
    REQUIRE_OK(kefir_opt_code_schedule_of(&function->schedule, instr_ref, &instruction_schedule));
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_lower_bound(&function->debug.opt_instruction_location_labels,
                                                    (kefir_hashtree_key_t) instruction_schedule->linear_index, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(kefir_asmcmp_label_index_t, prev_asmlabel, node->value);

        const struct kefir_asmcmp_label *label;
        REQUIRE_OK(kefir_asmcmp_context_get_label(&function->code.context, prev_asmlabel, &label));
        if (label->attached && label->position == KEFIR_ASMCMP_INDEX_NONE) {
            REQUIRE_OK(kefir_hashtree_insert(mem, &function->debug.opt_instruction_location_labels,
                                             (kefir_hashtree_key_t) instruction_schedule->linear_index,
                                             (kefir_hashtree_value_t) prev_asmlabel));
            return KEFIR_OK;
        }
    }

    kefir_asmcmp_label_index_t asmlabel;
    REQUIRE_OK(kefir_asmcmp_context_new_label(mem, &function->code.context, KEFIR_ASMCMP_INDEX_NONE, &asmlabel));
    REQUIRE_OK(kefir_asmcmp_context_bind_label_after_tail(mem, &function->code.context, asmlabel));
    REQUIRE_OK(kefir_asmcmp_context_label_mark_external_dependencies(mem, &function->code.context, asmlabel));
    REQUIRE_OK(kefir_hashtree_insert(mem, &function->debug.opt_instruction_location_labels,
                                     (kefir_hashtree_key_t) instruction_schedule->linear_index,
                                     (kefir_hashtree_value_t) asmlabel));

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_find_instruction_lifetime(
    const struct kefir_codegen_amd64_function *codegen_function, kefir_opt_instruction_ref_t instr_ref,
    kefir_asmcmp_label_index_t *begin_label, kefir_asmcmp_label_index_t *end_label) {
    REQUIRE(codegen_function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen module"));
    REQUIRE(begin_label != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp label"));
    REQUIRE(end_label != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp label"));

    *begin_label = KEFIR_ASMCMP_INDEX_NONE;
    *end_label = *begin_label;

    struct kefir_hashtree_node *node;
    const struct kefir_opt_code_instruction_schedule *instruction_schedule;
    kefir_result_t res = kefir_opt_code_schedule_of(&codegen_function->schedule, instr_ref, &instruction_schedule);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);

    res = kefir_hashtree_at(&codegen_function->debug.opt_instruction_location_labels,
                            (kefir_hashtree_key_t) instruction_schedule->liveness_range.begin_index, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *begin_label = (kefir_asmcmp_label_index_t) node->value;
    } else {
        res =
            kefir_hashtree_upper_bound(&codegen_function->debug.opt_instruction_location_labels,
                                       (kefir_hashtree_key_t) instruction_schedule->liveness_range.begin_index, &node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            if (node != NULL && (kefir_size_t) node->key >= instruction_schedule->liveness_range.begin_index &&
                (kefir_size_t) node->key < instruction_schedule->liveness_range.end_index) {
                *begin_label = (kefir_asmcmp_label_index_t) node->value;
            }
        }
    }

    res = kefir_hashtree_at(&codegen_function->debug.opt_instruction_location_labels,
                            (kefir_hashtree_key_t) instruction_schedule->liveness_range.end_index, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *end_label = (kefir_asmcmp_label_index_t) node->value;
    } else {
        res = kefir_hashtree_lower_bound(&codegen_function->debug.opt_instruction_location_labels,
                                         (kefir_hashtree_key_t) instruction_schedule->liveness_range.end_index, &node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            if ((kefir_size_t) node->key >= instruction_schedule->liveness_range.begin_index &&
                (kefir_size_t) node->key < instruction_schedule->liveness_range.end_index) {
                *end_label = (kefir_asmcmp_label_index_t) node->value;
            }
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_find_code_range_labels(
    const struct kefir_codegen_amd64_function *codegen_function, kefir_size_t begin_index, kefir_size_t end_index,
    kefir_asmcmp_label_index_t *begin_label, kefir_asmcmp_label_index_t *end_label) {
    REQUIRE(codegen_function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen module"));
    REQUIRE(begin_label != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp label"));
    REQUIRE(end_label != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp label"));

    kefir_asmcmp_label_index_t tmp_label, tmp2_label;
    kefir_size_t begin_linear_position = 0, end_linear_position = 0;
    *begin_label = KEFIR_ASMCMP_INDEX_NONE;
    *end_label = KEFIR_ASMCMP_INDEX_NONE;

    for (kefir_size_t i = begin_index; i <= end_index; i++) {
        struct kefir_hashtree_node *node;
        kefir_result_t res =
            kefir_hashtree_at(&codegen_function->debug.ir_instructions, (kefir_hashtree_key_t) i, &node);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, node->value);
        REQUIRE_OK(kefir_codegen_amd64_function_find_instruction_lifetime(codegen_function, instr_ref, &tmp_label,
                                                                          &tmp2_label));
        const struct kefir_opt_code_instruction_schedule *instr_schedule;
        REQUIRE_OK(kefir_opt_code_schedule_of(&codegen_function->schedule, instr_ref, &instr_schedule));

        if (*begin_label == KEFIR_ASMCMP_INDEX_NONE ||
            instr_schedule->liveness_range.begin_index < begin_linear_position) {
            *begin_label = tmp_label;
            begin_linear_position = instr_schedule->linear_index;
        }

        if (*end_label == KEFIR_ASMCMP_INDEX_NONE || instr_schedule->liveness_range.end_index > end_linear_position) {
            *end_label = tmp2_label;
            end_linear_position = instr_schedule->linear_index;
        }
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_find_instruction_linear_index_label(
    const struct kefir_codegen_amd64_function *function, kefir_size_t linear_index,
    kefir_asmcmp_label_index_t *label_ptr) {
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AMD64 codegen function"));
    REQUIRE(label_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to asmcmp label"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&function->debug.opt_instruction_location_labels, linear_index, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find label for requested schedule linear index");
    }
    REQUIRE_OK(res);

    *label_ptr = (kefir_asmcmp_label_index_t) node->value;
    return KEFIR_OK;
}

static kefir_result_t get_local_variable_type_layout(struct kefir_mem *mem,
                                                     struct kefir_codegen_amd64_function *function, kefir_id_t type_id,
                                                     const struct kefir_abi_amd64_type_layout **type_layout_ptr) {
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&function->type_layouts, (kefir_hashtree_key_t) type_id, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *type_layout_ptr = (const struct kefir_abi_amd64_type_layout *) node->value;
    } else {
        const struct kefir_ir_type *ir_type = kefir_ir_module_get_named_type(function->module->ir_module, type_id);
        REQUIRE(ir_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type"));
        struct kefir_abi_amd64_type_layout *type_layout = KEFIR_MALLOC(mem, sizeof(struct kefir_abi_amd64_type_layout));
        REQUIRE(type_layout != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate amd64 type"));
        kefir_result_t res = kefir_abi_amd64_type_layout(
            mem, function->codegen->abi_variant, KEFIR_ABI_AMD64_TYPE_LAYOUT_CONTEXT_STACK, ir_type, type_layout);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, type_layout);
            return res;
        });
        res = kefir_hashtree_insert(mem, &function->type_layouts, (kefir_hashtree_key_t) type_id,
                                    (kefir_hashtree_value_t) type_layout);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_abi_amd64_type_layout_free(mem, type_layout);
            KEFIR_FREE(mem, type_layout);
            return res;
        });
        *type_layout_ptr = type_layout;
    }
    return KEFIR_OK;
}

#define X87_STACK_CAPACITY 8

kefir_result_t kefir_codegen_amd64_function_x87_ensure(struct kefir_mem *mem,
                                                       struct kefir_codegen_amd64_function *function,
                                                       kefir_size_t capacity) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 code generator function"));
    REQUIRE(capacity < X87_STACK_CAPACITY,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Requested capacity exceeds x87 stack size"));
    REQUIRE(kefir_list_length(&function->x87_stack) + capacity > X87_STACK_CAPACITY, KEFIR_OK);

    for (; kefir_list_length(&function->x87_stack) + capacity > X87_STACK_CAPACITY;) {
        struct kefir_list_entry *iter = kefir_list_tail(&function->x87_stack);
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, (kefir_uptr_t) iter->value);
        kefir_asmcmp_virtual_register_index_t vreg;
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instr_ref, &vreg));

        REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));

        if (function->codegen->config->valgrind_compatible_x87) {
            for (kefir_size_t i = 1; i < kefir_list_length(&function->x87_stack); i++) {
                REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code,
                                                   kefir_asmcmp_context_instr_tail(&function->code.context),
                                                   &KEFIR_ASMCMP_MAKE_X87(i), NULL));
            }
        } else {
            REQUIRE_OK(kefir_asmcmp_amd64_fdecstp(mem, &function->code,
                                                  kefir_asmcmp_context_instr_tail(&function->code.context), NULL));
        }
        REQUIRE_OK(kefir_asmcmp_amd64_fstp(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

        REQUIRE_OK(kefir_list_pop(mem, &function->x87_stack, iter));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_x87_push(struct kefir_mem *mem,
                                                     struct kefir_codegen_amd64_function *function,
                                                     kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 code generator function"));

    REQUIRE_OK(kefir_codegen_amd64_function_x87_ensure(mem, function, 1));
    REQUIRE_OK(kefir_list_insert_after(mem, &function->x87_stack, NULL, (void *) (kefir_uptr_t) instr_ref));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_x87_pop(struct kefir_mem *mem,
                                                    struct kefir_codegen_amd64_function *function) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 code generator function"));
    REQUIRE(kefir_list_length(&function->x87_stack) > 0,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected non-empty amd64 code generator function x87 stack"));

    REQUIRE_OK(kefir_list_pop(mem, &function->x87_stack, kefir_list_head(&function->x87_stack)));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_x87_load(struct kefir_mem *mem,
                                                     struct kefir_codegen_amd64_function *function,
                                                     kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 code generator function"));

    kefir_size_t stack_index = 0;
    for (struct kefir_list_entry *iter = kefir_list_head(&function->x87_stack); iter != NULL;
         iter = iter->next, stack_index++) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, stack_instr_ref, (kefir_uptr_t) iter->value);

        if (stack_instr_ref == instr_ref) {
            // Move to the top
            for (kefir_size_t i = 1; i <= stack_index; i++) {
                REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));
                REQUIRE_OK(kefir_asmcmp_amd64_fxch(mem, &function->code,
                                                   kefir_asmcmp_context_instr_tail(&function->code.context),
                                                   &KEFIR_ASMCMP_MAKE_X87(i), NULL));
            }

            REQUIRE_OK(kefir_list_pop(mem, &function->x87_stack, iter));
            REQUIRE_OK(kefir_list_insert_after(mem, &function->x87_stack, NULL, (void *) (kefir_uptr_t) instr_ref));
            return KEFIR_OK;
        }
    }

    REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));
    REQUIRE_OK(kefir_codegen_amd64_function_x87_ensure(mem, function, 1));
    kefir_asmcmp_virtual_register_index_t vreg;
    REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instr_ref, &vreg));
    REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                                      &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
                                      NULL));
    REQUIRE_OK(kefir_list_insert_after(mem, &function->x87_stack, NULL, (void *) (kefir_uptr_t) instr_ref));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_x87_consume_by(struct kefir_mem *mem,
                                                           struct kefir_codegen_amd64_function *function,
                                                           kefir_opt_instruction_ref_t instr_ref,
                                                           kefir_opt_instruction_ref_t consumer_instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 code generator function"));

    kefir_size_t stack_index = 0;
    for (struct kefir_list_entry *iter = kefir_list_head(&function->x87_stack); iter != NULL;
         iter = iter->next, stack_index++) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, stack_instr_ref, (kefir_uptr_t) iter->value);

        if (stack_instr_ref == instr_ref) {
            kefir_opt_instruction_ref_t sole_use_ref;
            REQUIRE_OK(kefir_opt_instruction_get_sole_use(&function->function->code, instr_ref, &sole_use_ref));
            if (consumer_instr_ref != sole_use_ref) {
                REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));
                kefir_asmcmp_virtual_register_index_t vreg;
                REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, instr_ref, &vreg));

                if (kefir_list_length(&function->x87_stack) < X87_STACK_CAPACITY) {
                    REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, &function->code,
                                                      kefir_asmcmp_context_instr_tail(&function->code.context),
                                                      &KEFIR_ASMCMP_MAKE_X87(stack_index), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
                } else {
                    kefir_asmcmp_virtual_register_index_t tmp_vreg;
                    REQUIRE_OK(kefir_asmcmp_virtual_register_new_spill_space(
                        mem, &function->code.context,
                        kefir_abi_amd64_long_double_qword_size(function->codegen->abi_variant),
                        kefir_abi_amd64_long_double_qword_alignment(function->codegen->abi_variant), &tmp_vreg));

                    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
                    if (stack_index != 0) {
                        REQUIRE_OK(kefir_asmcmp_amd64_fld(mem, &function->code,
                                                          kefir_asmcmp_context_instr_tail(&function->code.context),
                                                          &KEFIR_ASMCMP_MAKE_X87(stack_index - 1), NULL));
                    } else {
                        REQUIRE_OK(kefir_asmcmp_amd64_fld(
                            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT),
                            NULL));
                    }
                    REQUIRE_OK(kefir_asmcmp_amd64_fstp(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
                    REQUIRE_OK(kefir_asmcmp_amd64_fld(
                        mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
                        &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(tmp_vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));
                }
            }

            return KEFIR_OK;
        }
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_x87_flush(struct kefir_mem *mem,
                                                      struct kefir_codegen_amd64_function *function) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 code generator function"));

    for (struct kefir_list_entry *iter = kefir_list_head(&function->x87_stack); iter != NULL;
         iter = kefir_list_head(&function->x87_stack)) {
        REQUIRE_OK(kefir_codegen_amd64_stack_frame_preserve_x87_control_word(&function->stack_frame));
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, stack_instr_ref, (kefir_uptr_t) iter->value);

        kefir_asmcmp_virtual_register_index_t vreg;
        REQUIRE_OK(kefir_codegen_amd64_function_vreg_of(function, stack_instr_ref, &vreg));
        REQUIRE_OK(kefir_asmcmp_amd64_fstp(
            mem, &function->code, kefir_asmcmp_context_instr_tail(&function->code.context),
            &KEFIR_ASMCMP_MAKE_INDIRECT_VIRTUAL(vreg, 0, KEFIR_ASMCMP_OPERAND_VARIANT_80BIT), NULL));

        REQUIRE_OK(kefir_list_pop(mem, &function->x87_stack, iter));
    }

    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_x87_locations_iter(
    const struct kefir_codegen_amd64_function *function, kefir_opt_instruction_ref_t instr_ref,
    struct kefir_codegen_amd64_function_x87_locations_iterator *iter, kefir_opt_instruction_ref_t *location_instr_ref,
    kefir_size_t *x87_stack_index) {
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 code generator function"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                          "Expected valid amd64 code generator function x87 locations iterator"));

    kefir_result_t res =
        kefir_hashtree_lower_bound(&function->debug.occupied_x87_stack_slots,
                                   (kefir_hashtree_key_t) (((kefir_uint64_t) instr_ref) << 32), &iter->node);
    if (res == KEFIR_NOT_FOUND) {
        REQUIRE_OK(kefir_hashtree_min(&function->debug.occupied_x87_stack_slots, &iter->node));
    } else {
        REQUIRE_OK(res);
    }

    for (; iter->node != NULL;
         iter->node = kefir_hashtree_next_node(&function->debug.occupied_x87_stack_slots, iter->node)) {
        const kefir_opt_instruction_ref_t stack_instr_ref = ((kefir_uint64_t) iter->node->key) >> 32;
        if (stack_instr_ref == instr_ref) {
            break;
        } else if (stack_instr_ref > instr_ref) {
            iter->node = NULL;
        }
    }

    REQUIRE(iter->node != NULL, KEFIR_ITERATOR_END);
    iter->x87_slots = &function->debug.occupied_x87_stack_slots;
    iter->instr_ref = instr_ref;
    ASSIGN_PTR(location_instr_ref, ((kefir_uint64_t) iter->node->key) & ((1ull << 32) - 1));
    ASSIGN_PTR(x87_stack_index, iter->node->value);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_function_x87_locations_next(
    struct kefir_codegen_amd64_function_x87_locations_iterator *iter, kefir_opt_instruction_ref_t *location_instr_ref,
    kefir_size_t *x87_stack_index) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                          "Expected valid amd64 code generator function x87 locations iterator"));

    iter->node = kefir_hashtree_next_node(iter->x87_slots, iter->node);
    if (iter->node != NULL) {
        const kefir_opt_instruction_ref_t stack_instr_ref = ((kefir_uint64_t) iter->node->key) >> 32;
        if (stack_instr_ref > iter->instr_ref) {
            iter->node = NULL;
        }
    }

    REQUIRE(iter->node != NULL, KEFIR_ITERATOR_END);
    ASSIGN_PTR(location_instr_ref, ((kefir_uint64_t) iter->node->key) & ((1ull << 32) - 1));
    ASSIGN_PTR(x87_stack_index, iter->node->value);
    return KEFIR_OK;
}
