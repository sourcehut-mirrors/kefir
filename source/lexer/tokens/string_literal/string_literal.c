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

#include "kefir/lexer/lexer.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

struct params {
    struct kefir_token *token;
    kefir_bool_t merge_adjacent;
};

static kefir_result_t scan_literals(struct kefir_mem *mem, struct kefir_lexer *lexer, struct kefir_list *literals) {
    kefir_bool_t scan = true;
    kefir_string_literal_token_type_t type = KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE;
    struct kefir_token token;
    do {
        struct kefir_lexer_source_cursor_state cursor_state;
        REQUIRE_OK(kefir_lexer_source_cursor_save(lexer->cursor, &cursor_state));
        kefir_result_t res = kefir_lexer_match_raw_string_literal(mem, lexer, &token);
        if (res == KEFIR_NO_MATCH) {
            REQUIRE(kefir_list_length(literals) > 0, res);
            scan = false;
        } else {
            REQUIRE_OK(res);
            scan = kefir_token_string_literal_type_concat(type, token.string_literal.type, &type);
            if (!scan) {
                REQUIRE_OK(kefir_token_free(mem, &token));
                REQUIRE_OK(kefir_lexer_source_cursor_restore(lexer->cursor, &cursor_state));
                break;
            }

            struct kefir_token *clone = KEFIR_MALLOC(mem, sizeof(struct kefir_token));
            REQUIRE_ELSE(clone != NULL, {
                kefir_token_free(mem, &token);
                return KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate token");
            });

            res = kefir_token_move(clone, &token);
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_FREE(mem, clone);
                kefir_token_free(mem, &token);
                return res;
            });

            res = kefir_list_insert_after(mem, literals, kefir_list_tail(literals), (void *) clone);
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_token_free(mem, clone);
                KEFIR_FREE(mem, clone);
                return res;
            });

            REQUIRE_OK(kefir_lexer_cursor_match_whitespace(mem, lexer, NULL));
        }
    } while (scan);
    return KEFIR_OK;
}

static kefir_result_t free_token(struct kefir_mem *mem, struct kefir_list *list, struct kefir_list_entry *entry,
                                 void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));

    ASSIGN_DECL_CAST(struct kefir_token *, token, entry->value);
    REQUIRE_OK(kefir_token_free(mem, token));
    KEFIR_FREE(mem, token);
    return KEFIR_OK;
}

static kefir_result_t match_impl(struct kefir_mem *mem, struct kefir_lexer *lexer, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(lexer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct params *, params, payload);

    if (!params->merge_adjacent) {
        REQUIRE_OK(kefir_lexer_match_raw_string_literal(mem, lexer, params->token));
    } else {
        struct kefir_list literals;
        REQUIRE_OK(kefir_list_init(&literals));
        REQUIRE_OK(kefir_list_on_remove(&literals, free_token, NULL));

        kefir_result_t res = scan_literals(mem, lexer, &literals);
        REQUIRE_CHAIN(&res, kefir_lexer_merge_raw_string_literals(mem, &literals, params->token));
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_list_free(mem, &literals);
            return res;
        });

        res = kefir_list_free(mem, &literals);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_token_free(mem, params->token);
            return res;
        });
    }
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_match_string_literal(struct kefir_mem *mem, struct kefir_lexer *lexer,
                                                struct kefir_token *token, kefir_bool_t merge_adjacent) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(lexer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token"));

    struct params params = {.token = token, .merge_adjacent = merge_adjacent};
    REQUIRE_OK(kefir_lexer_apply(mem, lexer, match_impl, &params));
    return KEFIR_OK;
}
