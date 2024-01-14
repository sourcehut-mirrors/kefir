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

#include "kefir/ast/node.h"
#include "kefir/ast/node_internal.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

NODE_VISIT_IMPL(ast_binary_operation_visit, kefir_ast_binary_operation, binary_operation)

struct kefir_ast_node_base *ast_binary_operation_clone(struct kefir_mem *, struct kefir_ast_node_base *);

kefir_result_t ast_binary_operation_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_binary_operation *, node, base->self);
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node->arg1));
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node->arg2));
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_BINARY_OPERATION_CLASS = {.type = KEFIR_AST_BINARY_OPERATION,
                                                                .visit = ast_binary_operation_visit,
                                                                .clone = ast_binary_operation_clone,
                                                                .free = ast_binary_operation_free};

struct kefir_ast_node_base *ast_binary_operation_clone(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(base != NULL, NULL);
    ASSIGN_DECL_CAST(struct kefir_ast_binary_operation *, node, base->self);
    struct kefir_ast_binary_operation *clone = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_binary_operation));
    REQUIRE(clone != NULL, NULL);
    clone->base.klass = &AST_BINARY_OPERATION_CLASS;
    clone->base.self = clone;
    clone->base.source_location = base->source_location;
    kefir_result_t res = kefir_ast_node_properties_clone(&clone->base.properties, &node->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, clone);
        return NULL;
    });
    clone->type = node->type;
    clone->arg1 = KEFIR_AST_NODE_CLONE(mem, node->arg1);
    REQUIRE_ELSE(clone->arg1 != NULL, {
        KEFIR_FREE(mem, clone);
        return NULL;
    });
    clone->arg2 = KEFIR_AST_NODE_CLONE(mem, node->arg2);
    REQUIRE_ELSE(clone->arg2 != NULL, {
        KEFIR_AST_NODE_FREE(mem, clone->arg1);
        KEFIR_FREE(mem, clone);
        return NULL;
    });
    return KEFIR_AST_NODE_BASE(clone);
}

struct kefir_ast_binary_operation *kefir_ast_new_binary_operation(struct kefir_mem *mem,
                                                                  kefir_ast_binary_operation_type_t type,
                                                                  struct kefir_ast_node_base *arg1,
                                                                  struct kefir_ast_node_base *arg2) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(arg1 != NULL, NULL);
    REQUIRE(arg2 != NULL, NULL);
    struct kefir_ast_binary_operation *oper = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_binary_operation));
    REQUIRE(oper != NULL, NULL);
    oper->base.klass = &AST_BINARY_OPERATION_CLASS;
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
    oper->type = type;
    oper->arg1 = arg1;
    oper->arg2 = arg2;
    return oper;
}
