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

struct dominator_tree_node {
    struct kefir_hashset children;
};

static kefir_result_t free_dominator_tree_node(struct kefir_mem *mem, struct kefir_hashtable *table,
                                               kefir_hashtable_key_t key, kefir_hashtable_value_t value,
                                               void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct dominator_tree_node *, node, value);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid dominator tree node"));

    REQUIRE_OK(kefir_hashset_free(mem, &node->children));
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_init(struct kefir_opt_code_control_flow *control_flow) {
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer code control flow"));

    REQUIRE_OK(kefir_hashset_init(&control_flow->indirect_jump_source_blocks, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashset_init(&control_flow->indirect_jump_target_blocks, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&control_flow->dominator_tree, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&control_flow->dominator_tree, free_dominator_tree_node, NULL));
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
        REQUIRE_OK(kefir_hashtable_free(mem, &control_flow->dominator_tree));
        REQUIRE_OK(kefir_hashset_free(mem, &control_flow->indirect_jump_source_blocks));
        REQUIRE_OK(kefir_hashset_free(mem, &control_flow->indirect_jump_target_blocks));

        for (kefir_size_t i = 0; i < control_flow->num_of_blocks; i++) {
            REQUIRE_OK(kefir_hashset_free(mem, &control_flow->blocks[i].predecessors));
            REQUIRE_OK(kefir_hashset_free(mem, &control_flow->blocks[i].successors));
            REQUIRE_OK(kefir_hashset_free(mem, &control_flow->blocks[i].dominance_frontier));
        }
        KEFIR_FREE(mem, control_flow->blocks);
        memset(control_flow, 0, sizeof(struct kefir_opt_code_control_flow));
    }
    return KEFIR_OK;
}

static kefir_result_t update_dominator_tree(struct kefir_mem *mem, struct kefir_opt_code_control_flow *control_flow,
                                            kefir_opt_block_id_t immediate_dominator_ref,
                                            kefir_opt_block_id_t block_ref) {
    struct dominator_tree_node *node = NULL;
    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&control_flow->dominator_tree,
                                            (kefir_hashtable_key_t) immediate_dominator_ref, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        node = KEFIR_MALLOC(mem, sizeof(struct dominator_tree_node));
        REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate dominator tree node"));
        res = kefir_hashset_init(&node->children, &kefir_hashtable_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashtable_insert(mem, &control_flow->dominator_tree,
                                                   (kefir_hashtable_key_t) immediate_dominator_ref,
                                                   (kefir_hashtable_value_t) node));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, node);
            return res;
        });
    } else {
        node = (struct dominator_tree_node *) table_value;
    }
    REQUIRE_OK(kefir_hashset_add(mem, &node->children, (kefir_hashset_key_t) block_ref));

    return KEFIR_OK;
}

