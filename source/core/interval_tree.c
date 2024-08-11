/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#include "kefir/core/interval_tree.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t on_node_removal(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                      kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_interval_tree_node *, node, value);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid interval tree node"));
    ASSIGN_DECL_CAST(struct kefir_interval_tree *, interval_tree, payload);
    REQUIRE(interval_tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid interval tree"));

    if (interval_tree->on_removal.callback != NULL) {
        REQUIRE_OK(interval_tree->on_removal.callback(mem, interval_tree, node->begin, node->end, node->value,
                                                      interval_tree->on_removal.payload));
    }

    memset(node, 0, sizeof(struct kefir_interval_tree_node));
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

static kefir_result_t on_entry_removal(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                       kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_interval_tree_entry *, entry, value);
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid interval tree entry"));

    REQUIRE_OK(kefir_hashtree_free(mem, &entry->nodes));

    memset(entry, 0, sizeof(struct kefir_interval_tree_entry));
    KEFIR_FREE(mem, entry);
    return KEFIR_OK;
}

static kefir_result_t update_node_subtree_max_end(struct kefir_hashtree_node *node) {
    ASSIGN_DECL_CAST(struct kefir_interval_tree_entry *, interval_entry, node->value);

    interval_entry->max_subtree_end = interval_entry->begin;

    struct kefir_hashtree_node *max_subnode;
    REQUIRE_OK(kefir_hashtree_max(&interval_entry->nodes, &max_subnode));
    if (max_subnode != NULL) {
        interval_entry->max_subtree_end = (kefir_interval_tree_key_t) max_subnode->key;
    }

    if (node->left_child != NULL) {
        ASSIGN_DECL_CAST(struct kefir_interval_tree_entry *, left_interval_subentry, node->left_child->value);
        interval_entry->max_subtree_end = MAX(interval_entry->max_subtree_end, left_interval_subentry->max_subtree_end);
    }
    if (node->right_child != NULL) {
        ASSIGN_DECL_CAST(struct kefir_interval_tree_entry *, right_interval_subentry, node->right_child->value);
        interval_entry->max_subtree_end =
            MAX(interval_entry->max_subtree_end, right_interval_subentry->max_subtree_end);
    }

    if (node->parent != NULL) {
        return update_node_subtree_max_end(node->parent);
    } else {
        return KEFIR_OK;
    }
}

static kefir_result_t on_link_modify(struct kefir_hashtree *tree, struct kefir_hashtree_node *node, void *payload) {
    UNUSED(tree);
    UNUSED(payload);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid interval tree node"));

    REQUIRE_OK(update_node_subtree_max_end(node));
    return KEFIR_OK;
}

kefir_result_t kefir_interval_tree_init(struct kefir_interval_tree *tree) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to interval tree"));

    REQUIRE_OK(kefir_hashtree_init(&tree->tree, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&tree->tree, on_entry_removal, tree));
    REQUIRE_OK(kefir_hashtree_on_link_modify(&tree->tree, on_link_modify, NULL));
    tree->on_removal.callback = NULL;
    tree->on_removal.payload = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_interval_tree_free(struct kefir_mem *mem, struct kefir_interval_tree *tree) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid interval tree"));

    REQUIRE_OK(kefir_hashtree_free(mem, &tree->tree));
    memset(tree, 0, sizeof(struct kefir_interval_tree));
    return KEFIR_OK;
}

kefir_result_t kefir_interval_tree_on_remove(struct kefir_interval_tree *tree,
                                             kefir_interval_tree_remove_callback_t callback, void *payload) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid interval tree"));

    tree->on_removal.callback = callback;
    tree->on_removal.payload = payload;
    return KEFIR_OK;
}

