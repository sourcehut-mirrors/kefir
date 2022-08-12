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

#include "kefir/parser/rule_helpers.h"
#include "kefir/parser/builder.h"
#include "kefir/core/source_error.h"

static kefir_result_t scan_qualifiers(struct kefir_parser *parser,
                                      struct kefir_ast_inline_assembly_qualifiers *qualifiers) {
    kefir_bool_t scan = true;
    while (scan) {
        if (PARSER_TOKEN_IS_KEYWORD(parser, 0, KEFIR_KEYWORD_VOLATILE)) {
            PARSER_SHIFT(parser);
            qualifiers->volatile_qualifier = true;
        } else if (PARSER_TOKEN_IS_KEYWORD(parser, 0, KEFIR_KEYWORD_INLINE)) {
            PARSER_SHIFT(parser);
            qualifiers->inline_qualifier = true;
        } else if (PARSER_TOKEN_IS_KEYWORD(parser, 0, KEFIR_KEYWORD_GOTO)) {
            PARSER_SHIFT(parser);
            qualifiers->goto_qualifier = true;
        } else {
            scan = false;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t scan_assembly_template(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder,
                                             const struct kefir_ast_inline_assembly_qualifiers *qualifiers) {
    REQUIRE(PARSER_TOKEN_IS_STRING_LITERAL(builder->parser, 0),
            KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                   "Expected assembler template"));
    const struct kefir_token *token = PARSER_CURSOR(builder->parser, 0);
    REQUIRE(!token->string_literal.raw_literal,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected raw string literal in parsing phase"));
    switch (token->string_literal.type) {
        case KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE:
        case KEFIR_STRING_LITERAL_TOKEN_UNICODE8:
            REQUIRE_OK(
                kefir_parser_ast_builder_inline_assembly(mem, builder, *qualifiers, token->string_literal.literal));
            break;

        case KEFIR_STRING_LITERAL_TOKEN_UNICODE16:
        case KEFIR_STRING_LITERAL_TOKEN_UNICODE32:
        case KEFIR_STRING_LITERAL_TOKEN_WIDE:
            return KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                          "Non-multibyte inline assembly template strings are not supported yet");
    }
    PARSER_SHIFT(builder->parser);
    return KEFIR_OK;
}

static kefir_result_t scan_operand(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder,
                                   kefir_bool_t output_param) {
    const char *parameter_name = NULL;
    const char *constraint = NULL;

    if (PARSER_TOKEN_IS_LEFT_BRACKET(builder->parser, 0)) {
        PARSER_SHIFT(builder->parser);
        REQUIRE(PARSER_TOKEN_IS_IDENTIFIER(builder->parser, 0),
                KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                       "Expected output symbolic name identifier"));
        parameter_name = PARSER_CURSOR(builder->parser, 0)->identifier;
        PARSER_SHIFT(builder->parser);
        REQUIRE(PARSER_TOKEN_IS_RIGHT_BRACKET(builder->parser, 0),
                KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                       "Expected right bracker"));
        PARSER_SHIFT(builder->parser);
    }

    REQUIRE(PARSER_TOKEN_IS_STRING_LITERAL(builder->parser, 0),
            KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                   "Expected output constraint string literal"));
    const struct kefir_token *token = PARSER_CURSOR(builder->parser, 0);
    REQUIRE(!token->string_literal.raw_literal,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected raw string literal in parsing phase"));
    switch (token->string_literal.type) {
        case KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE:
        case KEFIR_STRING_LITERAL_TOKEN_UNICODE8:
            constraint = token->string_literal.literal;
            break;

        case KEFIR_STRING_LITERAL_TOKEN_UNICODE16:
        case KEFIR_STRING_LITERAL_TOKEN_UNICODE32:
        case KEFIR_STRING_LITERAL_TOKEN_WIDE:
            return KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                          "Non-multibyte inline assembly parameter constraints are not supported yet");
    }
    PARSER_SHIFT(builder->parser);

    REQUIRE(PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_LEFT_PARENTHESE),
            KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                   "Expected left parenthese"));
    PARSER_SHIFT(builder->parser);
    REQUIRE_OK(kefir_parser_ast_builder_scan(mem, builder, KEFIR_PARSER_RULE_FN(builder->parser, assignment_expression),
                                             NULL));
    REQUIRE(PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_RIGHT_PARENTHESE),
            KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                   "Expected right parenthese"));
    PARSER_SHIFT(builder->parser);

    if (output_param) {
        REQUIRE_OK(kefir_parser_ast_builder_inline_assembly_add_output(mem, builder, parameter_name, constraint));
    } else {
        REQUIRE_OK(kefir_parser_ast_builder_inline_assembly_add_input(mem, builder, parameter_name, constraint));
    }
    return KEFIR_OK;
}

