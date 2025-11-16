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

#include "kefir/optimizer/local_variables.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t free_local_variable_occurences(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_local_variable_occurences *, occurences, value);
    REQUIRE(occurences != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code local variable occurences"));

    REQUIRE_OK(kefir_hashset_free(mem, &occurences->occurences));
    memset(occurences, 0, sizeof(struct kefir_opt_code_local_variable_occurences));
    KEFIR_FREE(mem, occurences);
    return KEFIR_OK;
}

static kefir_result_t free_block_local_variables(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_block_local_variables *, variables, value);
    REQUIRE(variables != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code block local variables"));

    REQUIRE_OK(kefir_hashset_free(mem, &variables->variables));
    memset(variables, 0, sizeof(struct kefir_opt_code_block_local_variables));
    KEFIR_FREE(mem, variables);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_variable_conflicts_init(struct kefir_opt_code_variable_conflicts *vars) {
    REQUIRE(vars != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code local variables analysis"));

    REQUIRE_OK(kefir_hashtree_init(&vars->block_alive_vars, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&vars->block_alive_vars, free_block_local_variables, NULL));
    REQUIRE_OK(kefir_hashtree_init(&vars->locally_alive, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&vars->locally_alive, free_local_variable_occurences, NULL));
    REQUIRE_OK(kefir_hashset_init(&vars->globally_alive, &kefir_hashtable_uint_ops));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_variable_conflicts_free(struct kefir_mem *mem, struct kefir_opt_code_variable_conflicts *vars) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(vars != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code local variables analysis"));

    REQUIRE_OK(kefir_hashtree_free(mem, &vars->locally_alive));
    REQUIRE_OK(kefir_hashtree_free(mem, &vars->block_alive_vars));
    REQUIRE_OK(kefir_hashset_free(mem, &vars->globally_alive));
    return KEFIR_OK;
}

static kefir_result_t has_local_lifetime_marks(const struct kefir_opt_code_liveness *liveness, kefir_opt_instruction_ref_t instr_ref, kefir_bool_t *has_local_markings) {
    *has_local_markings = false;
    
    kefir_result_t res;
    struct kefir_opt_instruction_use_iterator use_iter;
    for (res = kefir_opt_code_container_instruction_use_instr_iter(liveness->code, instr_ref, &use_iter);
        res == KEFIR_OK && !*has_local_markings;
        res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_instruction *use_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(liveness->code, use_iter.use_instr_ref, &use_instr));
        if (use_instr->operation.opcode == KEFIR_OPT_OPCODE_LOCAL_LIFETIME_MARK) {
            *has_local_markings = true;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t update_locally_alive(struct kefir_mem *mem, struct kefir_opt_code_variable_conflicts *vars, const struct kefir_opt_code_liveness *liveness, kefir_opt_block_id_t block_id, kefir_opt_instruction_ref_t instr_ref) {
    REQUIRE(!kefir_hashset_has(&vars->globally_alive, (kefir_hashset_key_t) instr_ref), KEFIR_OK);
    
    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(liveness->code, instr_ref, &instr));
    REQUIRE(instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL, KEFIR_OK);

    struct kefir_opt_code_local_variable_occurences *local_occurences = NULL;
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&vars->locally_alive, (kefir_hashtree_key_t) instr_ref, &node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        local_occurences = (struct kefir_opt_code_local_variable_occurences *) node->value;
    } else if (!kefir_hashset_has(&vars->globally_alive, (kefir_hashset_key_t) instr_ref)) {
        kefir_bool_t has_local_markings = false;
        REQUIRE_OK(has_local_lifetime_marks(liveness, instr_ref, &has_local_markings));

        if (has_local_markings) {
            local_occurences = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_local_variable_occurences));
            REQUIRE(local_occurences != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate variable local conflicts"));

            res = kefir_hashset_init(&local_occurences->occurences, &kefir_hashtable_uint_ops);
            REQUIRE_CHAIN(&res, kefir_hashset_init(&local_occurences->occurences, &kefir_hashtable_uint_ops));
            REQUIRE_CHAIN(&res, kefir_hashtree_insert(mem, &vars->locally_alive, (kefir_hashtree_key_t) instr_ref, (kefir_hashtree_value_t) local_occurences));
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_FREE(mem, local_occurences);
                return res;
            });
        } else {
            REQUIRE_OK(kefir_hashset_add(mem, &vars->globally_alive, (kefir_hashset_key_t) instr_ref));
        }
    }

    if (local_occurences != NULL) {
        REQUIRE_OK(kefir_hashset_add(mem, &local_occurences->occurences, (kefir_hashset_key_t) block_id));
        struct kefir_opt_code_block_local_variables *block_alive_vars = NULL;
        res = kefir_hashtree_at(&vars->block_alive_vars, (kefir_hashtree_key_t) block_id, &node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            block_alive_vars = (struct kefir_opt_code_block_local_variables *) node->value;
        } else {
            block_alive_vars = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_block_local_variables));
            REQUIRE(block_alive_vars != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate variables alive in block"));

            res = kefir_hashset_init(&block_alive_vars->variables, &kefir_hashtable_uint_ops);
            REQUIRE_CHAIN(&res, kefir_hashset_init(&block_alive_vars->variables, &kefir_hashtable_uint_ops));
            REQUIRE_CHAIN(&res, kefir_hashtree_insert(mem, &vars->block_alive_vars, (kefir_hashtree_key_t) block_id, (kefir_hashtree_value_t) block_alive_vars));
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_FREE(mem, block_alive_vars);
                return res;
            });
        }
        REQUIRE_OK(kefir_hashset_add(mem, &block_alive_vars->variables, (kefir_hashset_key_t) instr_ref));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_variable_conflicts_build(struct kefir_mem *mem, struct kefir_opt_code_variable_conflicts *vars, const struct kefir_opt_code_liveness *liveness) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(vars != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code local variables analysis"));
    REQUIRE(liveness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code liveness"));

    kefir_result_t res;
    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(liveness->code, &num_of_blocks));
    for (kefir_opt_block_id_t block_id = 0; block_id < num_of_blocks; block_id++) {
        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t entry;
        for (res = kefir_hashset_iter(&liveness->blocks[block_id].alive_instr,  &iter, &entry);
            res == KEFIR_OK;
            res = kefir_hashset_next(&iter, &entry)) {
            ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, entry);
            REQUIRE_OK(update_locally_alive(mem, vars, liveness, block_id, instr_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}
