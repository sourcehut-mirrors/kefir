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

#include <string.h>
#include <assert.h>
#include "kefir/core/hashtree.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

static kefir_result_t node_free(struct kefir_mem *mem, struct kefir_hashtree *tree, struct kefir_hashtree_node *node) {
    if (node == NULL) {
        return KEFIR_OK;
    }
    REQUIRE_OK(node_free(mem, tree, node->left_child));
    REQUIRE_OK(node_free(mem, tree, node->right_child));
    if (tree->node_remove.callback != NULL) {
        REQUIRE_OK(tree->node_remove.callback(mem, tree, node->key, node->value, tree->node_remove.data));
    }
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

static struct kefir_hashtree_node *node_alloc(struct kefir_mem *mem, kefir_hashtree_hash_t hash,
                                              struct kefir_hashtree_node *parent, kefir_hashtree_key_t key,
                                              kefir_hashtree_value_t value) {
    struct kefir_hashtree_node *node = KEFIR_MALLOC(mem, sizeof(struct kefir_hashtree_node));
    REQUIRE_ELSE(node != NULL, { return NULL; });
    node->hash = hash;
    node->key = key;
    node->value = value;
    node->height = 0;
    node->parent = parent;
    node->left_child = NULL;
    node->right_child = NULL;
    return node;
}

#define NODE_HEIGHT(_node) ((_node) != NULL ? (_node)->height + 1 : 0)
#define NODE_EVAL_HEIGHT(_node) \
    ((_node) != NULL ? MAX(NODE_HEIGHT((_node)->left_child), NODE_HEIGHT((_node)->right_child)) : 0)
#define NODE_BF(_node)                                                                                            \
    ((_node) != NULL                                                                                              \
         ? ((kefir_int64_t) NODE_HEIGHT((_node)->right_child)) - (kefir_int64_t) NODE_HEIGHT((_node)->left_child) \
         : 0)
#define NODE_UPDATE_HEIGHT(_node)                        \
    do {                                                 \
        if ((_node) != NULL) {                           \
            (_node)->height = NODE_EVAL_HEIGHT((_node)); \
        }                                                \
    } while (0)

static struct kefir_hashtree_node *right_rotate(struct kefir_hashtree_node *root) {
    assert(root != NULL);
    assert(root->left_child != NULL);

    // Rotate nodes
    struct kefir_hashtree_node *new_root = root->left_child;
    root->left_child = new_root->right_child;
    new_root->right_child = root;

    // Update parent references
    if (root->parent != NULL && root->parent->left_child == root) {
        root->parent->left_child = new_root;
    } else if (root->parent != NULL && root->parent->right_child == root) {
        root->parent->right_child = new_root;
    }
    new_root->parent = root->parent;
    root->parent = new_root;
    if (root->left_child != NULL) {
        root->left_child->parent = root;
    }

    // Update node heights
    NODE_UPDATE_HEIGHT(root);
    NODE_UPDATE_HEIGHT(new_root);
    return new_root;
}

static struct kefir_hashtree_node *left_rotate(struct kefir_hashtree_node *root) {
    assert(root != NULL);
    assert(root->right_child != NULL);

    // Rotate nodes
    struct kefir_hashtree_node *new_root = root->right_child;
    root->right_child = new_root->left_child;
    new_root->left_child = root;

    // Update node parent references
    if (root->parent != NULL && root->parent->left_child == root) {
        root->parent->left_child = new_root;
    } else if (root->parent != NULL && root->parent->right_child == root) {
        root->parent->right_child = new_root;
    }
    new_root->parent = root->parent;
    root->parent = new_root;
    if (root->right_child != NULL) {
        root->right_child->parent = root;
    }

    // Update node heights
    NODE_UPDATE_HEIGHT(root);
    NODE_UPDATE_HEIGHT(new_root);
    return new_root;
}

static struct kefir_hashtree_node *left_right_rotate(struct kefir_hashtree_node *root) {
    root->left_child = left_rotate(root->left_child);
    return right_rotate(root);
}

static struct kefir_hashtree_node *right_left_rotate(struct kefir_hashtree_node *root) {
    root->right_child = right_rotate(root->right_child);
    return left_rotate(root);
}

#define ON_LINK_MODIFY(_res, _tree, _node)                                                                       \
    do {                                                                                                         \
        if ((_tree)->node_link_modify.callback != NULL && (_node) != NULL) {                                     \
            REQUIRE_CHAIN((_res),                                                                                \
                          (_tree)->node_link_modify.callback((_tree), (_node), (_tree)->node_link_modify.data)); \
        }                                                                                                        \
    } while (0)

static kefir_result_t balance_node(struct kefir_hashtree *tree, struct kefir_hashtree_node **root_ptr) {
    assert(root_ptr != NULL);
    assert(*root_ptr != NULL);
    assert(NODE_BF(*root_ptr) >= -2 && NODE_BF(*root_ptr) <= 2);

    struct kefir_hashtree_node *root = *root_ptr;
    struct kefir_hashtree_node *new_root = root;
    if (NODE_HEIGHT(root->left_child) > NODE_HEIGHT(root->right_child) + 1) {
        assert(root->left_child != NULL);
        kefir_size_t leftLeftHeight = NODE_HEIGHT(root->left_child->left_child);
        kefir_size_t leftRightHeight = NODE_HEIGHT(root->left_child->right_child);
        if (leftLeftHeight >= leftRightHeight) {
            new_root = right_rotate(root);
        } else {
            new_root = left_right_rotate(root);
        }
    } else if (NODE_HEIGHT(root->right_child) > NODE_HEIGHT(root->left_child) + 1) {
        assert(root->right_child != NULL);
        kefir_size_t rightLeftHeight = NODE_HEIGHT(root->right_child->left_child);
        kefir_size_t rightRightHeight = NODE_HEIGHT(root->right_child->right_child);
        if (rightRightHeight >= rightLeftHeight) {
            new_root = left_rotate(root);
        } else {
            new_root = right_left_rotate(root);
        }
    }

    NODE_UPDATE_HEIGHT(new_root);
    if (root != new_root) {
        kefir_result_t res = KEFIR_OK;
        ON_LINK_MODIFY(&res, tree, root->left_child);
        ON_LINK_MODIFY(&res, tree, root->right_child);
        ON_LINK_MODIFY(&res, tree, root);
        REQUIRE_OK(res);
    }
    assert(NODE_BF(root) >= -1 && NODE_BF(root) <= 1);
    *root_ptr = new_root;
    return KEFIR_OK;
}

static kefir_result_t node_insert(struct kefir_mem *mem, struct kefir_hashtree *tree, struct kefir_hashtree_node *root,
                                  kefir_hashtree_hash_t hash, kefir_hashtree_key_t key, kefir_hashtree_value_t value,
                                  kefir_hashtree_value_t *oldvalue, bool replace,
                                  struct kefir_hashtree_node **root_ptr) {
    assert(NODE_BF(root) >= -1 && NODE_BF(root) <= 1);
    if (hash < root->hash || (hash == root->hash && tree->ops->compare(key, root->key, tree->ops->data) < 0)) {
        if (root->left_child == NULL) {
            struct kefir_hashtree_node *node = node_alloc(mem, hash, root, key, value);
            REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate hash tree node"));
            root->left_child = node;
            kefir_result_t res = KEFIR_OK;
            ON_LINK_MODIFY(&res, tree, node);
            ON_LINK_MODIFY(&res, tree, root);
        } else {
            REQUIRE_OK(node_insert(mem, tree, root->left_child, hash, key, value, oldvalue, replace, NULL));
        }

        struct kefir_hashtree_node *new_root = root;
        REQUIRE_OK(balance_node(tree, &new_root));
        ASSIGN_PTR(root_ptr, new_root);
    } else if (hash > root->hash || (hash == root->hash && tree->ops->compare(key, root->key, tree->ops->data) > 0)) {
        if (root->right_child == NULL) {
            struct kefir_hashtree_node *node = node_alloc(mem, hash, root, key, value);
            REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate hash tree node"));
            root->right_child = node;
            kefir_result_t res = KEFIR_OK;
            ON_LINK_MODIFY(&res, tree, node);
            ON_LINK_MODIFY(&res, tree, root);
        } else {
            REQUIRE_OK(node_insert(mem, tree, root->right_child, hash, key, value, oldvalue, replace, NULL));
        }

        struct kefir_hashtree_node *new_root = root;
        REQUIRE_OK(balance_node(tree, &new_root));
        ASSIGN_PTR(root_ptr, new_root);
    } else if (replace) {
        if (oldvalue != NULL) {
            *oldvalue = root->value;
        }
        root->value = value;
    } else {
        return KEFIR_SET_ERROR(KEFIR_ALREADY_EXISTS, "Hash tree node with specified key already exists in the tree");
    }
    assert(NODE_BF(root) >= -1 && NODE_BF(root) <= 1);
    return KEFIR_OK;
}

static kefir_result_t node_find(struct kefir_hashtree_node *root, const struct kefir_hashtree *tree,
                                kefir_hashtree_hash_t hash, kefir_hashtree_key_t key,
                                struct kefir_hashtree_node **result) {
    if (root == NULL) {
        return KEFIR_NOT_FOUND;
    }

    assert(NODE_BF(root) >= -1 && NODE_BF(root) <= 1);
    if (hash < root->hash || (hash == root->hash && tree->ops->compare(key, root->key, tree->ops->data) < 0)) {
        return node_find(root->left_child, tree, hash, key, result);
    } else if (hash > root->hash || (hash == root->hash && tree->ops->compare(key, root->key, tree->ops->data) > 0)) {
        return node_find(root->right_child, tree, hash, key, result);
    } else {
        *result = root;
        return KEFIR_OK;
    }
}

static kefir_result_t node_find_lower_bound(struct kefir_hashtree_node *root, const struct kefir_hashtree *tree,
                                            kefir_hashtree_hash_t hash, kefir_hashtree_key_t key,
                                            struct kefir_hashtree_node **result) {
    if (root == NULL) {
        return *result != NULL ? KEFIR_OK : KEFIR_NOT_FOUND;
    }

    assert(NODE_BF(root) >= -1 && NODE_BF(root) <= 1);
    if (hash < root->hash || (hash == root->hash && tree->ops->compare(key, root->key, tree->ops->data) < 0)) {
        return node_find_lower_bound(root->left_child, tree, hash, key, result);
    } else if (hash > root->hash || (hash == root->hash && tree->ops->compare(key, root->key, tree->ops->data) > 0)) {
        *result = root;
        return node_find_lower_bound(root->right_child, tree, hash, key, result);
    } else {
        *result = root;
        return KEFIR_OK;
    }
}

static kefir_result_t node_find_upper_bound(struct kefir_hashtree_node *root, const struct kefir_hashtree *tree,
                                            kefir_hashtree_hash_t hash, kefir_hashtree_key_t key,
                                            struct kefir_hashtree_node **result) {
    if (root == NULL) {
        return *result != NULL ? KEFIR_OK : KEFIR_NOT_FOUND;
    }

    assert(NODE_BF(root) >= -1 && NODE_BF(root) <= 1);
    if (hash < root->hash || (hash == root->hash && tree->ops->compare(key, root->key, tree->ops->data) < 0)) {
        *result = root;
        return node_find_upper_bound(root->left_child, tree, hash, key, result);
    } else if (hash > root->hash || (hash == root->hash && tree->ops->compare(key, root->key, tree->ops->data) > 0)) {
        return node_find_upper_bound(root->right_child, tree, hash, key, result);
    } else {
        *result = root;
        return KEFIR_OK;
    }
}

static struct kefir_hashtree_node *node_minimum(struct kefir_hashtree_node *root) {
    if (root == NULL || root->left_child == NULL) {
        return root;
    } else {
        return node_minimum(root->left_child);
    }
}

kefir_result_t kefir_hashtree_init(struct kefir_hashtree *tree, const struct kefir_hashtree_ops *ops) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hash tree pointer"));
    REQUIRE(ops != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hash tree operation pointer"));
    tree->ops = ops;
    tree->root = NULL;
    tree->node_remove.callback = NULL;
    tree->node_remove.data = NULL;
    tree->node_link_modify.callback = NULL;
    tree->node_link_modify.data = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_hashtree_on_removal(struct kefir_hashtree *tree,
                                         kefir_result_t (*callback)(struct kefir_mem *, struct kefir_hashtree *,
                                                                    kefir_hashtree_key_t, kefir_hashtree_value_t,
                                                                    void *),
                                         void *data) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hash tree pointer"));
    tree->node_remove.callback = callback;
    tree->node_remove.data = data;
    return KEFIR_OK;
}

kefir_result_t kefir_hashtree_on_link_modify(struct kefir_hashtree *tree,
                                             kefir_result_t (*callback)(struct kefir_hashtree *,
                                                                        struct kefir_hashtree_node *, void *),
                                             void *data) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hash tree pointer"));
    tree->node_link_modify.callback = callback;
    tree->node_link_modify.data = data;
    return KEFIR_OK;
}

