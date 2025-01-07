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

#include "kefir/core/mem.h"
#include "kefir/parser/parser.h"
#include "kefir/ast/format.h"
#include <stdio.h>

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    struct kefir_string_pool symbols;
    struct kefir_token TOKENS[1024];
    struct kefir_parser_token_cursor cursor;
    struct kefir_parser parser;

    REQUIRE_OK(kefir_string_pool_init(&symbols));

    kefir_size_t counter = 0;

    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_VOID, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "Fn1", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACE, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_STRUCT, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "structure1", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_SHORT, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_INT, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "field1", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_INT, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "field2", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_COLON, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_constant_int(4, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_TYPEDEF, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_STRUCT, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "structure1", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "structure1_t", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_LONG, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_UNSIGNED, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_STAR, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "Fn2", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_INT, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "x", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_COMMA, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_FLOAT, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "y", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_COMMA, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_CHAR, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "z", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACE, &TOKENS[counter++]));
    do {
        REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_LONG, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_UNSIGNED, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "base", &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_ASSIGN, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_LONG, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "y", &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));
    } while (0);
    do {
        REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_RETURN, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "base", &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_SHIFT, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "z", &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACKET, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "x", &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_PLUS, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_constant_int(1, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACKET, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));
    } while (0);
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACE, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_EXTERN, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_VOID, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_STAR, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "Ptr1", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_ASSIGN, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_constant_int(0, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_STATIC, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_DOUBLE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "callback1", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_STRUCT, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "structure1", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_VOID, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "SomeFn", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "x", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_COMMA, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "y", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_COMMA, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "z", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_VOID, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "SomeFn", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "x", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_COMMA, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "y", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_COMMA, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "z", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_INT, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_STAR, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_STAR, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "x", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_INT, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "y", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_COMMA, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "z", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_STAR, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "x", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "y", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_ASSIGN_ADD, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "z", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACE, &TOKENS[counter++]));

    REQUIRE_OK(kefir_parser_token_cursor_init_direct(&cursor, TOKENS, counter));
    REQUIRE_OK(kefir_parser_init(mem, &parser, &symbols, &cursor, NULL));

    struct kefir_json_output json;
    REQUIRE_OK(kefir_json_output_init(&json, stdout, 4));

    struct kefir_ast_node_base *node = NULL;
    REQUIRE_OK(KEFIR_PARSER_NEXT_TRANSLATION_UNIT(mem, &parser, &node));
    REQUIRE_OK(kefir_ast_format(&json, node, false));
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node));

    REQUIRE_OK(kefir_json_output_finalize(&json));

    REQUIRE_OK(kefir_parser_free(mem, &parser));
    for (kefir_size_t i = 0; i < counter; i++) {
        REQUIRE_OK(kefir_token_free(mem, &TOKENS[i]));
    }
    REQUIRE_OK(kefir_string_pool_free(mem, &symbols));
    return KEFIR_OK;
}
