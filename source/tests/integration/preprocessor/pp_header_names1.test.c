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
#include "kefir/preprocessor/tokenizer.h"
#include "kefir/lexer/format.h"
#include "kefir/ast/format.h"
#include <stdio.h>

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    const char CONTENT[] = "<directory/subdirectory/another_subdirectory/header.h> <1>\n"
                           "     \t\t\"local/directory/with/header_here.h\"       \v\f"
                           "   \"/absolute/path.h\"     <\\Windows\\style.h>";

    struct kefir_string_pool symbols;
    struct kefir_lexer_source_cursor cursor;
    struct kefir_lexer_context parser_context;
    struct kefir_lexer lexer;
    REQUIRE_OK(kefir_string_pool_init(&symbols));
    REQUIRE_OK(kefir_lexer_source_cursor_init(&cursor, CONTENT, sizeof(CONTENT), ""));
    REQUIRE_OK(kefir_lexer_context_default(&parser_context));
    REQUIRE_OK(kefir_lexer_init(mem, &lexer, KEFIR_LEXER_C_MODE, &symbols, &cursor, &parser_context, NULL));

    struct kefir_json_output json;
    REQUIRE_OK(kefir_json_output_init(&json, stdout, 4));
    REQUIRE_OK(kefir_json_output_array_begin(&json));

    while (kefir_lexer_source_cursor_at(&cursor, 0) != U'\0') {
        struct kefir_token token;
        REQUIRE_OK(kefir_lexer_match_pp_header_name(mem, &lexer, &token));
        REQUIRE_OK(kefir_lexer_cursor_match_whitespace(mem, &lexer, NULL));
        REQUIRE_OK(kefir_token_format(mem, &json, &token, false));
        REQUIRE_OK(kefir_token_free(mem, &token));
    }

    REQUIRE_OK(kefir_json_output_array_end(&json));
    REQUIRE_OK(kefir_json_output_finalize(&json));

    REQUIRE_OK(kefir_lexer_free(mem, &lexer));
    REQUIRE_OK(kefir_string_pool_free(mem, &symbols));
    return KEFIR_OK;
}
