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
#include "kefir/ast/attributes.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

static kefir_result_t attribute_list_free(struct kefir_mem *mem, struct kefir_list *list,
                                          struct kefir_list_entry *entry, void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));
    ASSIGN_DECL_CAST(struct kefir_ast_attribute_list *, attribute_list, entry->value);

    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(attribute_list)));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_node_attributes_init(struct kefir_ast_node_attributes *attributes) {
    REQUIRE(attributes != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST node attributes"));

    REQUIRE_OK(kefir_list_init(&attributes->attributes));
    REQUIRE_OK(kefir_list_on_remove(&attributes->attributes, attribute_list_free, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_node_attributes_free(struct kefir_mem *mem, struct kefir_ast_node_attributes *attributes) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(attributes != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node attributes"));

    REQUIRE_OK(kefir_list_free(mem, &attributes->attributes));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_node_attributes_append(struct kefir_mem *mem, struct kefir_ast_node_attributes *attributes,
                                                struct kefir_ast_attribute_list *attr_list) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(attributes != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node attributes"));
    REQUIRE(attr_list != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST attribute list"));

    REQUIRE_OK(
        kefir_list_insert_after(mem, &attributes->attributes, kefir_list_tail(&attributes->attributes), attr_list));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_node_attributes_move(struct kefir_ast_node_attributes *dest,
                                              struct kefir_ast_node_attributes *src) {
    REQUIRE(dest != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination AST node attributes"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source AST node attributes"));

    REQUIRE_OK(kefir_list_move_all(&dest->attributes, &src->attributes));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_node_attributes_clone(struct kefir_mem *mem, struct kefir_ast_node_attributes *dest,
                                               const struct kefir_ast_node_attributes *src) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dest != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination AST node attributes"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source AST node attributes"));

    for (const struct kefir_list_entry *iter = kefir_list_head(&src->attributes); iter != NULL;
         kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(struct kefir_ast_attribute_list *, attribute_list, iter->value);
        struct kefir_ast_node_base *clone = KEFIR_AST_NODE_CLONE(mem, KEFIR_AST_NODE_BASE(attribute_list));
        REQUIRE(clone != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to clone AST attribute list"));
        kefir_result_t res = kefir_ast_node_attributes_append(mem, dest, clone->self);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_AST_NODE_FREE(mem, clone);
            return res;
        });
    }
    return KEFIR_OK;
}
