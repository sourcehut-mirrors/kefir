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

#define KEFIR_OPTIMIZER_ANALYSIS_INTERNAL
#include "kefir/optimizer/analysis.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t block_schedule_dfs_impl(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                              kefir_result_t (*callback)(kefir_opt_block_id_t, void *), void *payload,
                                              struct kefir_list *work_queue, kefir_bool_t *marks) {
    REQUIRE_OK(kefir_list_insert_after(mem, work_queue, kefir_list_tail(work_queue),
                                       (void *) (kefir_uptr_t) code->entry_point));

    kefir_result_t res = KEFIR_OK;
    for (struct kefir_list_entry *head_entry = kefir_list_head(work_queue); res == KEFIR_OK && head_entry != NULL;
         res = kefir_list_pop(mem, work_queue, head_entry), head_entry = kefir_list_head(work_queue)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) head_entry->value);

        if (marks[block_id]) {
            continue;
        }
        REQUIRE_OK(callback(block_id, payload));
        marks[block_id] = true;

        struct kefir_opt_code_block *block = NULL;
        REQUIRE_OK(kefir_opt_code_container_block(code, block_id, &block));

        const struct kefir_opt_instruction *instr = NULL;
        REQUIRE_OK(kefir_opt_code_block_instr_control_tail(code, block, &instr));

        switch (instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_JUMP:
                REQUIRE_OK(kefir_list_insert_after(
                    mem, work_queue, NULL, (void *) (kefir_uptr_t) instr->operation.parameters.branch.target_block));
                break;

            case KEFIR_OPT_OPCODE_BRANCH:
            case KEFIR_OPT_OPCODE_COMPARE_BRANCH:
                REQUIRE_OK(kefir_list_insert_after(
                    mem, work_queue, NULL,
                    (void *) (kefir_uptr_t) instr->operation.parameters.branch.alternative_block));
                REQUIRE_OK(kefir_list_insert_after(
                    mem, work_queue, NULL, (void *) (kefir_uptr_t) instr->operation.parameters.branch.target_block));
                break;

            case KEFIR_OPT_OPCODE_IJUMP:
            case KEFIR_OPT_OPCODE_RETURN:
                // Intentionally left blank
                break;

            case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY: {
                struct kefir_opt_inline_assembly_node *inline_asm = NULL;
                REQUIRE_OK(kefir_opt_code_container_inline_assembly(code, instr->operation.parameters.inline_asm_ref,
                                                                    &inline_asm));
                if (!kefir_hashtree_empty(&inline_asm->jump_targets)) {
                    struct kefir_hashtree_node_iterator iter;
                    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->jump_targets, &iter);
                         node != NULL; node = kefir_hashtree_next(&iter)) {
                        ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block, node->value);
                        REQUIRE_OK(
                            kefir_list_insert_after(mem, work_queue, NULL, (void *) (kefir_uptr_t) target_block));
                    }

                    REQUIRE_OK(kefir_list_insert_after(mem, work_queue, NULL,
                                                       (void *) (kefir_uptr_t) inline_asm->default_jump_target));
                }
            } break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                       "Unexpected terminating instruction in optimizer block control flow");
        }
    }

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(code, &num_of_blocks));
    for (kefir_opt_block_id_t i = 0; i < num_of_blocks; i++) {
        if (!marks[i]) {
            REQUIRE_OK(callback(i, payload));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t block_schedule_dfs(struct kefir_mem *mem,
                                         const struct kefir_opt_code_analyze_block_scheduler *scheduler,
                                         const struct kefir_opt_code_container *code,
                                         kefir_result_t (*callback)(kefir_opt_block_id_t, void *), void *payload) {
    UNUSED(scheduler);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(callback != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer analysis block schedule callback"));

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(code, &num_of_blocks));

    struct kefir_list work_queue;
    REQUIRE_OK(kefir_list_init(&work_queue));

    kefir_bool_t *scheduled_marks = KEFIR_MALLOC(mem, sizeof(kefir_bool_t) * num_of_blocks);
    REQUIRE(scheduled_marks != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer DFS scheduler data"));
    memset(scheduled_marks, 0, sizeof(kefir_bool_t) * num_of_blocks);

    kefir_result_t res = block_schedule_dfs_impl(mem, code, callback, payload, &work_queue, scheduled_marks);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, scheduled_marks);
        kefir_list_free(mem, &work_queue);
        return res;
    });

    KEFIR_FREE(mem, scheduled_marks);
    REQUIRE_OK(kefir_list_free(mem, &work_queue));
    return KEFIR_OK;
}