kefir_result_t kefir_hashtree_free(struct kefir_mem *mem, struct kefir_hashtree *tree) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hash tree pointer"));
    REQUIRE_OK(node_free(mem, tree, tree->root));
    tree->root = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_hashtree_insert(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                     kefir_hashtree_value_t value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hash tree pointer"));
    assert(NODE_BF(tree->root) >= -1 && NODE_BF(tree->root) <= 1);
    kefir_hashtree_hash_t hash = tree->ops->hash(key, tree->ops->data);
    if (tree->root == NULL) {
        tree->root = node_alloc(mem, hash, NULL, key, value);
        kefir_result_t res = KEFIR_OK;
        ON_LINK_MODIFY(&res, tree, tree->root);
        REQUIRE_OK(res);
    } else {
        REQUIRE_OK(node_insert(mem, tree, tree->root, hash, key, value, NULL, false, &tree->root));
    }
    assert(NODE_BF(tree->root) >= -1 && NODE_BF(tree->root) <= 1);
    return KEFIR_OK;
}

kefir_result_t kefir_hashtree_at(const struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                 struct kefir_hashtree_node **result) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hash tree pointer"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid result pointer"));
    assert(NODE_BF(tree->root) >= -1 && NODE_BF(tree->root) <= 1);
    *result = NULL;
    kefir_hashtree_hash_t hash = tree->ops->hash(key, tree->ops->data);
    if (node_find(tree->root, tree, hash, key, result) != KEFIR_OK) {
        return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Could not find tree node with specified key");
    }
    return KEFIR_OK;
}

