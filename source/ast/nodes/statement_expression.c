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

#include "kefir/ast/node.h"
#include "kefir/ast/node_internal.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

NODE_VISIT_IMPL(ast_statement_expression_visit, kefir_ast_statement_expression, statement_expression)

kefir_result_t ast_statement_expression_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_statement_expression *, node, base->self);
    for (kefir_size_t i = 0; i < node->block_length; i++) {
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node->block_items[i]));
    }
    KEFIR_FREE(mem, node->block_items);
    if (node->result != NULL) {
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node->result));
        node->result = NULL;
    }
    REQUIRE_OK(kefir_ast_node_attributes_free(mem, &node->attributes));
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_STATEMENT_EXPRESSION_CLASS = {.type = KEFIR_AST_STATEMENT_EXPRESSION,
                                                                    .visit = ast_statement_expression_visit,
                                                                    .free = ast_statement_expression_free};

struct kefir_ast_statement_expression *kefir_ast_new_statement_expression(struct kefir_mem *mem) {
    REQUIRE(mem != NULL, NULL);

    struct kefir_ast_statement_expression *stmt = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_statement_expression));
    REQUIRE(stmt != NULL, NULL);
    stmt->base.refcount = 1;
    stmt->base.klass = &AST_STATEMENT_EXPRESSION_CLASS;
    stmt->base.self = stmt;
    stmt->block_items = NULL;
    stmt->block_capacity = 0;
    stmt->block_length = 0;
    kefir_result_t res = kefir_ast_node_properties_init(&stmt->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, stmt);
        return NULL;
    });
    res = kefir_source_location_empty(&stmt->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, stmt);
        return NULL;
    });

    res = kefir_ast_node_attributes_init(&stmt->attributes);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, stmt);
        return NULL;
    });

    stmt->result = NULL;
    return stmt;
}

kefir_result_t kefir_ast_statement_expression_append(struct kefir_mem *mem, struct kefir_ast_statement_expression *node,
                                                     struct kefir_ast_node_base *item) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST compound statement"));
    REQUIRE(item != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));

    if (node->block_length + 1 > node->block_capacity) {
        kefir_size_t new_capacity = MAX(1, 2 * node->block_capacity);
        struct kefir_ast_node_base **new_items =
            KEFIR_REALLOC(mem, node->block_items, sizeof(struct kefir_ast_node_base *) * new_capacity);
        REQUIRE(new_items != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST statment expression items"));

        node->block_capacity = new_capacity;
        node->block_items = new_items;
    }
    node->block_items[node->block_length++] = item;
    return KEFIR_OK;
}
