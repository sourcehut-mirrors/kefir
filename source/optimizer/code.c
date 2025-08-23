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

#include "kefir/optimizer/code.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_opt_comparison_operation_inverse(kefir_opt_comparison_operation_t original_comparison,
                                                      kefir_opt_comparison_operation_t *comparison_ptr) {
    kefir_opt_comparison_operation_t comparison;
    switch (original_comparison) {
        case KEFIR_OPT_COMPARISON_INT8_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT8_NOT_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT16_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT16_NOT_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT32_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT32_NOT_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT64_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT64_NOT_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT8_NOT_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT8_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT16_NOT_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT16_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT32_NOT_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT32_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT64_NOT_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT64_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT8_GREATER:
            comparison = KEFIR_OPT_COMPARISON_INT8_LESSER_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT16_GREATER:
            comparison = KEFIR_OPT_COMPARISON_INT16_LESSER_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT32_GREATER:
            comparison = KEFIR_OPT_COMPARISON_INT32_LESSER_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT64_GREATER:
            comparison = KEFIR_OPT_COMPARISON_INT64_LESSER_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT8_GREATER_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT8_LESSER;
            break;

        case KEFIR_OPT_COMPARISON_INT16_GREATER_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT16_LESSER;
            break;

        case KEFIR_OPT_COMPARISON_INT32_GREATER_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT32_LESSER;
            break;

        case KEFIR_OPT_COMPARISON_INT64_GREATER_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT64_LESSER;
            break;

        case KEFIR_OPT_COMPARISON_INT8_LESSER:
            comparison = KEFIR_OPT_COMPARISON_INT8_GREATER_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT16_LESSER:
            comparison = KEFIR_OPT_COMPARISON_INT16_GREATER_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT32_LESSER:
            comparison = KEFIR_OPT_COMPARISON_INT32_GREATER_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT64_LESSER:
            comparison = KEFIR_OPT_COMPARISON_INT64_GREATER_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT8_LESSER_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT8_GREATER;
            break;

        case KEFIR_OPT_COMPARISON_INT16_LESSER_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT16_GREATER;
            break;

        case KEFIR_OPT_COMPARISON_INT32_LESSER_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT32_GREATER;
            break;

        case KEFIR_OPT_COMPARISON_INT64_LESSER_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT64_GREATER;
            break;

        case KEFIR_OPT_COMPARISON_INT8_ABOVE:
            comparison = KEFIR_OPT_COMPARISON_INT8_BELOW_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT16_ABOVE:
            comparison = KEFIR_OPT_COMPARISON_INT16_BELOW_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT32_ABOVE:
            comparison = KEFIR_OPT_COMPARISON_INT32_BELOW_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT64_ABOVE:
            comparison = KEFIR_OPT_COMPARISON_INT64_BELOW_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT8_ABOVE_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT8_BELOW;
            break;

        case KEFIR_OPT_COMPARISON_INT16_ABOVE_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT16_BELOW;
            break;

        case KEFIR_OPT_COMPARISON_INT32_ABOVE_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT32_BELOW;
            break;

        case KEFIR_OPT_COMPARISON_INT64_ABOVE_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT64_BELOW;
            break;

        case KEFIR_OPT_COMPARISON_INT8_BELOW:
            comparison = KEFIR_OPT_COMPARISON_INT8_ABOVE_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT16_BELOW:
            comparison = KEFIR_OPT_COMPARISON_INT16_ABOVE_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT32_BELOW:
            comparison = KEFIR_OPT_COMPARISON_INT32_ABOVE_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT64_BELOW:
            comparison = KEFIR_OPT_COMPARISON_INT64_ABOVE_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT8_BELOW_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT8_ABOVE;
            break;

        case KEFIR_OPT_COMPARISON_INT16_BELOW_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT16_ABOVE;
            break;

        case KEFIR_OPT_COMPARISON_INT32_BELOW_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT32_ABOVE;
            break;

        case KEFIR_OPT_COMPARISON_INT64_BELOW_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT64_ABOVE;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_EQUAL:
            comparison = KEFIR_OPT_COMPARISON_FLOAT32_NOT_EQUAL;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_EQUAL:
            comparison = KEFIR_OPT_COMPARISON_FLOAT32_EQUAL;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_GREATER:
            comparison = KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_GREATER_OR_EQUAL:
            comparison = KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER_OR_EQUAL;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_LESSER:
            comparison = KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_LESSER_OR_EQUAL:
            comparison = KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER_OR_EQUAL;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER:
            comparison = KEFIR_OPT_COMPARISON_FLOAT32_GREATER;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER_OR_EQUAL:
            comparison = KEFIR_OPT_COMPARISON_FLOAT32_GREATER_OR_EQUAL;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER:
            comparison = KEFIR_OPT_COMPARISON_FLOAT32_LESSER;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER_OR_EQUAL:
            comparison = KEFIR_OPT_COMPARISON_FLOAT32_LESSER_OR_EQUAL;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_EQUAL:
            comparison = KEFIR_OPT_COMPARISON_FLOAT64_NOT_EQUAL;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_EQUAL:
            comparison = KEFIR_OPT_COMPARISON_FLOAT64_EQUAL;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_GREATER:
            comparison = KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_GREATER_OR_EQUAL:
            comparison = KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER_OR_EQUAL;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_LESSER:
            comparison = KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_LESSER_OR_EQUAL:
            comparison = KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER_OR_EQUAL;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER:
            comparison = KEFIR_OPT_COMPARISON_FLOAT64_GREATER;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER_OR_EQUAL:
            comparison = KEFIR_OPT_COMPARISON_FLOAT64_GREATER_OR_EQUAL;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER:
            comparison = KEFIR_OPT_COMPARISON_FLOAT64_LESSER;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER_OR_EQUAL:
            comparison = KEFIR_OPT_COMPARISON_FLOAT64_LESSER_OR_EQUAL;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected comparison operator");
    }
    *comparison_ptr = comparison;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_comparison_operation_reciprocal(kefir_opt_comparison_operation_t original_comparison,
                                                         kefir_opt_comparison_operation_t *comparison_ptr) {
    kefir_opt_comparison_operation_t comparison;
    switch (original_comparison) {
        case KEFIR_OPT_COMPARISON_INT8_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT8_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT16_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT16_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT32_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT32_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT64_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT64_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT8_NOT_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT8_NOT_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT16_NOT_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT16_NOT_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT32_NOT_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT32_NOT_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT64_NOT_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT64_NOT_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT8_GREATER:
            comparison = KEFIR_OPT_COMPARISON_INT8_LESSER;
            break;

        case KEFIR_OPT_COMPARISON_INT16_GREATER:
            comparison = KEFIR_OPT_COMPARISON_INT16_LESSER;
            break;

        case KEFIR_OPT_COMPARISON_INT32_GREATER:
            comparison = KEFIR_OPT_COMPARISON_INT32_LESSER;
            break;

        case KEFIR_OPT_COMPARISON_INT64_GREATER:
            comparison = KEFIR_OPT_COMPARISON_INT64_LESSER;
            break;

        case KEFIR_OPT_COMPARISON_INT8_GREATER_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT8_LESSER_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT16_GREATER_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT16_LESSER_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT32_GREATER_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT32_LESSER_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT64_GREATER_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT64_LESSER_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT8_LESSER:
            comparison = KEFIR_OPT_COMPARISON_INT8_GREATER;
            break;

        case KEFIR_OPT_COMPARISON_INT16_LESSER:
            comparison = KEFIR_OPT_COMPARISON_INT16_GREATER;
            break;

        case KEFIR_OPT_COMPARISON_INT32_LESSER:
            comparison = KEFIR_OPT_COMPARISON_INT32_GREATER;
            break;

        case KEFIR_OPT_COMPARISON_INT64_LESSER:
            comparison = KEFIR_OPT_COMPARISON_INT64_GREATER;
            break;

        case KEFIR_OPT_COMPARISON_INT8_LESSER_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT8_GREATER_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT16_LESSER_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT16_GREATER_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT32_LESSER_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT32_GREATER_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT64_LESSER_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT64_GREATER_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT8_ABOVE:
            comparison = KEFIR_OPT_COMPARISON_INT8_BELOW;
            break;

        case KEFIR_OPT_COMPARISON_INT16_ABOVE:
            comparison = KEFIR_OPT_COMPARISON_INT16_BELOW;
            break;

        case KEFIR_OPT_COMPARISON_INT32_ABOVE:
            comparison = KEFIR_OPT_COMPARISON_INT32_BELOW;
            break;

        case KEFIR_OPT_COMPARISON_INT64_ABOVE:
            comparison = KEFIR_OPT_COMPARISON_INT64_BELOW;
            break;

        case KEFIR_OPT_COMPARISON_INT8_ABOVE_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT8_BELOW_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT16_ABOVE_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT16_BELOW_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT32_ABOVE_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT32_BELOW_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT64_ABOVE_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT64_BELOW_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT8_BELOW:
            comparison = KEFIR_OPT_COMPARISON_INT8_ABOVE;
            break;

        case KEFIR_OPT_COMPARISON_INT16_BELOW:
            comparison = KEFIR_OPT_COMPARISON_INT16_ABOVE;
            break;

        case KEFIR_OPT_COMPARISON_INT32_BELOW:
            comparison = KEFIR_OPT_COMPARISON_INT32_ABOVE;
            break;

        case KEFIR_OPT_COMPARISON_INT64_BELOW:
            comparison = KEFIR_OPT_COMPARISON_INT64_ABOVE;
            break;

        case KEFIR_OPT_COMPARISON_INT8_BELOW_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT8_ABOVE_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT16_BELOW_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT16_ABOVE_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT32_BELOW_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT32_ABOVE_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_INT64_BELOW_OR_EQUALS:
            comparison = KEFIR_OPT_COMPARISON_INT64_ABOVE_OR_EQUALS;
            break;

        case KEFIR_OPT_COMPARISON_FLOAT32_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT32_GREATER:
        case KEFIR_OPT_COMPARISON_FLOAT32_GREATER_OR_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT32_LESSER:
        case KEFIR_OPT_COMPARISON_FLOAT32_LESSER_OR_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER:
        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_GREATER_OR_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER:
        case KEFIR_OPT_COMPARISON_FLOAT32_NOT_LESSER_OR_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT64_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT64_GREATER:
        case KEFIR_OPT_COMPARISON_FLOAT64_GREATER_OR_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT64_LESSER:
        case KEFIR_OPT_COMPARISON_FLOAT64_LESSER_OR_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER:
        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_GREATER_OR_EQUAL:
        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER:
        case KEFIR_OPT_COMPARISON_FLOAT64_NOT_LESSER_OR_EQUAL:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                   "Comparison operator reciprocal can only be computer for integral comparisons");

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected comparison operator");
    }
    *comparison_ptr = comparison;
    return KEFIR_OK;
}

static kefir_result_t free_call_node(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                     kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_call_node *, call_node, value);
    REQUIRE(call_node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer call node"));

    KEFIR_FREE(mem, call_node->arguments);
    memset(call_node, 0, sizeof(struct kefir_opt_call_node));
    KEFIR_FREE(mem, call_node);
    return KEFIR_OK;
}

