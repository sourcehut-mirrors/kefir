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

NODE_VISIT_IMPL(ast_attribute_list_visit, kefir_ast_attribute_list, attribute_list)

struct kefir_ast_node_base *ast_attribute_list_clone(struct kefir_mem *, struct kefir_ast_node_base *);

kefir_result_t ast_attribute_list_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_attribute_list *, node, base->self);
    REQUIRE_OK(kefir_list_free(mem, &node->list));
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_ATTRIBUTE_LIST_CLASS = {.type = KEFIR_AST_ATTRIBUTE_LIST,
                                                              .visit = ast_attribute_list_visit,
                                                              .clone = ast_attribute_list_clone,
                                                              .free = ast_attribute_list_free};

static kefir_result_t attribute_param_free(struct kefir_mem *mem, struct kefir_list *list,
                                           struct kefir_list_entry *entry, void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));
    ASSIGN_DECL_CAST(struct kefir_ast_node_base *, param, entry->value);

    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, param));
    return KEFIR_OK;
}

static kefir_result_t attribute_free(struct kefir_mem *mem, struct kefir_list *list, struct kefir_list_entry *entry,
                                     void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));
    ASSIGN_DECL_CAST(struct kefir_ast_attribute *, attribute, entry->value);

    REQUIRE_OK(kefir_list_free(mem, &attribute->parameters));
    attribute->name = NULL;
    KEFIR_FREE(mem, attribute);
    return KEFIR_OK;
}

struct kefir_ast_node_base *ast_attribute_list_clone(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(base != NULL, NULL);
    ASSIGN_DECL_CAST(struct kefir_ast_attribute_list *, node, base->self);
    struct kefir_ast_attribute_list *clone = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_attribute_list));
    REQUIRE(clone != NULL, NULL);
    clone->base.klass = &AST_ATTRIBUTE_LIST_CLASS;
    clone->base.self = clone;
    clone->base.source_location = base->source_location;
    kefir_result_t res = kefir_ast_node_properties_clone(&clone->base.properties, &node->base.properties);
    REQUIRE_CHAIN(&res, kefir_list_init(&clone->list));
    REQUIRE_CHAIN(&res, kefir_list_on_remove(&clone->list, attribute_free, NULL));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, clone);
        return NULL;
    });

    for (const struct kefir_list_entry *iter = kefir_list_head(&node->list); iter != NULL; kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(struct kefir_ast_attribute *, attribute, iter->value);
        struct kefir_ast_attribute *attribute_clone = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_attribute));
        REQUIRE_ELSE(attribute_clone != NULL, {
            kefir_list_free(mem, &clone->list);
            KEFIR_FREE(mem, clone);
            return NULL;
        });
        attribute_clone->name = attribute->name;
        res = kefir_list_init(&attribute_clone->parameters);
        REQUIRE_CHAIN(&res, kefir_list_on_remove(&attribute_clone->parameters, attribute_param_free, NULL));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, attribute_clone);
            kefir_list_free(mem, &clone->list);
            KEFIR_FREE(mem, clone);
            return NULL;
        });

        for (const struct kefir_list_entry *iter2 = kefir_list_head(&attribute->parameters); iter2 != NULL;
             kefir_list_next(&iter2)) {

            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, param, iter2->value);
            struct kefir_ast_node_base *param_clone = KEFIR_AST_NODE_CLONE(mem, param);
            REQUIRE_ELSE(param_clone != NULL, {
                kefir_list_free(mem, &attribute_clone->parameters);
                KEFIR_FREE(mem, attribute_clone);
                kefir_list_free(mem, &clone->list);
                KEFIR_FREE(mem, clone);
                return NULL;
            });

            res = kefir_list_insert_after(mem, &attribute_clone->parameters,
                                          kefir_list_tail(&attribute_clone->parameters), param_clone);
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_AST_NODE_FREE(mem, param_clone);
                kefir_list_free(mem, &attribute_clone->parameters);
                KEFIR_FREE(mem, attribute_clone);
                kefir_list_free(mem, &clone->list);
                KEFIR_FREE(mem, clone);
                return NULL;
            });
        }

        res = kefir_list_insert_after(mem, &clone->list, kefir_list_tail(&clone->list), attribute_clone);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_list_free(mem, &attribute_clone->parameters);
            KEFIR_FREE(mem, attribute_clone);
            kefir_list_free(mem, &clone->list);
            KEFIR_FREE(mem, clone);
            return NULL;
        });
    }
    return KEFIR_AST_NODE_BASE(clone);
}

struct kefir_ast_attribute_list *kefir_ast_new_attribute_list(struct kefir_mem *mem) {
    REQUIRE(mem != NULL, NULL);

    struct kefir_ast_attribute_list *attribute_list = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_attribute_list));
    REQUIRE(attribute_list != NULL, NULL);
    attribute_list->base.klass = &AST_ATTRIBUTE_LIST_CLASS;
    attribute_list->base.self = attribute_list;
    kefir_result_t res = kefir_ast_node_properties_init(&attribute_list->base.properties);
    REQUIRE_CHAIN(&res, kefir_source_location_empty(&attribute_list->base.source_location));
    REQUIRE_CHAIN(&res, kefir_list_init(&attribute_list->list));
    REQUIRE_CHAIN(&res, kefir_list_on_remove(&attribute_list->list, attribute_free, NULL));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, attribute_list);
        return NULL;
    });
    return attribute_list;
}

kefir_result_t kefir_ast_attribute_list_append(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                               const char *name, struct kefir_ast_attribute_list *attribute_list,
                                               struct kefir_ast_attribute **attribute) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expeted valid memory allocator"));
    REQUIRE(name != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expeted valid attribute name"));
    REQUIRE(attribute_list != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expeted valid attribute list"));
    REQUIRE(attribute != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expeted valid pointer to attribute"));

    if (symbols != NULL) {
        name = kefir_string_pool_insert(mem, symbols, name, NULL);
        REQUIRE(name != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert attribute name into symbol table"));
    }

    struct kefir_ast_attribute *attr = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_attribute));
    REQUIRE(attr != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST attribute"));
    attr->name = name;
    kefir_result_t res = kefir_list_init(&attr->parameters);
    REQUIRE_CHAIN(&res, kefir_list_on_remove(&attr->parameters, attribute_param_free, NULL));
    REQUIRE_CHAIN(&res,
                  kefir_list_insert_after(mem, &attribute_list->list, kefir_list_tail(&attribute_list->list), attr));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, attr);
        return res;
    });

    *attribute = attr;
    return KEFIR_OK;
}
