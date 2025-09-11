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
#include "kefir/parser/builder.h"
#include "kefir/core/source_error.h"

static kefir_result_t parse_labelled_stmt(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder,
                                          struct kefir_ast_node_attributes *attributes) {
    struct kefir_parser *parser = builder->parser;

    kefir_result_t res;
    if (PARSER_TOKEN_IS_IDENTIFIER(parser, 0) && PARSER_TOKEN_IS_PUNCTUATOR(parser, 1, KEFIR_PUNCTUATOR_COLON)) {
        const char *identifier = PARSER_CURSOR(parser, 0)->identifier;
        REQUIRE_OK(PARSER_SHIFT(parser));
        REQUIRE_OK(PARSER_SHIFT(parser));

        kefir_result_t res = KEFIR_OK;
        SKIP_ATTRIBUTES(&res, mem, parser);
        REQUIRE_OK(res);

        kefir_bool_t empty_statement = false;
        res = kefir_parser_ast_builder_scan(mem, builder, KEFIR_PARSER_RULE_FN(parser, declaration), NULL);
        if (res == KEFIR_NO_MATCH) {
            res = kefir_parser_ast_builder_scan(mem, builder, KEFIR_PARSER_RULE_FN(parser, statement), NULL);
        }
        if (res == KEFIR_NO_MATCH) {
            REQUIRE(PARSER_TOKEN_IS_RIGHT_BRACE(parser, 0),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0),
                                           "Expected a declaration, statement or right brace"));
            res = KEFIR_OK;
            empty_statement = true;
        }
        REQUIRE_OK(res);

        if (empty_statement) {
            REQUIRE_OK(kefir_parser_ast_builder_empty_labeled_statement(mem, builder, identifier, attributes));
        } else {
            REQUIRE_OK(kefir_parser_ast_builder_labeled_statement(mem, builder, identifier, attributes));
        }
    } else if (PARSER_TOKEN_IS_KEYWORD(parser, 0, KEFIR_KEYWORD_CASE)) {
        REQUIRE_OK(PARSER_SHIFT(parser));
        REQUIRE_MATCH_OK(
            &res, kefir_parser_ast_builder_scan(mem, builder, KEFIR_PARSER_RULE_FN(parser, constant_expression), NULL),
            KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0),
                                   "Expected constant expression"));
        kefir_bool_t range_expr = false;
        if (PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_ELLIPSIS) &&
            parser->configuration->switch_case_ranges) {
            REQUIRE_OK(PARSER_SHIFT(parser));
            REQUIRE_MATCH_OK(
                &res,
                kefir_parser_ast_builder_scan(mem, builder, KEFIR_PARSER_RULE_FN(parser, constant_expression), NULL),
                KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0),
                                       "Expected constant expression"));
            range_expr = true;
        }
        REQUIRE(PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_COLON),
                KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0), "Expected colon"));
        REQUIRE_OK(PARSER_SHIFT(parser));
        kefir_bool_t empty_statement = false;
        res = kefir_parser_ast_builder_scan(mem, builder, KEFIR_PARSER_RULE_FN(parser, declaration), NULL);
        if (res == KEFIR_NO_MATCH) {
            res = kefir_parser_ast_builder_scan(mem, builder, KEFIR_PARSER_RULE_FN(parser, statement), NULL);
        }
        if (res == KEFIR_NO_MATCH) {
            REQUIRE(PARSER_TOKEN_IS_RIGHT_BRACE(parser, 0),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0),
                                           "Expected a declaration, statement or right brace"));
            res = KEFIR_OK;
            empty_statement = true;
        }
        REQUIRE_OK(res);
        if (empty_statement) {
            if (range_expr) {
                REQUIRE_OK(kefir_parser_ast_builder_empty_range_case_statement(mem, builder, attributes));
            } else {
                REQUIRE_OK(kefir_parser_ast_builder_empty_case_statement(mem, builder, attributes));
            }
        } else {
            if (range_expr) {
                REQUIRE_OK(kefir_parser_ast_builder_range_case_statement(mem, builder, attributes));
            } else {
                REQUIRE_OK(kefir_parser_ast_builder_case_statement(mem, builder, attributes));
            }
        }
    } else if (PARSER_TOKEN_IS_KEYWORD(parser, 0, KEFIR_KEYWORD_DEFAULT)) {
        REQUIRE_OK(PARSER_SHIFT(parser));
        REQUIRE(PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_COLON),
                KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0), "Expected colon"));
        REQUIRE_OK(PARSER_SHIFT(parser));
        kefir_result_t res;
        kefir_bool_t empty_statement = false;
        res = kefir_parser_ast_builder_scan(mem, builder, KEFIR_PARSER_RULE_FN(parser, declaration), NULL);
        if (res == KEFIR_NO_MATCH) {
            res = kefir_parser_ast_builder_scan(mem, builder, KEFIR_PARSER_RULE_FN(parser, statement), NULL);
        }
        if (res == KEFIR_NO_MATCH) {
            REQUIRE(PARSER_TOKEN_IS_RIGHT_BRACE(parser, 0),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0),
                                           "Expected a declaration, statement or right brace"));
            res = KEFIR_OK;
            empty_statement = true;
        }
        REQUIRE_OK(res);

        if (empty_statement) {
            REQUIRE_OK(kefir_parser_ast_builder_empty_default_statement(mem, builder, attributes));
        } else {
            REQUIRE_OK(kefir_parser_ast_builder_default_statement(mem, builder, attributes));
        }
    } else {
        return KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match labeled statement");
    }

    return KEFIR_OK;
}

static kefir_result_t builder_callback(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder, void *payload) {
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser AST builder"));
    struct kefir_parser *parser = builder->parser;

    struct kefir_ast_node_attributes attributes;
    REQUIRE_OK(kefir_ast_node_attributes_init(&attributes));

    kefir_result_t res = KEFIR_OK;
    SCAN_ATTRIBUTES(&res, mem, parser, &attributes);
    REQUIRE_CHAIN(&res, parse_labelled_stmt(mem, builder, &attributes));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_node_attributes_free(mem, &attributes);
        return res;
    });
    REQUIRE_OK(kefir_ast_node_attributes_free(mem, &attributes));

    return KEFIR_OK;
}

kefir_result_t KEFIR_PARSER_RULE_FN_PREFIX(labeled_statement)(struct kefir_mem *mem, struct kefir_parser *parser,
                                                              struct kefir_ast_node_base **result, void *payload) {
    APPLY_PROLOGUE(mem, parser, result, payload);
    REQUIRE_OK(kefir_parser_ast_builder_wrap(mem, parser, result, builder_callback, NULL));
    return KEFIR_OK;
}
