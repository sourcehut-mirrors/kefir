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

#include "kefir/core/tree.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

kefir_result_t kefir_tree_init(struct kefir_tree_node *root, void *value) {
    REQUIRE(root != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tree node"));
    root->value = value;
    root->parent = NULL;
    root->prev_sibling = NULL;
    root->next_sibling = NULL;
    root->first_child = NULL;
    root->last_child = NULL;
    root->removal_callback = NULL;
    root->removal_payload = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_tree_free(struct kefir_mem *mem, struct kefir_tree_node *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tree node"));

    if (node->removal_callback != NULL) {
        REQUIRE_OK(node->removal_callback(mem, node->value, node->removal_payload));
    }
    for (struct kefir_tree_node *child = node->first_child; child != NULL;) {
        struct kefir_tree_node *next = child->next_sibling;
        REQUIRE_OK(kefir_tree_free(mem, child));
        KEFIR_FREE(mem, child);
        child = next;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_tree_on_removal(struct kefir_tree_node *node,
                                     kefir_result_t (*callback)(struct kefir_mem *, void *, void *), void *payload) {
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tree node"));
    node->removal_callback = callback;
    node->removal_payload = payload;
    return KEFIR_OK;
}

kefir_result_t kefir_tree_insert_child(struct kefir_mem *mem, struct kefir_tree_node *node, void *value,
                                       struct kefir_tree_node **subnode) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tree node"));
    struct kefir_tree_node *child = KEFIR_MALLOC(mem, sizeof(struct kefir_tree_node));
    REQUIRE(child != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate tree node"));
    child->value = value;
    child->parent = node;
    child->removal_callback = node->removal_callback;
    child->removal_payload = node->removal_payload;
    child->first_child = NULL;
    child->last_child = NULL;
    child->prev_sibling = node->last_child;
    child->next_sibling = NULL;

    if (node->first_child == NULL) {
        node->first_child = child;
    }
    node->last_child = child;

    if (child->prev_sibling != NULL) {
        child->prev_sibling->next_sibling = child;
    }
    if (subnode != NULL) {
        *subnode = child;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_tree_insert_parent(struct kefir_mem *mem, struct kefir_tree_node *node, void *value,
                                        struct kefir_tree_node **subnode) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tree node"));

    struct kefir_tree_node *child = KEFIR_MALLOC(mem, sizeof(struct kefir_tree_node));
    REQUIRE(child != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate tree node"));
    child->value = node->value;
    child->parent = node;
    child->prev_sibling = NULL;
    child->next_sibling = NULL;
    child->removal_callback = node->removal_callback;
    child->removal_payload = node->removal_payload;
    child->first_child = node->first_child;
    child->last_child = node->last_child;
    
    node->first_child = child;
    node->last_child = child;

    node->value = value;
    for (struct kefir_tree_node *subchild = kefir_tree_first_child(child); subchild != NULL;
         subchild = kefir_tree_next_sibling(subchild)) {
        subchild->parent = child;
    }

    ASSIGN_PTR(subnode, child);
    return KEFIR_OK;
}

struct kefir_tree_node *kefir_tree_first_child(const struct kefir_tree_node *node) {
    REQUIRE(node != NULL, NULL);
    return node->first_child;
}

struct kefir_tree_node *kefir_tree_next_sibling(const struct kefir_tree_node *node) {
    REQUIRE(node != NULL, NULL);
    return node->next_sibling;
}

struct kefir_tree_node *kefir_tree_prev_sibling(const struct kefir_tree_node *node) {
    REQUIRE(node != NULL, NULL);
    return node->prev_sibling;
}

kefir_result_t kefir_tree_iter(struct kefir_tree_node *root, struct kefir_tree_node_iterator *iter) {
    REQUIRE(root != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tree node"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tree node iterator"));
    iter->current = root;
    REQUIRE_OK(kefir_list_init(&iter->pending));
    return KEFIR_OK;
    ;
}

kefir_result_t kefir_tree_iter_next(struct kefir_mem *mem, struct kefir_tree_node_iterator *iter) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tree node iterator"));
    if (iter->current == NULL) {
        return KEFIR_OK;
    }
    for (struct kefir_tree_node *child = kefir_tree_first_child(iter->current); child != NULL;
         child = kefir_tree_next_sibling(child)) {
        REQUIRE_OK(kefir_list_insert_after(mem, &iter->pending, kefir_list_tail(&iter->pending), child));
    }
    struct kefir_list_entry *head = kefir_list_head(&iter->pending);
    if (head != NULL) {
        iter->current = (struct kefir_tree_node *) head->value;
        REQUIRE_OK(kefir_list_pop(mem, &iter->pending, head));
        return KEFIR_OK;
    } else {
        iter->current = NULL;
        REQUIRE_OK(kefir_list_free(mem, &iter->pending));
        return KEFIR_ITERATOR_END;
    }
}

kefir_result_t kefir_tree_iter_free(struct kefir_mem *mem, struct kefir_tree_node_iterator *iter) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid tree node iterator"));
    iter->current = NULL;
    REQUIRE_OK(kefir_list_free(mem, &iter->pending));
    return KEFIR_OK;
    ;
}
