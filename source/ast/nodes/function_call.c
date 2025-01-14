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

NODE_VISIT_IMPL(ast_function_call_visit, kefir_ast_function_call, function_call)

kefir_result_t ast_function_call_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_function_call *, node, base->self);
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node->function));
    REQUIRE_OK(kefir_list_free(mem, &node->arguments));
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_FUNCTION_CALL_CLASS = {
    .type = KEFIR_AST_FUNCTION_CALL, .visit = ast_function_call_visit, .free = ast_function_call_free};

static kefir_result_t function_call_argument_free(struct kefir_mem *mem, struct kefir_list *list,
                                                  struct kefir_list_entry *entry, void *payload) {
    UNUSED(list);
    UNUSED(payload);
    ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, entry->value);
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node));
    return KEFIR_OK;
}

struct kefir_ast_function_call *kefir_ast_new_function_call(struct kefir_mem *mem,
                                                            struct kefir_ast_node_base *function) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(function != NULL, NULL);
    struct kefir_ast_function_call *function_call = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_function_call));
    REQUIRE(function_call != NULL, NULL);
    function_call->base.refcount = 1;
    function_call->base.klass = &AST_FUNCTION_CALL_CLASS;
    function_call->base.self = function_call;
    kefir_result_t res = kefir_ast_node_properties_init(&function_call->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, function_call);
        return NULL;
    });
    res = kefir_source_location_empty(&function_call->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, function_call);
        return NULL;
    });
    function_call->function = function;
    res = kefir_list_init(&function_call->arguments);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, function_call);
        return NULL;
    });
    res = kefir_list_on_remove(&function_call->arguments, function_call_argument_free, NULL);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &function_call->arguments);
        KEFIR_FREE(mem, function_call);
        return NULL;
    });
    return function_call;
}

kefir_result_t kefir_ast_function_call_append(struct kefir_mem *mem, struct kefir_ast_function_call *call,
                                              struct kefir_ast_node_base *arg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(call != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST function call"));
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid function parameter AST node"));
    REQUIRE_OK(kefir_list_insert_after(mem, &call->arguments, kefir_list_tail(&call->arguments), arg));
    return KEFIR_OK;
}
