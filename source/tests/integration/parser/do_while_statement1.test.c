/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_DO, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_WHILE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_constant_uint(1, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_DO, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "arr", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "i", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_ASSIGN, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_constant_uint(1, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_DOUBLE_MINUS, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "i", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_WHILE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "i", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_GREATER_THAN, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_constant_uint(0, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_DO, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "i", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_DOUBLE_MINUS, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));
    do {
        REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_DO, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACE, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "j", &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_DOUBLE_MINUS, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));
        do {
            REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_DO, &TOKENS[counter++]));
            REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACE, &TOKENS[counter++]));
            REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "k", &TOKENS[counter++]));
            REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_DOUBLE_MINUS, &TOKENS[counter++]));
            REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));
            REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACE, &TOKENS[counter++]));
            REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_WHILE, &TOKENS[counter++]));
            REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
            REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "k", &TOKENS[counter++]));
            REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));
            REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));
        } while (0);
        REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACE, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_WHILE, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "j", &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));
        REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));
    } while (0);
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_WHILE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "i", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));

    REQUIRE_OK(kefir_parser_token_cursor_init(&cursor, TOKENS, counter));
    REQUIRE_OK(kefir_parser_init(mem, &parser, &symbols, &cursor, NULL));

    struct kefir_json_output json;
    REQUIRE_OK(kefir_json_output_init(&json, stdout, 4));

    REQUIRE_OK(kefir_json_output_array_begin(&json));

    while (kefir_parser_token_cursor_at(&cursor, 0)->klass != KEFIR_TOKEN_SENTINEL) {
        struct kefir_ast_node_base *node = NULL;
        REQUIRE_OK(KEFIR_PARSER_NEXT_STATEMENT(mem, &parser, &node));
        REQUIRE_OK(kefir_ast_format(&json, node, false));
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node));
    }

    REQUIRE_OK(kefir_json_output_array_end(&json));
    REQUIRE_OK(kefir_json_output_finalize(&json));

    REQUIRE_OK(kefir_parser_free(mem, &parser));
    for (kefir_size_t i = 0; i < counter; i++) {
        REQUIRE_OK(kefir_token_free(mem, &TOKENS[i]));
    }
    REQUIRE_OK(kefir_string_pool_free(mem, &symbols));
    return KEFIR_OK;
}
