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

#include "kefir/ast/flow_control.h"
#include "kefir/ast/node_base.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include <string.h>

static kefir_result_t kefir_ast_flow_control_branching_point_free(
    struct kefir_mem *mem, struct kefir_ast_flow_control_branching_point *brpoint) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(brpoint != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control branching point"));

    REQUIRE_OK(kefir_hashtree_free(mem, &brpoint->branches));
    KEFIR_FREE(mem, brpoint);
    return KEFIR_OK;
}

static kefir_result_t kefir_ast_flow_control_point_free(struct kefir_mem *mem,
                                                        struct kefir_ast_flow_control_point *point) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(point != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control point"));

    if (point->cleanup.callback != NULL) {
        REQUIRE_OK(point->cleanup.callback(mem, point, point->cleanup.payload));
        point->cleanup.callback = NULL;
        point->cleanup.payload = NULL;
    }
    point->ptr = NULL;
    KEFIR_FREE(mem, point);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_flow_control_branching_point_append(struct kefir_mem *mem,
                                                             struct kefir_ast_flow_control_branching_point *brpoint,
                                                             const char *identifier,
                                                             struct kefir_ast_flow_control_point *point) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(brpoint != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control branching point"));
    REQUIRE(identifier != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control branch identifier"));
    REQUIRE(point != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control point"));

    REQUIRE_OK(kefir_hashtree_insert(mem, &brpoint->branches, (kefir_hashtree_key_t) identifier,
                                     (kefir_hashtree_value_t) point));
    return KEFIR_OK;
}

static kefir_result_t flow_control_statement_free(struct kefir_mem *mem, void *node, void *payload) {
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(node != NULL, KEFIR_OK);

    ASSIGN_DECL_CAST(struct kefir_ast_flow_control_structure *, statement, node);
    if (statement->cleanup.callback != NULL) {
        statement->cleanup.callback(mem, statement, statement->cleanup.payload);
        statement->cleanup.callback = NULL;
        statement->cleanup.payload = NULL;
    }
    statement->payload.ptr = NULL;

    switch (statement->type) {
        case KEFIR_AST_FLOW_CONTROL_STRUCTURE_BLOCK:
            // Intentionally left blank
            break;

        case KEFIR_AST_FLOW_CONTROL_STRUCTURE_IF:
            if (statement->value.conditional.thenBranchEnd != NULL) {
                statement->value.conditional.thenBranchEnd = NULL;
            }
            if (statement->value.conditional.elseBranchEnd != NULL) {
                statement->value.conditional.elseBranchEnd = NULL;
            }
            break;

        case KEFIR_AST_FLOW_CONTROL_STRUCTURE_SWITCH:
            REQUIRE_OK(kefir_hashtree_free(mem, &statement->value.switchStatement.case_flow_control_points));
            REQUIRE_OK(kefir_hashtree_free(mem, &statement->value.switchStatement.case_label_nodes));
            REQUIRE_OK(kefir_hashtree_free(mem, &statement->value.switchStatement.case_range_end_nodes));
            if (statement->value.switchStatement.defaultCase != NULL) {
                statement->value.switchStatement.defaultCase = NULL;
            }

            if (statement->value.switchStatement.end != NULL) {
                statement->value.switchStatement.end = NULL;
            }
            break;

        case KEFIR_AST_FLOW_CONTROL_STRUCTURE_FOR:
        case KEFIR_AST_FLOW_CONTROL_STRUCTURE_WHILE:
        case KEFIR_AST_FLOW_CONTROL_STRUCTURE_DO:
            if (statement->value.loop.continuation != NULL) {
                statement->value.loop.continuation = NULL;
            }

            if (statement->value.loop.end != NULL) {
                statement->value.loop.end = NULL;
            }
            break;

        case KEFIR_AST_FLOW_CONTROL_POINT:
            REQUIRE_OK(kefir_ast_flow_control_point_free(mem, statement->value.control_point));
            statement->value.control_point = NULL;
            break;

        case KEFIR_AST_FLOW_CONTROL_BRANCHING_POINT:
            REQUIRE_OK(kefir_ast_flow_control_branching_point_free(mem, statement->value.branching_point));
            statement->value.branching_point = NULL;
            break;

        case KEFIR_AST_FLOW_CONTROL_VL_ARRAY:
            statement->value.vl_array_id = KEFIR_ID_NONE;
            break;
    }
    KEFIR_FREE(mem, statement);
    return KEFIR_OK;
}

static kefir_result_t free_unbound_control_point(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                                 kefir_hashtree_key_t key, kefir_hashtree_value_t value,
                                                 void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_ast_flow_control_point *, control_point, value);
    if (control_point != NULL) {
        REQUIRE_OK(kefir_ast_flow_control_point_free(mem, control_point));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_flow_control_tree_init(struct kefir_ast_flow_control_tree *tree) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control tree"));
    REQUIRE_OK(kefir_tree_init(&tree->root, NULL));
    REQUIRE_OK(kefir_tree_on_removal(&tree->root, flow_control_statement_free, NULL));
    REQUIRE_OK(kefir_hashtree_init(&tree->unbound_control_points, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&tree->unbound_control_points, free_unbound_control_point, NULL));
    tree->current = &tree->root;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_flow_control_tree_free(struct kefir_mem *mem, struct kefir_ast_flow_control_tree *tree) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control tree"));

    REQUIRE_OK(kefir_hashtree_free(mem, &tree->unbound_control_points));
    REQUIRE_OK(kefir_tree_free(mem, &tree->root));
    tree->current = NULL;
    return KEFIR_OK;
}

static struct kefir_ast_flow_control_point *control_point_alloc(struct kefir_mem *mem) {
    struct kefir_ast_flow_control_point *control_point = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_flow_control_point));
    REQUIRE(control_point != NULL, NULL);

    control_point->ptr = &control_point->content[0];
    memset(control_point->content, 0, KEFIR_AST_FLOW_CONTROL_PAYLOAD_SIZE);
    control_point->cleanup.callback = NULL;
    control_point->cleanup.payload = NULL;
    control_point->self = NULL;

    return control_point;
}

static kefir_result_t free_switch_statement_case(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                                 kefir_hashtree_key_t key, kefir_hashtree_value_t value,
                                                 void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, value);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));

    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node));
    return KEFIR_OK;
}

