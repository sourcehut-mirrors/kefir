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
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "arr", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "idx", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_BRACKET, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_SHIFT, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "value1", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_PLUS, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_constant_int(10, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_SHIFT, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "value2", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_SHIFT, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_constant_int(1, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_STAR, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_identifier(mem, &symbols, "callback", &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_LEFT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_SHIFT, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_constant_int(2, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_punctuator(KEFIR_PUNCTUATOR_PLUS, &TOKENS[counter++]));
    REQUIRE_OK(kefir_token_new_constant_int(11, &TOKENS[counter++]));

    REQUIRE_OK(kefir_parser_token_cursor_init(&cursor, TOKENS, counter));
    REQUIRE_OK(kefir_parser_init(mem, &parser, &symbols, &cursor, NULL));

    struct kefir_json_output json;
    REQUIRE_OK(kefir_json_output_init(&json, stdout, 4));

    struct kefir_ast_node_base *node = NULL;
    REQUIRE_OK(KEFIR_PARSER_NEXT_EXPRESSION(mem, &parser, &node));
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