static kefir_result_t free_inline_assembly(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                           kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_inline_assembly_node *, inline_asm_node, value);
    REQUIRE(inline_asm_node != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer inline assembly node"));

    REQUIRE_OK(kefir_hashtree_free(mem, &inline_asm_node->jump_targets));
    KEFIR_FREE(mem, inline_asm_node->parameters);
    memset(inline_asm_node, 0, sizeof(struct kefir_opt_inline_assembly_node));
    KEFIR_FREE(mem, inline_asm_node);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_init(struct kefir_opt_code_container *code) {
    REQUIRE(code != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code container"));

    code->code = NULL;
    code->capacity = 0;
    code->length = 0;
    code->blocks = NULL;
    code->blocks_length = 0;
    code->blocks_capacity = 0;
    code->phi_nodes = NULL;
    code->phi_nodes_length = 0;
    code->phi_nodes_capacity = 0;
    code->next_call_node_id = 0;
    code->next_inline_assembly_id = 0;
    code->entry_point = KEFIR_ID_NONE;
    code->event_listener = NULL;
    REQUIRE_OK(kefir_hashtree_init(&code->call_nodes, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&code->call_nodes, free_call_node, NULL));
    REQUIRE_OK(kefir_hashtree_init(&code->inline_assembly, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&code->inline_assembly, free_inline_assembly, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_free(struct kefir_mem *mem, struct kefir_opt_code_container *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    for (kefir_size_t i = 0; i < code->phi_nodes_length; i++) {
        REQUIRE_OK(kefir_hashtree_free(mem, &code->phi_nodes[i].links));
    }
    for (kefir_size_t i = 0; i < code->length; i++) {
        REQUIRE_OK(kefir_hashtreeset_free(mem, &code->code[i].uses.instruction));
    }
    for (kefir_size_t i = 0; i < code->blocks_length; i++) {
        REQUIRE_OK(kefir_hashtreeset_free(mem, &code->blocks[i].public_labels));
    }
    KEFIR_FREE(mem, code->blocks);
    REQUIRE_OK(kefir_hashtree_free(mem, &code->call_nodes));
    REQUIRE_OK(kefir_hashtree_free(mem, &code->inline_assembly));
    KEFIR_FREE(mem, code->phi_nodes);
    KEFIR_FREE(mem, code->code);
    memset(code, 0, sizeof(struct kefir_opt_code_container));
    return KEFIR_OK;
}

kefir_bool_t kefir_opt_code_container_is_empty(const struct kefir_opt_code_container *code) {
    REQUIRE(code != NULL, true);
    return code->blocks_length == 0 && code->length == 0 && code->entry_point == KEFIR_ID_NONE;
}

kefir_size_t kefir_opt_code_container_length(const struct kefir_opt_code_container *code) {
    REQUIRE(code != NULL, 0);
    return code->length;
}

kefir_result_t kefir_opt_code_container_new_block(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                  kefir_bool_t entry_point, kefir_opt_block_id_t *block_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(!entry_point || code->entry_point == KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Entry point to optimizer code container already exists"));
    REQUIRE(block_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code block identifier"));

    if (code->blocks_length == code->blocks_capacity) {
        const kefir_size_t new_capacity = code->blocks_capacity + 64;
        struct kefir_opt_code_block *new_blocks = KEFIR_REALLOC(mem, code->blocks, new_capacity * sizeof(struct kefir_opt_code_block));
        REQUIRE(new_blocks != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to reallocate optimizer code blocks"));
        code->blocks = new_blocks;
        code->blocks_capacity = new_capacity;
    }

    struct kefir_opt_code_block *block = &code->blocks[code->blocks_length];
    block->id = code->blocks_length;
    block->content.head = KEFIR_ID_NONE;
    block->content.tail = KEFIR_ID_NONE;
    block->control_flow.head = KEFIR_ID_NONE;
    block->control_flow.tail = KEFIR_ID_NONE;
    block->phi_nodes.head = KEFIR_ID_NONE;
    block->phi_nodes.tail = KEFIR_ID_NONE;
    block->call_nodes.head = KEFIR_ID_NONE;
    block->call_nodes.tail = KEFIR_ID_NONE;
    block->inline_assembly_nodes.head = KEFIR_ID_NONE;
    block->inline_assembly_nodes.tail = KEFIR_ID_NONE;
    REQUIRE_OK(kefir_hashtreeset_init(&block->public_labels, &kefir_hashtree_str_ops));

    code->blocks_length++;
    if (entry_point) {
        code->entry_point = block->id;
    }

    *block_id_ptr = block->id;
    return KEFIR_OK;
}

static kefir_result_t code_container_block_mutable(const struct kefir_opt_code_container *code,
                                                   kefir_opt_block_id_t block_id,
                                                   struct kefir_opt_code_block **block_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block_id < code->blocks_length, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Cannot find optimizer code block with specified identifier"));
    REQUIRE(block_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code block"));

    *block_ptr = &code->blocks[block_id];
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_block(const struct kefir_opt_code_container *code,
                                              kefir_opt_block_id_t block_id,
                                              const struct kefir_opt_code_block **block_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code block"));

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(code_container_block_mutable(code, block_id, &block));
    *block_ptr = block;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_block_count(const struct kefir_opt_code_container *code,
                                                    kefir_size_t *length_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(length_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code container length"));

    *length_ptr = code->blocks_length;
    return KEFIR_OK;
}

static kefir_result_t code_container_instr_mutable(const struct kefir_opt_code_container *code,
                                                   kefir_opt_instruction_ref_t instr_id,
                                                   struct kefir_opt_instruction **instr_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(instr_id != KEFIR_ID_NONE && instr_id < code->length,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS,
                            "Requested optimizer instruction identifier is out of bounds of the code container"));
    REQUIRE(instr_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction"));

    struct kefir_opt_instruction *instr = &code->code[instr_id];
    REQUIRE(instr->block_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Requested optimizer instruction was previously dropped"));
    *instr_ptr = instr;
    return KEFIR_OK;
}

static kefir_result_t drop_block_impl(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                      kefir_opt_block_id_t block_id, kefir_bool_t verify_uses) {
    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(code_container_block_mutable(code, block_id, &block));

    kefir_result_t res;
    kefir_opt_instruction_ref_t instr_ref;
    for (res = kefir_opt_code_block_instr_head(code, block, &instr_ref);
         res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;) {
        kefir_opt_instruction_ref_t next_instr_ref;
        REQUIRE_OK(kefir_opt_instruction_next_sibling(code, instr_ref, &next_instr_ref));

        struct kefir_opt_instruction *instr = NULL;
        REQUIRE_OK(code_container_instr_mutable(code, instr_ref, &instr));
        if (verify_uses) {
            struct kefir_hashtreeset_iterator iter;
            for (res = kefir_hashtreeset_iter(&instr->uses.instruction, &iter); res == KEFIR_OK;
                 res = kefir_hashtreeset_next(&iter)) {
                ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, use_instr_ref, iter.entry);
                struct kefir_opt_instruction *use_instr = NULL;
                REQUIRE_OK(code_container_instr_mutable(code, use_instr_ref, &use_instr));
                REQUIRE(use_instr->block_id != block_id,
                        KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "References from outside the block exist"));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
        res = KEFIR_OK;
        REQUIRE_OK(kefir_hashtreeset_clean(mem, &instr->uses.instruction));

        kefir_bool_t is_control_flow;
        REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(code, instr_ref, &is_control_flow));
        if (is_control_flow) {
            REQUIRE_OK(kefir_opt_code_container_drop_control(code, instr_ref));
        }
        REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, code, instr_ref));

        instr_ref = next_instr_ref;
    }
    REQUIRE_OK(res);

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_drop_block(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                   kefir_opt_block_id_t block_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(drop_block_impl(mem, code, block_id, true));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_add_block_public_label(struct kefir_mem *mem,
                                                               struct kefir_opt_code_container *code,
                                                               kefir_opt_block_id_t block_id,
                                                               const char *alternative_label) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(alternative_label != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid alternative optimizer code block label"));

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(code_container_block_mutable(code, block_id, &block));
    REQUIRE_OK(kefir_hashtreeset_add(mem, &block->public_labels, (kefir_hashtreeset_entry_t) alternative_label));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_block_public_labels_iter(
    const struct kefir_opt_code_container *code, kefir_opt_block_id_t block_id,
    struct kefir_opt_code_block_public_label_iterator *iter) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to code block public label iterator"));

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(code_container_block_mutable(code, block_id, &block));
    REQUIRE_OK(kefir_hashtreeset_iter(&block->public_labels, &iter->iter));
    iter->public_label = (const char *) iter->iter.entry;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_block_public_labels_next(
    struct kefir_opt_code_block_public_label_iterator *iter) {
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to code block public label iterator"));

    REQUIRE_OK(kefir_hashtreeset_next(&iter->iter));
    iter->public_label = (const char *) iter->iter.entry;
    return KEFIR_OK;
}

static kefir_result_t ensure_code_container_capacity(struct kefir_mem *mem, struct kefir_opt_code_container *code) {
    if (code->length == code->capacity) {
        const kefir_size_t new_capacity = (code->capacity * 9 / 8) + 512;
        struct kefir_opt_instruction *new_code =
            KEFIR_REALLOC(mem, code->code, sizeof(struct kefir_opt_instruction) * new_capacity);
        REQUIRE(new_code != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to reallocate optimizer code container"));
        code->code = new_code;
        code->capacity = new_capacity;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_instr(const struct kefir_opt_code_container *code,
                                              kefir_opt_instruction_ref_t instr_id,
                                              const struct kefir_opt_instruction **instr_ptr) {
    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(code, instr_id, &instr));
    *instr_ptr = instr;
    return KEFIR_OK;
}

struct update_uses_param {
    struct kefir_mem *mem;
    const struct kefir_opt_code_container *code;
    kefir_opt_instruction_ref_t user_ref;
};

static kefir_result_t update_uses_callback(kefir_opt_instruction_ref_t used_instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct update_uses_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                           "Expected valid optimizer instruction use update callback parameter"));

    struct kefir_opt_instruction *used_instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(param->code, used_instr_ref, &used_instr));
    REQUIRE_OK(kefir_hashtreeset_add(param->mem, &used_instr->uses.instruction, (kefir_hashtreeset_entry_t) param->user_ref));
    return KEFIR_OK;
}

static kefir_result_t update_used_instructions(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                               kefir_opt_instruction_ref_t user) {
    struct update_uses_param param = {.mem = mem, .code = code, .user_ref = user};

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(code, user, &instr));
    REQUIRE_OK(kefir_opt_instruction_extract_inputs(code, instr, true, update_uses_callback, &param));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_new_instruction(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                        kefir_opt_block_id_t block_id,
                                                        const struct kefir_opt_operation *operation,
                                                        kefir_opt_instruction_ref_t *instr_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(operation != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer operation"));
    REQUIRE(instr_id != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to instruction identifier"));

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(code_container_block_mutable(code, block_id, &block));

    REQUIRE_OK(ensure_code_container_capacity(mem, code));
    struct kefir_opt_instruction *instr = &code->code[code->length];
    instr->id = code->length;
    instr->operation = *operation;
    instr->block_id = block->id;
    instr->control_flow.prev = KEFIR_ID_NONE;
    instr->control_flow.next = KEFIR_ID_NONE;
    instr->siblings.prev = block->content.tail;
    instr->siblings.next = KEFIR_ID_NONE;
    REQUIRE_OK(kefir_hashtreeset_init(&instr->uses.instruction, &kefir_hashtree_uint_ops));

    if (block->content.tail != KEFIR_ID_NONE) {
        struct kefir_opt_instruction *prev_instr = &code->code[block->content.tail];
        REQUIRE(prev_instr->siblings.next == KEFIR_ID_NONE, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected previous instruction in block to have no successors"));
        prev_instr->siblings.next = instr->id;
    } else {
        block->content.head = instr->id;
    }
    block->content.tail = instr->id;

    code->length++;
    *instr_id = instr->id;

    REQUIRE_OK(update_used_instructions(mem, code, instr->id));

    if (code->event_listener != NULL && code->event_listener->on_new_instruction != NULL) {
        REQUIRE_OK(code->event_listener->on_new_instruction(mem, code, instr->id, code->event_listener->payload));
    }
    return KEFIR_OK;
}

static kefir_result_t drop_uses_callback(kefir_opt_instruction_ref_t used_instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct update_uses_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                           "Expected valid optimizer instruction use update callback parameter"));

    struct kefir_opt_instruction *used_instr = NULL;
    kefir_result_t res = code_container_instr_mutable(param->code, used_instr_ref, &used_instr);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        REQUIRE_OK(
            kefir_hashtreeset_delete(param->mem, &used_instr->uses.instruction, (kefir_hashtreeset_entry_t) param->user_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t drop_used_instructions(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                             kefir_opt_instruction_ref_t user) {
    struct update_uses_param param = {.mem = mem, .code = code, .user_ref = user};

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(code, user, &instr));
    REQUIRE_OK(kefir_opt_instruction_extract_inputs(code, instr, true, drop_uses_callback, &param));
    return KEFIR_OK;
}

static kefir_result_t kefir_opt_code_container_drop_phi(struct kefir_mem *, const struct kefir_opt_code_container *,
                                                        kefir_opt_phi_id_t);

static kefir_result_t kefir_opt_code_container_drop_call(struct kefir_mem *, const struct kefir_opt_code_container *,
                                                         kefir_opt_call_id_t);

static kefir_result_t kefir_opt_code_container_drop_inline_asm(struct kefir_mem *,
                                                               const struct kefir_opt_code_container *,
                                                               kefir_opt_inline_assembly_id_t);

kefir_result_t drop_instr_impl(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                               kefir_opt_instruction_ref_t instr_id, kefir_bool_t verify_uses) {
    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(code, instr_id, &instr));
    REQUIRE(instr->block_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Requested optimizer instruction was previously dropped"));

    if (verify_uses) {
        struct kefir_opt_instruction *used_instr = NULL;
        REQUIRE_OK(code_container_instr_mutable(code, instr_id, &used_instr));
        REQUIRE(kefir_hashtreeset_empty(&used_instr->uses.instruction),
                KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Instruction with active dependents cannot be dropped"));
    }

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(code_container_block_mutable(code, instr->block_id, &block));

    REQUIRE(instr->control_flow.prev == KEFIR_ID_NONE && instr->control_flow.next == KEFIR_ID_NONE &&
                block->control_flow.head != instr->id,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Instruction shall be removed from control flow prior to dropping"));
    REQUIRE_OK(drop_used_instructions(mem, code, instr->id));

    if (block->content.head == instr->id) {
        block->content.head = instr->siblings.next;
    }

    if (block->content.tail == instr->id) {
        block->content.tail = instr->siblings.prev;
    }

    if (instr->siblings.prev != KEFIR_ID_NONE) {
        struct kefir_opt_instruction *prev_instr = NULL;
        REQUIRE_OK(code_container_instr_mutable(code, instr->siblings.prev, &prev_instr));

        prev_instr->siblings.next = instr->siblings.next;
    }

    if (instr->siblings.next != KEFIR_ID_NONE) {
        struct kefir_opt_instruction *next_instr = NULL;
        REQUIRE_OK(code_container_instr_mutable(code, instr->siblings.next, &next_instr));

        next_instr->siblings.prev = instr->siblings.prev;
    }

    instr->siblings.prev = KEFIR_ID_NONE;
    instr->siblings.next = KEFIR_ID_NONE;
    instr->block_id = KEFIR_ID_NONE;

    if (instr->operation.opcode == KEFIR_OPT_OPCODE_PHI) {
        REQUIRE_OK(kefir_opt_code_container_drop_phi(mem, code, instr->operation.parameters.phi_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INVOKE ||
               instr->operation.opcode == KEFIR_OPT_OPCODE_INVOKE_VIRTUAL ||
               instr->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE ||
               instr->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL) {
        REQUIRE_OK(kefir_opt_code_container_drop_call(mem, code, instr->operation.parameters.function_call.call_ref));
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INLINE_ASSEMBLY) {
        REQUIRE_OK(kefir_opt_code_container_drop_inline_asm(mem, code, instr->operation.parameters.inline_asm_ref));
    }

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_drop_instr(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                                   kefir_opt_instruction_ref_t instr_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    REQUIRE_OK(drop_instr_impl(mem, code, instr_id, true));
    return KEFIR_OK;
}

static kefir_result_t code_container_phi_mutable(const struct kefir_opt_code_container *code,
                                                 kefir_opt_phi_id_t phi_ref, struct kefir_opt_phi_node **phi_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(phi_ref < code->phi_nodes_length,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Cannot find requested optimizer phi node"));
    REQUIRE(phi_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer phi node"));

    *phi_ptr = &code->phi_nodes[phi_ref];
    REQUIRE((*phi_ptr)->block_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Phi node has been previously dropped from block"));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_copy_instruction(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                         kefir_opt_block_id_t block_id,
                                                         kefir_opt_instruction_ref_t src_instr_ref,
                                                         kefir_opt_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(instr_ref_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction reference"));

    struct kefir_opt_instruction *src_instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(code, src_instr_ref, &src_instr));
    REQUIRE(src_instr->block_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Requested optimizer instruction was previously dropped"));

    struct kefir_opt_operation src_operation = src_instr->operation;

    switch (src_operation.opcode) {
        case KEFIR_OPT_OPCODE_PHI: {
            kefir_opt_phi_id_t phi_ref;
            REQUIRE_OK(kefir_opt_code_container_new_phi(mem, code, block_id, &phi_ref, instr_ref_ptr));

            struct kefir_opt_phi_node *src_phi_node = NULL;
            REQUIRE_OK(code_container_phi_mutable(code, src_operation.parameters.phi_ref, &src_phi_node));

            struct kefir_hashtree_node_iterator iter;
            for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&src_phi_node->links, &iter); node != NULL;
                 node = kefir_hashtree_next(&iter)) {
                ASSIGN_DECL_CAST(kefir_opt_block_id_t, link_block_id, node->key);
                ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, link_instr_ref, node->value);

                REQUIRE_OK(kefir_opt_code_container_phi_attach(mem, code, phi_ref, link_block_id, link_instr_ref));
            }
        } break;

        case KEFIR_OPT_OPCODE_INVOKE:
        case KEFIR_OPT_OPCODE_INVOKE_VIRTUAL: {
            const struct kefir_opt_call_node *src_call_node;
            REQUIRE_OK(
                kefir_opt_code_container_call(code, src_operation.parameters.function_call.call_ref, &src_call_node));

            kefir_opt_call_id_t call_ref;
            REQUIRE_OK(kefir_opt_code_container_new_call(
                mem, code, block_id, src_call_node->function_declaration_id, src_call_node->argument_count,
                src_operation.parameters.function_call.indirect_ref, &call_ref, instr_ref_ptr));
            REQUIRE_OK(
                kefir_opt_code_container_call(code, src_operation.parameters.function_call.call_ref, &src_call_node));

            for (kefir_size_t i = 0; i < src_call_node->argument_count; i++) {
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, code, call_ref, i, src_call_node->arguments[i]));
            }
            if (src_call_node->return_space != KEFIR_ID_NONE) {
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_return_space(mem, code, call_ref, src_call_node->return_space));
            }
        } break;

        case KEFIR_OPT_OPCODE_TAIL_INVOKE:
        case KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL: {
            const struct kefir_opt_call_node *src_call_node;
            REQUIRE_OK(
                kefir_opt_code_container_call(code, src_operation.parameters.function_call.call_ref, &src_call_node));

            kefir_opt_call_id_t call_ref;
            REQUIRE_OK(kefir_opt_code_container_new_tail_call(
                mem, code, block_id, src_call_node->function_declaration_id, src_call_node->argument_count,
                src_operation.parameters.function_call.indirect_ref, &call_ref, instr_ref_ptr));
            REQUIRE_OK(
                kefir_opt_code_container_call(code, src_operation.parameters.function_call.call_ref, &src_call_node));

            for (kefir_size_t i = 0; i < src_call_node->argument_count; i++) {
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_argument(mem, code, call_ref, i, src_call_node->arguments[i]));
            }
            if (src_call_node->return_space != KEFIR_ID_NONE) {
                REQUIRE_OK(
                    kefir_opt_code_container_call_set_return_space(mem, code, call_ref, src_call_node->return_space));
            }
        } break;

        case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY: {
            const struct kefir_opt_inline_assembly_node *src_inline_asm_node;
            REQUIRE_OK(kefir_opt_code_container_inline_assembly(code, src_operation.parameters.inline_asm_ref,
                                                                &src_inline_asm_node));

            kefir_opt_inline_assembly_id_t inline_asm_ref;
            REQUIRE_OK(kefir_opt_code_container_new_inline_assembly(
                mem, code, block_id, src_inline_asm_node->inline_asm_id, src_inline_asm_node->parameter_count,
                &inline_asm_ref, instr_ref_ptr));
            REQUIRE_OK(kefir_opt_code_container_inline_assembly(code, src_operation.parameters.inline_asm_ref,
                                                                &src_inline_asm_node));

            for (kefir_size_t i = 0; i < src_inline_asm_node->parameter_count; i++) {
                REQUIRE_OK(kefir_opt_code_container_inline_assembly_set_parameter(mem, code, inline_asm_ref, i,
                                                                                  &src_inline_asm_node->parameters[i]));
            }

            if (src_inline_asm_node->default_jump_target != KEFIR_ID_NONE) {
                REQUIRE_OK(kefir_opt_code_container_inline_assembly_set_default_jump_target(
                    code, inline_asm_ref, src_inline_asm_node->default_jump_target));
            }

            struct kefir_hashtree_node_iterator iter;
            for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&src_inline_asm_node->jump_targets, &iter);
                 node != NULL; node = kefir_hashtree_next(&iter)) {
                ASSIGN_DECL_CAST(kefir_id_t, target_id, node->key);
                ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block_id, node->value);
                REQUIRE_OK(kefir_opt_code_container_inline_assembly_add_jump_target(mem, code, inline_asm_ref,
                                                                                    target_id, target_block_id));
            }
        } break;

        default:
            REQUIRE_OK(kefir_opt_code_container_new_instruction(mem, code, block_id, &src_operation, instr_ref_ptr));
            break;
    }

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_add_control(const struct kefir_opt_code_container *code,
                                                    kefir_opt_block_id_t block_id,
                                                    kefir_opt_instruction_ref_t instr_id) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(code_container_block_mutable(code, block_id, &block));
    REQUIRE_OK(kefir_opt_code_container_insert_control(code, block_id, block->control_flow.tail, instr_id));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_insert_control(const struct kefir_opt_code_container *code,
                                                       kefir_opt_block_id_t block_id,
                                                       kefir_opt_instruction_ref_t after_instr_id,
                                                       kefir_opt_instruction_ref_t instr_id) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(code_container_block_mutable(code, block_id, &block));

    struct kefir_opt_instruction *after_instr = NULL;
    if (after_instr_id != KEFIR_ID_NONE) {
        REQUIRE_OK(code_container_instr_mutable(code, after_instr_id, &after_instr));
        if (block != NULL) {
            REQUIRE(
                after_instr->block_id == block->id,
                KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Provided target instruction is not a part of specified block"));
        } else {
            REQUIRE_OK(code_container_block_mutable(code, after_instr->block_id, &block));
        }
    }

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(code, instr_id, &instr));
    REQUIRE(instr->block_id == block->id,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Provided instruction is not a part of specified block"));
    REQUIRE(instr->control_flow.prev == KEFIR_ID_NONE && instr->control_flow.next == KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Provided instruction is already a part of control flow"));

    if (after_instr == NULL) {
        if (block->control_flow.head != KEFIR_ID_NONE) {
            struct kefir_opt_instruction *head_instr = NULL;
            REQUIRE_OK(code_container_instr_mutable(code, block->control_flow.head, &head_instr));
            head_instr->control_flow.prev = instr->id;
        }
        instr->control_flow.next = block->control_flow.head;
        block->control_flow.head = instr->id;
        if (block->control_flow.tail == KEFIR_ID_NONE) {
            block->control_flow.tail = instr->id;
        }
    } else {
        if (after_instr->control_flow.next != KEFIR_ID_NONE) {
            struct kefir_opt_instruction *next_instr = NULL;
            REQUIRE_OK(code_container_instr_mutable(code, after_instr->control_flow.next, &next_instr));
            next_instr->control_flow.prev = instr->id;
        }

        instr->control_flow.prev = after_instr->id;
        instr->control_flow.next = after_instr->control_flow.next;
        after_instr->control_flow.next = instr->id;

        if (block->control_flow.tail == after_instr->id) {
            block->control_flow.tail = instr->id;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t control_flow_remove(const struct kefir_opt_code_container *code,
                                          struct kefir_opt_code_block *block, struct kefir_opt_instruction *instr) {
    if (block->control_flow.head == instr->id) {
        block->control_flow.head = instr->control_flow.next;
    }

    if (block->control_flow.tail == instr->id) {
        block->control_flow.tail = instr->control_flow.prev;
    }

    if (instr->control_flow.prev != KEFIR_ID_NONE) {
        struct kefir_opt_instruction *prev_instr = NULL;
        REQUIRE_OK(code_container_instr_mutable(code, instr->control_flow.prev, &prev_instr));

        prev_instr->control_flow.next = instr->control_flow.next;
    }

    if (instr->control_flow.next != KEFIR_ID_NONE) {
        struct kefir_opt_instruction *next_instr = NULL;
        REQUIRE_OK(code_container_instr_mutable(code, instr->control_flow.next, &next_instr));

        next_instr->control_flow.prev = instr->control_flow.prev;
    }

    instr->control_flow.prev = KEFIR_ID_NONE;
    instr->control_flow.next = KEFIR_ID_NONE;

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_drop_control(const struct kefir_opt_code_container *code,
                                                     kefir_opt_instruction_ref_t instr_id) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(code, instr_id, &instr));
    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(code_container_block_mutable(code, instr->block_id, &block));

    REQUIRE_OK(control_flow_remove(code, block, instr));
    return KEFIR_OK;
}

static kefir_result_t ensure_phi_container_capacity(struct kefir_mem *mem, struct kefir_opt_code_container *code) {
    if (code->phi_nodes_length == code->phi_nodes_capacity) {
        kefir_size_t new_capacity = code->phi_nodes_capacity * 9 / 8 + 512;
        struct kefir_opt_phi_node *new_phis =
            KEFIR_REALLOC(mem, code->phi_nodes, sizeof(struct kefir_opt_phi_node) * new_capacity);
        REQUIRE(new_phis != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer phi node container"));
        code->phi_nodes = new_phis;
        code->phi_nodes_capacity = new_capacity;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_new_phi(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                kefir_opt_block_id_t block_id, kefir_opt_phi_id_t *phi_ptr,
                                                kefir_opt_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(phi_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer phi identifier"));
    REQUIRE(instr_ref_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction reference"));

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(code_container_block_mutable(code, block_id, &block));

    REQUIRE_OK(ensure_phi_container_capacity(mem, code));

    struct kefir_opt_phi_node *phi_node = &code->phi_nodes[code->phi_nodes_length];

    phi_node->block_id = block_id;
    phi_node->node_id = code->phi_nodes_length;
    phi_node->output_ref = KEFIR_ID_NONE;
    phi_node->number_of_links = 0u;

    kefir_result_t res = kefir_hashtree_init(&phi_node->links, &kefir_hashtree_uint_ops);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, phi_node);
        return res;
    });

    phi_node->siblings.prev = block->phi_nodes.tail;
    if (phi_node->siblings.prev != KEFIR_ID_NONE) {
        struct kefir_opt_phi_node *prev_phi_node = NULL;
        REQUIRE_OK(code_container_phi_mutable(code, phi_node->siblings.prev, &prev_phi_node));
        REQUIRE(prev_phi_node->siblings.next == KEFIR_ID_NONE,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected optimizer phi node to be the last in the block"));
        prev_phi_node->siblings.next = phi_node->node_id;
    }
    phi_node->siblings.next = KEFIR_ID_NONE;
    block->phi_nodes.tail = phi_node->node_id;
    if (block->phi_nodes.head == KEFIR_ID_NONE) {
        block->phi_nodes.head = phi_node->node_id;
    }

    code->phi_nodes_length++;

    kefir_opt_instruction_ref_t phi_instr_ref;
    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_PHI, .parameters = {.phi_ref = phi_node->node_id}},
        &phi_instr_ref));
    phi_node->output_ref = phi_instr_ref;

    *phi_ptr = phi_node->node_id;
    *instr_ref_ptr = phi_instr_ref;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_phi(const struct kefir_opt_code_container *code, kefir_opt_phi_id_t phi_ref,
                                            const struct kefir_opt_phi_node **phi_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(phi_ref < code->phi_nodes_length,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Cannot find requested optimizer phi node"));
    REQUIRE(phi_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer phi node"));

    struct kefir_opt_phi_node *phi = NULL;
    REQUIRE_OK(code_container_phi_mutable(code, phi_ref, &phi));
    *phi_ptr = phi;
    return KEFIR_OK;
}

static kefir_result_t kefir_opt_code_container_drop_phi(struct kefir_mem *mem,
                                                        const struct kefir_opt_code_container *code,
                                                        kefir_opt_phi_id_t phi_ref) {
    REQUIRE(phi_ref < code->phi_nodes_length,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Cannot find requested optimizer phi node"));

    struct kefir_opt_phi_node *phi_node = NULL;
    REQUIRE_OK(code_container_phi_mutable(code, phi_ref, &phi_node));

    REQUIRE(phi_node->block_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Phi node has been previously dropped from block"));

    if (phi_node->output_ref != KEFIR_ID_NONE) {
        struct kefir_opt_instruction *instr = &code->code[phi_node->output_ref];
        REQUIRE(instr->block_id == KEFIR_ID_NONE,
                KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                                "Prior to dropping phi node its output reference shall be dropped from the block"));
    }

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(code_container_block_mutable(code, phi_node->block_id, &block));

    if (block->phi_nodes.head == phi_node->node_id) {
        block->phi_nodes.head = phi_node->siblings.next;
    }

    if (block->phi_nodes.tail == phi_node->node_id) {
        block->phi_nodes.tail = phi_node->siblings.prev;
    }

    if (phi_node->siblings.prev != KEFIR_ID_NONE) {
        struct kefir_opt_phi_node *prev_phi = NULL;
        REQUIRE_OK(code_container_phi_mutable(code, phi_node->siblings.prev, &prev_phi));

        prev_phi->siblings.next = phi_node->siblings.next;
    }

    if (phi_node->siblings.next != KEFIR_ID_NONE) {
        struct kefir_opt_phi_node *next_phi = NULL;
        REQUIRE_OK(code_container_phi_mutable(code, phi_node->siblings.next, &next_phi));

        next_phi->siblings.prev = phi_node->siblings.prev;
    }

    phi_node->siblings.prev = KEFIR_ID_NONE;
    phi_node->siblings.next = KEFIR_ID_NONE;
    phi_node->block_id = KEFIR_ID_NONE;

    struct kefir_hashtree_node_iterator iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&phi_node->links, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, used_instr_ref, node->value);

        struct kefir_opt_instruction *used_instr = NULL;
        kefir_result_t res = code_container_instr_mutable(code, used_instr_ref, &used_instr);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            REQUIRE_OK(
                kefir_hashtreeset_delete(mem, &used_instr->uses.instruction, (kefir_hashtreeset_entry_t) phi_node->output_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t code_container_call_mutable(const struct kefir_opt_code_container *code,
                                                  kefir_opt_call_id_t call_ref, struct kefir_opt_call_node **call_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(call_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer call node"));

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&code->call_nodes, (kefir_hashtree_key_t) call_ref, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Cannot find requested optimizer call node");
    }
    REQUIRE_OK(res);

    *call_ptr = (struct kefir_opt_call_node *) node->value;

    return KEFIR_OK;
}

static kefir_result_t kefir_opt_code_container_drop_call(struct kefir_mem *mem,
                                                         const struct kefir_opt_code_container *code,
                                                         kefir_opt_call_id_t call_ref) {
    REQUIRE(call_ref < code->next_call_node_id,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Cannot find requested optimizer call node"));

    struct kefir_opt_call_node *call_node = NULL;
    REQUIRE_OK(code_container_call_mutable(code, call_ref, &call_node));

    REQUIRE(call_node->block_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Call node has been previously dropped from block"));

    if (call_node->output_ref != KEFIR_ID_NONE) {
        struct kefir_opt_instruction *instr = &code->code[call_node->output_ref];
        REQUIRE(instr->block_id == KEFIR_ID_NONE,
                KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                                "Prior to dropping call node its output reference shall be dropped from the block"));
    }

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(code_container_block_mutable(code, call_node->block_id, &block));

    if (block->call_nodes.head == call_node->node_id) {
        block->call_nodes.head = call_node->siblings.next;
    }

    if (block->call_nodes.tail == call_node->node_id) {
        block->call_nodes.tail = call_node->siblings.prev;
    }

    if (call_node->siblings.prev != KEFIR_ID_NONE) {
        struct kefir_opt_call_node *prev_call = NULL;
        REQUIRE_OK(code_container_call_mutable(code, call_node->siblings.prev, &prev_call));

        prev_call->siblings.next = call_node->siblings.next;
    }

    if (call_node->siblings.next != KEFIR_ID_NONE) {
        struct kefir_opt_call_node *next_call = NULL;
        REQUIRE_OK(code_container_call_mutable(code, call_node->siblings.next, &next_call));

        next_call->siblings.prev = call_node->siblings.prev;
    }

    call_node->siblings.prev = KEFIR_ID_NONE;
    call_node->siblings.next = KEFIR_ID_NONE;
    call_node->block_id = KEFIR_ID_NONE;

    for (kefir_size_t i = 0; i < call_node->argument_count; i++) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, used_instr_ref, call_node->arguments[i]);

        struct kefir_opt_instruction *used_instr = NULL;
        kefir_result_t res = code_container_instr_mutable(code, used_instr_ref, &used_instr);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            REQUIRE_OK(
                kefir_hashtreeset_delete(mem, &used_instr->uses.instruction, (kefir_hashtreeset_entry_t) call_node->output_ref));
        }
    }
    if (call_node->return_space != KEFIR_ID_NONE) {
        struct kefir_opt_instruction *used_instr = NULL;
        kefir_result_t res = code_container_instr_mutable(code, call_node->return_space, &used_instr);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            REQUIRE_OK(
                kefir_hashtreeset_delete(mem, &used_instr->uses.instruction, (kefir_hashtreeset_entry_t) call_node->output_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t code_container_inline_assembly_mutable(const struct kefir_opt_code_container *code,
                                                             kefir_opt_inline_assembly_id_t inline_asm_ref,
                                                             struct kefir_opt_inline_assembly_node **inline_asm_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(inline_asm_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer inline assembly node"));

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&code->inline_assembly, (kefir_hashtree_key_t) inline_asm_ref, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Cannot find requested optimizer inline assembly node");
    }
    REQUIRE_OK(res);

    *inline_asm_ptr = (struct kefir_opt_inline_assembly_node *) node->value;

    return KEFIR_OK;
}

static kefir_result_t kefir_opt_code_container_drop_inline_asm(struct kefir_mem *mem,
                                                               const struct kefir_opt_code_container *code,
                                                               kefir_opt_inline_assembly_id_t inline_asm_ref) {
    REQUIRE(inline_asm_ref < code->next_inline_assembly_id,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Cannot find requested optimizer inline assembly node"));

    struct kefir_opt_inline_assembly_node *inline_asm_node = NULL;
    REQUIRE_OK(code_container_inline_assembly_mutable(code, inline_asm_ref, &inline_asm_node));

    REQUIRE(inline_asm_node->block_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Inline assembly node has been previously dropped from block"));

    if (inline_asm_node->output_ref != KEFIR_ID_NONE) {
        struct kefir_opt_instruction *instr = &code->code[inline_asm_node->output_ref];
        REQUIRE(instr->block_id == KEFIR_ID_NONE,
                KEFIR_SET_ERROR(
                    KEFIR_INVALID_REQUEST,
                    "Prior to dropping inline assembly node its output reference shall be dropped from the block"));
    }

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(code_container_block_mutable(code, inline_asm_node->block_id, &block));

    if (block->inline_assembly_nodes.head == inline_asm_node->node_id) {
        block->inline_assembly_nodes.head = inline_asm_node->siblings.next;
    }

    if (block->inline_assembly_nodes.tail == inline_asm_node->node_id) {
        block->inline_assembly_nodes.tail = inline_asm_node->siblings.prev;
    }

    if (inline_asm_node->siblings.prev != KEFIR_ID_NONE) {
        struct kefir_opt_inline_assembly_node *prev_inline_asm = NULL;
        REQUIRE_OK(code_container_inline_assembly_mutable(code, inline_asm_node->siblings.prev, &prev_inline_asm));

        prev_inline_asm->siblings.next = inline_asm_node->siblings.next;
    }

    if (inline_asm_node->siblings.next != KEFIR_ID_NONE) {
        struct kefir_opt_inline_assembly_node *next_inline_asm = NULL;
        REQUIRE_OK(code_container_inline_assembly_mutable(code, inline_asm_node->siblings.next, &next_inline_asm));

        next_inline_asm->siblings.prev = inline_asm_node->siblings.prev;
    }

    inline_asm_node->siblings.prev = KEFIR_ID_NONE;
    inline_asm_node->siblings.next = KEFIR_ID_NONE;
    inline_asm_node->block_id = KEFIR_ID_NONE;

    for (kefir_size_t i = 0; i < inline_asm_node->parameter_count; i++) {
        if (inline_asm_node->parameters[i].load_store_ref != KEFIR_ID_NONE) {
            struct kefir_opt_instruction *used_instr = NULL;
            kefir_result_t res = code_container_instr_mutable(code, inline_asm_node->parameters[i].load_store_ref, &used_instr);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                REQUIRE_OK(kefir_hashtreeset_delete(mem, &used_instr->uses.instruction,
                                                    (kefir_hashtreeset_entry_t) inline_asm_node->output_ref));
            }
        }
        if (inline_asm_node->parameters[i].read_ref != KEFIR_ID_NONE) {
            struct kefir_opt_instruction *used_instr = NULL;
            kefir_result_t res = code_container_instr_mutable(code, inline_asm_node->parameters[i].read_ref, &used_instr);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
                REQUIRE_OK(kefir_hashtreeset_delete(mem, &used_instr->uses.instruction,
                                                    (kefir_hashtreeset_entry_t) inline_asm_node->output_ref));
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t add_used_instructions(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                            kefir_opt_instruction_ref_t use_instr_ref,
                                            kefir_opt_instruction_ref_t instr_ref) {
    struct kefir_opt_instruction *used_instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(code, instr_ref, &used_instr));
    REQUIRE_OK(kefir_hashtreeset_add(mem, &used_instr->uses.instruction, (kefir_hashtreeset_entry_t) use_instr_ref));
    return KEFIR_OK;
}

static kefir_result_t remove_used_instructions(struct kefir_mem *mem, const struct kefir_opt_code_container *code,
                                               kefir_opt_instruction_ref_t use_instr_ref,
                                               kefir_opt_instruction_ref_t instr_ref) {
    struct kefir_opt_instruction *used_instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(code, instr_ref, &used_instr));
    REQUIRE_OK(kefir_hashtreeset_delete(mem, &used_instr->uses.instruction, (kefir_hashtreeset_entry_t) use_instr_ref));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_phi_attach(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                   kefir_opt_phi_id_t phi_ref, kefir_opt_block_id_t block_id,
                                                   kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    struct kefir_opt_phi_node *phi_node = NULL;
    REQUIRE_OK(code_container_phi_mutable(code, phi_ref, &phi_node));

    REQUIRE(phi_node->block_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Phi node has been previously dropped from block"));

    REQUIRE(!kefir_hashtree_has(&phi_node->links, (kefir_hashtree_key_t) block_id),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Optimizer phi node already links provided block"));

    REQUIRE_OK(kefir_hashtree_insert(mem, &phi_node->links, (kefir_hashtree_key_t) block_id,
                                     (kefir_hashtree_value_t) instr_ref));
    phi_node->number_of_links++;

    REQUIRE_OK(add_used_instructions(mem, code, phi_node->output_ref, instr_ref));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_phi_link_for(const struct kefir_opt_code_container *code,
                                                     kefir_opt_phi_id_t phi_ref, kefir_opt_block_id_t block_id,
                                                     kefir_opt_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(instr_ref_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction reference"));

    struct kefir_opt_phi_node *phi_node = NULL;
    REQUIRE_OK(code_container_phi_mutable(code, phi_ref, &phi_node));

    REQUIRE(phi_node->block_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Phi node has been previously dropped from block"));

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&phi_node->links, (kefir_hashtree_key_t) block_id, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested optimizer phi node link");
    }
    REQUIRE_OK(res);

    *instr_ref_ptr = (kefir_opt_instruction_ref_t) node->value;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_phi_drop_link(struct kefir_mem *mem,
                                                      const struct kefir_opt_code_container *code,
                                                      kefir_opt_phi_id_t phi_ref, kefir_opt_block_id_t block_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    struct kefir_opt_phi_node *phi_node = NULL;
    REQUIRE_OK(code_container_phi_mutable(code, phi_ref, &phi_node));
    const kefir_opt_instruction_ref_t phi_instr = phi_node->output_ref;

    REQUIRE(phi_node->block_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Phi node has been previously dropped from block"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&phi_node->links, (kefir_hashtree_key_t) block_id, &node);
    REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, link_ref, node->value);
    REQUIRE_OK(kefir_hashtree_delete(mem, &phi_node->links, (kefir_hashtree_key_t) block_id));
    phi_node->number_of_links--;

    kefir_bool_t drop_use = true;
    struct kefir_hashtree_node_iterator iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&phi_node->links, &iter); node != NULL && drop_use;
         node = kefir_hashtree_next(&iter)) {
        if ((kefir_opt_instruction_ref_t) node->value == link_ref) {
            drop_use = false;
        }
    }

    if (drop_use) {
        struct kefir_opt_instruction *used_instr = NULL;
        REQUIRE_OK(code_container_instr_mutable(code, link_ref, &used_instr));
        REQUIRE_OK(kefir_hashtreeset_delete(mem, &used_instr->uses.instruction, (kefir_hashtreeset_entry_t) phi_instr));
    }
    return KEFIR_OK;
}

static kefir_result_t new_call_impl(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                    kefir_opt_block_id_t block_id, kefir_id_t func_decl_id, kefir_size_t argc,
                                    kefir_opt_instruction_ref_t function_ref, kefir_opt_opcode_t opcode,
                                    kefir_opt_call_id_t *call_ptr, kefir_opt_instruction_ref_t *instr_ref_ptr) {
    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(code_container_block_mutable(code, block_id, &block));

    struct kefir_opt_call_node *call_node = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_call_node));
    REQUIRE(call_node != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer call node"));

    call_node->block_id = block_id;
    call_node->node_id = code->next_call_node_id;
    call_node->function_declaration_id = func_decl_id;
    call_node->argument_count = argc;
    if (argc > 0) {
        call_node->arguments = KEFIR_MALLOC(mem, sizeof(kefir_opt_instruction_ref_t) * argc);
        REQUIRE_ELSE(call_node->arguments != NULL, {
            KEFIR_FREE(mem, call_node);
            return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer call node arguments");
        });
        for (kefir_size_t i = 0; i < argc; i++) {
            call_node->arguments[i] = KEFIR_ID_NONE;
        }
    } else {
        call_node->arguments = NULL;
    }
    call_node->return_space = KEFIR_ID_NONE;

    kefir_result_t res = kefir_hashtree_insert(mem, &code->call_nodes, (kefir_hashtree_key_t) call_node->node_id,
                                               (kefir_hashtree_value_t) call_node);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, call_node->arguments);
        KEFIR_FREE(mem, call_node);
        return res;
    });

    call_node->siblings.prev = block->call_nodes.tail;
    if (call_node->siblings.prev != KEFIR_ID_NONE) {
        struct kefir_opt_call_node *prev_call_node = NULL;
        REQUIRE_OK(code_container_call_mutable(code, call_node->siblings.prev, &prev_call_node));
        REQUIRE(prev_call_node->siblings.next == KEFIR_ID_NONE,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected optimizer call node to be the last in the block"));
        prev_call_node->siblings.next = call_node->node_id;
    }
    call_node->siblings.next = KEFIR_ID_NONE;
    block->call_nodes.tail = call_node->node_id;
    if (block->call_nodes.head == KEFIR_ID_NONE) {
        block->call_nodes.head = call_node->node_id;
    }

    code->next_call_node_id++;

    kefir_opt_instruction_ref_t call_instr_ref;
    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation) {
            .opcode = opcode,
            .parameters.function_call = {.call_ref = call_node->node_id, .indirect_ref = function_ref}},
        &call_instr_ref));
    call_node->output_ref = call_instr_ref;

    *call_ptr = call_node->node_id;
    *instr_ref_ptr = call_instr_ref;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_new_call(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                 kefir_opt_block_id_t block_id, kefir_id_t func_decl_id,
                                                 kefir_size_t argc, kefir_opt_instruction_ref_t function_ref,
                                                 kefir_opt_call_id_t *call_ptr,
                                                 kefir_opt_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(call_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer call identifier"));
    REQUIRE(instr_ref_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer call instruction reference"));
    REQUIRE(function_ref == KEFIR_ID_NONE || function_ref < code->length,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid function instruction reference"));

    REQUIRE_OK(new_call_impl(mem, code, block_id, func_decl_id, argc, function_ref,
                             function_ref != KEFIR_ID_NONE ? KEFIR_OPT_OPCODE_INVOKE_VIRTUAL : KEFIR_OPT_OPCODE_INVOKE,
                             call_ptr, instr_ref_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_new_tail_call(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                      kefir_opt_block_id_t block_id, kefir_id_t func_decl_id,
                                                      kefir_size_t argc, kefir_opt_instruction_ref_t function_ref,
                                                      kefir_opt_call_id_t *call_ptr,
                                                      kefir_opt_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(call_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer call identifier"));
    REQUIRE(instr_ref_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer call instruction reference"));
    REQUIRE(function_ref == KEFIR_ID_NONE || function_ref < code->length,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid function instruction reference"));

    REQUIRE_OK(new_call_impl(
        mem, code, block_id, func_decl_id, argc, function_ref,
        function_ref != KEFIR_ID_NONE ? KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL : KEFIR_OPT_OPCODE_TAIL_INVOKE, call_ptr,
        instr_ref_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_call(const struct kefir_opt_code_container *code, kefir_opt_call_id_t call_ref,
                                             const struct kefir_opt_call_node **call_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(call_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer call node"));

    struct kefir_opt_call_node *call = NULL;
    REQUIRE_OK(code_container_call_mutable(code, call_ref, &call));
    *call_ptr = call;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_call_set_argument(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                          kefir_opt_call_id_t call_ref, kefir_size_t argument_index,
                                                          kefir_opt_instruction_ref_t argument_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    struct kefir_opt_call_node *call_node = NULL;
    REQUIRE_OK(code_container_call_mutable(code, call_ref, &call_node));

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(code, argument_ref, &instr));

    REQUIRE(argument_index < call_node->argument_count,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided argument index exceeds optimizer call node argument count"));

    if (call_node->arguments[argument_index] != KEFIR_ID_NONE) {
        REQUIRE_OK(remove_used_instructions(mem, code, call_node->output_ref, call_node->arguments[argument_index]));
    }
    call_node->arguments[argument_index] = argument_ref;

    REQUIRE_OK(add_used_instructions(mem, code, call_node->output_ref, argument_ref));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_call_get_argument(const struct kefir_opt_code_container *code,
                                                          kefir_opt_call_id_t call_ref, kefir_size_t argument_index,
                                                          kefir_opt_instruction_ref_t *argument_ref) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(argument_ref != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer optimizer instruction reference"));

    struct kefir_opt_call_node *call_node = NULL;
    REQUIRE_OK(code_container_call_mutable(code, call_ref, &call_node));

    REQUIRE(argument_index < call_node->argument_count,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested argument is out of call node bounds"));
    *argument_ref = call_node->arguments[argument_index];
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_call_set_return_space(struct kefir_mem *mem,
                                                              struct kefir_opt_code_container *code,
                                                              kefir_opt_call_id_t call_ref,
                                                              kefir_opt_instruction_ref_t return_space_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    struct kefir_opt_call_node *call_node = NULL;
    REQUIRE_OK(code_container_call_mutable(code, call_ref, &call_node));
    call_node->return_space = return_space_ref;

    REQUIRE_OK(add_used_instructions(mem, code, call_node->output_ref, return_space_ref));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_new_inline_assembly(struct kefir_mem *mem,
                                                            struct kefir_opt_code_container *code,
                                                            kefir_opt_block_id_t block_id, kefir_id_t inline_asm_id,
                                                            kefir_size_t param_count,
                                                            kefir_opt_inline_assembly_id_t *inline_asm_ref_ptr,
                                                            kefir_opt_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(inline_asm_ref_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer inline assembly identifier"));
    REQUIRE(instr_ref_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                                   "Expected valid pointer to optimizer inline assembly instruction"));

    struct kefir_opt_code_block *block = NULL;
    REQUIRE_OK(code_container_block_mutable(code, block_id, &block));

    struct kefir_opt_inline_assembly_node *inline_asm =
        KEFIR_MALLOC(mem, sizeof(struct kefir_opt_inline_assembly_node));
    REQUIRE(inline_asm != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer inline assembly node"));

    inline_asm->block_id = block_id;
    inline_asm->node_id = code->next_inline_assembly_id;
    inline_asm->inline_asm_id = inline_asm_id;
    inline_asm->parameter_count = param_count;
    inline_asm->default_jump_target = KEFIR_ID_NONE;

    if (param_count > 0) {
        inline_asm->parameters = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_inline_assembly_parameter) * param_count);
        REQUIRE_ELSE(inline_asm->parameters != NULL, {
            KEFIR_FREE(mem, inline_asm);
            return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE,
                                   "Failed to allocate optimizer inline assembly node parameters");
        });
        for (kefir_size_t i = 0; i < param_count; i++) {
            inline_asm->parameters[i].read_ref = KEFIR_ID_NONE;
            inline_asm->parameters[i].load_store_ref = KEFIR_ID_NONE;
        }
    } else {
        inline_asm->parameters = NULL;
    }

    kefir_result_t res = kefir_hashtree_init(&inline_asm->jump_targets, &kefir_hashtree_uint_ops);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, inline_asm->parameters);
        KEFIR_FREE(mem, inline_asm);
        return res;
    });

    res = kefir_hashtree_insert(mem, &code->inline_assembly, (kefir_hashtree_key_t) inline_asm->node_id,
                                (kefir_hashtree_value_t) inline_asm);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &inline_asm->jump_targets);
        KEFIR_FREE(mem, inline_asm->parameters);
        KEFIR_FREE(mem, inline_asm);
        return res;
    });

    inline_asm->siblings.prev = block->inline_assembly_nodes.tail;
    if (inline_asm->siblings.prev != KEFIR_ID_NONE) {
        struct kefir_opt_inline_assembly_node *prev_inline_asm_node = NULL;
        REQUIRE_OK(code_container_inline_assembly_mutable(code, inline_asm->siblings.prev, &prev_inline_asm_node));
        REQUIRE(prev_inline_asm_node->siblings.next == KEFIR_ID_NONE,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                "Expected optimizer inline assembly node to be the last in the block"));
        prev_inline_asm_node->siblings.next = inline_asm->node_id;
    }
    inline_asm->siblings.next = KEFIR_ID_NONE;
    block->inline_assembly_nodes.tail = inline_asm->node_id;
    if (block->inline_assembly_nodes.head == KEFIR_ID_NONE) {
        block->inline_assembly_nodes.head = inline_asm->node_id;
    }

    kefir_opt_instruction_ref_t instr_ref;
    REQUIRE_OK(kefir_opt_code_container_new_instruction(
        mem, code, block_id,
        &(struct kefir_opt_operation) {.opcode = KEFIR_OPT_OPCODE_INLINE_ASSEMBLY,
                                       .parameters.inline_asm_ref = inline_asm->node_id},
        &instr_ref));
    inline_asm->output_ref = instr_ref;

    code->next_inline_assembly_id++;
    *inline_asm_ref_ptr = inline_asm->node_id;
    *instr_ref_ptr = instr_ref;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_inline_assembly(const struct kefir_opt_code_container *code,
                                                        kefir_opt_inline_assembly_id_t inline_asm_ref,
                                                        const struct kefir_opt_inline_assembly_node **inline_asm_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(inline_asm_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer inline assembly node"));

    struct kefir_opt_inline_assembly_node *inline_asm = NULL;
    REQUIRE_OK(code_container_inline_assembly_mutable(code, inline_asm_ref, &inline_asm));
    *inline_asm_ptr = inline_asm;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_inline_assembly_get_parameter(
    const struct kefir_opt_code_container *code, kefir_opt_inline_assembly_id_t inline_asm_ref,
    kefir_size_t param_index, const struct kefir_opt_inline_assembly_parameter **param_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(param_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer inline assembly parameter"));

    struct kefir_opt_inline_assembly_node *inline_asm_node = NULL;
    REQUIRE_OK(code_container_inline_assembly_mutable(code, inline_asm_ref, &inline_asm_node));

    REQUIRE(param_index < inline_asm_node->parameter_count,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested parameter is out of inline assembly node bounds"));
    *param_ptr = &inline_asm_node->parameters[param_index];
    return KEFIR_OK;
}

static kefir_result_t inline_asm_update_used_instructions(struct kefir_mem *mem,
                                                          const struct kefir_opt_code_container *code,
                                                          kefir_opt_instruction_ref_t use_instr_ref,
                                                          const struct kefir_opt_inline_assembly_parameter *param_ptr) {
    if (param_ptr->load_store_ref != KEFIR_ID_NONE) {
        REQUIRE_OK(add_used_instructions(mem, code, use_instr_ref, param_ptr->load_store_ref));
    }
    if (param_ptr->read_ref != KEFIR_ID_NONE) {
        REQUIRE_OK(add_used_instructions(mem, code, use_instr_ref, param_ptr->read_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t inline_asm_drop_used_instructions(struct kefir_mem *mem,
                                                        const struct kefir_opt_code_container *code,
                                                        kefir_opt_instruction_ref_t use_instr_ref,
                                                        const struct kefir_opt_inline_assembly_parameter *param_ptr) {
    if (param_ptr->load_store_ref != KEFIR_ID_NONE) {
        REQUIRE_OK(remove_used_instructions(mem, code, use_instr_ref, param_ptr->load_store_ref));
    }
    if (param_ptr->read_ref != KEFIR_ID_NONE) {
        REQUIRE_OK(remove_used_instructions(mem, code, use_instr_ref, param_ptr->read_ref));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_inline_assembly_set_parameter(
    struct kefir_mem *mem, const struct kefir_opt_code_container *code, kefir_opt_inline_assembly_id_t inline_asm_ref,
    kefir_size_t param_index, const struct kefir_opt_inline_assembly_parameter *param_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(param_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer inline assembly parameter"));

    struct kefir_opt_inline_assembly_node *inline_asm_node = NULL;
    REQUIRE_OK(code_container_inline_assembly_mutable(code, inline_asm_ref, &inline_asm_node));

    REQUIRE(param_index < inline_asm_node->parameter_count,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested parameter is out of inline assembly node bounds"));
    REQUIRE_OK(inline_asm_drop_used_instructions(mem, code, inline_asm_node->output_ref,
                                                 &inline_asm_node->parameters[param_index]));
    inline_asm_node->parameters[param_index] = *param_ptr;

    REQUIRE_OK(inline_asm_update_used_instructions(mem, code, inline_asm_node->output_ref, param_ptr));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_inline_assembly_set_default_jump_target(
    const struct kefir_opt_code_container *code, kefir_opt_inline_assembly_id_t inline_asm_ref,
    kefir_opt_block_id_t target_block) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    struct kefir_opt_inline_assembly_node *inline_asm_node = NULL;
    REQUIRE_OK(code_container_inline_assembly_mutable(code, inline_asm_ref, &inline_asm_node));

    REQUIRE(inline_asm_node->default_jump_target == KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected empty default jump target of optimizer inline assembly"));

    inline_asm_node->default_jump_target = target_block;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_inline_assembly_add_jump_target(struct kefir_mem *mem,
                                                                        const struct kefir_opt_code_container *code,
                                                                        kefir_opt_inline_assembly_id_t inline_asm_ref,
                                                                        kefir_id_t target_id,
                                                                        kefir_opt_block_id_t target_block) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    struct kefir_opt_inline_assembly_node *inline_asm_node = NULL;
    REQUIRE_OK(code_container_inline_assembly_mutable(code, inline_asm_ref, &inline_asm_node));

    REQUIRE(inline_asm_node->default_jump_target != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected valid default jump target of optimizer inline assembly"));

    kefir_result_t res = kefir_hashtree_insert(mem, &inline_asm_node->jump_targets, (kefir_hashtree_key_t) target_id,
                                               (kefir_hashtree_value_t) target_block);
    if (res == KEFIR_ALREADY_EXISTS) {
        res = KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Optimizer inline assembly jump target already exists");
    }
    REQUIRE_OK(res);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_inline_assembly_jump_target(const struct kefir_opt_code_container *code,
                                                                    kefir_opt_inline_assembly_id_t inline_asm_ref,
                                                                    kefir_id_t target_id,
                                                                    kefir_opt_block_id_t *target_block_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(target_block_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid pointer to optimizer inline assembly jump target block"));

    struct kefir_opt_inline_assembly_node *inline_asm_node = NULL;
    REQUIRE_OK(code_container_inline_assembly_mutable(code, inline_asm_ref, &inline_asm_node));

    struct kefir_hashtree_node *node = NULL;
    kefir_result_t res = kefir_hashtree_at(&inline_asm_node->jump_targets, (kefir_hashtree_key_t) target_id, &node);
    if (res == KEFIR_ALREADY_EXISTS) {
        res = KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Optimizer inline assembly jump target already exists");
    }
    REQUIRE_OK(res);

    *target_block_ptr = (kefir_opt_block_id_t) node->value;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_instr_head(const struct kefir_opt_code_container *code,
                                               const struct kefir_opt_code_block *block,
                                               kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(instr_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction identifier"));

    *instr_id_ptr = block->content.head;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_instr_tail(const struct kefir_opt_code_container *code,
                                               const struct kefir_opt_code_block *block,
                                               kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(instr_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction identifier"));

    *instr_id_ptr = block->content.tail;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_instr_control_head(const struct kefir_opt_code_container *code,
                                                       const struct kefir_opt_code_block *block,
                                                       kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(instr_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction identifier"));

    *instr_id_ptr = block->control_flow.head;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_instr_control_tail(const struct kefir_opt_code_container *code,
                                                       const struct kefir_opt_code_block *block,
                                                       kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(instr_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction identifier"));

    *instr_id_ptr = block->control_flow.tail;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_phi_head(const struct kefir_opt_code_container *code,
                                             const struct kefir_opt_code_block *block, kefir_opt_phi_id_t *phi_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(phi_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer phi node identifier"));

    *phi_id_ptr = block->phi_nodes.head;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_phi_tail(const struct kefir_opt_code_container *code,
                                             const struct kefir_opt_code_block *block, kefir_opt_phi_id_t *phi_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(phi_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer phi node identifier"));

    *phi_id_ptr = block->phi_nodes.tail;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_call_head(const struct kefir_opt_code_container *code,
                                              const struct kefir_opt_code_block *block,
                                              kefir_opt_call_id_t *call_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(call_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer call node identifier"));

    *call_id_ptr = block->call_nodes.head;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_call_tail(const struct kefir_opt_code_container *code,
                                              const struct kefir_opt_code_block *block,
                                              kefir_opt_call_id_t *call_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(call_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer call node identifier"));

    *call_id_ptr = block->call_nodes.tail;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_inline_assembly_head(const struct kefir_opt_code_container *code,
                                                         const struct kefir_opt_code_block *block,
                                                         kefir_opt_inline_assembly_id_t *inline_asm_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(inline_asm_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid pointer to optimizer inline assembly node identifier"));

    *inline_asm_id_ptr = block->inline_assembly_nodes.head;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_block_inline_assembly_tail(const struct kefir_opt_code_container *code,
                                                         const struct kefir_opt_code_block *block,
                                                         kefir_opt_inline_assembly_id_t *inline_asm_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(block != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block"));
    REQUIRE(inline_asm_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid pointer to optimizer inline assembly node identifier"));

    *inline_asm_id_ptr = block->inline_assembly_nodes.tail;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_instruction_prev_sibling(const struct kefir_opt_code_container *code,
                                                  kefir_opt_instruction_ref_t instr_id,
                                                  kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(instr_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction identifier"));
    REQUIRE(instr_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction identifier"));

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(code, instr_id, &instr));
    *instr_id_ptr = instr->siblings.prev;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_instruction_next_sibling(const struct kefir_opt_code_container *code,
                                                  kefir_opt_instruction_ref_t instr_id,
                                                  kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(instr_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction identifier"));
    REQUIRE(instr_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction identifier"));

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(code, instr_id, &instr));
    *instr_id_ptr = instr->siblings.next;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_instruction_prev_control(const struct kefir_opt_code_container *code,
                                                  kefir_opt_instruction_ref_t instr_id,
                                                  kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(instr_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction identifier"));
    REQUIRE(instr_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction identifier"));

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(code, instr_id, &instr));
    *instr_id_ptr = instr->control_flow.prev;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_instruction_next_control(const struct kefir_opt_code_container *code,
                                                  kefir_opt_instruction_ref_t instr_id,
                                                  kefir_opt_instruction_ref_t *instr_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(instr_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer instruction identifier"));
    REQUIRE(instr_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction identifier"));

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(code, instr_id, &instr));
    *instr_id_ptr = instr->control_flow.next;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_phi_prev_sibling(const struct kefir_opt_code_container *code, kefir_opt_phi_id_t phi_id,
                                          kefir_opt_phi_id_t *phi_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(phi_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer phi node identifier"));
    REQUIRE(phi_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer phi node identifier"));

    struct kefir_opt_phi_node *phi = NULL;
    REQUIRE_OK(code_container_phi_mutable(code, phi_id, &phi));
    *phi_id_ptr = phi->siblings.prev;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_phi_next_sibling(const struct kefir_opt_code_container *code, kefir_opt_phi_id_t phi_id,
                                          kefir_opt_phi_id_t *phi_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(phi_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer phi node identifier"));
    REQUIRE(phi_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer phi node identifier"));

    struct kefir_opt_phi_node *phi = NULL;
    REQUIRE_OK(code_container_phi_mutable(code, phi_id, &phi));
    *phi_id_ptr = phi->siblings.next;

    return KEFIR_OK;
}

kefir_result_t kefir_opt_call_prev_sibling(const struct kefir_opt_code_container *code, kefir_opt_call_id_t call_id,
                                           kefir_opt_call_id_t *call_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(call_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer call node identifier"));
    REQUIRE(call_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer call node identifier"));

    struct kefir_opt_call_node *call;
    REQUIRE_OK(code_container_call_mutable(code, call_id, &call));
    *call_id_ptr = call->siblings.prev;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_call_next_sibling(const struct kefir_opt_code_container *code, kefir_opt_call_id_t call_id,
                                           kefir_opt_call_id_t *call_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(call_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer call node identifier"));
    REQUIRE(call_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer call node identifier"));

    struct kefir_opt_call_node *call;
    REQUIRE_OK(code_container_call_mutable(code, call_id, &call));
    *call_id_ptr = call->siblings.next;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_inline_assembly_prev_sibling(const struct kefir_opt_code_container *code,
                                                      kefir_opt_inline_assembly_id_t inline_asm_id,
                                                      kefir_opt_inline_assembly_id_t *inline_asm_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(inline_asm_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer inline assembly node identifier"));
    REQUIRE(inline_asm_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid pointer to optimizer inline assembly node identifier"));

    struct kefir_opt_inline_assembly_node *inline_asm;
    REQUIRE_OK(code_container_inline_assembly_mutable(code, inline_asm_id, &inline_asm));
    *inline_asm_id_ptr = inline_asm->siblings.prev;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_inline_assembly_next_sibling(const struct kefir_opt_code_container *code,
                                                      kefir_opt_inline_assembly_id_t inline_asm_id,
                                                      kefir_opt_inline_assembly_id_t *inline_asm_id_ptr) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(inline_asm_id != KEFIR_ID_NONE,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer inline assembly node identifier"));
    REQUIRE(inline_asm_id_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected valid pointer to optimizer inline assembly node identifier"));

    struct kefir_opt_inline_assembly_node *inline_asm;
    REQUIRE_OK(code_container_inline_assembly_mutable(code, inline_asm_id, &inline_asm));
    *inline_asm_id_ptr = inline_asm->siblings.next;
    return KEFIR_OK;
}

static kefir_result_t verify_dead_uses(struct kefir_opt_code_container *code,
                                       const struct kefir_opt_code_container_dead_code_index *index,
                                       kefir_opt_instruction_ref_t instr_ref) {
    struct kefir_opt_instruction *used_instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(code, instr_ref, &used_instr));
    struct kefir_hashtreeset_iterator iter;
    kefir_result_t res;
    for (res = kefir_hashtreeset_iter(&used_instr->uses.instruction, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, use_instr_ref, iter.entry);
        kefir_bool_t use_alive;
        REQUIRE_OK(index->is_instruction_alive(use_instr_ref, &use_alive, index->payload));
        REQUIRE(!use_alive, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Instruction with alive dependents cannot be dead"));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t verify_dead_block(struct kefir_opt_code_container *code,
                                        const struct kefir_opt_code_container_dead_code_index *index,
                                        struct kefir_opt_code_block *block) {
    UNUSED(index);
    kefir_result_t res;
    kefir_opt_instruction_ref_t instr_ref;
    for (res = kefir_opt_code_block_instr_head(code, block, &instr_ref); res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
         res = kefir_opt_instruction_next_sibling(code, instr_ref, &instr_ref)) {
        REQUIRE_OK(verify_dead_uses(code, index, instr_ref));
    }
    REQUIRE_OK(res);

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_drop_dead_code(struct kefir_mem *mem, struct kefir_opt_code_container *code,
                                                       const struct kefir_opt_code_container_dead_code_index *index) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));
    REQUIRE(index != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer dead code index"));

    for (kefir_opt_block_id_t block_id = 0; block_id < code->blocks_length; block_id++) {
        struct kefir_opt_code_block *block = NULL;
        REQUIRE_OK(code_container_block_mutable(code, block_id, &block));

        kefir_opt_phi_id_t phi_ref;
        kefir_result_t res;
        for (res = kefir_opt_code_block_phi_head(code, block, &phi_ref); res == KEFIR_OK && phi_ref != KEFIR_ID_NONE;
             res = kefir_opt_phi_next_sibling(code, phi_ref, &phi_ref)) {
            const struct kefir_opt_phi_node *phi_node;
            REQUIRE_OK(kefir_opt_code_container_phi(code, phi_ref, &phi_node));
            struct kefir_hashtree_node_iterator iter;
            for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&phi_node->links, &iter); node != NULL;) {
                ASSIGN_DECL_CAST(kefir_opt_block_id_t, link_block_id, node->key);
                node = kefir_hashtree_next(&iter);

                kefir_bool_t reachable, predecessor;
                REQUIRE_OK(index->is_block_alive(link_block_id, &reachable, index->payload));
                REQUIRE_OK(index->is_block_predecessor(link_block_id, block_id, &predecessor, index->payload));
                if (!reachable || !predecessor) {
                    REQUIRE_OK(kefir_opt_code_container_phi_drop_link(mem, code, phi_ref, link_block_id));
                }
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    for (kefir_opt_block_id_t block_id = 0; block_id < code->blocks_length; block_id++) {
        struct kefir_opt_code_block *block = NULL;
        REQUIRE_OK(code_container_block_mutable(code, block_id, &block));

        kefir_bool_t is_alive;
        REQUIRE_OK(index->is_block_alive(block_id, &is_alive, index->payload));
        if (!is_alive) {
            REQUIRE_OK(verify_dead_block(code, index, block));
            REQUIRE_OK(drop_block_impl(mem, code, block_id, false));
        }
    }

    for (kefir_opt_instruction_ref_t instr_ref = 0; instr_ref < code->length; instr_ref++) {
        struct kefir_opt_instruction *instr = &code->code[instr_ref];
        if (instr->block_id == KEFIR_ID_NONE) {
            continue;
        }

        kefir_bool_t is_alive;
        REQUIRE_OK(index->is_instruction_alive(instr_ref, &is_alive, index->payload));
        if (!is_alive) {
            REQUIRE_OK(verify_dead_uses(code, index, instr_ref));
            kefir_bool_t is_control_flow;
            REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(code, instr_ref, &is_control_flow));
            if (is_control_flow) {
                REQUIRE_OK(kefir_opt_code_container_drop_control(code, instr_ref));
            }
            REQUIRE_OK(drop_instr_impl(mem, code, instr_ref, false));
        }
    }

    return KEFIR_OK;
}

struct kefir_opt_code_block *kefir_opt_code_container_iter(const struct kefir_opt_code_container *code,
                                                           struct kefir_opt_code_container_iterator *iter) {
    REQUIRE(code != NULL, NULL);
    REQUIRE(iter != NULL, NULL);

    iter->code = code;
    iter->block_id = 0;
    if (iter->block_id < code->blocks_length) {
        return &code->blocks[iter->block_id];
    } else {
        return NULL;
    }
}

struct kefir_opt_code_block *kefir_opt_code_container_next(struct kefir_opt_code_container_iterator *iter) {
    REQUIRE(iter != NULL, NULL);

    iter->block_id++;
    if (iter->block_id < iter->code->blocks_length) {
        return &iter->code->blocks[iter->block_id];
    } else {
        return NULL;
    }
}

kefir_result_t kefir_opt_phi_node_link_iter(const struct kefir_opt_phi_node *phi_node,
                                            struct kefir_opt_phi_node_link_iterator *iter,
                                            kefir_opt_block_id_t *block_id_ptr,
                                            kefir_opt_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(phi_node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer phi node"));
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer phi node link iterator"));

    const struct kefir_hashtree_node *node = kefir_hashtree_iter(&phi_node->links, &iter->iter);
    REQUIRE(node != NULL, KEFIR_ITERATOR_END);
    ASSIGN_PTR(block_id_ptr, (kefir_opt_block_id_t) node->key);
    ASSIGN_PTR(instr_ref_ptr, (kefir_opt_instruction_ref_t) node->value);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_phi_node_link_next(struct kefir_opt_phi_node_link_iterator *iter,
                                            kefir_opt_block_id_t *block_id_ptr,
                                            kefir_opt_instruction_ref_t *instr_ref_ptr) {
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer phi node link iterator"));

    const struct kefir_hashtree_node *node = kefir_hashtree_next(&iter->iter);
    REQUIRE(node != NULL, KEFIR_ITERATOR_END);
    ASSIGN_PTR(block_id_ptr, (kefir_opt_block_id_t) node->key);
    ASSIGN_PTR(instr_ref_ptr, (kefir_opt_instruction_ref_t) node->value);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_instruction_use_instr_iter(const struct kefir_opt_code_container *code,
                                                                   kefir_opt_instruction_ref_t instr_ref,
                                                                   struct kefir_opt_instruction_use_iterator *iter) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction use iterator"));

    struct kefir_opt_instruction *used_instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(code, instr_ref, &used_instr));

    kefir_result_t res = kefir_hashtreeset_iter(&used_instr->uses.instruction, &iter->iter);
    REQUIRE_OK(res);
    iter->use_instr_ref = (kefir_opt_instruction_ref_t) iter->iter.entry;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_instruction_use_next(struct kefir_opt_instruction_use_iterator *iter) {
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer instruction use iterator"));

    kefir_result_t res = kefir_hashtreeset_next(&iter->iter);
    REQUIRE_OK(res);
    iter->use_instr_ref = (kefir_opt_instruction_ref_t) iter->iter.entry;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_instruction_replace_control_flow_target(
    struct kefir_opt_code_container *code, kefir_opt_instruction_ref_t instr_ref,
    kefir_opt_block_id_t current_target_block_id, kefir_opt_block_id_t desired_target_block_id) {
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));

    struct kefir_opt_instruction *instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(code, instr_ref, &instr));

    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_BRANCH:
        case KEFIR_OPT_OPCODE_BRANCH_COMPARE:
            if (instr->operation.parameters.branch.alternative_block == current_target_block_id) {
                instr->operation.parameters.branch.alternative_block = desired_target_block_id;
            }
            // Fallthrough

        case KEFIR_OPT_OPCODE_JUMP:
            if (instr->operation.parameters.branch.target_block == current_target_block_id) {
                instr->operation.parameters.branch.target_block = desired_target_block_id;
            }
            break;

        case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY: {
            struct kefir_opt_inline_assembly_node *inline_asm;
            REQUIRE_OK(
                code_container_inline_assembly_mutable(code, instr->operation.parameters.inline_asm_ref, &inline_asm));

            if (inline_asm->default_jump_target == current_target_block_id) {
                inline_asm->default_jump_target = desired_target_block_id;
            }

            struct kefir_hashtree_node_iterator iter;
            for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->jump_targets, &iter); node != NULL;
                 node = kefir_hashtree_next(&iter)) {
                if (current_target_block_id == (kefir_opt_block_id_t) node->value) {
                    node->value = (kefir_hashtree_value_t) desired_target_block_id;
                }
            }
        } break;

        case KEFIR_OPT_OPCODE_BLOCK_LABEL:
            if (instr->operation.parameters.imm.block_ref == current_target_block_id) {
                instr->operation.parameters.imm.block_ref = desired_target_block_id;
            }
            break;

        case KEFIR_OPT_OPCODE_PHI:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                                   "Unable to safely replace control flow references in phi node");

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                                   "Specified instruction does not have control flow references");
    }

    return KEFIR_OK;
}

#define REPLACE_REF(_ref_ptr, _to, _from) \
    do {                                  \
        if (*(_ref_ptr) == (_from)) {     \
            *(_ref_ptr) = (_to);          \
        }                                 \
    } while (0)

static kefir_result_t replace_references_branch(struct kefir_opt_instruction *instr, kefir_opt_instruction_ref_t to_ref,
                                                kefir_opt_instruction_ref_t from_ref) {
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_BRANCH) {
        REPLACE_REF(&instr->operation.parameters.branch.condition_ref, to_ref, from_ref);
    }
    return KEFIR_OK;
}

static kefir_result_t replace_references_branch_compare(struct kefir_opt_instruction *instr,
                                                        kefir_opt_instruction_ref_t to_ref,
                                                        kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[0], to_ref, from_ref);
    REPLACE_REF(&instr->operation.parameters.refs[1], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_ref1(struct kefir_opt_instruction *instr, kefir_opt_instruction_ref_t to_ref,
                                              kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[0], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_bitint_ref1(struct kefir_opt_instruction *instr,
                                                     kefir_opt_instruction_ref_t to_ref,
                                                     kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[0], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_bitint_ref2(struct kefir_opt_instruction *instr,
                                                     kefir_opt_instruction_ref_t to_ref,
                                                     kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[0], to_ref, from_ref);
    REPLACE_REF(&instr->operation.parameters.refs[1], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_bitint2_ref1(struct kefir_opt_instruction *instr,
                                                      kefir_opt_instruction_ref_t to_ref,
                                                      kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[0], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_ref2(struct kefir_opt_instruction *instr, kefir_opt_instruction_ref_t to_ref,
                                              kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[0], to_ref, from_ref);
    REPLACE_REF(&instr->operation.parameters.refs[1], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_ref3_cond(struct kefir_opt_instruction *instr,
                                                   kefir_opt_instruction_ref_t to_ref,
                                                   kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[0], to_ref, from_ref);
    REPLACE_REF(&instr->operation.parameters.refs[1], to_ref, from_ref);
    REPLACE_REF(&instr->operation.parameters.refs[2], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_ref4_compare(struct kefir_opt_instruction *instr,
                                                      kefir_opt_instruction_ref_t to_ref,
                                                      kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[0], to_ref, from_ref);
    REPLACE_REF(&instr->operation.parameters.refs[1], to_ref, from_ref);
    REPLACE_REF(&instr->operation.parameters.refs[2], to_ref, from_ref);
    REPLACE_REF(&instr->operation.parameters.refs[3], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_load_mem(struct kefir_opt_instruction *instr,
                                                  kefir_opt_instruction_ref_t to_ref,
                                                  kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_bitint_load(struct kefir_opt_instruction *instr,
                                                     kefir_opt_instruction_ref_t to_ref,
                                                     kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_tmpobj(struct kefir_opt_instruction *instr, kefir_opt_instruction_ref_t to_ref,
                                                kefir_opt_instruction_ref_t from_ref) {
    UNUSED(instr);
    UNUSED(to_ref);
    UNUSED(from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_bitint_store(struct kefir_opt_instruction *instr,
                                                      kefir_opt_instruction_ref_t to_ref,
                                                      kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF], to_ref, from_ref);
    REPLACE_REF(&instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_bitint_atomic(struct kefir_opt_instruction *instr,
                                                       kefir_opt_instruction_ref_t to_ref,
                                                       kefir_opt_instruction_ref_t from_ref) {
    switch (instr->operation.opcode) {
        case KEFIR_OPT_OPCODE_BITINT_ATOMIC_LOAD:
            REPLACE_REF(&instr->operation.parameters.refs[0], to_ref, from_ref);
            break;

        case KEFIR_OPT_OPCODE_BITINT_ATOMIC_STORE:
            REPLACE_REF(&instr->operation.parameters.refs[0], to_ref, from_ref);
            REPLACE_REF(&instr->operation.parameters.refs[1], to_ref, from_ref);
            break;

        case KEFIR_OPT_OPCODE_BITINT_ATOMIC_COMPARE_EXCHANGE:
            REPLACE_REF(&instr->operation.parameters.refs[0], to_ref, from_ref);
            REPLACE_REF(&instr->operation.parameters.refs[1], to_ref, from_ref);
            REPLACE_REF(&instr->operation.parameters.refs[2], to_ref, from_ref);
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected optimizer instruction opcode");
    }
    return KEFIR_OK;
}

static kefir_result_t replace_references_bitint_bitfield(struct kefir_opt_instruction *instr,
                                                         kefir_opt_instruction_ref_t to_ref,
                                                         kefir_opt_instruction_ref_t from_ref) {
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_BITINT_INSERT) {
        REPLACE_REF(&instr->operation.parameters.refs[0], to_ref, from_ref);
        REPLACE_REF(&instr->operation.parameters.refs[1], to_ref, from_ref);
    } else {
        REPLACE_REF(&instr->operation.parameters.refs[0], to_ref, from_ref);
    }
    return KEFIR_OK;
}
static kefir_result_t replace_references_store_mem(struct kefir_opt_instruction *instr,
                                                   kefir_opt_instruction_ref_t to_ref,
                                                   kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF], to_ref, from_ref);
    REPLACE_REF(&instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_bitfield(struct kefir_opt_instruction *instr,
                                                  kefir_opt_instruction_ref_t to_ref,
                                                  kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[KEFIR_OPT_BITFIELD_BASE_REF], to_ref, from_ref);
    REPLACE_REF(&instr->operation.parameters.refs[KEFIR_OPT_BITFIELD_VALUE_REF], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_type(struct kefir_opt_instruction *instr, kefir_opt_instruction_ref_t to_ref,
                                              kefir_opt_instruction_ref_t from_ref) {
    UNUSED(instr);
    UNUSED(to_ref);
    UNUSED(from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_compare_ref2(struct kefir_opt_instruction *instr,
                                                      kefir_opt_instruction_ref_t to_ref,
                                                      kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[0], to_ref, from_ref);
    REPLACE_REF(&instr->operation.parameters.refs[1], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_typed_ref2(struct kefir_opt_instruction *instr,
                                                    kefir_opt_instruction_ref_t to_ref,
                                                    kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[0], to_ref, from_ref);
    REPLACE_REF(&instr->operation.parameters.refs[1], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_ref_offset(struct kefir_opt_instruction *instr,
                                                    kefir_opt_instruction_ref_t to_ref,
                                                    kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[0], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_overflow_arith(struct kefir_opt_instruction *instr,
                                                        kefir_opt_instruction_ref_t to_ref,
                                                        kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[0], to_ref, from_ref);
    REPLACE_REF(&instr->operation.parameters.refs[1], to_ref, from_ref);
    REPLACE_REF(&instr->operation.parameters.refs[2], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_atomic_op(struct kefir_opt_instruction *instr,
                                                   kefir_opt_instruction_ref_t to_ref,
                                                   kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[0], to_ref, from_ref);
    REPLACE_REF(&instr->operation.parameters.refs[1], to_ref, from_ref);
    REPLACE_REF(&instr->operation.parameters.refs[2], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_stack_alloc(struct kefir_opt_instruction *instr,
                                                     kefir_opt_instruction_ref_t to_ref,
                                                     kefir_opt_instruction_ref_t from_ref) {
    REPLACE_REF(&instr->operation.parameters.refs[KEFIR_OPT_STACK_ALLOCATION_SIZE_REF], to_ref, from_ref);
    REPLACE_REF(&instr->operation.parameters.refs[KEFIR_OPT_STACK_ALLOCATION_ALIGNMENT_REF], to_ref, from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_call_ref(struct kefir_opt_instruction *instr,
                                                  kefir_opt_instruction_ref_t to_ref,
                                                  kefir_opt_instruction_ref_t from_ref) {
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_INVOKE_VIRTUAL ||
        instr->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL) {
        REPLACE_REF(&instr->operation.parameters.function_call.indirect_ref, to_ref, from_ref);
    }
    return KEFIR_OK;
}

static kefir_result_t replace_references_index(struct kefir_opt_instruction *instr, kefir_opt_instruction_ref_t to_ref,
                                               kefir_opt_instruction_ref_t from_ref) {
    UNUSED(instr);
    UNUSED(to_ref);
    UNUSED(from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_variable(struct kefir_opt_instruction *instr,
                                                  kefir_opt_instruction_ref_t to_ref,
                                                  kefir_opt_instruction_ref_t from_ref) {
    UNUSED(instr);
    UNUSED(to_ref);
    UNUSED(from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_phi_ref(struct kefir_opt_instruction *instr,
                                                 kefir_opt_instruction_ref_t to_ref,
                                                 kefir_opt_instruction_ref_t from_ref) {
    UNUSED(instr);
    UNUSED(to_ref);
    UNUSED(from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_inline_asm(struct kefir_opt_instruction *instr,
                                                    kefir_opt_instruction_ref_t to_ref,
                                                    kefir_opt_instruction_ref_t from_ref) {
    UNUSED(instr);
    UNUSED(to_ref);
    UNUSED(from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_immediate(struct kefir_opt_instruction *instr,
                                                   kefir_opt_instruction_ref_t to_ref,
                                                   kefir_opt_instruction_ref_t from_ref) {
    UNUSED(instr);
    UNUSED(to_ref);
    UNUSED(from_ref);
    return KEFIR_OK;
}

static kefir_result_t replace_references_none(struct kefir_opt_instruction *instr, kefir_opt_instruction_ref_t to_ref,
                                              kefir_opt_instruction_ref_t from_ref) {
    UNUSED(instr);
    UNUSED(to_ref);
    UNUSED(from_ref);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_container_replace_references(struct kefir_mem *mem,
                                                           const struct kefir_opt_code_container *code,
                                                           kefir_opt_instruction_ref_t to_ref,
                                                           kefir_opt_instruction_ref_t from_ref) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code container"));

    struct kefir_opt_instruction *to_instr = NULL;
    struct kefir_opt_instruction *from_instr = NULL;
    REQUIRE_OK(code_container_instr_mutable(code, to_ref, &to_instr));
    REQUIRE_OK(code_container_instr_mutable(code, from_ref, &from_instr));

    kefir_result_t res;
    struct kefir_opt_instruction *instr = NULL;

    struct kefir_hashtreeset_iterator user_iter;
    for (res = kefir_hashtreeset_iter(&from_instr->uses.instruction, &user_iter); res == KEFIR_OK;) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, user_ref, user_iter.entry);
        res = kefir_hashtreeset_next(&user_iter);

        instr = &code->code[user_ref];
        REQUIRE(instr->block_id != KEFIR_ID_NONE,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected instruction block identifier"));

        if (instr->operation.opcode == KEFIR_OPT_OPCODE_PHI) {
            const struct kefir_opt_phi_node *phi = &code->phi_nodes[instr->operation.parameters.phi_ref];
            REQUIRE(phi->block_id != KEFIR_ID_NONE,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected phi block identifier"));

            struct kefir_hashtree_node *node;
            REQUIRE_OK(kefir_hashtree_min(&phi->links, &node));
            for (; node != NULL; node = kefir_hashtree_next_node(&phi->links, node)) {
                if (node->value == (kefir_hashtree_value_t) from_ref) {
                    node->value = (kefir_hashtree_value_t) to_ref;
                }
            }
        } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INVOKE ||
                   instr->operation.opcode == KEFIR_OPT_OPCODE_INVOKE_VIRTUAL ||
                   instr->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE ||
                   instr->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL) {
            struct kefir_opt_call_node *call = NULL;
            REQUIRE_OK(code_container_call_mutable(code, instr->operation.parameters.function_call.call_ref, &call));
            for (kefir_size_t i = 0; i < call->argument_count; i++) {
                REPLACE_REF(&call->arguments[i], to_ref, from_ref);
            }
            REPLACE_REF(&call->return_space, to_ref, from_ref);
        } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_INLINE_ASSEMBLY) {
            struct kefir_opt_inline_assembly_node *inline_asm = NULL;
            REQUIRE_OK(
                code_container_inline_assembly_mutable(code, instr->operation.parameters.inline_asm_ref, &inline_asm));

            for (kefir_size_t i = 0; i < inline_asm->parameter_count; i++) {
                REPLACE_REF(&inline_asm->parameters[i].load_store_ref, to_ref, from_ref);
                REPLACE_REF(&inline_asm->parameters[i].read_ref, to_ref, from_ref);
            }
        }

        switch (instr->operation.opcode) {
#define OPCODE_DEF(_id, _symbolic, _class)                                \
    case KEFIR_OPT_OPCODE_##_id:                                          \
        REQUIRE_OK(replace_references_##_class(instr, to_ref, from_ref)); \
        break;

            KEFIR_OPTIMIZER_OPCODE_DEFS(OPCODE_DEF, )
#undef OPCODE_DEF
        }

        REQUIRE_OK(remove_used_instructions(mem, code, user_ref, from_ref));
        REQUIRE_OK(add_used_instructions(mem, code, user_ref, to_ref));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

#undef REPLACE_REF
