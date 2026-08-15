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

#include "kefir/optimizer/variable_scope.h"
#include "kefir/core/basic-types.h"
#include "kefir/core/error.h"
#include "kefir/core/hashset.h"
#include "kefir/core/hashtree.h"
#include "kefir/core/mem.h"
#include "kefir/core/util.h"
#include "kefir/optimizer/code.h"
#include <string.h>

static kefir_result_t free_block_scopes(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                        kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));

    ASSIGN_DECL_CAST(struct kefir_opt_code_block_variable_scopes *, scopes, value);
    REQUIRE_OK(kefir_hashset_free(mem, &scopes->scopes));
    KEFIR_FREE(mem, scopes);
    return KEFIR_OK;
}

static kefir_result_t free_scope_variables(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                           kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));

    ASSIGN_DECL_CAST(struct kefir_opt_code_variable_scope *, variables, value);
    REQUIRE_OK(kefir_hashset_free(mem, &variables->blocks));
    REQUIRE_OK(kefir_hashset_free(mem, &variables->allocations));
    KEFIR_FREE(mem, variables);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_variable_scopes_init(struct kefir_opt_code_variable_scopes *scopes) {
    REQUIRE(scopes != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer variables scopes"));

    REQUIRE_OK(kefir_hashtree_init(&scopes->scopes, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&scopes->scopes, free_scope_variables, NULL));
    REQUIRE_OK(kefir_hashset_init(&scopes->global_scopes, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&scopes->block_scopes, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&scopes->block_scopes, free_block_scopes, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_variable_scopes_free(struct kefir_mem *mem,
                                                   struct kefir_opt_code_variable_scopes *scopes) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(scopes != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer variables scopes"));

    REQUIRE_OK(kefir_hashtree_free(mem, &scopes->scopes));
    REQUIRE_OK(kefir_hashset_free(mem, &scopes->global_scopes));
    REQUIRE_OK(kefir_hashtree_free(mem, &scopes->block_scopes));
    return KEFIR_OK;
}

static kefir_result_t get_scope(struct kefir_mem *mem, struct kefir_opt_code_variable_scopes *scopes, kefir_opt_instruction_ref_t scope_ref,
    struct kefir_opt_code_variable_scope **scope_ptr) {
    struct kefir_opt_code_variable_scope *scope = NULL;
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&scopes->scopes,
                            (kefir_hashtree_key_t) scope_ref, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        scope = (struct kefir_opt_code_variable_scope *) node->value;
    } else {
        scope = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_variable_scope));
        REQUIRE(scope != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer scope variables"));
        scope->global = false;
        res = kefir_hashset_init(&scope->allocations, &kefir_hashtable_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashset_init(&scope->blocks, &kefir_hashtable_uint_ops));
        REQUIRE_CHAIN(&res,
                        kefir_hashtree_insert(mem, &scopes->scopes,
                                            (kefir_hashtree_key_t) scope_ref,
                                            (kefir_hashtree_value_t) scope));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, scope);
            return res;
        });
    }

    *scope_ptr = scope;
    return KEFIR_OK;
}

