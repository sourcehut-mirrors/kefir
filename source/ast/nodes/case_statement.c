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

#include "kefir/ast/node.h"
#include "kefir/ast/node_internal.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

NODE_VISIT_IMPL(ast_case_statement_visit, kefir_ast_case_statement, case_statement)

kefir_result_t ast_case_statement_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_case_statement *, node, base->self);
    if (node->expression != NULL) {
        KEFIR_AST_NODE_FREE(mem, node->expression);
    }
    if (node->range_end_expression != NULL) {
        KEFIR_AST_NODE_FREE(mem, node->range_end_expression);
    }
    KEFIR_AST_NODE_FREE(mem, node->statement);
    REQUIRE_OK(kefir_ast_node_attributes_free(mem, &node->attributes));
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_CASE_STATEMENT_CLASS = {
    .type = KEFIR_AST_CASE_STATEMENT, .visit = ast_case_statement_visit, .free = ast_case_statement_free};

struct kefir_ast_case_statement *kefir_ast_new_case_statement(struct kefir_mem *mem,
                                                              struct kefir_ast_node_base *expression,
                                                              struct kefir_ast_node_base *statement) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(statement != NULL, NULL);

    struct kefir_ast_case_statement *case_stmt = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_case_statement));
    REQUIRE(case_stmt != NULL, NULL);
    case_stmt->base.refcount = 1;
    case_stmt->base.klass = &AST_CASE_STATEMENT_CLASS;
    case_stmt->base.self = case_stmt;
    kefir_result_t res = kefir_ast_node_properties_init(&case_stmt->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, case_stmt);
        return NULL;
    });
    res = kefir_source_location_empty(&case_stmt->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, case_stmt);
        return NULL;
    });
    res = kefir_ast_node_attributes_init(&case_stmt->attributes);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, case_stmt);
        return NULL;
    });
    case_stmt->expression = expression;
    case_stmt->range_end_expression = NULL;
    case_stmt->statement = statement;
    return case_stmt;
}

struct kefir_ast_case_statement *kefir_ast_new_range_case_statement(struct kefir_mem *mem,
                                                                    struct kefir_ast_node_base *expression,
                                                                    struct kefir_ast_node_base *range_end_expression,
                                                                    struct kefir_ast_node_base *statement) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(expression != NULL, NULL);
    REQUIRE(range_end_expression != NULL, NULL);
    REQUIRE(statement != NULL, NULL);

    struct kefir_ast_case_statement *case_stmt = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_case_statement));
    REQUIRE(case_stmt != NULL, NULL);
    case_stmt->base.refcount = 1;
    case_stmt->base.klass = &AST_CASE_STATEMENT_CLASS;
    case_stmt->base.self = case_stmt;
    kefir_result_t res = kefir_ast_node_properties_init(&case_stmt->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, case_stmt);
        return NULL;
    });
    res = kefir_source_location_empty(&case_stmt->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, case_stmt);
        return NULL;
    });
    res = kefir_ast_node_attributes_init(&case_stmt->attributes);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, case_stmt);
        return NULL;
    });
    case_stmt->expression = expression;
    case_stmt->range_end_expression = range_end_expression;
    case_stmt->statement = statement;
    return case_stmt;
}