static struct kefir_hashtree_node *min_node(struct kefir_hashtree_node *node) {
    if (node == NULL) {
        return NULL;
    }
    struct kefir_hashtree_node *child = min_node(node->left_child);
    if (child != NULL) {
        return child;
    } else {
        return node;
    }
}

static struct kefir_hashtree_node *max_node(struct kefir_hashtree_node *node) {
    if (node == NULL) {
        return NULL;
    }
    struct kefir_hashtree_node *child = max_node(node->right_child);
    if (child != NULL) {
        return child;
    } else {
        return node;
    }
}

kefir_result_t kefir_hashtree_min(const struct kefir_hashtree *tree, struct kefir_hashtree_node **result) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hash tree pointer"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid result pointer"));
    assert(NODE_BF(tree->root) >= -1 && NODE_BF(tree->root) <= 1);
    *result = min_node(tree->root);
    return KEFIR_OK;
}

kefir_result_t kefir_hashtree_max(const struct kefir_hashtree *tree, struct kefir_hashtree_node **result) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hash tree pointer"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid result pointer"));
    assert(NODE_BF(tree->root) >= -1 && NODE_BF(tree->root) <= 1);
    *result = max_node(tree->root);
    return KEFIR_OK;
}

kefir_result_t kefir_hashtree_lower_bound(const struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                          struct kefir_hashtree_node **result) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hash tree pointer"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid result pointer"));
    assert(NODE_BF(tree->root) >= -1 && NODE_BF(tree->root) <= 1);
    *result = NULL;
    kefir_hashtree_hash_t hash = tree->ops->hash(key, tree->ops->data);
    if (node_find_lower_bound(tree->root, tree, hash, key, result) != KEFIR_OK) {
        return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Could not find the lower bound tree node for provided key");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_hashtree_upper_bound(const struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                          struct kefir_hashtree_node **result) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hash tree pointer"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid result pointer"));
    assert(NODE_BF(tree->root) >= -1 && NODE_BF(tree->root) <= 1);
    *result = NULL;
    kefir_hashtree_hash_t hash = tree->ops->hash(key, tree->ops->data);
    if (node_find_upper_bound(tree->root, tree, hash, key, result) != KEFIR_OK) {
        return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Could not find the upper bound tree node for provided key");
    }
    return KEFIR_OK;
}

