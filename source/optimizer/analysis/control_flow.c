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

#define KEFIR_OPTIMIZER_CONTROL_FLOW_INTERNAL
#include "kefir/optimizer/control_flow.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

kefir_result_t kefir_opt_code_control_flow_init(struct kefir_opt_code_control_flow *control_flow) {
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code control flow"));

    REQUIRE_OK(kefir_hashtreeset_init(&control_flow->indirect_jump_target_blocks, &kefir_hashtree_uint_ops));
    control_flow->blocks = NULL;
    control_flow->code = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_free(struct kefir_mem *mem,
                                                struct kefir_opt_code_control_flow *control_flow) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));

    if (control_flow->code != NULL) {
        REQUIRE_OK(kefir_hashtreeset_free(mem, &control_flow->indirect_jump_target_blocks));

        for (kefir_size_t i = 0; i < control_flow->num_of_blocks; i++) {
            REQUIRE_OK(kefir_list_free(mem, &control_flow->blocks[i].predecessors));
            REQUIRE_OK(kefir_list_free(mem, &control_flow->blocks[i].successors));
        }
        KEFIR_FREE(mem, control_flow->blocks);
        memset(control_flow, 0, sizeof(struct kefir_opt_code_control_flow));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_build(struct kefir_mem *mem,
                                                 struct kefir_opt_code_control_flow *control_flow,
                                                 const struct kefir_opt_code_container *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));
    REQUIRE(control_flow->code == NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Optimizer code control flow has already been built"));

    kefir_result_t res;
    control_flow->code = code;
    REQUIRE_OK(kefir_opt_code_container_block_count(control_flow->code, &control_flow->num_of_blocks));
    control_flow->blocks =
        KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_control_flow_block) * control_flow->num_of_blocks);
    REQUIRE(control_flow->blocks != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer code control flow blocks"));
    for (kefir_size_t i = 0; i < control_flow->num_of_blocks; i++) {
        control_flow->blocks[i].immediate_dominator = KEFIR_ID_NONE;
        res = kefir_list_init(&control_flow->blocks[i].predecessors);
        REQUIRE_CHAIN(&res, kefir_list_init(&control_flow->blocks[i].successors));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, control_flow->blocks);
            memset(control_flow, 0, sizeof(struct kefir_opt_code_control_flow));
            return res;
        });
    }
    res = kefir_hashtreeset_init(&control_flow->indirect_jump_target_blocks, &kefir_hashtree_uint_ops);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, control_flow->blocks);
        memset(control_flow, 0, sizeof(struct kefir_opt_code_control_flow));
        return res;
    });

    res = kefir_opt_code_control_flow_link_blocks(mem, control_flow);
    REQUIRE_CHAIN(&res, kefir_opt_code_control_flow_find_dominators(mem, control_flow));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_control_flow_free(mem, control_flow);
        kefir_opt_code_control_flow_init(control_flow);
        return res;
    });

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_is_dominator(const struct kefir_opt_code_control_flow *control_flow,
                                                        kefir_opt_block_id_t dominated_block,
                                                        kefir_opt_block_id_t dominator_block,
                                                        kefir_bool_t *result_ptr) {
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    if (dominated_block == dominator_block) {
        *result_ptr = true;
    } else if (control_flow->blocks[dominated_block].immediate_dominator != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_code_control_flow_is_dominator(
            control_flow, control_flow->blocks[dominated_block].immediate_dominator, dominator_block, result_ptr));
    } else {
        *result_ptr = false;
    }
    return KEFIR_OK;
}

static kefir_result_t redirect_edge(struct kefir_mem *mem, struct kefir_opt_code_control_flow *control_flow,
                                    kefir_opt_block_id_t old_source_block_id, kefir_opt_block_id_t new_source_block_id,
                                    kefir_opt_block_id_t target_block_id) {
    for (struct kefir_list_entry *iter = kefir_list_head(&control_flow->blocks[old_source_block_id].successors);
         iter != NULL;) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) iter->value);
        struct kefir_list_entry *next_iter = iter->next;
        if (block_id == target_block_id) {
            REQUIRE_OK(kefir_list_pop(mem, &control_flow->blocks[old_source_block_id].successors, iter));
        }
        iter = next_iter;
    }
    REQUIRE_OK(kefir_list_insert_after(mem, &control_flow->blocks[new_source_block_id].successors,
                                       kefir_list_tail(&control_flow->blocks[new_source_block_id].successors),
                                       (void *) (kefir_uptr_t) target_block_id));

    for (struct kefir_list_entry *iter = kefir_list_head(&control_flow->blocks[target_block_id].predecessors);
         iter != NULL; iter = iter->next) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) iter->value);
        if (block_id == old_source_block_id) {
            iter->value = (void *) (kefir_uptr_t) new_source_block_id;
        }
    }

    if (control_flow->blocks[target_block_id].immediate_dominator == old_source_block_id) {
        control_flow->blocks[target_block_id].immediate_dominator = new_source_block_id;
    }

    return KEFIR_OK;
}

