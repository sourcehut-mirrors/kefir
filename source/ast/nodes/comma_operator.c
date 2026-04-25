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

NODE_VISIT_IMPL(ast_comma_operator_visit, kefir_ast_comma_operator, comma_operator)

kefir_result_t ast_comma_operator_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_comma_operator *, node, base->self);
    for (kefir_size_t i = 0; i < node->expressions_length; i++) {
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node->expressions[i]));
    }
    KEFIR_FREE(mem, node->expressions);
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_COMMA_OPERATOR_CLASS = {
    .type = KEFIR_AST_COMMA_OPERATOR, .visit = ast_comma_operator_visit, .free = ast_comma_operator_free};

struct kefir_ast_comma_operator *kefir_ast_new_comma_operator(struct kefir_mem *mem) {
    REQUIRE(mem != NULL, NULL);

    struct kefir_ast_comma_operator *comma = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_comma_operator));
    REQUIRE(comma != NULL, NULL);
    comma->base.refcount = 1;
    comma->base.klass = &AST_COMMA_OPERATOR_CLASS;
    comma->base.self = comma;
    comma->expressions = NULL;
    comma->expressions_capacity = 0;
    comma->expressions_length = 0;
    kefir_result_t res = kefir_ast_node_properties_init(&comma->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, comma);
        return NULL;
    });
    res = kefir_source_location_empty(&comma->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, comma);
        return NULL;
    });
    return comma;
}

kefir_result_t kefir_ast_comma_append(struct kefir_mem *mem, struct kefir_ast_comma_operator *comma,
                                      struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(comma != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST comma operator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));

    if (comma->expressions_length + 1 > comma->expressions_capacity) {
        kefir_size_t new_capacity = MAX(1, 2 * comma->expressions_capacity);
        struct kefir_ast_node_base **new_expressions =
            KEFIR_REALLOC(mem, comma->expressions, sizeof(struct kefir_ast_node_base *) * new_capacity);
        REQUIRE(new_expressions != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST comma node expressions"));

        comma->expressions = new_expressions;
        comma->expressions_capacity = new_capacity;
    }
    comma->expressions[comma->expressions_length++] = base;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_compound_statement_prepend(struct kefir_mem *mem, struct kefir_ast_compound_statement *node,
                                                    struct kefir_ast_node_base *item) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST compound statement"));
    REQUIRE(item != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));

    if (node->block_length + 1 > node->block_capacity) {
        kefir_size_t new_capacity = MAX(1, 2 * node->block_capacity);
        struct kefir_ast_node_base **new_items =
            KEFIR_REALLOC(mem, node->block_items, sizeof(struct kefir_ast_node_base *) * new_capacity);
        REQUIRE(new_items != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST compound statment items"));

        node->block_capacity = new_capacity;
        node->block_items = new_items;
    }
    if (node->block_length > 0) {
        memmove(&node->block_items[1], &node->block_items[0],
                sizeof(struct kefir_ast_node_base *) * node->block_length);
    }
    node->block_items[0] = item;
    node->block_length++;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_compound_statement_append(struct kefir_mem *mem, struct kefir_ast_compound_statement *node,
                                                   struct kefir_ast_node_base *item) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST compound statement"));
    REQUIRE(item != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));

    if (node->block_length + 1 > node->block_capacity) {
        kefir_size_t new_capacity = MAX(1, 2 * node->block_capacity);
        struct kefir_ast_node_base **new_items =
            KEFIR_REALLOC(mem, node->block_items, sizeof(struct kefir_ast_node_base *) * new_capacity);
        REQUIRE(new_items != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST compound statment items"));

        node->block_capacity = new_capacity;
        node->block_items = new_items;
    }
    node->block_items[node->block_length++] = item;
    return KEFIR_OK;
}
