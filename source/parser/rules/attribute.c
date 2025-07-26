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
#include "kefir/lexer/buffer.h"
#include "kefir/core/source_error.h"

static kefir_result_t consume_gnu_attribute_parameters(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder,
                                                       struct kefir_parser *parser) {
    kefir_bool_t consume_attribute_parameters = true;
    while (consume_attribute_parameters) {
        if (PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_RIGHT_PARENTHESE)) {
            consume_attribute_parameters = false;
        } else {
            kefir_result_t res;
            REQUIRE_MATCH_OK(
                &res,
                kefir_parser_ast_builder_scan(mem, builder, KEFIR_PARSER_RULE_FN(parser, assignment_expression), NULL),
                KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0),
                                       "Expected assignment expression"));
            REQUIRE_OK(kefir_parser_ast_builder_attribute_parameter(mem, builder));

            if (!PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_RIGHT_PARENTHESE)) {
                REQUIRE(PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_COMMA),
                        KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0),
                                               "Expected either comma, or right parenthese"));
                REQUIRE_OK(PARSER_SHIFT(parser));
            }
        }
    }
    REQUIRE_OK(PARSER_SHIFT(parser));

    return KEFIR_OK;
}

static kefir_result_t consume_nongnu_attribute_parameters(struct kefir_mem *mem,
                                                          struct kefir_parser_ast_builder *builder,
                                                          struct kefir_parser *parser) {
    UNUSED(mem);
    UNUSED(builder);
    kefir_size_t depth = 1;
    kefir_result_t res = KEFIR_OK;
    while (depth > 0 && res == KEFIR_OK) {
        if (PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_RIGHT_PARENTHESE)) {
            depth--;
        } else if (PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_LEFT_PARENTHESE)) {
            depth++;
        }

        if (depth > 0) {
            REQUIRE_CHAIN(&res, kefir_parser_ast_builder_attribute_unstructured_parameter(mem, builder,
                                                                                          PARSER_CURSOR(parser, 0)));
        }

        REQUIRE_CHAIN(&res, PARSER_SHIFT(parser));
    }

    return KEFIR_OK;
}

static kefir_result_t get_identifier(struct kefir_parser *parser, const char **identifier) {
    if (PARSER_TOKEN_IS_IDENTIFIER(parser, 0)) {
        *identifier = PARSER_CURSOR(parser, 0)->identifier;
    } else if (PARSER_TOKEN_IS(parser, 0, KEFIR_TOKEN_KEYWORD)) {
        REQUIRE(PARSER_CURSOR(parser, 0)->keyword_spelling != NULL,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected keyword spelling to be available"));
        *identifier = PARSER_CURSOR(parser, 0)->keyword_spelling;
    } else {
        return KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0),
                                      "Expected an identifier or a keyword");
    }
    return KEFIR_OK;
}

static kefir_result_t consume_attribute(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder,
                                        struct kefir_parser *parser) {
    const char *prefix = NULL;
    const char *name = NULL;
    REQUIRE_OK(get_identifier(parser, &name));

    REQUIRE_OK(PARSER_SHIFT(parser));

    if (PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_COLON)) {
        REQUIRE(PARSER_TOKEN_IS_PUNCTUATOR(parser, 1, KEFIR_PUNCTUATOR_COLON),
                KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 1), "Expected colon"));
        REQUIRE_OK(PARSER_SHIFT(parser));
        REQUIRE_OK(PARSER_SHIFT(parser));

        REQUIRE(PARSER_TOKEN_IS_IDENTIFIER(parser, 0) || PARSER_TOKEN_IS(parser, 0, KEFIR_TOKEN_KEYWORD),
                KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0),
                                       "Expected an identifier or a keyword"));
        prefix = name;
        REQUIRE_OK(get_identifier(parser, &name));

        REQUIRE_OK(PARSER_SHIFT(parser));
    }

    REQUIRE_OK(kefir_parser_ast_builder_attribute(mem, builder, prefix, name));

    if (PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_LEFT_PARENTHESE)) {
        REQUIRE_OK(PARSER_SHIFT(parser));
        if (prefix != NULL && strcmp(prefix, "gnu") == 0) {
            REQUIRE_OK(consume_gnu_attribute_parameters(mem, builder, parser));
        } else {
            REQUIRE_OK(consume_nongnu_attribute_parameters(mem, builder, parser));
        }
    }

    return KEFIR_OK;
}

static kefir_result_t builder_callback(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder, void *payload) {
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser AST builder"));
    struct kefir_parser *parser = builder->parser;

    if (PARSER_TOKEN_IS_KEYWORD(parser, 0, KEFIR_KEYWORD_ATTRIBUTE)) {
        kefir_result_t res;
        REQUIRE_MATCH_OK(
            &res, kefir_parser_ast_builder_scan(mem, builder, KEFIR_PARSER_RULE_FN(parser, gnu_attribute_list), NULL),
            KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0), "Expected GNU attribute"));
        return KEFIR_OK;
    }

    REQUIRE(PARSER_TOKEN_IS_LEFT_BRACKET(parser, 0) && PARSER_TOKEN_IS_LEFT_BRACKET(parser, 1),
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match an attribute"));

    REQUIRE_OK(PARSER_SHIFT(parser));
    REQUIRE_OK(PARSER_SHIFT(parser));

    REQUIRE_OK(kefir_parser_ast_builder_attribute_list(mem, builder));

    while (PARSER_TOKEN_IS_IDENTIFIER(parser, 0) || PARSER_TOKEN_IS(parser, 0, KEFIR_TOKEN_KEYWORD)) {
        REQUIRE_OK(consume_attribute(mem, builder, parser));

        if (PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_COMMA)) {
            REQUIRE_OK(PARSER_SHIFT(parser));
        } else {
            REQUIRE(PARSER_TOKEN_IS_RIGHT_BRACKET(parser, 0),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0),
                                           "Expected comma or right bracket"));
        }
    }

    REQUIRE(PARSER_TOKEN_IS_RIGHT_BRACKET(parser, 0) && PARSER_TOKEN_IS_RIGHT_BRACKET(parser, 1),
            KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0), "Expected right bracket"));

    REQUIRE_OK(PARSER_SHIFT(parser));
    REQUIRE_OK(PARSER_SHIFT(parser));

    return KEFIR_OK;
}

kefir_result_t KEFIR_PARSER_RULE_FN_PREFIX(attribute_list)(struct kefir_mem *mem, struct kefir_parser *parser,
                                                           struct kefir_ast_node_base **result, void *payload) {
    APPLY_PROLOGUE(mem, parser, result, payload);
    REQUIRE_OK(kefir_parser_ast_builder_wrap(mem, parser, result, builder_callback, NULL));
    return KEFIR_OK;
}
