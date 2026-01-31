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
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct block_variable_scopes {
    struct kefir_hashset scopes;
};

struct state {
    struct kefir_hashset global_scopes;
    struct kefir_hashtree block_scopes;
};

static kefir_result_t free_block_scopes(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));

    ASSIGN_DECL_CAST(struct block_variable_scopes *, scopes,
        value);
    REQUIRE_OK(kefir_hashset_free(mem, &scopes->scopes));
    KEFIR_FREE(mem, scopes);
    return KEFIR_OK;
}

static kefir_result_t free_scope_variables(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));

    ASSIGN_DECL_CAST(struct kefir_opt_code_scope_variables *, variables,
        value);
    REQUIRE_OK(kefir_hashset_free(mem, &variables->allocations));
    KEFIR_FREE(mem, variables);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_variable_scopes_init(struct kefir_opt_code_variable_scopes *scopes) {
    REQUIRE(scopes != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer variables scopes"));

    REQUIRE_OK(kefir_graph_init(&scopes->scope_interference));
    REQUIRE_OK(kefir_hashtree_init(&scopes->scope_variables, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&scopes->scope_variables, free_scope_variables, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_variable_scopes_free(struct kefir_mem *mem, struct kefir_opt_code_variable_scopes *scopes) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(scopes != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer variables scopes"));

    REQUIRE_OK(kefir_graph_free(mem, &scopes->scope_interference));
    REQUIRE_OK(kefir_hashtree_free(mem, &scopes->scope_variables));
    return KEFIR_OK;
}

static kefir_result_t process_scope(struct kefir_mem *mem, const struct kefir_opt_code_container *code, kefir_opt_block_id_t block_ref, kefir_opt_instruction_ref_t scope_ref, struct state *state) {
    REQUIRE(!kefir_hashset_has(&state->global_scopes, (kefir_hashset_key_t) scope_ref), KEFIR_OK);

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

    if (!has_liveness_marks) {
        REQUIRE_OK(kefir_hashset_add(mem, &state->global_scopes, (kefir_hashset_key_t) scope_ref));
        return KEFIR_OK;
    }

    struct block_variable_scopes *block_scopes = NULL;

    struct kefir_hashtree_node *node;
    res = kefir_hashtree_at(&state->block_scopes, (kefir_hashtree_key_t) block_ref, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        block_scopes = (struct block_variable_scopes *) node->value;
    } else {
        block_scopes = KEFIR_MALLOC(mem, sizeof(struct block_variable_scopes));
        REQUIRE(block_scopes != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer block variable scopes"));
        res = kefir_hashset_init(&block_scopes->scopes, &kefir_hashtable_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashtree_insert(mem, &state->block_scopes, (kefir_hashtree_key_t) block_ref, (kefir_hashtree_value_t) block_scopes));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, block_scopes);
            return res;
        });
    }
    REQUIRE_OK(kefir_hashset_add(mem, &block_scopes->scopes, (kefir_hashset_key_t) scope_ref));
    return KEFIR_OK;
}

static kefir_result_t build_scope_interferences(struct kefir_mem *mem, struct kefir_opt_code_variable_scopes *scopes, struct state *state) {
    kefir_result_t res;

    kefir_bool_t has_unscoped_vars = kefir_hashtree_has(&scopes->scope_variables, (kefir_hashtree_key_t) KEFIR_ID_NONE);

    struct kefir_hashtree_node_iterator block_iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&state->block_scopes, &block_iter);
        node != NULL;
        node = kefir_hashtree_next(&block_iter)) {
        ASSIGN_DECL_CAST(const struct block_variable_scopes *, block_scopes,
            node->value);

        kefir_hashset_key_t entry;
        struct kefir_hashset_iterator iter;
        for (res = kefir_hashset_iter(&block_scopes->scopes, &iter, &entry);
            res == KEFIR_OK;
            res = kefir_hashset_next(&iter, &entry)) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, scope_ref, entry);

            kefir_hashset_key_t entry2;
            struct kefir_hashset_iterator iter2;
            for (res = kefir_hashset_iter(&block_scopes->scopes, &iter2, &entry2);
                res == KEFIR_OK;
                res = kefir_hashset_next(&iter2, &entry2)) {
                ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, scope_ref2, entry2);
                if (scope_ref != scope_ref2) {
                    REQUIRE_OK(kefir_graph_add_edge(mem, &scopes->scope_interference, (kefir_graph_vertex_id_t) scope_ref, (kefir_graph_vertex_id_t) scope_ref2));
                    REQUIRE_OK(kefir_graph_add_edge(mem, &scopes->scope_interference, (kefir_graph_vertex_id_t) scope_ref2, (kefir_graph_vertex_id_t) scope_ref));
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }

            for (res = kefir_hashset_iter(&state->global_scopes, &iter2, &entry2);
                res == KEFIR_OK;
                res = kefir_hashset_next(&iter2, &entry2)) {
                ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, scope_ref2, entry2);
                REQUIRE_OK(kefir_graph_add_edge(mem, &scopes->scope_interference, (kefir_graph_vertex_id_t) scope_ref, (kefir_graph_vertex_id_t) scope_ref2));
                REQUIRE_OK(kefir_graph_add_edge(mem, &scopes->scope_interference, (kefir_graph_vertex_id_t) scope_ref2, (kefir_graph_vertex_id_t) scope_ref));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
            if (has_unscoped_vars) {
                REQUIRE_OK(kefir_graph_add_edge(mem, &scopes->scope_interference, (kefir_graph_vertex_id_t) scope_ref, (kefir_graph_vertex_id_t) KEFIR_ID_NONE));
                REQUIRE_OK(kefir_graph_add_edge(mem, &scopes->scope_interference, (kefir_graph_vertex_id_t) KEFIR_ID_NONE, (kefir_graph_vertex_id_t) scope_ref));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    kefir_hashset_key_t entry;
    struct kefir_hashset_iterator iter;
    for (res = kefir_hashset_iter(&state->global_scopes, &iter, &entry);
        res == KEFIR_OK;
        res = kefir_hashset_next(&iter, &entry)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, scope_ref, entry);

        kefir_hashset_key_t entry2;
        struct kefir_hashset_iterator iter2;
        for (res = kefir_hashset_iter(&state->global_scopes, &iter2, &entry2);
            res == KEFIR_OK;
            res = kefir_hashset_next(&iter2, &entry2)) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, scope_ref2, entry2);
            REQUIRE_OK(kefir_graph_add_edge(mem, &scopes->scope_interference, (kefir_graph_vertex_id_t) scope_ref, (kefir_graph_vertex_id_t) scope_ref2));
            REQUIRE_OK(kefir_graph_add_edge(mem, &scopes->scope_interference, (kefir_graph_vertex_id_t) scope_ref2, (kefir_graph_vertex_id_t) scope_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        if (has_unscoped_vars) {
            REQUIRE_OK(kefir_graph_add_edge(mem, &scopes->scope_interference, (kefir_graph_vertex_id_t) scope_ref, (kefir_graph_vertex_id_t) KEFIR_ID_NONE));
            REQUIRE_OK(kefir_graph_add_edge(mem, &scopes->scope_interference, (kefir_graph_vertex_id_t) KEFIR_ID_NONE, (kefir_graph_vertex_id_t) scope_ref));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t scopes_build_impl(struct kefir_mem *mem, struct kefir_opt_code_variable_scopes *scopes, const struct kefir_opt_code_liveness *liveness, struct state *state) {
    kefir_result_t res;
    kefir_size_t block_count;
    REQUIRE_OK(kefir_opt_code_container_block_count(liveness->code, &block_count));
    for (kefir_opt_block_id_t block_id = 0; block_id  < block_count; block_id++) {
        kefir_hashset_key_t entry;
        struct kefir_hashset_iterator iter;
        for (res = kefir_hashset_iter(&liveness->blocks[block_id].alive_instr, &iter, &entry);
            res == KEFIR_OK;
            res = kefir_hashset_next(&iter, &entry)) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, entry);

            const struct kefir_opt_instruction *instr;
            REQUIRE_OK(kefir_opt_code_container_instr(liveness->code, instr_ref, &instr));
            if (instr->operation.opcode == KEFIR_OPT_OPCODE_LOCAL_SCOPE) {
                REQUIRE_OK(process_scope(mem, liveness->code, block_id, instr_ref, state));
            } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) {
                struct kefir_opt_code_scope_variables *variables = NULL;
                struct kefir_hashtree_node *node;
                res = kefir_hashtree_at(&scopes->scope_variables, (kefir_hashtree_key_t) instr->operation.parameters.refs[0], &node);
                if (res != KEFIR_NOT_FOUND) {
                    REQUIRE_OK(res);
                    variables = (struct kefir_opt_code_scope_variables *) node->value;
                } else {
                    variables = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_scope_variables));
                    REQUIRE(variables != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer scope variables"));
                    res = kefir_hashset_init(&variables->allocations, &kefir_hashtable_uint_ops);
                    REQUIRE_CHAIN(&res, kefir_hashtree_insert(mem, &scopes->scope_variables, (kefir_hashtree_key_t) instr->operation.parameters.refs[0], (kefir_hashtree_value_t) variables));
                    REQUIRE_ELSE(res == KEFIR_OK, {
                        KEFIR_FREE(mem, variables);
                        return res;
                    });
                }
                REQUIRE_OK(kefir_hashset_add(mem, &variables->allocations, (kefir_hashset_key_t) instr_ref));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    REQUIRE_OK(build_scope_interferences(mem, scopes, state));

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_variable_scopes_build(struct kefir_mem *mem, struct kefir_opt_code_variable_scopes *scopes, const struct kefir_opt_code_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(scopes != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer variables scopes"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code liveness"));

    struct state state;
    REQUIRE_OK(kefir_hashset_init(&state.global_scopes, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&state.block_scopes, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&state.block_scopes, free_block_scopes, NULL));

    kefir_result_t res = scopes_build_impl(mem, scopes, liveness, &state);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.global_scopes);
        kefir_hashtree_free(mem, &state.block_scopes);
        return res;
    });
    res = kefir_hashset_free(mem, &state.global_scopes);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &state.block_scopes);
        return res;
    });
    REQUIRE_OK(kefir_hashtree_free(mem, &state.block_scopes));

    return KEFIR_OK;
}