const struct kefir_opt_code_analyze_block_scheduler kefir_opt_code_analyze_dfs_block_scheduler = {
    .schedule = block_schedule_dfs, .payload = NULL};

static kefir_result_t find_successors(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis) {
    struct kefir_opt_code_container_iterator iter;
    for (const struct kefir_opt_code_block *block = kefir_opt_code_container_iter(analysis->code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {
        struct kefir_list *successors = &analysis->blocks[block->id].successors;

        const struct kefir_opt_instruction *tail_instr = NULL;
        REQUIRE_OK(kefir_opt_code_block_instr_control_tail(analysis->code, block, &tail_instr));
        switch (tail_instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_JUMP:
                REQUIRE_OK(kefir_list_insert_after(
                    mem, successors, kefir_list_tail(successors),
                    (void *) (kefir_uptr_t) tail_instr->operation.parameters.branch.target_block));
                break;

            case KEFIR_OPT_OPCODE_BRANCH:
            case KEFIR_OPT_OPCODE_COMPARE_BRANCH:
                REQUIRE_OK(kefir_list_insert_after(
                    mem, successors, kefir_list_tail(successors),
                    (void *) (kefir_uptr_t) tail_instr->operation.parameters.branch.target_block));
                REQUIRE_OK(kefir_list_insert_after(
                    mem, successors, kefir_list_tail(successors),
                    (void *) (kefir_uptr_t) tail_instr->operation.parameters.branch.alternative_block));
                break;

            case KEFIR_OPT_OPCODE_IJUMP:
                for (const struct kefir_list_entry *indirect_iter =
                         kefir_list_head(&analysis->indirect_jump_target_blocks);
                     indirect_iter != NULL; kefir_list_next(&indirect_iter)) {
                    REQUIRE_OK(
                        kefir_list_insert_after(mem, successors, kefir_list_tail(successors), indirect_iter->value));
                }
                break;

            case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY: {
                struct kefir_opt_inline_assembly_node *inline_asm = NULL;
                REQUIRE_OK(kefir_opt_code_container_inline_assembly(
                    analysis->code, tail_instr->operation.parameters.inline_asm_ref, &inline_asm));
                if (!kefir_hashtree_empty(&inline_asm->jump_targets)) {
                    REQUIRE_OK(kefir_list_insert_after(mem, successors, kefir_list_tail(successors),
                                                       (void *) (kefir_uptr_t) inline_asm->default_jump_target));

                    struct kefir_hashtree_node_iterator iter;
                    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->jump_targets, &iter);
                         node != NULL; node = kefir_hashtree_next(&iter)) {
                        ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block, node->value);
                        REQUIRE_OK(kefir_list_insert_after(mem, successors, kefir_list_tail(successors),
                                                           (void *) (kefir_uptr_t) target_block));
                    }
                }
            } break;

            case KEFIR_OPT_OPCODE_RETURN:
                // Intentionally left blank
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                       "Unexpected terminating instruction of optimizer code block");
        }

        for (const struct kefir_list_entry *iter = kefir_list_head(successors); iter != NULL; kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block, (kefir_uptr_t) iter->value);

            REQUIRE_OK(kefir_list_insert_after(mem, &analysis->blocks[successor_block].predecessors,
                                               kefir_list_tail(&analysis->blocks[successor_block].predecessors),
                                               (void *) (kefir_uptr_t) block->id));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t analyze_code(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis) {
    REQUIRE_OK(kefir_opt_code_analyze_reachability(mem, analysis));
    REQUIRE_OK(find_successors(mem, analysis));
    REQUIRE_OK(kefir_opt_code_analyze_linearize(mem, analysis, &kefir_opt_code_analyze_dfs_block_scheduler));
    REQUIRE_OK(kefir_opt_code_liveness_intervals_build(mem, analysis, &analysis->liveness));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_analyze(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                      struct kefir_opt_code_analysis *analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(analysis != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code analysis"));

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(code, &num_of_blocks));

    REQUIRE_OK(kefir_list_init(&analysis->indirect_jump_target_blocks));

    analysis->blocks = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_analysis_block_properties) * num_of_blocks);
    REQUIRE(analysis->blocks != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer analysis code block"));
    for (kefir_size_t i = 0; i < num_of_blocks; i++) {
        analysis->blocks[i] = (struct kefir_opt_code_analysis_block_properties){
            .block_id = i,
            .reachable = false,
            .linear_position = ~(kefir_size_t) 0ull,
            .linear_range = {.begin_index = KEFIR_OPT_CODE_ANALYSIS_LINEAR_INDEX_UNDEFINED,
                             .end_index = KEFIR_OPT_CODE_ANALYSIS_LINEAR_INDEX_UNDEFINED}};
        kefir_result_t res = kefir_list_init(&analysis->blocks[i].successors);
        REQUIRE_CHAIN(&res, kefir_list_init(&analysis->blocks[i].predecessors));
        REQUIRE_ELSE(res == KEFIR_OK, {
            for (kefir_size_t j = 0; j < i; j++) {
                kefir_list_free(mem, &analysis->blocks[j].successors);
                kefir_list_free(mem, &analysis->blocks[j].predecessors);
            }
            KEFIR_FREE(mem, analysis->blocks);
            kefir_list_free(mem, &analysis->indirect_jump_target_blocks);
            analysis->blocks = NULL;
            return res;
        });
    }

    analysis->instructions = KEFIR_MALLOC(
        mem, sizeof(kefir_opt_code_analysis_instruction_properties_t) * kefir_opt_code_container_length(code));
    REQUIRE_ELSE(analysis->instructions != NULL, {
        for (kefir_size_t i = 0; i < num_of_blocks; i++) {
            kefir_list_free(mem, &analysis->blocks[i].successors);
            kefir_list_free(mem, &analysis->blocks[i].predecessors);
        }
        KEFIR_FREE(mem, analysis->blocks);
        analysis->blocks = NULL;
        kefir_list_free(mem, &analysis->indirect_jump_target_blocks);
        return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer analysis code block");
    });

    for (kefir_size_t i = 0; i < kefir_opt_code_container_length(code); i++) {
        analysis->instructions[i] = (struct kefir_opt_code_analysis_instruction_properties){
            .instr_ref = i, .reachable = false, .linear_position = KEFIR_OPT_CODE_ANALYSIS_LINEAR_INDEX_UNDEFINED};
    }

    analysis->code = code;
    analysis->block_linearization_length = 0;
    analysis->block_linearization = NULL;
    analysis->linearization_length = 0;
    analysis->linearization = NULL;

    kefir_result_t res = analyze_code(mem, analysis);
    REQUIRE_ELSE(res == KEFIR_OK, {
        for (kefir_size_t i = 0; i < num_of_blocks; i++) {
            REQUIRE_OK(kefir_list_free(mem, &analysis->blocks[i].successors));
            REQUIRE_OK(kefir_list_free(mem, &analysis->blocks[i].predecessors));
        }
        KEFIR_FREE(mem, analysis->linearization);
        KEFIR_FREE(mem, analysis->block_linearization);
        KEFIR_FREE(mem, analysis->instructions);
        KEFIR_FREE(mem, analysis->blocks);
        kefir_list_free(mem, &analysis->indirect_jump_target_blocks);
        memset(analysis, 0, sizeof(struct kefir_opt_code_analysis));
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_analysis_free(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));

    REQUIRE_OK(kefir_opt_code_liveness_intervals_free(mem, &analysis->liveness));

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(analysis->code, &num_of_blocks));
    for (kefir_size_t i = 0; i < num_of_blocks; i++) {
        REQUIRE_OK(kefir_list_free(mem, &analysis->blocks[i].successors));
        REQUIRE_OK(kefir_list_free(mem, &analysis->blocks[i].predecessors));
    }

    REQUIRE_OK(kefir_list_free(mem, &analysis->indirect_jump_target_blocks));

    KEFIR_FREE(mem, analysis->linearization);
    KEFIR_FREE(mem, analysis->block_linearization);
    KEFIR_FREE(mem, analysis->instructions);
    KEFIR_FREE(mem, analysis->blocks);
    memset(analysis, 0, sizeof(struct kefir_opt_code_analysis));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_analysis_block_properties(
    const struct kefir_opt_code_analysis *analysis, kefir_opt_block_id_t block_id,
    const struct kefir_opt_code_analysis_block_properties **props_ptr) {
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));
    REQUIRE(props_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                               "Expected valid pointer to optimizer code analysis block properties"));

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(analysis->code, &num_of_blocks));
    REQUIRE(block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested optimizer block analysis properties"));

    *props_ptr = &analysis->blocks[block_id];
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_analysis_instruction_properties(
    const struct kefir_opt_code_analysis *analysis, kefir_opt_instruction_ref_t instr_ref,
    const struct kefir_opt_code_analysis_instruction_properties **props_ptr) {
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));
    REQUIRE(props_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                               "Expected valid pointer to optimizer code analysis block properties"));
    REQUIRE(instr_ref < kefir_opt_code_container_length(analysis->code),
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested optimizer instruction analysis properties"));

    *props_ptr = &analysis->instructions[instr_ref];
    return KEFIR_OK;
}

