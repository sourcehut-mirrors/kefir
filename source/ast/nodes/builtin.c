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

NODE_VISIT_IMPL(ast_builtin_visit, kefir_ast_builtin, builtin)

kefir_result_t ast_builtin_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_builtin *, node, base->self);
    for (kefir_size_t i = 0; i < node->argument_length; i++) {
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node->arguments[i]));
    }
    KEFIR_FREE(mem, node->arguments);
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_BUILTIN_CLASS = {
    .type = KEFIR_AST_BUILTIN, .visit = ast_builtin_visit, .free = ast_builtin_free};

struct kefir_ast_builtin *kefir_ast_new_builtin(struct kefir_mem *mem, kefir_ast_builtin_operator_t builtin_op) {
    REQUIRE(mem != NULL, NULL);

    struct kefir_ast_builtin *builtin = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_builtin));
    REQUIRE(builtin != NULL, NULL);
    builtin->base.refcount = 1;
    builtin->base.klass = &AST_BUILTIN_CLASS;
    builtin->base.self = builtin;
    kefir_result_t res = kefir_ast_node_properties_init(&builtin->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, builtin);
        return NULL;
    });
    res = kefir_source_location_empty(&builtin->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, builtin);
        return NULL;
    });
    builtin->builtin = builtin_op;
    builtin->arguments = NULL;
    builtin->argument_capacity = 0;
    builtin->argument_length = 0;
    return builtin;
}

kefir_result_t kefir_ast_builtin_append(struct kefir_mem *mem, struct kefir_ast_builtin *call,
                                        struct kefir_ast_node_base *arg) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(call != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST builtin"));
    REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid builtin parameter AST node"));

    if (call->argument_length + 1 > call->argument_capacity) {
        kefir_size_t new_capacity = MAX(1, 2 * call->argument_capacity);
        struct kefir_ast_node_base **new_arguments =
            KEFIR_REALLOC(mem, call->arguments, sizeof(struct kefir_ast_node_base *) * new_capacity);
        REQUIRE(new_arguments != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST function call arguments"));

        call->arguments = new_arguments;
        call->argument_capacity = new_capacity;
    }
    call->arguments[call->argument_length++] = arg;
    return KEFIR_OK;
}
