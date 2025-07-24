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

NODE_VISIT_IMPL(ast_attribute_declaration_visit, kefir_ast_attribute_declaration, attribute_declaration)

kefir_result_t ast_attribute_declaration_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_attribute_declaration *, node, base->self);
    REQUIRE_OK(kefir_ast_node_attributes_free(mem, &node->attributes));
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_ATTRIBUTE_DECLARATION_LIST_CLASS = {.type = KEFIR_AST_ATTRIBUTE_DECLARATION,
                                                                          .visit = ast_attribute_declaration_visit,
                                                                          .free = ast_attribute_declaration_free};

struct kefir_ast_attribute_declaration *kefir_ast_new_attribute_declaration(struct kefir_mem *mem) {
    REQUIRE(mem != NULL, NULL);

    struct kefir_ast_attribute_declaration *attribute_declaration =
        KEFIR_MALLOC(mem, sizeof(struct kefir_ast_attribute_declaration));
    REQUIRE(attribute_declaration != NULL, NULL);
    attribute_declaration->base.refcount = 1;
    attribute_declaration->base.klass = &AST_ATTRIBUTE_DECLARATION_LIST_CLASS;
    attribute_declaration->base.self = attribute_declaration;
    kefir_result_t res = kefir_ast_node_properties_init(&attribute_declaration->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, attribute_declaration);
        return NULL;
    });
    res = kefir_source_location_empty(&attribute_declaration->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, attribute_declaration);
        return NULL;
    });

    res = kefir_ast_node_attributes_init(&attribute_declaration->attributes);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, attribute_declaration);
        return NULL;
    });
    return attribute_declaration;
}