static kefir_result_t free_code_analysis(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                         kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_analysis *, analysis, value);
    REQUIRE(analysis != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code analysis"));

    REQUIRE_OK(kefir_opt_code_analysis_free(mem, analysis));
    KEFIR_FREE(mem, analysis);
    return KEFIR_OK;
}

static kefir_result_t module_analyze_impl(struct kefir_mem *mem, struct kefir_opt_module_analysis *analysis) {
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_ir_function *func = kefir_ir_module_function_iter(analysis->module->ir_module, &iter);
         func != NULL; func = kefir_ir_module_function_next(&iter)) {

        struct kefir_opt_function *opt_func = NULL;
        REQUIRE_OK(kefir_opt_module_get_function(analysis->module, func->declaration->id, &opt_func));

        struct kefir_opt_code_analysis *code_analysis = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_analysis));
        REQUIRE(code_analysis != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer code analysis data"));

        kefir_result_t res = kefir_opt_code_analyze(mem, &opt_func->code, code_analysis);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, code_analysis);
            return res;
        });

        res = kefir_hashtree_insert(mem, &analysis->functions, (kefir_hashtree_key_t) func->declaration->id,
                                    (kefir_hashtree_value_t) code_analysis);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_opt_code_analysis_free(mem, code_analysis);
            KEFIR_FREE(mem, code_analysis);
            return res;
        });
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_module_analyze(struct kefir_mem *mem, struct kefir_opt_module *module,
                                        struct kefir_opt_module_analysis *analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(analysis != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer module analysis"));

    REQUIRE_OK(kefir_hashtree_init(&analysis->functions, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&analysis->functions, free_code_analysis, NULL));
    analysis->module = module;

    kefir_result_t res = module_analyze_impl(mem, analysis);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &analysis->functions);
        analysis->module = NULL;
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_opt_module_analysis_free(struct kefir_mem *mem, struct kefir_opt_module_analysis *analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module analysis"));

    REQUIRE_OK(kefir_hashtree_free(mem, &analysis->functions));
    analysis->module = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_module_analysis_get_function(const struct kefir_opt_module_analysis *analysis,
                                                      kefir_id_t func_id,
                                                      const struct kefir_opt_code_analysis **code_analysis_ptr) {
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module analysis"));
    REQUIRE(code_analysis_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code analysis"));

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&analysis->functions, (kefir_hashtree_key_t) func_id, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested optimizer code analysis");
    }
    REQUIRE_OK(res);

    *code_analysis_ptr = (const struct kefir_opt_code_analysis *) node->value;
    return KEFIR_OK;
}
