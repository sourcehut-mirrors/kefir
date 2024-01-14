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

#include "kefir/core/extensions.h"
#include "kefir/preprocessor/tokenizer.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

static kefir_result_t preprocessor_next_impl(struct kefir_mem *mem, struct kefir_lexer *lexer, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(lexer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));

    ASSIGN_DECL_CAST(struct kefir_token *, token, payload);
    struct kefir_source_location source_location = lexer->cursor->location;

    kefir_result_t res = KEFIR_NO_MATCH;
    if (lexer->extensions != NULL && lexer->extensions->before_token_lex != NULL) {
        KEFIR_RUN_EXTENSION(&res, mem, lexer, before_token_lex, token);
    }
    if (res == KEFIR_YIELD) {
        return KEFIR_OK;
    } else if (res != KEFIR_NO_MATCH) {
        REQUIRE_OK(res);
    }

    res = kefir_lexer_cursor_match_whitespace(mem, lexer, token);
    if (res == KEFIR_NO_MATCH && kefir_lexer_source_cursor_at(lexer->cursor, 0) == U'\0') {
        res = kefir_token_new_sentinel(token);
    }
    if (res == KEFIR_NO_MATCH) {
        res = kefir_lexer_match_character_constant(mem, lexer, token);
    }
    if (res == KEFIR_NO_MATCH) {
        res = kefir_lexer_match_pp_number(mem, lexer, token);
    }
    if (res == KEFIR_NO_MATCH) {
        res = kefir_lexer_match_string_literal(mem, lexer, token, false);
    }
    if (res == KEFIR_NO_MATCH) {
        res = kefir_lexer_match_identifier(mem, lexer, token);
    }
    if (res == KEFIR_NO_MATCH) {
        res = kefir_lexer_match_punctuator(mem, lexer, token);
    }

    if (res == KEFIR_NO_MATCH && lexer->extensions != NULL && lexer->extensions->failed_token_lex != NULL) {
        KEFIR_RUN_EXTENSION(&res, mem, lexer, failed_token_lex, token);
    }

    if (res == KEFIR_NO_MATCH) {
        return KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &source_location,
                                      "Expected constant, string literal, identifier, keyword or punctuator");
    } else {
        REQUIRE_OK(res);
    }

    token->source_location = source_location;

    KEFIR_RUN_EXTENSION(&res, mem, lexer, after_token_lex, token);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_token_free(mem, token);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_tokenize_next(struct kefir_mem *mem, struct kefir_lexer *lexer,
                                                struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(lexer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));

    REQUIRE_OK(kefir_lexer_apply(mem, lexer, preprocessor_next_impl, token));
    return KEFIR_OK;
}