static kefir_result_t process_scope(struct kefir_mem *mem, const struct kefir_opt_code_container *code, struct kefir_opt_code_variable_scopes *scopes,
                                    kefir_opt_block_id_t block_ref, kefir_opt_instruction_ref_t scope_ref) {    
    REQUIRE(!kefir_hashset_has(&scopes->global_scopes, (kefir_hashset_key_t) scope_ref), KEFIR_OK);

    kefir_bool_t has_liveness_marks = false;
    kefir_result_t res;
    struct kefir_opt_instruction_use_iterator use_iter;
    for (res = kefir_opt_code_container_instruction_use_instr_iter(code, scope_ref, &use_iter);
         res == KEFIR_OK && !has_liveness_marks; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_instruction *use_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(code, use_iter.use_instr_ref, &use_instr));
        if (use_instr->operation.opcode == KEFIR_OPT_OPCODE_LOCAL_LIFETIME_MARK) {
            has_liveness_marks = true;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    struct kefir_opt_code_variable_scope *scope = NULL;
    REQUIRE_OK(get_scope(mem, scopes, scope_ref, &scope));

    if (!has_liveness_marks) {
        REQUIRE_OK(kefir_hashset_add(mem, &scopes->global_scopes, (kefir_hashset_key_t) scope_ref));
        scope->global = true;
        return KEFIR_OK;
    }

    REQUIRE_OK(kefir_hashset_add(mem, &scope->blocks, (kefir_hashset_key_t) block_ref));

    struct kefir_opt_code_block_variable_scopes *block_scopes = NULL;

    struct kefir_hashtree_node *node;
    res = kefir_hashtree_at(&scopes->block_scopes, (kefir_hashtree_key_t) block_ref, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        block_scopes = (struct kefir_opt_code_block_variable_scopes *) node->value;
    } else {
        block_scopes = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_block_variable_scopes));
        REQUIRE(block_scopes != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer block variable scopes"));
        res = kefir_hashset_init(&block_scopes->scopes, &kefir_hashtable_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashtree_insert(mem, &scopes->block_scopes, (kefir_hashtree_key_t) block_ref,
                                                  (kefir_hashtree_value_t) block_scopes));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, block_scopes);
            return res;
        });
    }
    REQUIRE_OK(kefir_hashset_add(mem, &block_scopes->scopes, (kefir_hashset_key_t) scope_ref));
    return KEFIR_OK;
}

static kefir_result_t scopes_build_impl(struct kefir_mem *mem, struct kefir_opt_code_variable_scopes *scopes,
                                        const struct kefir_opt_code_liveness *liveness) {
    kefir_result_t res;
    kefir_size_t block_count = kefir_opt_code_container_block_count(liveness->code);
    for (kefir_opt_block_id_t block_id = 0; block_id < block_count; block_id++) {
        kefir_hashset_key_t entry;
        struct kefir_hashset_iterator iter;
        for (res = kefir_hashset_iter(&liveness->blocks[block_id].alive_instr, &iter, &entry); res == KEFIR_OK;
             res = kefir_hashset_next(&iter, &entry)) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, entry);

            const struct kefir_opt_instruction *instr;
            REQUIRE_OK(kefir_opt_code_container_instr(liveness->code, instr_ref, &instr));
            if (instr->operation.opcode == KEFIR_OPT_OPCODE_LOCAL_SCOPE) {
                REQUIRE_OK(process_scope(mem, liveness->code, scopes, block_id, instr_ref));
            } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) {
                struct kefir_opt_code_variable_scope *scope = NULL;
                REQUIRE_OK(get_scope(mem, scopes, instr->operation.parameters.refs[0], &scope));
                REQUIRE_OK(kefir_hashset_add(mem, &scope->allocations, (kefir_hashset_key_t) instr_ref));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    kefir_bool_t has_unscoped_vars = kefir_hashtree_has(&scopes->scopes, (kefir_hashtree_key_t) KEFIR_ID_NONE);
    if (has_unscoped_vars) {
        REQUIRE_OK(kefir_hashset_add(mem, &scopes->global_scopes, (kefir_hashset_key_t) KEFIR_ID_NONE));
    };

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_variable_scopes_build(struct kefir_mem *mem,
                                                    struct kefir_opt_code_variable_scopes *scopes,
                                                    const struct kefir_opt_code_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(scopes != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer variables scopes"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code liveness"));

    REQUIRE_OK(scopes_build_impl(mem, scopes, liveness));
    return KEFIR_OK;
}