kefir_result_t kefir_interval_tree_insert(struct kefir_mem *mem, struct kefir_interval_tree *tree,
                                          kefir_interval_tree_key_t begin, kefir_interval_tree_key_t end,
                                          kefir_interval_tree_value_t value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid interval tree"));
    REQUIRE(begin <= end, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Interval end shall be greater or equal to begin"));

    struct kefir_hashtree_node *tree_node = NULL;
    kefir_result_t res = kefir_hashtree_at(&tree->tree, (kefir_hashtree_key_t) begin, &tree_node);
    struct kefir_interval_tree_entry *entry = NULL;
    if (res == KEFIR_NOT_FOUND) {
        entry = KEFIR_MALLOC(mem, sizeof(struct kefir_interval_tree_entry));
        REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate interval tree entry"));

        entry->begin = begin;
        entry->max_subtree_end = end;
        res = kefir_hashtree_init(&entry->nodes, &kefir_hashtree_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashtree_on_removal(&entry->nodes, on_node_removal, tree));
        REQUIRE_CHAIN(&res, kefir_hashtree_insert(mem, &tree->tree, (kefir_hashtree_key_t) begin,
                                                  (kefir_hashtree_value_t) entry));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, entry);
            return res;
        });
        REQUIRE_OK(kefir_hashtree_at(&tree->tree, (kefir_hashtree_key_t) begin, &tree_node));
    } else {
        REQUIRE_OK(res);
        entry = (struct kefir_interval_tree_entry *) tree_node->value;
    }

    struct kefir_interval_tree_node *node = KEFIR_MALLOC(mem, sizeof(struct kefir_interval_tree_node));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate interval tree node"));

    node->begin = begin;
    node->end = end;
    node->value = value;
    res = kefir_hashtree_insert(mem, &entry->nodes, (kefir_hashtree_key_t) end, (kefir_hashtree_value_t) node);
    if (res == KEFIR_ALREADY_EXISTS) {
        res = KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Identical interval already exists in the interval tree");
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, node);
        return res;
    });

    entry->max_subtree_end = MAX(entry->max_subtree_end, end);
    if (tree_node->parent != NULL) {
        REQUIRE_OK(update_node_subtree_max_end(tree_node->parent));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_interval_tree_get(const struct kefir_interval_tree *tree, kefir_interval_tree_key_t begin,
                                       kefir_interval_tree_key_t end, struct kefir_interval_tree_node **node_ptr) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid interval tree"));
    REQUIRE(begin <= end, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Interval end shall be greater or equal to begin"));
    REQUIRE(node_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to interval tree node"));

    struct kefir_hashtree_node *tree_node = NULL;
    kefir_result_t res = kefir_hashtree_at(&tree->tree, (kefir_hashtree_key_t) begin, &tree_node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find interval in the interval tree");
    }
    REQUIRE_OK(res);

    ASSIGN_DECL_CAST(const struct kefir_interval_tree_entry *, entry, tree_node->value);
    res = kefir_hashtree_at(&entry->nodes, (kefir_hashtree_key_t) end, &tree_node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find interval in the interval tree");
    }
    REQUIRE_OK(res);

    *node_ptr = (struct kefir_interval_tree_node *) tree_node->value;
    return KEFIR_OK;
}

static kefir_result_t find_min_node(struct kefir_hashtree_node *node, kefir_interval_tree_key_t position, struct kefir_interval_tree_node **min_node) {
    REQUIRE(node != NULL, KEFIR_OK);
    ASSIGN_DECL_CAST(const struct kefir_interval_tree_entry *, entry,
        node->value);

    if (position < entry->begin) {
        return find_min_node(node->left_child, position, min_node);
    } else if (node->left_child != NULL) {
        ASSIGN_DECL_CAST(const struct kefir_interval_tree_entry *, left_child_entry,
            node->left_child->value);
        if (position >= left_child_entry->begin && position < left_child_entry->max_subtree_end) {
            return find_min_node(node->left_child, position, min_node);
        }
    }
    REQUIRE(position >= entry->begin && position < entry->max_subtree_end, KEFIR_OK);

    struct kefir_hashtree_node *position_iter;
    kefir_result_t res = kefir_hashtree_lower_bound(&entry->nodes, (kefir_hashtree_key_t) position, &position_iter);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
    } else {
        REQUIRE_OK(kefir_hashtree_min(&entry->nodes, &position_iter));
    }

    for (; position_iter != NULL;) {
        ASSIGN_DECL_CAST(struct kefir_interval_tree_node *, interval_node,
            position_iter->value);
        if (position < interval_node->begin) {
            break;
        }

        if (position >= interval_node->begin && position < interval_node->end) {
            *min_node = interval_node;
            return KEFIR_OK;
        }

        position_iter = kefir_hashtree_next_node(&entry->nodes, position_iter);
    }
    
    return find_min_node(node->right_child, position, min_node);
}

kefir_result_t kefir_interval_tree_find(const struct kefir_interval_tree *tree, kefir_interval_tree_key_t position,
                                       struct kefir_interval_tree_finder *finder, struct kefir_interval_tree_node **interval_node_ptr) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid interval tree"));
    REQUIRE(finder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to interval tree finder"));

    finder->node = NULL;
    finder->position = position;
    REQUIRE_OK(find_min_node(tree->tree.root, position, &finder->node));

    REQUIRE(finder->node != NULL, KEFIR_ITERATOR_END);
    ASSIGN_PTR(interval_node_ptr, finder->node);
    return KEFIR_OK;
}

kefir_result_t kefir_interval_tree_find_next(const struct kefir_interval_tree *tree, struct kefir_interval_tree_finder *finder, struct kefir_interval_tree_node **interval_node_ptr) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid interval tree"));
    REQUIRE(finder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to interval tree finder"));
    REQUIRE(finder->node != NULL, KEFIR_ITERATOR_END);

    struct kefir_hashtree_node *entry_node, *node;
    REQUIRE_OK(kefir_hashtree_at(&tree->tree, (kefir_hashtree_key_t) finder->node->begin, &entry_node));
    ASSIGN_DECL_CAST(const struct kefir_interval_tree_entry *, entry,
        entry_node->value);

    REQUIRE_OK(kefir_hashtree_at(&entry->nodes, (kefir_hashtree_key_t) finder->node->end, &node));
    node = kefir_hashtree_next_node(&tree->tree, node);
    if (node != NULL) {
        ASSIGN_DECL_CAST(struct kefir_interval_tree_node *, interval_node,
            node->value);
        REQUIRE(finder->position >= interval_node->begin && finder->position < interval_node->end,
            KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected interval nodes of the same entry to have ascending end positions"));
        
        finder->node = interval_node;
        ASSIGN_PTR(interval_node_ptr, interval_node);
        return KEFIR_OK;
    }

    for (entry_node = kefir_hashtree_next_node(&tree->tree, entry_node); entry_node != NULL; entry_node = kefir_hashtree_next_node(&tree->tree, entry_node)) {
        ASSIGN_DECL_CAST(const struct kefir_interval_tree_entry *, entry,
            entry_node->value);

        REQUIRE(finder->position >= entry->begin, KEFIR_ITERATOR_END);
        if (finder->position < entry->max_subtree_end) {
            struct kefir_hashtree_node *position_iter;
            kefir_result_t res = kefir_hashtree_lower_bound(&entry->nodes, (kefir_hashtree_key_t) finder->position, &position_iter);
            if (res != KEFIR_NOT_FOUND) {
                REQUIRE_OK(res);
            }

            for (; position_iter != NULL;) {
                ASSIGN_DECL_CAST(struct kefir_interval_tree_node *, interval_node,
                    position_iter->value);
                if (finder->position >= interval_node->begin && finder->position < interval_node->end) {
                    finder->node = interval_node;
                    ASSIGN_PTR(interval_node_ptr, interval_node);
                    return KEFIR_OK;
                }

                position_iter = kefir_hashtree_next_node(&entry->nodes, position_iter);
            }
        }
    }

    return KEFIR_ITERATOR_END;
}

#define ITER_NEXT                                                                            \
    do {                                                                                     \
        for (; entry_node != NULL;) {                                                        \
            ASSIGN_DECL_CAST(struct kefir_interval_tree_entry *, entry, entry_node->value);  \
            tree_node = kefir_hashtree_iter(&entry->nodes, &iter->node);                     \
            if (tree_node != NULL) {                                                         \
                ASSIGN_DECL_CAST(struct kefir_interval_tree_node *, node, tree_node->value); \
                ASSIGN_PTR(node_ptr, node);                                                  \
                return KEFIR_OK;                                                             \
            } else {                                                                         \
                entry_node = kefir_hashtree_next(&iter->entry);                              \
            }                                                                                \
        }                                                                                    \
    } while (0)

kefir_result_t kefir_interval_tree_iter(const struct kefir_interval_tree *tree,
                                        struct kefir_interval_tree_iterator *iter,
                                        struct kefir_interval_tree_node **node_ptr) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid interval tree"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to interval tree iterator"));

    struct kefir_hashtree_node *entry_node = kefir_hashtree_iter(&tree->tree, &iter->entry);
    struct kefir_hashtree_node *tree_node;
    ITER_NEXT;

    return KEFIR_ITERATOR_END;
}

kefir_result_t kefir_interval_tree_next(struct kefir_interval_tree_iterator *iter,
                                        struct kefir_interval_tree_node **node_ptr) {
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to interval tree iterator"));

    struct kefir_hashtree_node *tree_node = kefir_hashtree_next(&iter->node);
    if (tree_node != NULL) {
        ASSIGN_DECL_CAST(struct kefir_interval_tree_node *, node, tree_node->value);
        ASSIGN_PTR(node_ptr, node);
        return KEFIR_OK;
    }

    struct kefir_hashtree_node *entry_node = kefir_hashtree_next(&iter->entry);
    ITER_NEXT;

    return KEFIR_ITERATOR_END;
}

#undef ITER_NEXT