static kefir_result_t build_dominance_tree(struct kefir_mem *mem, struct kefir_opt_code_control_flow *control_flow) {
    kefir_size_t block_count = kefir_opt_code_container_block_count(control_flow->code);
    for (kefir_opt_block_id_t block_ref = 0; block_ref < block_count; block_ref++) {
        kefir_opt_block_id_t immediate_dominator_ref = control_flow->blocks[block_ref].immediate_dominator;
        if (immediate_dominator_ref == KEFIR_ID_NONE) {
            continue;
        }

        REQUIRE_OK(update_dominator_tree(mem, control_flow, immediate_dominator_ref, block_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t walk_dominance_tree_impl(struct kefir_mem *mem, struct kefir_opt_code_control_flow *control_flow,
                                               struct kefir_list *queue) {
    kefir_size_t linear_index = 0;
    if (control_flow->code->entry_point != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_list_insert_after(mem, queue, kefir_list_tail(queue),
                                           (void *) (kefir_uptr_t) control_flow->code->entry_point));
    }

    kefir_result_t res;
    for (struct kefir_list_entry *iter = kefir_list_head(queue); iter != NULL; iter = kefir_list_head(queue)) {
        ASSIGN_DECL_CAST(kefir_uint64_t, entry, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(mem, queue, iter));

        kefir_opt_block_id_t block_ref = (kefir_uint32_t) entry;
        if (entry & (1ull << 63)) {
            control_flow->blocks[block_ref].dominated_block_max_linear = linear_index;
        } else {
            REQUIRE_OK(kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) (block_ref | (1ull << 63))));

            control_flow->blocks[block_ref].dominance_linear_index = linear_index++;
            struct kefir_opt_control_flow_dominator_tree_iterator dom_iter;
            kefir_opt_block_id_t dominated_block_ref;
            for (res = kefir_opt_control_flow_dominator_tree_iter(control_flow, &dom_iter, block_ref,
                                                                  &dominated_block_ref);
                 res == KEFIR_OK; res = kefir_opt_control_flow_dominator_tree_next(&dom_iter, &dominated_block_ref)) {
                REQUIRE_OK(kefir_list_insert_after(mem, queue, NULL, (void *) (kefir_uptr_t) dominated_block_ref));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
    }

    return KEFIR_OK;
}

static kefir_result_t walk_dominance_tree(struct kefir_mem *mem, struct kefir_opt_code_control_flow *control_flow) {
    struct kefir_list queue;
    REQUIRE_OK(kefir_list_init(&queue));
    kefir_result_t res = walk_dominance_tree_impl(mem, control_flow, &queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &queue));
    return KEFIR_OK;
}

static kefir_result_t find_dominance_frontier(struct kefir_mem *mem, struct kefir_opt_code_control_flow *control_flow) {
    kefir_size_t block_count = kefir_opt_code_container_block_count(control_flow->code);

    struct kefir_list queue;
    struct kefir_hashset visited;
    REQUIRE_OK(kefir_list_init(&queue));
    REQUIRE_OK(kefir_hashset_init(&visited, &kefir_hashtable_uint_ops));
    for (kefir_opt_block_id_t block_ref = 0; block_ref < block_count; block_ref++) {
        kefir_bool_t reachable = true;
        REQUIRE_OK(kefir_opt_code_control_flow_is_reachable_from_entry(control_flow, block_ref, &reachable));
        if (!reachable) {
            continue;
        }

        if (kefir_hashset_size(&control_flow->blocks[block_ref].predecessors) <= 1 &&
            block_ref != control_flow->code->entry_point) {
            continue;
        }

        kefir_result_t res;
        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t key;
        for (res = kefir_hashset_iter(&control_flow->blocks[block_ref].predecessors, &iter, &key); res == KEFIR_OK;
             res = kefir_hashset_next(&iter, &key)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, iter_block_ref, key);
            REQUIRE_OK(kefir_opt_code_control_flow_is_reachable_from_entry(control_flow, iter_block_ref, &reachable));
            if (!reachable) {
                continue;
            }
            for (; iter_block_ref != control_flow->blocks[block_ref].immediate_dominator;
                 iter_block_ref = control_flow->blocks[iter_block_ref].immediate_dominator) {
                REQUIRE_OK(kefir_hashset_add(mem, &control_flow->blocks[iter_block_ref].dominance_frontier,
                                             (kefir_hashset_key_t) block_ref));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    kefir_result_t res = kefir_list_free(mem, &queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &visited);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &visited));

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_control_flow_build(struct kefir_mem *mem,
                                                 struct kefir_opt_code_control_flow *control_flow,
                                                 const struct kefir_opt_code_container *code) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code"));

    if (control_flow->code != NULL) {
        REQUIRE_OK(kefir_hashtable_clear(mem, &control_flow->dominator_tree));
        REQUIRE_OK(kefir_hashset_clear(mem, &control_flow->indirect_jump_source_blocks));
        REQUIRE_OK(kefir_hashset_clear(mem, &control_flow->indirect_jump_target_blocks));

        for (kefir_size_t i = 0; i < control_flow->num_of_blocks; i++) {
            REQUIRE_OK(kefir_hashset_free(mem, &control_flow->blocks[i].predecessors));
            REQUIRE_OK(kefir_hashset_free(mem, &control_flow->blocks[i].successors));
            REQUIRE_OK(kefir_hashset_free(mem, &control_flow->blocks[i].dominance_frontier));
        }
        KEFIR_FREE(mem, control_flow->blocks);
    }

    kefir_result_t res;
    control_flow->code = code;
    control_flow->num_of_blocks = kefir_opt_code_container_block_count(control_flow->code);
    control_flow->blocks =
        KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_control_flow_block) * control_flow->num_of_blocks);
    REQUIRE(control_flow->blocks != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer code control flow blocks"));
    for (kefir_size_t i = 0; i < control_flow->num_of_blocks; i++) {
        control_flow->blocks[i].immediate_dominator = KEFIR_ID_NONE;
        control_flow->blocks[i].dominance_linear_index = (kefir_size_t) -1ll;
        control_flow->blocks[i].dominated_block_max_linear = (kefir_size_t) -1ll;
        res = kefir_hashset_init(&control_flow->blocks[i].predecessors, &kefir_hashtable_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashset_init(&control_flow->blocks[i].successors, &kefir_hashtable_uint_ops));
        REQUIRE_CHAIN(&res, kefir_hashset_init(&control_flow->blocks[i].dominance_frontier, &kefir_hashtable_uint_ops));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, control_flow->blocks);
            control_flow->blocks = NULL;
            control_flow->code = NULL;
            return res;
        });
    }

    res = kefir_opt_code_control_flow_link_blocks(mem, control_flow);
    REQUIRE_CHAIN(&res, kefir_opt_code_control_flow_find_dominators(mem, control_flow));
    REQUIRE_CHAIN(&res, build_dominance_tree(mem, control_flow));
    REQUIRE_CHAIN(&res, walk_dominance_tree(mem, control_flow));
    REQUIRE_CHAIN(&res, find_dominance_frontier(mem, control_flow));
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
    } else if (dominated_block != KEFIR_ID_NONE && dominator_block != KEFIR_ID_NONE &&
               control_flow->blocks[dominated_block].immediate_dominator != KEFIR_ID_NONE) {
        kefir_size_t linear_index = control_flow->blocks[dominator_block].dominance_linear_index;
        kefir_size_t dominated_block_max_linear = control_flow->blocks[dominator_block].dominated_block_max_linear;
        kefir_size_t dominated_linear = control_flow->blocks[dominated_block].dominance_linear_index;
        *result_ptr = linear_index != (kefir_size_t) -1ll && dominated_block_max_linear != (kefir_size_t) -1ll &&
                      linear_index < dominated_linear && dominated_linear < dominated_block_max_linear;
    } else {
        *result_ptr = false;
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

    kefir_size_t num_of_blocks = kefir_opt_code_container_block_count(control_flow->code);
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

    kefir_size_t num_of_blocks = kefir_opt_code_container_block_count(control_flow->code);
    REQUIRE(block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));
    REQUIRE(successor_block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));

    const struct kefir_opt_code_block *successor_block;
    REQUIRE_OK(kefir_opt_code_container_block(control_flow->code, successor_block_id, &successor_block));

    *result_ptr =
        kefir_hashset_has(&control_flow->blocks[successor_block_id].predecessors, (kefir_hashset_key_t) block_id);
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

    kefir_size_t num_of_blocks = kefir_opt_code_container_block_count(control_flow->code);
    REQUIRE(block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));
    REQUIRE(successor_block_id < num_of_blocks,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided block identifier is outside of code container bounds"));

    const struct kefir_opt_code_block *successor_block;
    REQUIRE_OK(kefir_opt_code_container_block(control_flow->code, successor_block_id, &successor_block));

    kefir_bool_t result = block_id != successor_block_id && kefir_hashtreeset_empty(&successor_block->public_labels);
    kefir_bool_t found_block_id = false;

    kefir_result_t res;
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t entry;
    for (res = kefir_hashset_iter(&control_flow->blocks[successor_block_id].predecessors, &iter, &entry);
         result && res == KEFIR_OK; res = kefir_hashset_next(&iter, &entry)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, predecessor_block_id, (kefir_uptr_t) entry);

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
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    *result_ptr = result && found_block_id;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_control_flow_dominator_tree_iter(const struct kefir_opt_code_control_flow *control_flow,
                                                          struct kefir_opt_control_flow_dominator_tree_iterator *iter,
                                                          kefir_opt_block_id_t block_ref,
                                                          kefir_opt_block_id_t *dominated_block_ref_ptr) {
    REQUIRE(control_flow != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow"));
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid target IR control flow dominator tree iterator"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res =
        kefir_hashtable_at(&control_flow->dominator_tree, (kefir_hashtable_key_t) block_ref, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR dominator tree iterator");
    }
    REQUIRE_OK(res);
    ASSIGN_DECL_CAST(const struct dominator_tree_node *, tree_node, table_value);

    kefir_hashset_key_t entry;
    res = kefir_hashset_iter(&tree_node->children, &iter->iter, &entry);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of target IR dominator tree iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(dominated_block_ref_ptr, (kefir_opt_block_id_t) entry);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_control_flow_dominator_tree_next(struct kefir_opt_control_flow_dominator_tree_iterator *iter,
                                                          kefir_opt_block_id_t *dominated_block_ref_ptr) {
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code dominator tree iterator"));

    kefir_hashset_key_t entry;
    kefir_result_t res = kefir_hashset_next(&iter->iter, &entry);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of optimizer code dominator tree iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(dominated_block_ref_ptr, (kefir_opt_block_id_t) entry);
    return KEFIR_OK;
}