kefir_bool_t kefir_hashtree_has(const struct kefir_hashtree *tree, kefir_hashtree_key_t key) {
    REQUIRE(tree != NULL, false);
    assert(NODE_BF(tree->root) >= -1 && NODE_BF(tree->root) <= 1);
    kefir_hashtree_hash_t hash = tree->ops->hash(key, tree->ops->data);
    struct kefir_hashtree_node *result = NULL;
    return node_find(tree->root, tree, hash, key, &result) == KEFIR_OK;
}

kefir_bool_t kefir_hashtree_empty(const struct kefir_hashtree *tree) {
    REQUIRE(tree != NULL, true);
    return tree->root == NULL;
}

kefir_result_t kefir_hashtree_delete(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hash tree pointer"));
    assert(NODE_BF(tree->root) >= -1 && NODE_BF(tree->root) <= 1);
    kefir_hashtree_hash_t hash = tree->ops->hash(key, tree->ops->data);
    struct kefir_hashtree_node *node = NULL;
    REQUIRE_OK(node_find(tree->root, tree, hash, key, &node));
    struct kefir_hashtree_node *replacement = NULL;
    struct kefir_hashtree_node *lowest_modified_node = NULL;

    if (node->left_child == NULL && node->right_child == NULL) {
        // Removed node is a leaf node itself
        lowest_modified_node = node->parent;
    } else if (node->left_child != NULL && node->right_child == NULL) {
        // Removed node is replaced by it's left child
        replacement = node->left_child;
        replacement->parent = node->parent;
        lowest_modified_node = node->parent;
    } else if (node->left_child == NULL && node->right_child != NULL) {
        // Removed node is replaced by it's right child
        replacement = node->right_child;
        replacement->parent = node->parent;
        lowest_modified_node = node->parent;
    } else {
        // Removed node is replaced by left-most leaf of it's right branch
        replacement = node_minimum(node->right_child);
        assert(replacement != NULL);
        assert(replacement->left_child == NULL);
        lowest_modified_node = replacement->parent;
        replacement->left_child = node->left_child;
        replacement->left_child->parent = replacement;
        if (lowest_modified_node != node) {
            if (replacement->right_child != NULL) {
                replacement->parent->left_child = replacement->right_child;
                replacement->right_child = NULL;
                replacement->parent->left_child->parent = replacement->parent;
            } else {
                replacement->parent->left_child = NULL;
            }
            replacement->right_child = node->right_child;
            replacement->right_child->parent = replacement;
        } else {
            lowest_modified_node = replacement;
        }
        replacement->parent = node->parent;
        NODE_UPDATE_HEIGHT(replacement);
    }
    // Node parent is updated to point to replaced node
    if (node->parent != NULL) {
        if (node->parent->left_child == node) {
            node->parent->left_child = replacement;
        } else if (node->parent->right_child == node) {
            node->parent->right_child = replacement;
        } else {
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Malformed binary tree");
        }
    } else {
        tree->root = replacement;
    }

    // Tree is traversed and rebalanced starting from replaced node parent to the root
    for (struct kefir_hashtree_node *iter = lowest_modified_node; iter != NULL; iter = iter->parent) {
        if (tree->root == iter) {
            REQUIRE_OK(balance_node(tree, &iter));
            tree->root = iter;
        } else {
            REQUIRE_OK(balance_node(tree, &iter));
        }
        assert(NODE_BF(iter) >= -1 && NODE_BF(iter) <= 1);
    }

    // Node is removed
    if (tree->node_remove.callback != NULL) {
        REQUIRE_OK(tree->node_remove.callback(mem, tree, node->key, node->value, tree->node_remove.data));
    }
    KEFIR_FREE(mem, node);
    assert(NODE_BF(tree->root) >= -1 && NODE_BF(tree->root) <= 1);
    return KEFIR_OK;
}

