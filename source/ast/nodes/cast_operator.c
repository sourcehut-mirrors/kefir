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

NODE_VISIT_IMPL(ast_cast_operator_visit, kefir_ast_cast_operator, cast_operator)

kefir_result_t ast_cast_operator_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_cast_operator *, node, base->self);
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(node->type_name)));
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node->expr));
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_CAST_OPERATOR_CLASS = {
    .type = KEFIR_AST_CAST_OPERATOR, .visit = ast_cast_operator_visit, .free = ast_cast_operator_free};

struct kefir_ast_cast_operator *kefir_ast_new_cast_operator(struct kefir_mem *mem,
                                                            struct kefir_ast_type_name *type_name,
                                                            struct kefir_ast_node_base *expr) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(type_name != NULL, NULL);
    REQUIRE(expr != NULL, NULL);
    struct kefir_ast_cast_operator *cast = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_cast_operator));
    REQUIRE(cast != NULL, NULL);
    cast->base.refcount = 1;
    cast->base.klass = &AST_CAST_OPERATOR_CLASS;
    cast->base.self = cast;
    kefir_result_t res = kefir_ast_node_properties_init(&cast->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, cast);
        return NULL;
    });
    res = kefir_source_location_empty(&cast->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, cast);
        return NULL;
    });
    cast->type_name = type_name;
    cast->expr = expr;
    return cast;
}
