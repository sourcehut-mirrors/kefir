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
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "array", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_VOLATILE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_CONST, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "array2", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "another_array", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "A", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_STAR, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_constant_int(2, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_PLUS, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "B", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "another_array2", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_VOLATILE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_RESTRICT, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_MINUS, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "A", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_STAR, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "D", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_STAR, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_constant_int(3, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "vla_array", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_STAR, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "vla_array2", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_VOLATILE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_STAR, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "static_array", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_STATIC, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "LENGTH1", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));

    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "static_array2", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_CONST, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_RESTRICT, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_keyword(KEFIR_KEYWORD_STATIC, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "LENGTH2", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_SEMICOLON, &TOKENS[counter++]));

    REQUIRE_OK(kefir_parser_token_cursor_init_direct(&cursor, TOKENS, counter));
    REQUIRE_OK(kefir_parser_init(mem, &parser, &symbols, &cursor, NULL));

    struct kefir_json_output json;
    REQUIRE_OK(kefir_json_output_init(&json, stdout, 4));

    REQUIRE_OK(kefir_json_output_array_begin(&json));

    while (kefir_parser_token_cursor_at(&cursor, 0, true)->klass != KEFIR_TOKEN_SENTINEL) {
        struct kefir_ast_declarator *declarator = NULL;
        REQUIRE_OK(parser.ruleset.declarator(mem, &parser, &declarator));
        REQUIRE(kefir_parser_token_cursor_at(&cursor, 0, true)->klass == KEFIR_TOKEN_PUNCTUATOR, KEFIR_INTERNAL_ERROR);
        REQUIRE(kefir_parser_token_cursor_at(&cursor, 0, true)->punctuator == KEFIR_PUNCTUATOR_SEMICOLON,
                KEFIR_INTERNAL_ERROR);
        REQUIRE_OK(kefir_parser_token_cursor_next(&cursor, true));
        REQUIRE_OK(kefir_ast_format_declarator(&json, declarator, false));
        REQUIRE_OK(kefir_ast_declarator_free(mem, declarator));
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
