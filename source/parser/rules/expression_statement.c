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

#include "kefir/parser/rule_helpers.h"
#include "kefir/core/source_error.h"

kefir_result_t KEFIR_PARSER_RULE_FN_PREFIX(expression_statement)(struct kefir_mem *mem, struct kefir_parser *parser,
                                                                 struct kefir_ast_node_base **result, void *payload) {
    APPLY_PROLOGUE(mem, parser, result, payload);
    struct kefir_ast_node_base *expression = NULL;
    kefir_result_t res = KEFIR_OK;

    struct kefir_ast_node_attributes attributes;
    REQUIRE_OK(kefir_ast_node_attributes_init(&attributes));
    SCAN_ATTRIBUTES(&res, mem, parser, &attributes);

    if (res == KEFIR_OK && !PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_SEMICOLON)) {
        REQUIRE_CHAIN(&res, KEFIR_PARSER_NEXT_EXPRESSION(mem, parser, &expression));
    } else if (res == KEFIR_OK && kefir_list_length(&attributes.attributes) > 0) {
        res = KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match expression statement");
    }

    REQUIRE_CHAIN_SET(
        &res, PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_SEMICOLON),
        KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0), "Expected semicolon"));
    REQUIRE_CHAIN(&res, PARSER_SHIFT(parser));

    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_node_attributes_free(mem, &attributes);
        if (expression != NULL) {
            KEFIR_AST_NODE_FREE(mem, expression);
        }
        return res;
    });
    struct kefir_ast_expression_statement *stmt = kefir_ast_new_expression_statement(mem, expression);
    REQUIRE_ELSE(stmt != NULL, {
        kefir_ast_node_attributes_free(mem, &attributes);
        if (expression != NULL) {
            KEFIR_AST_NODE_FREE(mem, expression);
        }
        return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST expression statement");
    });

    res = kefir_ast_node_attributes_move(&stmt->attributes, &attributes);
    REQUIRE_ELSE(stmt != NULL, {
        kefir_ast_node_attributes_free(mem, &attributes);
        KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(stmt));
        return res;
    });
    res = kefir_ast_node_attributes_free(mem, &attributes);
    REQUIRE_ELSE(stmt != NULL, {
        KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(stmt));
        return res;
    });
    *result = KEFIR_AST_NODE_BASE(stmt);
    return KEFIR_OK;
}
