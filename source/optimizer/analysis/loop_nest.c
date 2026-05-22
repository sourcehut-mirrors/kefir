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

#include "kefir/optimizer/loop_nest.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

struct kefir_opt_code_loop *kefir_opt_loop_nest_top(const struct kefir_opt_loop_nest *nest) {
    REQUIRE(nest != NULL, NULL);
    return nest->nest.value;
}

struct kefir_opt_code_loop *kefir_opt_code_loop_parent(const struct kefir_opt_code_loop *loop) {
    REQUIRE(loop != NULL, NULL);
    struct kefir_tree_node *node = loop->nest_node->parent;
    REQUIRE(node != NULL, NULL);
    return node->value;
}

struct kefir_opt_code_loop *kefir_opt_code_loop_first_child(const struct kefir_opt_code_loop *loop) {
    REQUIRE(loop != NULL, NULL);
    struct kefir_tree_node *node = kefir_tree_first_child(loop->nest_node);
    REQUIRE(node != NULL, NULL);
    return node->value;
}

struct kefir_opt_code_loop *kefir_opt_code_loop_next_sibling(const struct kefir_opt_code_loop *loop) {
    REQUIRE(loop != NULL, NULL);
    struct kefir_tree_node *node = kefir_tree_next_sibling(loop->nest_node);
    REQUIRE(node != NULL, NULL);
    return node->value;
}

static kefir_result_t free_loop(struct kefir_mem *mem, struct kefir_hashtable *table, kefir_hashtable_key_t key,
                                kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_loop *, loop, value);
    REQUIRE(loop != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code loop"));

    REQUIRE_OK(kefir_hashset_free(mem, &loop->blocks));
    REQUIRE_OK(kefir_hashset_free(mem, &loop->latches));
    REQUIRE_OK(kefir_hashset_free(mem, &loop->exits));
    memset(loop, 0, sizeof(struct kefir_opt_code_loop));
    KEFIR_FREE(mem, loop);
    return KEFIR_OK;
}

