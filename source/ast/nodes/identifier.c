/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

NODE_VISIT_IMPL(ast_identifier_visit, kefir_ast_identifier, identifier)

struct kefir_ast_node_base *ast_identifier_clone(struct kefir_mem *, struct kefir_ast_node_base *);

kefir_result_t ast_identifier_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_identifier *, node, base->self);
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_IDENTIFIER_CLASS = {.type = KEFIR_AST_IDENTIFIER,
                                                          .visit = ast_identifier_visit,
                                                          .clone = ast_identifier_clone,
                                                          .free = ast_identifier_free};

struct kefir_ast_node_base *ast_identifier_clone(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(base != NULL, NULL);
    ASSIGN_DECL_CAST(struct kefir_ast_identifier *, node, base->self);
    struct kefir_ast_identifier *clone = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_identifier));
    REQUIRE(clone != NULL, NULL);
    clone->base.klass = &AST_IDENTIFIER_CLASS;
    clone->base.self = clone;
    clone->base.source_location = base->source_location;
    kefir_result_t res = kefir_ast_node_properties_clone(&clone->base.properties, &node->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, clone);
        return NULL;
    });
    clone->identifier = node->identifier;
    return KEFIR_AST_NODE_BASE(clone);
}

struct kefir_ast_identifier *kefir_ast_new_identifier(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
                                                      const char *identifier) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(symbols != NULL, NULL);
    REQUIRE(identifier != NULL, NULL);
    const char *id_copy = kefir_symbol_table_insert(mem, symbols, identifier, NULL);
    REQUIRE(id_copy != NULL, NULL);
    struct kefir_ast_identifier *id = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_identifier));
    REQUIRE(id != NULL, NULL);
    id->base.klass = &AST_IDENTIFIER_CLASS;
    id->base.self = id;
    kefir_result_t res = kefir_ast_node_properties_init(&id->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, id);
        return NULL;
    });
    res = kefir_source_location_empty(&id->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, id);
        return NULL;
    });
    id->identifier = id_copy;
    return id;
}