static kefir_result_t interference_enumerate_impl(const struct kefir_opt_code_variable_scopes *scopes, kefir_bool_t *visited, kefir_opt_instruction_ref_t scope_ref, kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *), void *payload) {
    if (kefir_hashset_has(&scopes->global_scopes, (kefir_hashset_key_t) scope_ref)) {
        struct kefir_hashtree_node_iterator iter;
        for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&scopes->scopes, &iter);
            node != NULL;
            node = kefir_hashtree_next(&iter)) {
            REQUIRE_OK(callback((kefir_opt_instruction_ref_t) node->key, payload));
        }
    } else {
        const struct kefir_opt_code_variable_scope *scope = NULL;
        struct kefir_hashtree_node *node;
        kefir_result_t res = kefir_hashtree_at(&scopes->scopes, (kefir_hashtree_key_t) scope_ref, &node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            scope = (const struct kefir_opt_code_variable_scope *) node->value;
        } else {
            return KEFIR_OK;
        }

        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t key;
        for (res = kefir_hashset_iter(&scopes->global_scopes, &iter, &key);
            res == KEFIR_OK;
            res = kefir_hashset_next(&iter, &key)) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, other_scope_ref, key);
            if (other_scope_ref == KEFIR_ID_NONE && other_scope_ref != scope_ref) {
                REQUIRE_OK(callback(other_scope_ref, payload));
            } else if (other_scope_ref != scope_ref && !visited[KEFIR_OPT_INSTR_REF_INDEX_OF(other_scope_ref)]) {
                visited[KEFIR_OPT_INSTR_REF_INDEX_OF(other_scope_ref)] = true;
                REQUIRE_OK(callback(other_scope_ref, payload));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        for (res = kefir_hashset_iter(&scope->blocks, &iter, &key);
            res == KEFIR_OK;
            res = kefir_hashset_next(&iter, &key)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, key);

            res = kefir_hashtree_at(&scopes->block_scopes, (kefir_hashtree_key_t) block_id, &node);
            if (res == KEFIR_NOT_FOUND) {
                continue;
            }
            REQUIRE_OK(res);
            ASSIGN_DECL_CAST(const struct kefir_opt_code_block_variable_scopes *, block_scopes, node->value);

            struct kefir_hashset_iterator iter2;
            for (res = kefir_hashset_iter(&block_scopes->scopes, &iter2, &key);
                res == KEFIR_OK;
                res = kefir_hashset_next(&iter2, &key)) {
                ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, other_scope_ref, key);
                if (other_scope_ref != scope_ref && !visited[KEFIR_OPT_INSTR_REF_INDEX_OF(other_scope_ref)] && !kefir_hashset_has(&scopes->global_scopes, (kefir_hashset_key_t) other_scope_ref)) {
                    visited[KEFIR_OPT_INSTR_REF_INDEX_OF(other_scope_ref)] = true;
                    REQUIRE_OK(callback(other_scope_ref, payload));
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_variable_scope_interference_enumerate(struct kefir_mem *mem, const struct kefir_opt_code_variable_scopes *scopes, kefir_opt_instruction_ref_t scope_ref, kefir_result_t (*callback)(kefir_opt_instruction_ref_t, void *), void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(scopes != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer variables scopes"));
    REQUIRE(callback != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer variables scope enumerator callback"));

    struct kefir_hashtree_node *node;
    REQUIRE_OK(kefir_hashtree_max(&scopes->scopes, &node));
    kefir_size_t visited_size = 1;
    if (node != NULL) {
        visited_size = ((kefir_size_t) node->key) + 1;
    }
    kefir_bool_t *visited = KEFIR_MALLOC(mem, sizeof(kefir_bool_t) * visited_size);
    REQUIRE(visited != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate visited set"));
    memset(visited, 0, sizeof(kefir_bool_t) * visited_size);

    kefir_result_t res = interference_enumerate_impl(scopes, visited, scope_ref, callback, payload);
    KEFIR_FREE(mem, visited);
    REQUIRE_OK(res);
    return KEFIR_OK;
}