kefir_result_t kefir_hashtree_clean(struct kefir_mem *mem, struct kefir_hashtree *tree) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid hash tree pointer"));
    REQUIRE_OK(node_free(mem, tree, tree->root));
    tree->root = NULL;
    return KEFIR_OK;
}

struct kefir_hashtree_node *kefir_hashtree_next_node(const struct kefir_hashtree *tree,
                                                     struct kefir_hashtree_node *node) {
    REQUIRE(tree != NULL, NULL);
    REQUIRE(node != NULL, NULL);

    if (node->right_child != NULL) {
        return min_node(node->right_child);
    }

    struct kefir_hashtree_node *next_node = node;
    while (next_node->parent != NULL) {
        next_node = next_node->parent;
        if (next_node->hash > node->hash ||
            (next_node->hash == node->hash && tree->ops->compare(next_node->key, node->key, tree->ops->data) > 0)) {
            return next_node;
        }
    }

    return NULL;
}

struct kefir_hashtree_node *kefir_hashtree_iter(const struct kefir_hashtree *tree,
                                                struct kefir_hashtree_node_iterator *iter) {
    REQUIRE(tree != NULL, NULL);
    REQUIRE(iter != NULL, NULL);
    iter->node = min_node(tree->root);
    iter->tree = tree;
    return iter->node;
}

