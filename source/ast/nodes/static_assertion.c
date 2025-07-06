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

NODE_VISIT_IMPL(ast_static_assertion_visit, kefir_ast_static_assertion, static_assertion)

kefir_result_t ast_static_assertion_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_static_assertion *, node, base->self);
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node->condition));
    if (node->string != NULL) {
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(node->string)));
    }
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_STATIC_ASSERTION_CLASS = {
    .type = KEFIR_AST_STATIC_ASSERTION, .visit = ast_static_assertion_visit, .free = ast_static_assertion_free};

struct kefir_ast_static_assertion *kefir_ast_new_static_assertion(struct kefir_mem *mem,
                                                                  struct kefir_ast_node_base *condition,
                                                                  struct kefir_ast_string_literal *string) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(condition != NULL, NULL);

    struct kefir_ast_static_assertion *static_assertion = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_static_assertion));
    REQUIRE(static_assertion != NULL, NULL);
    static_assertion->base.refcount = 1;
    static_assertion->base.klass = &AST_STATIC_ASSERTION_CLASS;
    static_assertion->base.self = static_assertion;
    kefir_result_t res = kefir_ast_node_properties_init(&static_assertion->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, static_assertion);
        return NULL;
    });
    res = kefir_source_location_empty(&static_assertion->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, static_assertion);
        return NULL;
    });

    static_assertion->condition = condition;
    static_assertion->string = string;
    return static_assertion;
}
