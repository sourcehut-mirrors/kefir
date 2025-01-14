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
#include "kefir/ast/downcast.h"

NODE_VISIT_IMPL(ast_declaration_visit, kefir_ast_declaration, declaration)

kefir_result_t ast_declaration_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_declaration *, node, base->self);
    REQUIRE_OK(kefir_ast_declarator_specifier_list_free(mem, &node->specifiers));
    REQUIRE_OK(kefir_list_free(mem, &node->init_declarators));
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_DECLARATION_LIST_CLASS = {
    .type = KEFIR_AST_DECLARATION, .visit = ast_declaration_visit, .free = ast_declaration_free};

static kefir_result_t declaration_free(struct kefir_mem *mem, struct kefir_list *list, struct kefir_list_entry *entry,
                                       void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));

    ASSIGN_DECL_CAST(struct kefir_ast_node_base *, declaration, entry->value);
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, declaration));
    return KEFIR_OK;
}

struct kefir_ast_declaration *kefir_ast_new_declaration(struct kefir_mem *mem) {
    REQUIRE(mem != NULL, NULL);

    struct kefir_ast_declaration *declaration = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_declaration));
    REQUIRE(declaration != NULL, NULL);
    declaration->base.refcount = 1;
    declaration->base.klass = &AST_DECLARATION_LIST_CLASS;
    declaration->base.self = declaration;
    kefir_result_t res = kefir_ast_node_properties_init(&declaration->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, declaration);
        return NULL;
    });
    res = kefir_source_location_empty(&declaration->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, declaration);
        return NULL;
    });

    res = kefir_ast_declarator_specifier_list_init(&declaration->specifiers);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, declaration);
        return NULL;
    });

    REQUIRE_CHAIN(&res, kefir_list_init(&declaration->init_declarators));
    REQUIRE_CHAIN(&res, kefir_list_on_remove(&declaration->init_declarators, declaration_free, NULL));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_declarator_specifier_list_free(mem, &declaration->specifiers);
        KEFIR_FREE(mem, declaration);
        return NULL;
    });
    return declaration;
}

struct kefir_ast_declaration *kefir_ast_new_single_declaration(struct kefir_mem *mem,
                                                               struct kefir_ast_declarator *declarator,
                                                               struct kefir_ast_initializer *initializer,
                                                               struct kefir_ast_init_declarator **declaration_ptr) {
    REQUIRE(mem != NULL, NULL);

    struct kefir_ast_declaration *decl_list = kefir_ast_new_declaration(mem);
    REQUIRE(decl_list != NULL, NULL);
    struct kefir_ast_init_declarator *declaration = kefir_ast_new_init_declarator(mem, declarator, initializer);
    REQUIRE_ELSE(declaration != NULL, {
        KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(decl_list));
        return NULL;
    });
    kefir_result_t res =
        kefir_list_insert_after(mem, &decl_list->init_declarators, kefir_list_tail(&decl_list->init_declarators),
                                KEFIR_AST_NODE_BASE(declaration));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(decl_list));
        KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(declaration));
        return NULL;
    });
    ASSIGN_PTR(declaration_ptr, declaration);
    return decl_list;
}

kefir_result_t kefir_ast_declaration_unpack_single(struct kefir_ast_declaration *list,
                                                   struct kefir_ast_init_declarator **declaration_ptr) {
    REQUIRE(list != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST declaration list"));
    REQUIRE(declaration_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST declaration"));

    REQUIRE(kefir_list_length(&list->init_declarators) == 1,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected declaration list to contain a single declaration"));
    struct kefir_ast_node_base *node = kefir_list_head(&list->init_declarators)->value;

    kefir_result_t res;
    REQUIRE_MATCH_OK(
        &res, kefir_ast_downcast_init_declarator(node, declaration_ptr, false),
        KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected declaration list to contain a single declaration"));
    return KEFIR_OK;
}
