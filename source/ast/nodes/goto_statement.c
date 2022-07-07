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

NODE_VISIT_IMPL(ast_goto_statement_visit, kefir_ast_goto_statement, goto_statement)
NODE_VISIT_IMPL(ast_goto_address_statement_visit, kefir_ast_goto_statement, goto_address_statement)

struct kefir_ast_node_base *ast_goto_statement_clone(struct kefir_mem *, struct kefir_ast_node_base *);

kefir_result_t ast_goto_statement_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_goto_statement *, node, base->self);
    if (base->klass->type == KEFIR_AST_GOTO_STATEMENT) {
        node->identifier = NULL;
    } else {
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node->target));
        node->target = NULL;
    }
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_GOTO_STATEMENT_CLASS = {.type = KEFIR_AST_GOTO_STATEMENT,
                                                              .visit = ast_goto_statement_visit,
                                                              .clone = ast_goto_statement_clone,
                                                              .free = ast_goto_statement_free};

const struct kefir_ast_node_class AST_GOTO_ADDRESS_STATEMENT_CLASS = {.type = KEFIR_AST_GOTO_ADDRESS_STATEMENT,
                                                                      .visit = ast_goto_address_statement_visit,
                                                                      .clone = ast_goto_statement_clone,
                                                                      .free = ast_goto_statement_free};

struct kefir_ast_node_base *ast_goto_statement_clone(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(base != NULL, NULL);
    ASSIGN_DECL_CAST(struct kefir_ast_goto_statement *, node, base->self);
    struct kefir_ast_goto_statement *clone = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_goto_statement));
    REQUIRE(clone != NULL, NULL);
    clone->base.klass = node->base.klass;
    clone->base.self = clone;
    clone->base.source_location = base->source_location;
    kefir_result_t res = kefir_ast_node_properties_clone(&clone->base.properties, &node->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, clone);
        return NULL;
    });

    if (node->base.klass->type == KEFIR_AST_GOTO_STATEMENT) {
        clone->identifier = node->identifier;
    } else {
        clone->target = KEFIR_AST_NODE_CLONE(mem, node->target);
        REQUIRE_ELSE(clone->target != NULL, {
            KEFIR_FREE(mem, clone);
            return NULL;
        });
    }
    return KEFIR_AST_NODE_BASE(clone);
}

struct kefir_ast_goto_statement *kefir_ast_new_goto_statement(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
                                                              const char *identifier) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(identifier != NULL, NULL);

    if (symbols != NULL) {
        identifier = kefir_symbol_table_insert(mem, symbols, identifier, NULL);
        REQUIRE(identifier != NULL, NULL);
    }

    struct kefir_ast_goto_statement *stmt = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_goto_statement));
    REQUIRE(stmt != NULL, NULL);
    stmt->base.klass = &AST_GOTO_STATEMENT_CLASS;
    stmt->base.self = stmt;
    kefir_result_t res = kefir_ast_node_properties_init(&stmt->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, stmt);
        return NULL;
    });
    res = kefir_source_location_empty(&stmt->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, stmt);
        return NULL;
    });

    stmt->identifier = identifier;
    return stmt;
}

struct kefir_ast_goto_statement *kefir_ast_new_goto_address_statement(struct kefir_mem *mem,
                                                                      struct kefir_ast_node_base *target) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(target != NULL, NULL);

    struct kefir_ast_goto_statement *stmt = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_goto_statement));
    REQUIRE(stmt != NULL, NULL);
    stmt->base.klass = &AST_GOTO_ADDRESS_STATEMENT_CLASS;
    stmt->base.self = stmt;
    kefir_result_t res = kefir_ast_node_properties_init(&stmt->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, stmt);
        return NULL;
    });
    res = kefir_source_location_empty(&stmt->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, stmt);
        return NULL;
    });

    stmt->target = target;
    return stmt;
}