struct kefir_hashtree_node *kefir_hashtree_next(struct kefir_hashtree_node_iterator *iter) {
    REQUIRE(iter != NULL, NULL);
    REQUIRE(iter->node != NULL, NULL);
    iter->node = kefir_hashtree_next_node(iter->tree, iter->node);
    return iter->node;
}

static kefir_hashtree_hash_t str_hash(kefir_hashtree_key_t key, void *data) {
    UNUSED(data);
    const char *str = (const char *) key;
    REQUIRE(str != NULL, 0);
    kefir_hashtree_hash_t hash = 7;
    const kefir_size_t length = strlen(str);
    for (kefir_size_t i = 0; i < length; i++) {
        hash = (hash * 31) + str[i];
    }
    return hash;
}

static kefir_int_t str_compare(kefir_hashtree_key_t key1, kefir_hashtree_key_t key2, void *data) {
    UNUSED(data);
    const char *str1 = (const char *) key1;
    const char *str2 = (const char *) key2;
    if (str1 == NULL && str2 == NULL) {
        return 0;
    } else if (str1 == NULL) {
        return -1;
    } else if (str2 == NULL) {
        return 1;
    }

    int cmp = strcmp(str1, str2);
    // Clamp the result
    if (cmp == 0) {
        return 0;
    } else if (cmp < 0) {
        return -1;
    } else {
        return 1;
    }
}

const struct kefir_hashtree_ops kefir_hashtree_str_ops = {.hash = str_hash, .compare = str_compare, .data = NULL};

static kefir_hashtree_hash_t uint_hash(kefir_hashtree_key_t key, void *data) {
    UNUSED(data);
    return (kefir_hashtree_hash_t) key;
}

static kefir_int_t uint_compare(kefir_hashtree_key_t key1, kefir_hashtree_key_t key2, void *data) {
    UNUSED(data);
    if (key1 == key2) {
        return 0;
    } else if (key1 < key2) {
        return -1;
    } else {
        return 1;
    }
}

const struct kefir_hashtree_ops kefir_hashtree_uint_ops = {.hash = uint_hash, .compare = uint_compare, .data = NULL};
