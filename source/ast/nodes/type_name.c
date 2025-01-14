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

NODE_VISIT_IMPL(ast_type_name_visit, kefir_ast_type_name, type_name)

kefir_result_t ast_type_name_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_type_name *, node, base->self);
    REQUIRE_OK(kefir_ast_declarator_specifier_list_free(mem, &node->type_decl.specifiers));
    REQUIRE_OK(kefir_ast_declarator_free(mem, node->type_decl.declarator));
    node->type_decl.declarator = NULL;
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_TYPE_NAME_CLASS = {
    .type = KEFIR_AST_TYPE_NAME, .visit = ast_type_name_visit, .free = ast_type_name_free};

struct kefir_ast_type_name *kefir_ast_new_type_name(struct kefir_mem *mem, struct kefir_ast_declarator *decl) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(decl != NULL, NULL);

    kefir_bool_t abstract = false;
    REQUIRE(kefir_ast_declarator_is_abstract(decl, &abstract) == KEFIR_OK && abstract, NULL);

    struct kefir_ast_type_name *type_name = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_type_name));
    REQUIRE(type_name != NULL, NULL);
    type_name->base.refcount = 1;
    type_name->base.klass = &AST_TYPE_NAME_CLASS;
    type_name->base.self = type_name;
    kefir_result_t res = kefir_ast_node_properties_init(&type_name->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, type_name);
        return NULL;
    });
    res = kefir_source_location_empty(&type_name->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, type_name);
        return NULL;
    });

    res = kefir_ast_declarator_specifier_list_init(&type_name->type_decl.specifiers);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, type_name);
        return NULL;
    });
    type_name->type_decl.declarator = decl;
    return type_name;
}