static kefir_result_t free_loop_nest(struct kefir_mem *mem, struct kefir_list *list, struct kefir_list_entry *entry,
                                     void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));
    ASSIGN_DECL_CAST(struct kefir_opt_loop_nest *, nest, entry->value);
    REQUIRE(nest != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer loop nest"));

    REQUIRE_OK(kefir_tree_free(mem, &nest->nest));
    KEFIR_FREE(mem, nest);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_loop_collection_init(struct kefir_opt_code_loop_collection *loops) {
    REQUIRE(loops != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer loop collection"));

    REQUIRE_OK(kefir_hashtable_init(&loops->loops, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&loops->loops, free_loop, NULL));
    REQUIRE_OK(kefir_list_init(&loops->nests));
    REQUIRE_OK(kefir_list_on_remove(&loops->nests, free_loop_nest, NULL));
    REQUIRE_OK(kefir_hashtable_init(&loops->block_index, &kefir_hashtable_uint_ops));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_loop_collection_free(struct kefir_mem *mem,
                                                   struct kefir_opt_code_loop_collection *loops) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(loops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer loop collection"));

    REQUIRE_OK(kefir_hashtable_free(mem, &loops->block_index));
    REQUIRE_OK(kefir_list_free(mem, &loops->nests));
    REQUIRE_OK(kefir_hashtable_free(mem, &loops->loops));
    return KEFIR_OK;
}

#define IS_BLOCK_REACHABLE(_control_flow, _block_id)      \
    ((_block_id) == (_control_flow)->code->entry_point || \
     (_control_flow)->blocks[(_block_id)].immediate_dominator != KEFIR_ID_NONE)

static kefir_result_t build_loop_impl(struct kefir_mem *mem, const struct kefir_opt_code_control_flow *control_flow,
                                      struct kefir_opt_code_loop *loop, kefir_opt_block_id_t loop_exit_block_id,
                                      struct kefir_list *traversal_queue) {
    REQUIRE_OK(kefir_hashset_clear(mem, &loop->blocks));
    REQUIRE_OK(kefir_list_clear(mem, traversal_queue));

    REQUIRE_OK(kefir_list_insert_after(mem, traversal_queue, NULL, (void *) (kefir_uptr_t) loop_exit_block_id));
    for (struct kefir_list_entry *iter = kefir_list_head(traversal_queue); iter != NULL;
         iter = kefir_list_head(traversal_queue)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(mem, traversal_queue, iter));

        if (kefir_hashset_has(&loop->blocks, (kefir_hashtreeset_entry_t) block_id)) {
            continue;
        }
        REQUIRE_OK(kefir_hashset_add(mem, &loop->blocks, (kefir_hashtreeset_entry_t) block_id));

        if (block_id != loop->header_ref) {
            kefir_result_t res;
            struct kefir_hashset_iterator pred_iter;
            kefir_hashset_key_t entry;
            for (res = kefir_hashset_iter(&control_flow->blocks[block_id].predecessors, &pred_iter, &entry);
                 res == KEFIR_OK; res = kefir_hashset_next(&pred_iter, &entry)) {
                kefir_bool_t is_dominator;
                REQUIRE_OK(
                    kefir_opt_code_control_flow_is_dominator(control_flow, entry, loop->header_ref, &is_dominator));
                if (is_dominator) {
                    REQUIRE_OK(kefir_list_insert_after(mem, traversal_queue, kefir_list_tail(traversal_queue),
                                                       (void *) (kefir_uptr_t) entry));
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
    }

    kefir_result_t res;
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t key;
    for (res = kefir_hashset_iter(&loop->blocks, &iter, &key); res == KEFIR_OK; res = kefir_hashset_next(&iter, &key)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, key);

        struct kefir_hashset_iterator succ_iter;
        kefir_hashset_key_t entry;
        res = kefir_hashset_iter(&control_flow->blocks[block_id].successors, &succ_iter, &entry);
        if (res == KEFIR_ITERATOR_END) {
            REQUIRE_OK(kefir_hashset_add(mem, &loop->exits, (kefir_hashset_key_t) block_id));
            continue;
        }
        for (; res == KEFIR_OK; res = kefir_hashset_next(&succ_iter, &entry)) {
            if (entry == loop->header_ref) {
                REQUIRE_OK(kefir_hashset_add(mem, &loop->latches, (kefir_hashset_key_t) block_id));
                break;
            } else if (!kefir_hashset_has(&loop->blocks, (kefir_hashtreeset_entry_t) entry)) {
                REQUIRE_OK(kefir_hashset_add(mem, &loop->exits, (kefir_hashset_key_t) block_id));
                break;
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

static kefir_result_t build_loop(struct kefir_mem *mem, struct kefir_opt_code_loop_collection *loops,
                                 const struct kefir_opt_code_control_flow *control_flow,
                                 kefir_opt_block_id_t loop_entry_block_id, kefir_opt_block_id_t loop_exit_block_id) {
    struct kefir_opt_code_loop *loop = NULL;

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&loops->loops, (kefir_hashtable_key_t) loop_entry_block_id, &table_value);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        loop = (struct kefir_opt_code_loop *) table_value;
    } else {
        loop = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_loop));
        REQUIRE(loop != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Unable to allocate optimzier code loop"));

        loop->preheader_ref = KEFIR_ID_NONE;
        loop->header_ref = loop_entry_block_id;
        loop->nest_node = NULL;
        loop->level = 0;
        res = kefir_hashset_init(&loop->blocks, &kefir_hashtable_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashset_init(&loop->latches, &kefir_hashtable_uint_ops));
        REQUIRE_CHAIN(&res, kefir_hashset_init(&loop->exits, &kefir_hashtable_uint_ops));
        REQUIRE_CHAIN(&res, kefir_hashtable_insert(mem, &loops->loops, (kefir_hashtable_key_t) loop_entry_block_id,
                                                   (kefir_hashtable_value_t) loop));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, loop);
            return res;
        });

        kefir_hashset_key_t entry;
        struct kefir_hashset_iterator iter;
        for (res = kefir_hashset_iter(&control_flow->blocks[loop_entry_block_id].predecessors, &iter, &entry);
             res == KEFIR_OK && loop->preheader_ref == KEFIR_ID_NONE; res = kefir_hashset_next(&iter, &entry)) {
            if (entry == loop_entry_block_id) {
                continue;
            }
            kefir_bool_t is_dominator;
            REQUIRE_OK(
                kefir_opt_code_control_flow_is_dominator(control_flow, loop_entry_block_id, entry, &is_dominator));
            if (is_dominator && control_flow->blocks[entry].successors.occupied == 1 &&
                kefir_hashset_has(&control_flow->blocks[entry].successors, (kefir_hashset_key_t) loop_entry_block_id)) {
                loop->preheader_ref = entry;
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    struct kefir_list traversal_queue;
    REQUIRE_OK(kefir_list_init(&traversal_queue));
    res = build_loop_impl(mem, control_flow, loop, loop_exit_block_id, &traversal_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &traversal_queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &traversal_queue));
    return KEFIR_OK;
}

static kefir_bool_t loop_contained_within(const struct kefir_opt_code_loop *loop,
                                          const struct kefir_opt_code_loop *contained_loop) {
    return kefir_hashset_subset(&contained_loop->blocks, &loop->blocks);
}

static kefir_result_t insert_into_nest(struct kefir_mem *mem, struct kefir_opt_code_loop *loop,
                                       struct kefir_tree_node *nest) {
    for (struct kefir_tree_node *child = kefir_tree_first_child(nest); child != NULL;
         child = kefir_tree_next_sibling(child)) {
        if (loop_contained_within(loop, (const struct kefir_opt_code_loop *) child->value)) {
            return insert_into_nest(mem, loop, child);
        }
    }

    struct kefir_tree_node *nest_node;
    REQUIRE_OK(kefir_tree_insert_child(mem, nest, (void *) loop, &nest_node));
    loop->nest_node = nest_node;
    return KEFIR_OK;
}

static kefir_result_t update_loop_nest(struct kefir_mem *mem, struct kefir_opt_code_loop_collection *loops,
                                       struct kefir_opt_code_loop *loop) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&loops->nests); iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_opt_loop_nest *, nest, iter->value);

        if (loop_contained_within(loop, (const struct kefir_opt_code_loop *) nest->nest.value)) {
            REQUIRE_OK(insert_into_nest(mem, loop, &nest->nest));
            return KEFIR_OK;
        } else if (loop_contained_within((const struct kefir_opt_code_loop *) nest->nest.value, loop)) {
            struct kefir_tree_node *nest_node;
            REQUIRE_OK(kefir_tree_insert_parent(mem, &nest->nest, (void *) loop, &nest_node));
            loop->nest_node = nest_node->parent;
            ((struct kefir_opt_code_loop *) nest_node->value)->nest_node = nest_node;
            return KEFIR_OK;
        }
    }

    struct kefir_opt_loop_nest *nest = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_loop_nest));
    REQUIRE(nest != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer loop nest"));
    kefir_result_t res = kefir_tree_init(&nest->nest, (void *) loop);
    REQUIRE_CHAIN(&res, kefir_list_insert_after(mem, &loops->nests, kefir_list_tail(&loops->nests), nest));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, nest);
        return res;
    });
    loop->nest_node = &nest->nest;
    return KEFIR_OK;
}

