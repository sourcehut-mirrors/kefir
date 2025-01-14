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

NODE_VISIT_IMPL(ast_init_declarator_visit, kefir_ast_init_declarator, init_declarator)

kefir_result_t ast_init_declarator_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_init_declarator *, node, base->self);
    if (node->declarator != NULL) {
        REQUIRE_OK(kefir_ast_declarator_free(mem, node->declarator));
        node->declarator = NULL;
    }
    if (node->initializer != NULL) {
        REQUIRE_OK(kefir_ast_initializer_free(mem, node->initializer));
        node->initializer = NULL;
    }
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_DECLARATION_CLASS = {
    .type = KEFIR_AST_INIT_DECLARATOR, .visit = ast_init_declarator_visit, .free = ast_init_declarator_free};

struct kefir_ast_init_declarator *kefir_ast_new_init_declarator(struct kefir_mem *mem,
                                                                struct kefir_ast_declarator *decl,
                                                                struct kefir_ast_initializer *initializer) {
    REQUIRE(mem != NULL, NULL);

    struct kefir_ast_init_declarator *init_declarator = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_init_declarator));
    REQUIRE(init_declarator != NULL, NULL);
    init_declarator->base.refcount = 1;
    init_declarator->base.klass = &AST_DECLARATION_CLASS;
    init_declarator->base.self = init_declarator;
    kefir_result_t res = kefir_ast_node_properties_init(&init_declarator->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, init_declarator);
        return NULL;
    });
    res = kefir_source_location_empty(&init_declarator->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, init_declarator);
        return NULL;
    });

    init_declarator->declarator = decl;
    init_declarator->initializer = initializer;
    return init_declarator;
}
