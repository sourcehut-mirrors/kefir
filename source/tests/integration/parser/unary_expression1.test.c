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
    struct kefir_symbol_table symbols;
    struct kefir_token TOKENS[1024];
    struct kefir_parser_token_cursor cursor;
    struct kefir_parser parser;

    REQUIRE_OK(kefir_symbol_table_init(&symbols));

    kefir_size_t counter = 0;
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_MINUS, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_TILDE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_DOUBLE_PLUS, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "somevar", &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_EXCLAMATION_MARK, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_PLUS, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_STAR, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_AMPERSAND, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "someothervar", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_SIZEOF, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_DOUBLE_MINUS, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "somevar", &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_SIZEOF, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "someobj", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_DOT, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "field1", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_ARROW, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "field2", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_SIZEOF, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_LONG, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_constant_long(8, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_ALIGNOF, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_CHAR, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_STAR, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_INT, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "arr", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_TILDE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "fn1", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_MINUS, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_constant_long(1, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_COMMA, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_DOUBLE_PLUS, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "var1", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_SIZEOF, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_DOUBLE_AMPERSAND, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "somelabel", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));

    REQUIRE_OK(kefir_parser_token_cursor_init(&cursor, TOKENS, counter));
    REQUIRE_OK(kefir_parser_init(mem, &parser, &symbols, &cursor, NULL));

    struct kefir_parser_configuration parser_config = {.label_addressing = true};
    parser.configuration = &parser_config;

    struct kefir_json_output json;
    REQUIRE_OK(kefir_json_output_init(&json, stdout, 4));
    REQUIRE_OK(kefir_json_output_array_begin(&json));

    struct kefir_ast_node_base *node = NULL;
    while (kefir_parser_token_cursor_at(&cursor, 0)->klass != KEFIR_TOKEN_SENTINEL) {
        REQUIRE_OK(KEFIR_PARSER_NEXT_EXPRESSION(mem, &parser, &node));
        REQUIRE_OK(kefir_ast_format(&json, node, false));
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node));
    }

    REQUIRE_OK(kefir_json_output_array_end(&json));
    REQUIRE_OK(kefir_json_output_finalize(&json));

    REQUIRE_OK(kefir_parser_free(mem, &parser));
    for (kefir_size_t i = 0; i < counter; i++) {
        REQUIRE_OK(kefir_token_free(mem, &TOKENS[i]));
    }
    REQUIRE_OK(kefir_symbol_table_free(mem, &symbols));
    return KEFIR_OK;
}
