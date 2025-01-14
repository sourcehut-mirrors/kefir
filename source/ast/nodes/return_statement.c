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

NODE_VISIT_IMPL(ast_return_statement_visit, kefir_ast_return_statement, return_statement)

kefir_result_t ast_return_statement_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_return_statement *, node, base->self);
    if (node->expression != NULL) {
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node->expression));
        node->expression = NULL;
    }
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_RETURN_STATEMENT_CLASS = {
    .type = KEFIR_AST_RETURN_STATEMENT, .visit = ast_return_statement_visit, .free = ast_return_statement_free};

struct kefir_ast_return_statement *kefir_ast_new_return_statement(struct kefir_mem *mem,
                                                                  struct kefir_ast_node_base *expression) {
    REQUIRE(mem != NULL, NULL);

    struct kefir_ast_return_statement *stmt = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_return_statement));
    REQUIRE(stmt != NULL, NULL);
    stmt->base.refcount = 1;
    stmt->base.klass = &AST_RETURN_STATEMENT_CLASS;
    stmt->base.self = stmt;
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

    stmt->expression = expression;
    return stmt;
}
