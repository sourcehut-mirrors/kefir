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
#include "kefir/ast/downcast.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

NODE_VISIT_IMPL(ast_generic_selection_visit, kefir_ast_generic_selection, generic_selection)

kefir_result_t ast_generic_selection_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_generic_selection *, node, base->self);
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node->control));
    for (kefir_size_t i = 0; i < node->associations_length; i++) {
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(node->associations[i].type_name)));
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node->associations[i].expr));
    }
    KEFIR_FREE(mem, node->associations);
    if (node->default_assoc != NULL) {
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node->default_assoc));
    }
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_GENERIC_SELECTION_CLASS = {
    .type = KEFIR_AST_GENERIC_SELECTION, .visit = ast_generic_selection_visit, .free = ast_generic_selection_free};

struct kefir_ast_generic_selection *kefir_ast_new_generic_selection(struct kefir_mem *mem,
                                                                    struct kefir_ast_node_base *control) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(control != NULL, NULL);

    struct kefir_ast_generic_selection *selection = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_generic_selection));
    REQUIRE(selection != NULL, NULL);
    selection->base.refcount = 1;
    selection->base.klass = &AST_GENERIC_SELECTION_CLASS;
    selection->base.self = selection;
    kefir_result_t res = kefir_ast_node_properties_init(&selection->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, selection);
        return NULL;
    });
    res = kefir_source_location_empty(&selection->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, selection);
        return NULL;
    });
    selection->control = control;
    selection->default_assoc = NULL;
    selection->associations = NULL;
    selection->associations_capacity = 0;
    selection->associations_length = 0;
    return selection;
}

kefir_result_t kefir_ast_generic_selection_append(struct kefir_mem *mem, struct kefir_ast_generic_selection *selection,
                                                  struct kefir_ast_type_name *type_name,
                                                  struct kefir_ast_node_base *expr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(selection != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST generic selection"));
    REQUIRE(expr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST expression"));

    if (type_name != NULL) {
        if (selection->associations_length + 1 > selection->associations_capacity) {
            kefir_size_t new_capacity = MAX(1, 2 * selection->associations_capacity);
            struct kefir_ast_generic_selection_assoc *new_assoc = KEFIR_REALLOC(
                mem, selection->associations, sizeof(struct kefir_ast_generic_selection_assoc) * new_capacity);
            REQUIRE(new_assoc != NULL,
                    KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate generic selection associations"));

            selection->associations = new_assoc;
            selection->associations_capacity = new_capacity;
        }

        selection->associations[selection->associations_length].type_name = type_name;
        selection->associations[selection->associations_length++].expr = expr;
    } else {
        REQUIRE(
            selection->default_assoc == NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "AST generic selection cannot have multiple default associations"));
        selection->default_assoc = expr;
    }
    return KEFIR_OK;
}
