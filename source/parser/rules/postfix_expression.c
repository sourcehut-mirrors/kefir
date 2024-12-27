/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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
#include "kefir/parser/builtins.h"

static kefir_result_t scan_subscript(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder) {
    REQUIRE_OK(PARSER_SHIFT(builder->parser));
    kefir_result_t res;
    REQUIRE_MATCH_OK(
        &res, kefir_parser_ast_builder_scan(mem, builder, KEFIR_PARSER_RULE_FN(builder->parser, expression), NULL),
        KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0), "Expected expression"));
    REQUIRE(PARSER_TOKEN_IS_RIGHT_BRACKET(builder->parser, 0),
            KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                   "Expected right bracket"));
    REQUIRE_OK(PARSER_SHIFT(builder->parser));
    return KEFIR_OK;
}

static kefir_result_t scan_argument_list(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder) {
    REQUIRE_OK(PARSER_SHIFT(builder->parser));
    while (!PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_RIGHT_PARENTHESE)) {
        kefir_result_t res;
        REQUIRE_MATCH_OK(&res,
                         kefir_parser_ast_builder_scan(
                             mem, builder, KEFIR_PARSER_RULE_FN(builder->parser, assignment_expression), NULL),
                         KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                                "Expected assignment expresion"));
        REQUIRE_OK(kefir_parser_ast_builder_function_call_append(mem, builder));

        if (PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_COMMA)) {
            REQUIRE_OK(PARSER_SHIFT(builder->parser));
        } else {
            REQUIRE(PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_RIGHT_PARENTHESE),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                           "Expected either comma, or right parenthese"));
        }
    }
    REQUIRE_OK(PARSER_SHIFT(builder->parser));
    return KEFIR_OK;
}

static kefir_result_t scan_member(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder) {
    kefir_bool_t direct = PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_DOT);
    REQUIRE_OK(PARSER_SHIFT(builder->parser));
    REQUIRE(
        PARSER_TOKEN_IS_IDENTIFIER(builder->parser, 0),
        KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0), "Expected identifier"));
    const struct kefir_token *token = PARSER_CURSOR(builder->parser, 0);
    REQUIRE_OK(PARSER_SHIFT(builder->parser));
    REQUIRE_OK(kefir_parser_ast_builder_struct_member(mem, builder, direct, token->identifier));
    return KEFIR_OK;
}

static kefir_result_t scan_postfixes(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder) {
    struct kefir_parser *parser = builder->parser;
    kefir_bool_t scan_postfix = true;

    do {
        struct kefir_source_location location = *PARSER_TOKEN_LOCATION(builder->parser, 0);
        if (PARSER_TOKEN_IS_LEFT_BRACKET(parser, 0)) {
            REQUIRE_OK(scan_subscript(mem, builder));
            REQUIRE_OK(kefir_parser_ast_builder_array_subscript(mem, builder));
            REQUIRE_OK(kefir_parser_ast_builder_set_source_location(mem, builder, &location));
        } else if (PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_LEFT_PARENTHESE)) {
            REQUIRE_OK(kefir_parser_ast_builder_function_call(mem, builder));
            REQUIRE_OK(scan_argument_list(mem, builder));
            REQUIRE_OK(kefir_parser_ast_builder_set_source_location(mem, builder, &location));
        } else if (PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_DOT) ||
                   PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_RIGHT_ARROW)) {
            REQUIRE_OK(scan_member(mem, builder));
            REQUIRE_OK(kefir_parser_ast_builder_set_source_location(mem, builder, &location));
        } else if (PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_DOUBLE_PLUS)) {
            REQUIRE_OK(PARSER_SHIFT(parser));
            REQUIRE_OK(kefir_parser_ast_builder_unary_operation(mem, builder, KEFIR_AST_OPERATION_POSTFIX_INCREMENT));
            REQUIRE_OK(kefir_parser_ast_builder_set_source_location(mem, builder, &location));
        } else if (PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_DOUBLE_MINUS)) {
            REQUIRE_OK(PARSER_SHIFT(parser));
            REQUIRE_OK(kefir_parser_ast_builder_unary_operation(mem, builder, KEFIR_AST_OPERATION_POSTFIX_DECREMENT));
            REQUIRE_OK(kefir_parser_ast_builder_set_source_location(mem, builder, &location));
        } else {
            scan_postfix = false;
        }
    } while (scan_postfix);
    return KEFIR_OK;
}