static kefir_result_t drop_edge(struct kefir_mem *mem, struct kefir_opt_code_control_flow *control_flow,
                                kefir_opt_block_id_t src_block_id, kefir_opt_block_id_t dst_block_id) {
    for (struct kefir_list_entry *iter = kefir_list_head(&control_flow->blocks[src_block_id].successors);
         iter != NULL;) {
        struct kefir_list_entry *next_iter = iter->next;
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, (kefir_uptr_t) iter->value);
        if (successor_block_id == dst_block_id) {
            REQUIRE_OK(kefir_list_pop(mem, &control_flow->blocks[src_block_id].successors, iter));
        }
        iter = next_iter;
    }

    for (struct kefir_list_entry *iter = kefir_list_head(&control_flow->blocks[dst_block_id].predecessors);
         iter != NULL;) {
        struct kefir_list_entry *next_iter = iter->next;
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, predecessor_block_id, (kefir_uptr_t) iter->value);
        if (predecessor_block_id == src_block_id) {
            REQUIRE_OK(kefir_list_pop(mem, &control_flow->blocks[dst_block_id].predecessors, iter));
        }
        iter = next_iter;
    }

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_modify_bypass_block(struct kefir_mem *mem,
                                                               struct kefir_opt_code_control_flow *control_flow,
                                                               kefir_opt_block_id_t bypassed_block_id,
                                                               kefir_opt_block_id_t block_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));

    kefir_size_t total_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(control_flow->code, &total_blocks));

    REQUIRE(bypassed_block_id < total_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is out of code container bounds"));
    REQUIRE(block_id < total_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is out of code container bounds"));

    kefir_bool_t found_successor = false;
    for (const struct kefir_list_entry *iter = kefir_list_head(&control_flow->blocks[block_id].successors);
         iter != NULL && !found_successor; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, (kefir_uptr_t) iter->value);
        found_successor = successor_block_id == bypassed_block_id;
    }
    REQUIRE(found_successor, KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                                             "Expected bypassed optimizer block to be a successor of the other one"));

    REQUIRE_OK(drop_edge(mem, control_flow, block_id, bypassed_block_id));

    for (const struct kefir_list_entry *iter = kefir_list_head(&control_flow->blocks[bypassed_block_id].successors);
         iter != NULL; iter = kefir_list_head(&control_flow->blocks[bypassed_block_id].successors)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, (kefir_uptr_t) iter->value);
        REQUIRE_OK(redirect_edge(mem, control_flow, bypassed_block_id, block_id, successor_block_id));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_is_reachable_from_entry(
    const struct kefir_opt_code_control_flow *control_flow, kefir_opt_block_id_t block_id,
    kefir_bool_t *reachable_flag_ptr) {
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));
    REQUIRE(reachable_flag_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(control_flow->code, &num_of_blocks));
    REQUIRE(block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));

    *reachable_flag_ptr = block_id == control_flow->code->entry_point ||
                          control_flow->blocks[block_id].immediate_dominator != KEFIR_ID_NONE;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_block_direct_predecessor(
    const struct kefir_opt_code_control_flow *control_flow, kefir_opt_block_id_t block_id,
    kefir_opt_block_id_t successor_block_id, kefir_bool_t *result_ptr) {
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(control_flow->code, &num_of_blocks));
    REQUIRE(block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));
    REQUIRE(successor_block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));

    const struct kefir_opt_code_block *successor_block;
    REQUIRE_OK(kefir_opt_code_container_block(control_flow->code, successor_block_id, &successor_block));

    kefir_bool_t found_block_id = false;
    for (const struct kefir_list_entry *iter = kefir_list_head(&control_flow->blocks[successor_block_id].predecessors);
         !found_block_id && iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, predecessor_block_id, (kefir_uptr_t) iter->value);

        if (predecessor_block_id == block_id) {
            found_block_id = true;
        }
    }

    *result_ptr = found_block_id;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_block_exclusive_direct_predecessor(
    const struct kefir_opt_code_control_flow *control_flow, kefir_opt_block_id_t block_id,
    kefir_opt_block_id_t successor_block_id, kefir_bool_t *result_ptr) {
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));
    REQUIRE(result_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    if (successor_block_id == control_flow->code->entry_point) {
        *result_ptr = false;
        return KEFIR_OK;
    }

    kefir_size_t num_of_blocks;
    REQUIRE_OK(kefir_opt_code_container_block_count(control_flow->code, &num_of_blocks));
    REQUIRE(block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));
    REQUIRE(successor_block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));

    const struct kefir_opt_code_block *successor_block;
    REQUIRE_OK(kefir_opt_code_container_block(control_flow->code, successor_block_id, &successor_block));

    kefir_bool_t result = block_id != successor_block_id && kefir_hashtreeset_empty(&successor_block->public_labels);
    kefir_bool_t found_block_id = false;
    for (const struct kefir_list_entry *iter = kefir_list_head(&control_flow->blocks[successor_block_id].predecessors);
         result && iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, predecessor_block_id, (kefir_uptr_t) iter->value);

        if (successor_block_id == predecessor_block_id) {
            result = false;
        } else if (predecessor_block_id != block_id) {
            kefir_bool_t reachable_from_entry;
            REQUIRE_OK(kefir_opt_code_control_flow_is_reachable_from_entry(control_flow, predecessor_block_id,
                                                                           &reachable_from_entry));
            result = result && !reachable_from_entry;
        } else {
            found_block_id = true;
        }
    }

    *result_ptr = result && found_block_id;
    return KEFIR_OK;
}
