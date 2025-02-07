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

#define KEFIR_OPTIMIZER_ANALYSIS_INTERNAL
#include "kefir/optimizer/analysis.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_opt_code_analyze(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                      struct kefir_opt_code_analysis *analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(analysis != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code analysis"));

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(code, &num_of_blocks));

    REQUIRE_OK(kefir_hashtreeset_init(&analysis->indirect_jump_target_blocks, &kefir_hashtree_uint_ops));

    analysis->blocks = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_analysis_block_properties) * num_of_blocks);
    REQUIRE(analysis->blocks != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer analysis code block"));
    for (kefir_size_t i = 0; i < num_of_blocks; i++) {
        analysis->blocks[i] =
            (struct kefir_opt_code_analysis_block_properties) {.block_id = i, .immediate_dominator = KEFIR_ID_NONE};
        kefir_result_t res = kefir_list_init(&analysis->blocks[i].successors);
        REQUIRE_CHAIN(&res, kefir_list_init(&analysis->blocks[i].predecessors));
        REQUIRE_ELSE(res == KEFIR_OK, {
            for (kefir_size_t j = 0; j < i; j++) {
                kefir_list_free(mem, &analysis->blocks[j].successors);
                kefir_list_free(mem, &analysis->blocks[j].predecessors);
            }
            KEFIR_FREE(mem, analysis->blocks);
            kefir_hashtreeset_free(mem, &analysis->indirect_jump_target_blocks);
            analysis->blocks = NULL;
            return res;
        });
    }

    analysis->code = code;

    kefir_result_t res = kefir_opt_code_container_link_blocks(mem, analysis);
    REQUIRE_CHAIN(&res, kefir_opt_code_container_find_dominators(mem, analysis));
    REQUIRE_ELSE(res == KEFIR_OK, {
        for (kefir_size_t i = 0; i < num_of_blocks; i++) {
            kefir_list_free(mem, &analysis->blocks[i].successors);
            kefir_list_free(mem, &analysis->blocks[i].predecessors);
        }
        KEFIR_FREE(mem, analysis->blocks);
        kefir_hashtreeset_free(mem, &analysis->indirect_jump_target_blocks);
        memset(analysis, 0, sizeof(struct kefir_opt_code_analysis));
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_analysis_free(struct kefir_mem *mem, struct kefir_opt_code_analysis *analysis) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(analysis->code, &num_of_blocks));
    for (kefir_size_t i = 0; i < num_of_blocks; i++) {
        REQUIRE_OK(kefir_list_free(mem, &analysis->blocks[i].successors));
        REQUIRE_OK(kefir_list_free(mem, &analysis->blocks[i].predecessors));
    }

    REQUIRE_OK(kefir_hashtreeset_free(mem, &analysis->indirect_jump_target_blocks));

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