static kefir_result_t scan_builtin(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder,
                                   kefir_ast_builtin_operator_t builtin_op) {
    struct kefir_parser *parser = builder->parser;
    REQUIRE_OK(kefir_parser_ast_builder_builtin(mem, builder, builtin_op));
    REQUIRE_OK(PARSER_SHIFT(builder->parser));

    REQUIRE(PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_LEFT_PARENTHESE),
            KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(parser, 0), "Expected left parenthese"));
    REQUIRE_OK(PARSER_SHIFT(builder->parser));
    kefir_result_t res;
    switch (builtin_op) {
        case KEFIR_AST_BUILTIN_OFFSETOF:
            REQUIRE_MATCH_OK(
                &res,
                kefir_parser_ast_builder_scan(mem, builder, KEFIR_PARSER_RULE_FN(builder->parser, type_name), NULL),
                KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                       "Expected type name"));
            REQUIRE_OK(kefir_parser_ast_builder_builtin_append(mem, builder));
            REQUIRE(PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_COMMA),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                           "Expected comma"));
            REQUIRE_OK(PARSER_SHIFT(builder->parser));
            REQUIRE(PARSER_TOKEN_IS_IDENTIFIER(parser, 0),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                           "Expected identifier"));
            struct kefir_ast_identifier *identifier =
                kefir_ast_new_identifier(mem, parser->symbols, PARSER_CURSOR(parser, 0)->identifier);
            REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate identifier"));
            REQUIRE_OK(kefir_parser_ast_builder_push(mem, builder, KEFIR_AST_NODE_BASE(identifier)));
            REQUIRE_OK(PARSER_SHIFT(builder->parser));
            REQUIRE_OK(scan_postfixes(mem, builder));
            REQUIRE_OK(kefir_parser_ast_builder_builtin_append(mem, builder));
            break;

        case KEFIR_AST_BUILTIN_VA_START:
        case KEFIR_AST_BUILTIN_VA_END:
        case KEFIR_AST_BUILTIN_VA_ARG:
        case KEFIR_AST_BUILTIN_VA_COPY:
        case KEFIR_AST_BUILTIN_ALLOCA:
        case KEFIR_AST_BUILTIN_ALLOCA_WITH_ALIGN:
        case KEFIR_AST_BUILTIN_ALLOCA_WITH_ALIGN_AND_MAX:
        case KEFIR_AST_BUILTIN_TYPES_COMPATIBLE:
        case KEFIR_AST_BUILTIN_CHOOSE_EXPRESSION:
        case KEFIR_AST_BUILTIN_CONSTANT:
        case KEFIR_AST_BUILTIN_CLASSIFY_TYPE:
            while (!PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_RIGHT_PARENTHESE)) {
                res =
                    kefir_parser_ast_builder_scan(mem, builder, KEFIR_PARSER_RULE_FN(builder->parser, type_name), NULL);
                if (res == KEFIR_NO_MATCH) {
                    res = kefir_parser_ast_builder_scan(
                        mem, builder, KEFIR_PARSER_RULE_FN(builder->parser, assignment_expression), NULL);
                }
                if (res == KEFIR_NO_MATCH) {
                    return KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                                  "Expected either assignment expresion or type name");
                } else {
                    REQUIRE_OK(res);
                }
                REQUIRE_OK(kefir_parser_ast_builder_builtin_append(mem, builder));

                if (PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_COMMA)) {
                    REQUIRE_OK(PARSER_SHIFT(builder->parser));
                } else {
                    REQUIRE(PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_RIGHT_PARENTHESE),
                            KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                                   "Expected either comma, or right parenthese"));
                }
            }
            break;

        case KEFIR_AST_BUILTIN_INFINITY_FLOAT64:
        case KEFIR_AST_BUILTIN_INFINITY_FLOAT32:
        case KEFIR_AST_BUILTIN_INFINITY_LONG_DOUBLE:
            // Intentionally left blank
            break;
    }

    REQUIRE(PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_RIGHT_PARENTHESE),
            KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                   "Expected right parenthese"));
    REQUIRE_OK(PARSER_SHIFT(builder->parser));
    return KEFIR_OK;
}

static kefir_result_t builder_callback(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder, void *payload) {
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser AST builder"));

    kefir_result_t res = KEFIR_NOT_FOUND;
    if (PARSER_TOKEN_IS_IDENTIFIER(builder->parser, 0)) {
        kefir_ast_builtin_operator_t builtin_op;
        res = kefir_parser_get_builtin_operation(PARSER_CURSOR(builder->parser, 0)->identifier, &builtin_op);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            REQUIRE_OK(scan_builtin(mem, builder, builtin_op));
            REQUIRE_OK(scan_postfixes(mem, builder));
        }
    }

    if (res == KEFIR_NOT_FOUND) {
        res =
            kefir_parser_ast_builder_scan(mem, builder, KEFIR_PARSER_RULE_FN(builder->parser, compound_literal), NULL);
        if (res == KEFIR_NO_MATCH) {
            res = kefir_parser_ast_builder_scan(mem, builder, KEFIR_PARSER_RULE_FN(builder->parser, primary_expression),
                                                NULL);
        }
        REQUIRE_CHAIN(&res, scan_postfixes(mem, builder));
    }
    REQUIRE_OK(res);
    return KEFIR_OK;
}

kefir_result_t KEFIR_PARSER_RULE_FN_PREFIX(postfix_expression)(struct kefir_mem *mem, struct kefir_parser *parser,
                                                               struct kefir_ast_node_base **result, void *payload) {
    APPLY_PROLOGUE(mem, parser, result, payload);
    REQUIRE_OK(kefir_parser_ast_builder_wrap(mem, parser, result, builder_callback, NULL));
    return KEFIR_OK;
}
