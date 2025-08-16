#include "kefir/optimizer/loop_nest.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t free_loop(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_loop *, loop, value);
    REQUIRE(loop != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code loop"));

    REQUIRE_OK(kefir_hashtreeset_free(mem, &loop->loop_blocks));
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

    REQUIRE_OK(kefir_hashtree_init(&loops->loops, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&loops->loops, free_loop, NULL));
    REQUIRE_OK(kefir_list_init(&loops->nests));
    REQUIRE_OK(kefir_list_on_remove(&loops->nests, free_loop_nest, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_loop_collection_free(struct kefir_mem *mem,
                                                   struct kefir_opt_code_loop_collection *loops) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(loops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer loop collection"));

    REQUIRE_OK(kefir_list_free(mem, &loops->nests));
    REQUIRE_OK(kefir_hashtree_free(mem, &loops->loops));
    return KEFIR_OK;
}

#define IS_BLOCK_REACHABLE(_structure, _block_id)      \
    ((_block_id) == (_structure)->code->entry_point || \
     (_structure)->blocks[(_block_id)].immediate_dominator != KEFIR_ID_NONE)

static kefir_result_t build_loop_impl(struct kefir_mem *mem, const struct kefir_opt_code_structure *structure,
                                      struct kefir_opt_code_loop *loop, struct kefir_list *traversal_queue) {
    REQUIRE_OK(kefir_hashtreeset_clean(mem, &loop->loop_blocks));
    REQUIRE_OK(kefir_list_clear(mem, traversal_queue));

    REQUIRE_OK(kefir_list_insert_after(mem, traversal_queue, NULL, (void *) (kefir_uptr_t) loop->loop_exit_block_id));
    for (struct kefir_list_entry *iter = kefir_list_head(traversal_queue); iter != NULL;
         iter = kefir_list_head(traversal_queue)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(mem, traversal_queue, iter));

        if (kefir_hashtreeset_has(&loop->loop_blocks, (kefir_hashtreeset_entry_t) block_id)) {
            continue;
        }
        REQUIRE_OK(kefir_hashtreeset_add(mem, &loop->loop_blocks, (kefir_hashtreeset_entry_t) block_id));

        if (block_id != loop->loop_entry_block_id) {
            for (const struct kefir_list_entry *pred_iter = kefir_list_head(&structure->blocks[block_id].predecessors);
                 pred_iter != NULL; kefir_list_next(&pred_iter)) {
                REQUIRE_OK(
                    kefir_list_insert_after(mem, traversal_queue, kefir_list_tail(traversal_queue), pred_iter->value));
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t build_loop(struct kefir_mem *mem, struct kefir_opt_code_loop_collection *loops,
                                 const struct kefir_opt_code_structure *structure,
                                 kefir_opt_block_id_t loop_entry_block_id, kefir_opt_block_id_t loop_exit_block_id) {
    struct kefir_opt_code_loop *loop = KEFIR_MALLOC(mem, sizeof(struct kefir_opt_code_loop));
    REQUIRE(loop != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Unable to allocate optimzier code loop"));

    loop->loop_entry_block_id = loop_entry_block_id;
    loop->loop_exit_block_id = loop_exit_block_id;
    kefir_result_t res = kefir_hashtreeset_init(&loop->loop_blocks, &kefir_hashtree_uint_ops);
    REQUIRE_CHAIN(&res, kefir_hashtree_insert(mem, &loops->loops, (kefir_hashtree_key_t) KEFIR_OPT_CODE_LOOP_ID(loop),
                                              (kefir_hashtree_value_t) loop));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, loops);
        return res;
    });

    struct kefir_list traversal_queue;
    REQUIRE_OK(kefir_list_init(&traversal_queue));
    res = build_loop_impl(mem, structure, loop, &traversal_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &traversal_queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &traversal_queue));
    return KEFIR_OK;
}

static kefir_bool_t loop_contained_within(const struct kefir_opt_code_loop *loop,
                                          const struct kefir_opt_code_loop *contained_loop) {
    return kefir_hashtreeset_has(&contained_loop->loop_blocks, (kefir_hashtreeset_entry_t) loop->loop_entry_block_id) &&
           kefir_hashtreeset_has(&contained_loop->loop_blocks, (kefir_hashtreeset_entry_t) loop->loop_exit_block_id);
}

static kefir_result_t insert_into_nest(struct kefir_mem *mem, const struct kefir_opt_code_loop *loop,
                                       struct kefir_tree_node *nest) {
    for (struct kefir_tree_node *child = kefir_tree_first_child(nest); child != NULL;
         child = kefir_tree_next_sibling(child)) {
        if (loop_contained_within(loop, (const struct kefir_opt_code_loop *) child->value)) {
            return insert_into_nest(mem, loop, child);
        }
    }

    REQUIRE_OK(kefir_tree_insert_child(mem, nest, (void *) loop, NULL));
    return KEFIR_OK;
}

static kefir_result_t update_loop_nest(struct kefir_mem *mem, struct kefir_opt_code_loop_collection *loops,
                                       const struct kefir_opt_code_loop *loop) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&loops->nests); iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_opt_loop_nest *, nest, iter->value);

        if (loop_contained_within(loop, (const struct kefir_opt_code_loop *) nest->nest.value)) {
            REQUIRE_OK(insert_into_nest(mem, loop, &nest->nest));
        } else if (loop_contained_within((const struct kefir_opt_code_loop *) nest->nest.value, loop)) {
            REQUIRE_OK(kefir_tree_insert_parent(mem, &nest->nest, (void *) loop, NULL));
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
    return KEFIR_OK;
}

static kefir_result_t build_loop_nests(struct kefir_mem *mem, struct kefir_opt_code_loop_collection *loops) {
    kefir_result_t res;
    const struct kefir_opt_code_loop *loop;
    struct kefir_opt_code_loop_collection_iterator iter;
    for (res = kefir_opt_code_loop_collection_iter(loops, &loop, &iter); res == KEFIR_OK && loop != NULL;
         res = kefir_opt_code_loop_collection_next(&loop, &iter)) {
        REQUIRE_OK(update_loop_nest(mem, loops, loop));
    }

    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_loop_collection_build(struct kefir_mem *mem, struct kefir_opt_code_loop_collection *loops,
                                                    const struct kefir_opt_code_structure *structure) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(loops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer loop collection"));
    REQUIRE(structure != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code structure"));

    kefir_size_t block_count;
    REQUIRE_OK(kefir_opt_code_container_block_count(structure->code, &block_count));
    for (kefir_opt_block_id_t block_id = 0; block_id < block_count; block_id++) {
        if (!IS_BLOCK_REACHABLE(structure, block_id)) {
            continue;
        }

        for (const struct kefir_list_entry *succ_iter = kefir_list_head(&structure->blocks[block_id].successors);
             succ_iter != NULL; kefir_list_next(&succ_iter)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, (kefir_uptr_t) succ_iter->value);
            if (!IS_BLOCK_REACHABLE(structure, successor_block_id)) {
                continue;
            }

            const struct kefir_opt_code_block *successor_block;
            REQUIRE_OK(kefir_opt_code_container_block(structure->code, successor_block_id, &successor_block));

            kefir_bool_t is_dominator;
            REQUIRE_OK(kefir_opt_code_structure_is_dominator(structure, block_id, successor_block_id, &is_dominator));
            if (is_dominator) {
                REQUIRE_OK(build_loop(mem, loops, structure, successor_block_id, block_id));
            }
        }
    }

    REQUIRE_OK(build_loop_nests(mem, loops));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_loop_collection_iter(const struct kefir_opt_code_loop_collection *loops,
                                                   const struct kefir_opt_code_loop **loop_ptr,
                                                   struct kefir_opt_code_loop_collection_iterator *iter) {
    REQUIRE(loops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer loop collection"));
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer optimizer loop collection iterator"));

    struct kefir_hashtree_node *node = kefir_hashtree_iter(&loops->loops, &iter->iter);
    if (node == NULL) {
        ASSIGN_PTR(loop_ptr, NULL);
        return KEFIR_ITERATOR_END;
    }

    ASSIGN_PTR(loop_ptr, (const struct kefir_opt_code_loop *) node->value);
    return KEFIR_OK;
}

kefir_result_t kefir_opt_code_loop_collection_next(const struct kefir_opt_code_loop **loop_ptr,
                                                   struct kefir_opt_code_loop_collection_iterator *iter) {
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer optimizer loop collection iterator"));

    struct kefir_hashtree_node *node = kefir_hashtree_next(&iter->iter);
    if (node == NULL) {
        ASSIGN_PTR(loop_ptr, NULL);
        return KEFIR_ITERATOR_END;
    }

    ASSIGN_PTR(loop_ptr, (const struct kefir_opt_code_loop *) node->value);
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