static kefir_result_t free_switch_statement_case_range_end(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                                           kefir_hashtree_key_t key, kefir_hashtree_value_t value,
                                                           void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, value);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));

    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node));
    return KEFIR_OK;
}

static kefir_result_t alloc_control_structure(
    struct kefir_mem *mem, struct kefir_ast_flow_control_tree *tree, kefir_ast_flow_control_structure_type_t type,
    struct kefir_tree_node *parent_node,
    const struct kefir_ast_flow_control_structure_associated_scopes *associated_scopes,
    struct kefir_ast_flow_control_structure **statement) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(associated_scopes != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control associated scopes"));
    REQUIRE(associated_scopes->ordinary_scope != NULL && associated_scopes->tag_scope != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control associated scopes"));
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control tree"));

    struct kefir_ast_flow_control_structure *stmt = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_flow_control_structure));
    REQUIRE(stmt != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST flow control statement"));
    stmt->type = type;
    stmt->cleanup.callback = NULL;
    stmt->cleanup.payload = NULL;
    stmt->payload.ptr = &stmt->payload.content[0];
    memset(&stmt->payload.content[0], 0, KEFIR_AST_FLOW_CONTROL_PAYLOAD_SIZE);

    stmt->associated_scopes = *associated_scopes;

    switch (type) {
        case KEFIR_AST_FLOW_CONTROL_STRUCTURE_BLOCK:
            // Intentionally left blank
            break;

        case KEFIR_AST_FLOW_CONTROL_STRUCTURE_IF:
            stmt->value.conditional.thenBranchEnd = NULL;
            stmt->value.conditional.elseBranchEnd = NULL;
            break;

        case KEFIR_AST_FLOW_CONTROL_STRUCTURE_SWITCH: {
            kefir_result_t res =
                kefir_hashtree_init(&stmt->value.switchStatement.case_flow_control_points, &kefir_hashtree_uint_ops);
            REQUIRE_CHAIN(&res,
                          kefir_hashtree_init(&stmt->value.switchStatement.case_label_nodes, &kefir_hashtree_uint_ops));
            REQUIRE_CHAIN(&res, kefir_hashtree_on_removal(&stmt->value.switchStatement.case_label_nodes,
                                                          free_switch_statement_case, NULL));
            REQUIRE_CHAIN(
                &res, kefir_hashtree_init(&stmt->value.switchStatement.case_range_end_nodes, &kefir_hashtree_uint_ops));
            REQUIRE_CHAIN(&res, kefir_hashtree_on_removal(&stmt->value.switchStatement.case_range_end_nodes,
                                                          free_switch_statement_case_range_end, NULL));
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_FREE(mem, stmt);
                return res;
            });

            stmt->value.switchStatement.num_of_cases = 0;
            stmt->value.switchStatement.defaultCase = NULL;
            stmt->value.switchStatement.controlling_expression_type = NULL;
            stmt->value.switchStatement.end = NULL;
        } break;

        case KEFIR_AST_FLOW_CONTROL_STRUCTURE_FOR:
        case KEFIR_AST_FLOW_CONTROL_STRUCTURE_WHILE:
        case KEFIR_AST_FLOW_CONTROL_STRUCTURE_DO:
            stmt->value.loop.continuation = NULL;
            stmt->value.loop.end = NULL;
            break;

        case KEFIR_AST_FLOW_CONTROL_POINT:
            stmt->value.control_point = control_point_alloc(mem);
            REQUIRE_ELSE(stmt->value.control_point != NULL, {
                KEFIR_FREE(mem, stmt);
                return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST flow control point");
            });
            stmt->value.control_point->self = stmt;
            break;

        case KEFIR_AST_FLOW_CONTROL_BRANCHING_POINT: {
            stmt->value.branching_point = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_flow_control_branching_point));
            REQUIRE_ELSE(stmt->value.branching_point != NULL, {
                KEFIR_FREE(mem, stmt);
                return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST flow control branching point");
            });

            kefir_result_t res = kefir_hashtree_init(&stmt->value.branching_point->branches, &kefir_hashtree_str_ops);
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_FREE(mem, stmt->value.branching_point);
                KEFIR_FREE(mem, stmt);
                return res;
            });
        } break;

        case KEFIR_AST_FLOW_CONTROL_VL_ARRAY:
            stmt->value.vl_array_id = KEFIR_ID_NONE;
            break;
    }

    struct kefir_tree_node *control_struct_tree_node;
    kefir_result_t res = kefir_tree_insert_child(mem, parent_node, stmt, &control_struct_tree_node);
    REQUIRE_ELSE(res == KEFIR_OK, {
        if (type == KEFIR_AST_FLOW_CONTROL_STRUCTURE_SWITCH) {
            kefir_hashtree_free(mem, &stmt->value.switchStatement.case_range_end_nodes);
            kefir_hashtree_free(mem, &stmt->value.switchStatement.case_label_nodes);
            kefir_hashtree_free(mem, &stmt->value.switchStatement.case_flow_control_points);
        } else if (type == KEFIR_AST_FLOW_CONTROL_BRANCHING_POINT) {
            KEFIR_FREE(mem, stmt->value.branching_point);
        }
        KEFIR_FREE(mem, stmt);
        return res;
    });
    stmt->node = control_struct_tree_node;
    ASSIGN_PTR(statement, stmt);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_flow_control_tree_push(
    struct kefir_mem *mem, struct kefir_ast_flow_control_tree *tree, kefir_ast_flow_control_structure_type_t type,
    const struct kefir_ast_flow_control_structure_associated_scopes *associated_scopes,
    struct kefir_ast_flow_control_structure **statement) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(associated_scopes != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control associated scopes"));
    REQUIRE(associated_scopes->ordinary_scope != NULL && associated_scopes->tag_scope != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control associated scopes"));
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control tree"));

    struct kefir_ast_flow_control_structure *control_struct;
    REQUIRE_OK(alloc_control_structure(mem, tree, type, tree->current, associated_scopes, &control_struct));
    tree->current = control_struct->node;
    ASSIGN_PTR(statement, control_struct);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_flow_control_tree_pop(struct kefir_ast_flow_control_tree *tree) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control tree"));
    REQUIRE(tree->current->parent != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Cannot pop flow control tree top-level statement"));

    tree->current = tree->current->parent;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_flow_control_tree_top(struct kefir_ast_flow_control_tree *tree,
                                               struct kefir_ast_flow_control_structure **stmt) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control tree"));
    REQUIRE(stmt != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST flow control tree statement"));

    ASSIGN_DECL_CAST(struct kefir_ast_flow_control_structure *, statement, tree->current->value);
    ASSIGN_PTR(stmt, statement);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_flow_control_tree_traverse(
    struct kefir_ast_flow_control_tree *tree,
    kefir_result_t (*callback)(const struct kefir_ast_flow_control_structure *, void *, kefir_bool_t *), void *payload,
    struct kefir_ast_flow_control_structure **stmt) {
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control tree"));
    REQUIRE(callback != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid traversal callback"));
    REQUIRE(stmt != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST flow control tree statement"));

    struct kefir_ast_flow_control_structure *current = NULL;
    REQUIRE_OK(kefir_ast_flow_control_tree_top(tree, &current));

    while (current != NULL) {
        kefir_bool_t match = false;
        REQUIRE_OK(callback(current, payload, &match));
        if (match) {
            ASSIGN_PTR(stmt, current);
            return KEFIR_OK;
        } else {
            current = kefir_ast_flow_control_structure_parent(current);
        }
    }

    return KEFIR_NOT_FOUND;
}

struct kefir_ast_flow_control_structure *kefir_ast_flow_control_structure_parent(
    const struct kefir_ast_flow_control_structure *control_struct) {
    REQUIRE(control_struct != NULL, NULL);

    struct kefir_tree_node *const parent_node = control_struct->node->parent;
    REQUIRE(parent_node != NULL, NULL);

    ASSIGN_DECL_CAST(struct kefir_ast_flow_control_structure *, parent_control_struct, parent_node->value);
    return parent_control_struct;
}

struct kefir_ast_flow_control_structure *kefir_ast_flow_control_structure_prev_sibling(
    const struct kefir_ast_flow_control_structure *control_struct) {
    REQUIRE(control_struct != NULL, NULL);

    struct kefir_tree_node *const sibling_node = kefir_tree_prev_sibling(control_struct->node);
    REQUIRE(sibling_node != NULL, NULL);

    ASSIGN_DECL_CAST(struct kefir_ast_flow_control_structure *, sibling_control_struct, sibling_node->value);
    return sibling_control_struct;
}

struct kefir_ast_flow_control_structure *kefir_ast_flow_control_structure_next_sibling(
    const struct kefir_ast_flow_control_structure *control_struct) {
    REQUIRE(control_struct != NULL, NULL);

    struct kefir_tree_node *const sibling_node = kefir_tree_next_sibling(control_struct->node);
    REQUIRE(sibling_node != NULL, NULL);

    ASSIGN_DECL_CAST(struct kefir_ast_flow_control_structure *, sibling_control_struct, sibling_node->value);
    return sibling_control_struct;
}

struct kefir_ast_flow_control_structure *kefir_ast_flow_control_structure_first_child(
    const struct kefir_ast_flow_control_structure *control_struct) {
    REQUIRE(control_struct != NULL, NULL);

    struct kefir_tree_node *const child_node = kefir_tree_first_child(control_struct->node);
    REQUIRE(child_node != NULL, NULL);

    ASSIGN_DECL_CAST(struct kefir_ast_flow_control_structure *, child_control_struct, child_node->value);
    return child_control_struct;
}

kefir_result_t kefir_ast_flow_control_block_add_vl_array(struct kefir_mem *mem,
                                                         struct kefir_ast_flow_control_tree *tree,
                                                         struct kefir_ast_flow_control_structure *stmt,
                                                         kefir_id_t element) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control tree"));
    REQUIRE(stmt != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control tree statement"));
    REQUIRE(stmt->type == KEFIR_AST_FLOW_CONTROL_STRUCTURE_BLOCK,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected AST flow control tree statement to be block"));

    struct kefir_ast_flow_control_structure *control_struct = NULL;
    REQUIRE_OK(alloc_control_structure(mem, tree, KEFIR_AST_FLOW_CONTROL_VL_ARRAY, stmt->node, &stmt->associated_scopes,
                                       &control_struct));
    control_struct->value.vl_array_id = element;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_flow_control_block_vl_array_head(const struct kefir_ast_flow_control_structure *stmt,
                                                          kefir_id_t *vl_array_ptr) {
    REQUIRE(stmt != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control tree statement"));
    REQUIRE(stmt->type == KEFIR_AST_FLOW_CONTROL_STRUCTURE_BLOCK,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected AST flow control tree statement to be block"));
    REQUIRE(vl_array_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to variable-length array identifier"));

    for (struct kefir_ast_flow_control_structure *iter = kefir_ast_flow_control_structure_first_child(stmt);
         iter != NULL; iter = kefir_ast_flow_control_structure_next_sibling(iter)) {
        if (iter->type == KEFIR_AST_FLOW_CONTROL_VL_ARRAY) {
            *vl_array_ptr = iter->value.vl_array_id;
            return KEFIR_OK;
        }
    }

    return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                           "AST flow control block does not have associated variable-length arrays");
}

kefir_bool_t kefir_ast_flow_control_block_contains_vl_arrays(const struct kefir_ast_flow_control_structure *stmt) {
    REQUIRE(stmt != NULL, false);

    for (struct kefir_ast_flow_control_structure *iter = kefir_ast_flow_control_structure_first_child(stmt);
         iter != NULL; iter = kefir_ast_flow_control_structure_next_sibling(iter)) {
        if (iter->type == KEFIR_AST_FLOW_CONTROL_VL_ARRAY) {
            return true;
        }
    }

    return false;
}

struct kefir_ast_flow_control_point *kefir_ast_flow_control_point_alloc(
    struct kefir_mem *mem, struct kefir_ast_flow_control_tree *flow_control,
    struct kefir_ast_flow_control_structure *stmt) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(flow_control != NULL, NULL);

    if (stmt != NULL) {
        struct kefir_ast_flow_control_structure *control_struct = NULL;
        kefir_result_t res = alloc_control_structure(mem, flow_control, KEFIR_AST_FLOW_CONTROL_POINT, stmt->node,
                                                     &stmt->associated_scopes, &control_struct);
        REQUIRE(res == KEFIR_OK, NULL);
        return control_struct->value.control_point;
    } else {
        struct kefir_ast_flow_control_point *control_point = control_point_alloc(mem);
        REQUIRE(control_point != NULL, NULL);

        kefir_result_t res =
            kefir_hashtree_insert(mem, &flow_control->unbound_control_points, (kefir_hashtree_key_t) control_point,
                                  (kefir_hashtree_value_t) control_point);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, control_point);
            return NULL;
        });
        return control_point;
    }
}

kefir_result_t kefir_ast_flow_control_point_bind(struct kefir_mem *mem, struct kefir_ast_flow_control_tree *tree,
                                                 struct kefir_ast_flow_control_point *point,
                                                 struct kefir_ast_flow_control_structure *parent) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control tree"));
    REQUIRE(point != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control point"));
    REQUIRE(point->self == NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected unbound AST flow control point"));
    REQUIRE(parent != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control structure"));

    struct kefir_hashtree_node *unbound_node;
    kefir_result_t res = kefir_hashtree_at(&tree->unbound_control_points, (kefir_hashtree_key_t) point, &unbound_node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected unbound AST flow control point");
    }
    REQUIRE_OK(res);

    struct kefir_ast_flow_control_structure *control_struct = NULL;
    REQUIRE_OK(alloc_control_structure(mem, tree, KEFIR_AST_FLOW_CONTROL_POINT, parent->node,
                                       &parent->associated_scopes, &control_struct));
    KEFIR_FREE(mem, control_struct->value.control_point);
    control_struct->value.control_point = point;
    point->self = control_struct;
    unbound_node->value = 0;

    REQUIRE_OK(kefir_hashtree_delete(mem, &tree->unbound_control_points, (kefir_hashtree_key_t) point));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_flow_control_block_add_branching_point(
    struct kefir_mem *mem, struct kefir_ast_flow_control_tree *tree, struct kefir_ast_flow_control_structure *stmt,
    struct kefir_ast_flow_control_branching_point **brpoint_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(tree != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control tree"));
    REQUIRE(stmt != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control tree statement"));
    REQUIRE(brpoint_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST flow control branching point"));

    struct kefir_ast_flow_control_structure *control_struct = NULL;
    REQUIRE_OK(alloc_control_structure(mem, tree, KEFIR_AST_FLOW_CONTROL_BRANCHING_POINT, stmt->node,
                                       &stmt->associated_scopes, &control_struct));

    *brpoint_ptr = control_struct->value.branching_point;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_flow_control_point_common_parent(struct kefir_ast_flow_control_point *point1,
                                                          struct kefir_ast_flow_control_point *point2,
                                                          struct kefir_ast_flow_control_structure **common_parent) {
    REQUIRE(point1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control point"));
    REQUIRE(point2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control point"));
    REQUIRE(common_parent != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST flow control structure"));

    *common_parent = NULL;
    struct kefir_ast_flow_control_structure *current_parent1 = point1->self;
    while (current_parent1 != NULL && *common_parent == NULL) {
        struct kefir_ast_flow_control_structure *current_parent2 = point2->self;
        while (current_parent2 != NULL && *common_parent == NULL) {
            if (current_parent2 == current_parent1) {
                *common_parent = current_parent2;
            } else {
                current_parent2 = kefir_ast_flow_control_structure_parent(current_parent2);
            }
        }
        if (*common_parent == NULL) {
            current_parent1 = kefir_ast_flow_control_structure_parent(current_parent1);
        }
    }
    REQUIRE(*common_parent != NULL,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to determine common parent for two flow control points"));

    return KEFIR_OK;
}

kefir_result_t kefir_ast_flow_control_point_parents(struct kefir_mem *mem, struct kefir_ast_flow_control_point *point,
                                                    struct kefir_list *parents,
                                                    struct kefir_ast_flow_control_structure *top_parent) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(point != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control point"));
    REQUIRE(parents != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST list"));

    struct kefir_ast_flow_control_structure *current_parent = point->self;
    while (current_parent != top_parent && current_parent != NULL) {
        REQUIRE_OK(kefir_list_insert_after(mem, parents, kefir_list_tail(parents), current_parent));
        current_parent = kefir_ast_flow_control_structure_parent(current_parent);
    }
    return KEFIR_OK;
}