static kefir_result_t scan_goto(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder) {
    kefir_bool_t scan = true;
    while (scan) {
        if (PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_RIGHT_PARENTHESE)) {
            scan = false;
        } else {
            REQUIRE(PARSER_TOKEN_IS_IDENTIFIER(builder->parser, 0),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                           "Expected goto identifier"));
            REQUIRE_OK(kefir_parser_ast_builder_inline_assembly_add_jump_target(
                mem, builder, PARSER_CURSOR(builder->parser, 0)->identifier));
            PARSER_SHIFT(builder->parser);
            if (PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_COMMA)) {
                PARSER_SHIFT(builder->parser);
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t scan_clobbers(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder) {
    kefir_bool_t scan = true;
    while (scan) {
        if (PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_COLON)) {
            PARSER_SHIFT(builder->parser);
            REQUIRE_OK(scan_goto(mem, builder));
            scan = false;
        } else if (PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_RIGHT_PARENTHESE)) {
            scan = false;
        } else {
            REQUIRE(PARSER_TOKEN_IS_STRING_LITERAL(builder->parser, 0),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                           "Expected clobber string literal"));
            const struct kefir_token *token = PARSER_CURSOR(builder->parser, 0);
            REQUIRE(!token->string_literal.raw_literal,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected raw string literal in parsing phase"));
            switch (token->string_literal.type) {
                case KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE:
                case KEFIR_STRING_LITERAL_TOKEN_UNICODE8:
                    REQUIRE_OK(kefir_parser_ast_builder_inline_assembly_add_clobber(mem, builder,
                                                                                    token->string_literal.literal));
                    break;

                case KEFIR_STRING_LITERAL_TOKEN_UNICODE16:
                case KEFIR_STRING_LITERAL_TOKEN_UNICODE32:
                case KEFIR_STRING_LITERAL_TOKEN_WIDE:
                    return KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                                  "Non-multibyte inline assembly clobbers are not supported yet");
            }
            PARSER_SHIFT(builder->parser);
            if (PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_COMMA)) {
                PARSER_SHIFT(builder->parser);
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t scan_input_operands(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder) {
    kefir_bool_t scan = true;
    while (scan) {
        if (PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_COLON)) {
            PARSER_SHIFT(builder->parser);
            REQUIRE_OK(scan_clobbers(mem, builder));
            scan = false;
        } else if (PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_RIGHT_PARENTHESE)) {
            scan = false;
        } else {
            REQUIRE_OK(scan_operand(mem, builder, false));
            if (PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_COMMA)) {
                PARSER_SHIFT(builder->parser);
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t scan_output_operands(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder) {
    kefir_bool_t scan = true;
    while (scan) {
        if (PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_COLON)) {
            PARSER_SHIFT(builder->parser);
            REQUIRE_OK(scan_input_operands(mem, builder));
            scan = false;
        } else if (PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_RIGHT_PARENTHESE)) {
            scan = false;
        } else {
            REQUIRE_OK(scan_operand(mem, builder, true));
            if (PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_COMMA)) {
                PARSER_SHIFT(builder->parser);
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t builder_callback(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder, void *payload) {
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser AST builder"));

    REQUIRE(PARSER_TOKEN_IS_KEYWORD(builder->parser, 0, KEFIR_KEYWORD_ASM),
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match assembly directive"));

    REQUIRE(!builder->parser->configuration->fail_on_assembly,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                   "Support of inline assembly is disabled"));

    struct kefir_ast_inline_assembly_qualifiers qualifiers = {0};

    PARSER_SHIFT(builder->parser);
    REQUIRE_OK(scan_qualifiers(builder->parser, &qualifiers));
    REQUIRE(PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_LEFT_PARENTHESE),
            KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                   "Expected left parenthese"));
    PARSER_SHIFT(builder->parser);

    REQUIRE_OK(scan_assembly_template(mem, builder, &qualifiers));

    if (PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_COLON)) {
        PARSER_SHIFT(builder->parser);
        REQUIRE_OK(scan_output_operands(mem, builder));
    }

    REQUIRE(PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_RIGHT_PARENTHESE),
            KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0),
                                   "Expected right parenthese"));
    PARSER_SHIFT(builder->parser);
    REQUIRE(
        PARSER_TOKEN_IS_PUNCTUATOR(builder->parser, 0, KEFIR_PUNCTUATOR_SEMICOLON),
        KEFIR_SET_SOURCE_ERROR(KEFIR_SYNTAX_ERROR, PARSER_TOKEN_LOCATION(builder->parser, 0), "Expected semicolon"));
    PARSER_SHIFT(builder->parser);
    return KEFIR_OK;
}

kefir_result_t KEFIR_PARSER_RULE_FN_PREFIX(assembly)(struct kefir_mem *mem, struct kefir_parser *parser,
                                                     struct kefir_ast_node_base **result, void *payload) {
    APPLY_PROLOGUE(mem, parser, result, payload);
    REQUIRE_OK(kefir_parser_ast_builder_wrap(mem, parser, result, builder_callback, NULL));
    return KEFIR_OK;
}