static kefir_result_t finalize_loop(struct kefir_mem *mem, struct kefir_opt_code_loop_collection *loops,
                                    struct kefir_opt_code_loop *loop) {
    struct kefir_opt_code_loop *parent = kefir_opt_code_loop_parent(loop);
    if (parent != NULL) {
        loop->level = parent->level + 1;
    } else {
        loop->level = 1;
    }

    for (struct kefir_opt_code_loop *nested = kefir_opt_code_loop_first_child(loop); nested != NULL;
         nested = kefir_opt_code_loop_next_sibling(nested)) {
        REQUIRE_OK(finalize_loop(mem, loops, nested));
    }

    kefir_result_t res;
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t iter_key;
    for (res = kefir_hashset_iter(&loop->blocks, &iter, &iter_key); res == KEFIR_OK;
         res = kefir_hashset_next(&iter, &iter_key)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_ref, iter_key);

        kefir_hashtable_value_t *table_value_ptr;
        res = kefir_hashtable_at_mut(&loops->block_index, (kefir_hashtable_key_t) block_ref, &table_value_ptr);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            ASSIGN_DECL_CAST(struct kefir_opt_code_loop *, current, *table_value_ptr);
            if (loop->level > current->level) {
                *table_value_ptr = (kefir_hashtable_value_t) loop;
            }
        } else {
            REQUIRE_OK(kefir_hashtable_insert(mem, &loops->block_index, (kefir_hashtable_key_t) block_ref,
                                              (kefir_hashtable_value_t) loop));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t build_loop_nests(struct kefir_mem *mem, struct kefir_opt_code_loop_collection *loops) {
    kefir_result_t res;
    struct kefir_opt_code_loop *loop;
    struct kefir_opt_code_loop_collection_iterator iter;
    for (res = kefir_opt_code_loop_collection_iter(loops, &loop, &iter); res == KEFIR_OK && loop != NULL;
         res = kefir_opt_code_loop_collection_next(&loop, &iter)) {
        REQUIRE_OK(update_loop_nest(mem, loops, loop));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    const struct kefir_opt_loop_nest *nest;
    struct kefir_opt_code_loop_nest_collection_iterator nest_iter;
    for (res = kefir_opt_code_loop_nest_collection_iter(loops, &nest, &nest_iter); res == KEFIR_OK && nest != NULL;
         res = kefir_opt_code_loop_nest_collection_next(&nest, &nest_iter)) {
        REQUIRE_OK(finalize_loop(mem, loops, kefir_opt_loop_nest_top(nest)));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_loop_collection_build(struct kefir_mem *mem, struct kefir_opt_code_loop_collection *loops,
                                                    const struct kefir_opt_code_control_flow *control_flow) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(loops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer loop collection"));
    REQUIRE(control_flow != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code control flow"));

    kefir_size_t block_count = kefir_opt_code_container_block_count(control_flow->code);
    for (kefir_opt_block_id_t block_id = 0; block_id < block_count; block_id++) {
        if (!IS_BLOCK_REACHABLE(control_flow, block_id)) {
            continue;
        }

        kefir_result_t res;
        struct kefir_hashset_iterator succ_iter;
        kefir_hashset_key_t entry;
        for (res = kefir_hashset_iter(&control_flow->blocks[block_id].successors, &succ_iter, &entry); res == KEFIR_OK;
             res = kefir_hashset_next(&succ_iter, &entry)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, entry);
            if (!IS_BLOCK_REACHABLE(control_flow, successor_block_id)) {
                continue;
            }

            const struct kefir_opt_code_block *successor_block;
            REQUIRE_OK(kefir_opt_code_container_block(control_flow->code, successor_block_id, &successor_block));

            kefir_bool_t is_dominator;
            REQUIRE_OK(
                kefir_opt_code_control_flow_is_dominator(control_flow, block_id, successor_block_id, &is_dominator));
            if (is_dominator) {
                REQUIRE_OK(build_loop(mem, loops, control_flow, successor_block_id, block_id));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    REQUIRE_OK(build_loop_nests(mem, loops));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_loop_collection_find_loop(const struct kefir_opt_code_loop_collection *loops,
                                                        kefir_opt_block_id_t block_id,
                                                        struct kefir_opt_code_loop **loop_ptr) {
    REQUIRE(loops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer loop collection"));

    kefir_hashtable_value_t table_value;
    kefir_result_t res = kefir_hashtable_at(&loops->block_index, (kefir_hashtable_key_t) block_id, &table_value);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find loop containing requested block");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(loop_ptr, (struct kefir_opt_code_loop *) table_value);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_loop_level(const struct kefir_opt_code_loop_collection *loops,
                                         kefir_opt_block_id_t block_id, kefir_uint32_t *level) {
    REQUIRE(loops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer loop collection"));
    REQUIRE(level != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to loop nest level"));

    struct kefir_opt_code_loop *loop;
    kefir_result_t res = kefir_opt_code_loop_collection_find_loop(loops, block_id, &loop);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        *level = loop->level;
    } else {
        *level = 0;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_loop_collection_iter(const struct kefir_opt_code_loop_collection *loops,
                                                   struct kefir_opt_code_loop **loop_ptr,
                                                   struct kefir_opt_code_loop_collection_iterator *iter) {
    REQUIRE(loops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer loop collection"));
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer optimizer loop collection iterator"));

    kefir_hashtable_value_t value;
    kefir_result_t res = kefir_hashtable_iter(&loops->loops, &iter->iter, NULL, &value);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of loop collection iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(loop_ptr, (struct kefir_opt_code_loop *) value);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_loop_collection_next(struct kefir_opt_code_loop **loop_ptr,
                                                   struct kefir_opt_code_loop_collection_iterator *iter) {
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer optimizer loop collection iterator"));

    kefir_hashtable_value_t value;
    kefir_result_t res = kefir_hashtable_next(&iter->iter, NULL, &value);
    if (res == KEFIR_ITERATOR_END) {
        res = KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "End of loop collection iterator");
    }
    REQUIRE_OK(res);

    ASSIGN_PTR(loop_ptr, (struct kefir_opt_code_loop *) value);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_loop_nest_collection_iter(const struct kefir_opt_code_loop_collection *loops,
                                                        const struct kefir_opt_loop_nest **nest_ptr,
                                                        struct kefir_opt_code_loop_nest_collection_iterator *iter) {
    REQUIRE(loops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer loop collection"));
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer optimizer loop nest collection iterator"));

    iter->iter = kefir_list_head(&loops->nests);
    if (iter->iter == NULL) {
        ASSIGN_PTR(nest_ptr, NULL);
        return KEFIR_ITERATOR_END;
    }

    ASSIGN_PTR(nest_ptr, (const struct kefir_opt_loop_nest *) iter->iter->value);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_loop_nest_collection_next(const struct kefir_opt_loop_nest **nest_ptr,
                                                        struct kefir_opt_code_loop_nest_collection_iterator *iter) {
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer optimizer loop nest collection iterator"));

    kefir_list_next(&iter->iter);
    if (iter->iter == NULL) {
        ASSIGN_PTR(nest_ptr, NULL);
        return KEFIR_ITERATOR_END;
    }

    ASSIGN_PTR(nest_ptr, (const struct kefir_opt_loop_nest *) iter->iter->value);
    return KEFIR_OK;
}
