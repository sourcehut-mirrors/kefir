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
#include "kefir/ast/attributes.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_ast_node_attributes_init(struct kefir_ast_node_attributes *attributes) {
    REQUIRE(attributes != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST node attributes"));

    attributes->content = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_node_attributes_free(struct kefir_mem *mem, struct kefir_ast_node_attributes *attributes) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(attributes != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node attributes"));

    if (attributes->content != NULL) {
        for (kefir_size_t i = 0; i < attributes->content->attributes_length; i++) {
            REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(attributes->content->attributes[i])));
        }
        memset(attributes->content, 0, sizeof(struct kefir_ast_node_attributes_content));
        KEFIR_FREE(mem, attributes->content);
        attributes->content = NULL;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_node_attributes_append(struct kefir_mem *mem, struct kefir_ast_node_attributes *attributes,
                                                struct kefir_ast_attribute_list *attr_list) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(attributes != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node attributes"));
    REQUIRE(attr_list != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST attribute list"));

    const kefir_size_t current_length = kefir_ast_node_attributes_length(attributes);
    const kefir_size_t new_length = current_length + 1;
    struct kefir_ast_node_attributes_content *new_attributes = KEFIR_REALLOC(mem, attributes->content, sizeof(struct kefir_ast_node_attributes_content) + sizeof(struct kefir_ast_attribute_list *) * new_length);
    REQUIRE(new_attributes != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST node attributes"));
    new_attributes->attributes[current_length] = attr_list;
    new_attributes->attributes_length = new_length;
    attributes->content = new_attributes;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_node_attributes_move(struct kefir_mem *mem, struct kefir_ast_node_attributes *dest,
                                              struct kefir_ast_node_attributes *src) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dest != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination AST node attributes"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source AST node attributes"));
    REQUIRE(kefir_ast_node_attributes_length(src) > 0, KEFIR_OK);

    const kefir_size_t current_length = kefir_ast_node_attributes_length(dest);
    const kefir_size_t new_length = current_length + kefir_ast_node_attributes_length(src);
    struct kefir_ast_node_attributes_content *new_attributes = KEFIR_REALLOC(mem, dest->content, sizeof(struct kefir_ast_node_attributes_content) + sizeof(struct kefir_ast_attribute_list *) * new_length);
    REQUIRE(new_attributes != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST node attributes"));
    memcpy(&new_attributes->attributes[current_length], src->content->attributes, sizeof(struct kefir_ast_attribute_list *) * src->content->attributes_length);
    new_attributes->attributes_length = new_length;
    dest->content = new_attributes;
    
    KEFIR_FREE(mem, src->content);
    src->content = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_node_attributes_clone(struct kefir_mem *mem, struct kefir_ast_node_attributes *dest,
                                               const struct kefir_ast_node_attributes *src) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dest != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination AST node attributes"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source AST node attributes"));

    for (kefir_size_t i = 0; i < kefir_ast_node_attributes_length(src); i++) {
        struct kefir_ast_node_base *clone = KEFIR_AST_NODE_REF(KEFIR_AST_NODE_BASE(kefir_ast_node_attributes_at(src, i)));
        REQUIRE(clone != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to clone AST attribute list"));
        kefir_result_t res = kefir_ast_node_attributes_append(mem, dest, KEFIR_AST_NODE_SELF(clone));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_AST_NODE_FREE(mem, clone);
            return res;
        });
    }
    return KEFIR_OK;
}

kefir_size_t kefir_ast_node_attributes_length(const struct kefir_ast_node_attributes *attr) {
    REQUIRE(attr != NULL, 0);
    REQUIRE(attr->content != NULL, 0);
    return attr->content->attributes_length;
}

struct kefir_ast_attribute_list *kefir_ast_node_attributes_at(const struct kefir_ast_node_attributes *attr, kefir_size_t index) {
    REQUIRE(attr != NULL, NULL);
    REQUIRE(attr->content != NULL, NULL);
    REQUIRE(index < attr->content->attributes_length, NULL);

    return attr->content->attributes[index];
}
