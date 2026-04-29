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

#include "kefir/ir/type_tree.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

struct traversal_payload {
    struct kefir_mem *mem;
    struct kefir_ir_type_tree *tree;
    struct kefir_ir_type_tree_node *parent;
    struct kefir_ir_type_visitor *visitor;
    kefir_size_t slot;
};

static struct kefir_ir_type_tree_node *alloc_node(struct kefir_mem *mem, struct kefir_ir_type_tree *tree,
                                                  struct kefir_ir_type_tree_node *parent, kefir_size_t index,
                                                  kefir_size_t relative_slot, kefir_size_t slot_width) {
    struct kefir_ir_type_tree_node *node = &tree->nodes[index];

    node->index = index;
    node->parent = parent;
    node->relative_slot = relative_slot;
    node->slot_width = slot_width;
    node->type = tree->type;

    kefir_result_t res;
    if (parent != NULL) {
        res = kefir_list_insert_after(mem, &parent->subtypes, kefir_list_tail(&parent->subtypes), node);
    } else {
        res = kefir_list_insert_after(mem, &tree->roots, kefir_list_tail(&tree->roots), node);
    }
    REQUIRE(res == KEFIR_OK, NULL);

    return node;
}

static kefir_result_t visit_typeentry(const struct kefir_ir_type *type, kefir_size_t index,
                                      const struct kefir_ir_typeentry *typeentry, void *payload) {
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type"));
    REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid IR type entry"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct traversal_payload *, param, payload);

    struct kefir_ir_type_tree_node *node = alloc_node(param->mem, param->tree, param->parent, index, param->slot, 1);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR type tree node"));
    switch (typeentry->typecode) {
        case KEFIR_IR_TYPE_STRUCT:
        case KEFIR_IR_TYPE_UNION: {
            struct traversal_payload nested_param = {
                .mem = param->mem, .parent = node, .slot = 1, .tree = param->tree, .visitor = param->visitor};
            REQUIRE_OK(
                kefir_ir_type_visitor_list_nodes(type, param->visitor, &nested_param, index + 1, typeentry->param));
            node->slot_width = nested_param.slot;
        } break;

        case KEFIR_IR_TYPE_ARRAY: {
            struct traversal_payload nested_param = {
                .mem = param->mem, .parent = node, .slot = 1, .tree = param->tree, .visitor = param->visitor};
            REQUIRE_OK(kefir_ir_type_visitor_list_nodes(type, param->visitor, &nested_param, index + 1, 1));
            node->slot_width = (nested_param.slot - 1) * typeentry->param + 1;
        } break;

        default:
            break;
    }

    param->slot += node->slot_width;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_type_tree_init(struct kefir_mem *mem, const struct kefir_ir_type *type,
                                       struct kefir_ir_type_tree *tree) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type tree"));

    struct kefir_ir_type_visitor visitor;
    REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, visit_typeentry));

    tree->type = type;
    REQUIRE_OK(kefir_list_init(&tree->roots));
    tree->nodes = KEFIR_MALLOC(mem, sizeof(struct kefir_ir_type_tree_node) * kefir_ir_type_length(type));
    REQUIRE(tree->nodes != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR type tree"));
    for (kefir_size_t i = 0; i < kefir_ir_type_length(type); i++) {
        kefir_result_t res = kefir_list_init(&tree->nodes[i].subtypes);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, tree->nodes);
            return res;
        });
    }

    struct traversal_payload param = {.mem = mem, .parent = NULL, .slot = 0, .tree = tree, .visitor = &visitor};
    kefir_result_t res = kefir_ir_type_visitor_list_nodes(type, &visitor, &param, 0, kefir_ir_type_children(type));
    REQUIRE_ELSE(res == KEFIR_OK, {
        for (kefir_size_t i = 0; i < kefir_ir_type_length(type); i++) {
            kefir_list_free(mem, &tree->nodes[i].subtypes);
        }
        KEFIR_FREE(mem, tree->nodes);
        kefir_list_free(mem, &tree->roots);
        return KEFIR_OK;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_ir_type_tree_free(struct kefir_mem *mem, struct kefir_ir_type_tree *tree) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type tree"));

    REQUIRE_OK(kefir_list_free(mem, &tree->roots));
    for (kefir_size_t i = 0; i < kefir_ir_type_length(tree->type); i++) {
        REQUIRE_OK(kefir_list_free(mem, &tree->nodes[i].subtypes));
    }
    KEFIR_FREE(mem, tree->nodes);
    tree->type = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_ir_type_tree_at(const struct kefir_ir_type_tree *tree, kefir_size_t index,
                                     const struct kefir_ir_type_tree_node **node) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type tree"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to IR type tree node"));
    REQUIRE(index < kefir_ir_type_length(tree->type),
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find requested IR type tree node"));

    *node = &tree->nodes[index];
    return KEFIR_OK;
}
