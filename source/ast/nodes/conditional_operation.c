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

NODE_VISIT_IMPL(ast_conditional_operator_visit, kefir_ast_conditional_operator, conditional_operator)

kefir_result_t ast_conditional_operator_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_conditional_operator *, node, base->self);
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node->expr2));
    if (node->expr1 != NULL) {
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node->expr1));
    }
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node->condition));
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_CONDITIONAL_OPERATION_CLASS = {.type = KEFIR_AST_CONDITIONAL_OPERATION,
                                                                     .visit = ast_conditional_operator_visit,
                                                                     .free = ast_conditional_operator_free};

struct kefir_ast_conditional_operator *kefir_ast_new_conditional_operator(struct kefir_mem *mem,
                                                                          struct kefir_ast_node_base *condition,
                                                                          struct kefir_ast_node_base *expr1,
                                                                          struct kefir_ast_node_base *expr2) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(condition != NULL, NULL);
    REQUIRE(expr2 != NULL, NULL);

    struct kefir_ast_conditional_operator *oper = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_conditional_operator));
    REQUIRE(oper != NULL, NULL);
    oper->base.refcount = 1;
    oper->base.klass = &AST_CONDITIONAL_OPERATION_CLASS;
    oper->base.self = oper;
    kefir_result_t res = kefir_ast_node_properties_init(&oper->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, oper);
        return NULL;
    });
    res = kefir_source_location_empty(&oper->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, oper);
        return NULL;
    });
    oper->condition = condition;
    oper->expr1 = expr1;
    oper->expr2 = expr2;
    return oper;
}
