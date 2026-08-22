/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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
#include "kefir/lexer/util.h"
#include "kefir/core/source_error.h"

#define MAX_IDENTIFIER_LENGTH 4095

static kefir_result_t scan_identifier_nondigit(struct kefir_lexer_source_cursor *cursor, char *target, kefir_size_t *target_avail) {
    kefir_lexer_char_t chr = kefir_lexer_source_cursor_at(cursor, 0);
    if (kefir_lexer_char_isnondigit(chr)) {
        REQUIRE(*target_avail > 0, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Identifier out of target buffer bounds"));
        *target = chr;
        (*target_avail)--;
        REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 1));
    } else {
        kefir_char32_t chr;
        REQUIRE_OK(kefir_lexer_cursor_next_universal_character(cursor, &chr));

        mbstate_t mbstate = {0};
        char mbs[MB_LEN_MAX];
        size_t rc = c32rtomb(mbs, chr, &mbstate);
        switch (rc) {
            case (size_t) -1:
            case 0:
                REQUIRE(*target_avail > 0, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Identifier out of target buffer bounds"));
                *target = (char) chr;
                (*target_avail)--;
                break;

            default:
                REQUIRE(*target_avail >= rc, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Identifier out of target buffer bounds"));
                memcpy(target, mbs, sizeof(char) * rc);
                *target_avail -= rc;
                break;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t scan_identifier_digit(struct kefir_lexer_source_cursor *cursor, char *target, kefir_size_t *target_avail) {
    kefir_lexer_char_t chr = kefir_lexer_source_cursor_at(cursor, 0);
    if (kefir_lexer_char_isdigit(chr)) {
        REQUIRE(*target_avail > 0, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Identifier out of target buffer bounds"));
        *target = chr;
        (*target_avail)--;
        REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 1));
    } else {
        REQUIRE_OK(scan_identifier_nondigit(cursor, target, target_avail));
    }
    return KEFIR_OK;
}

struct match_payload {
    struct kefir_token *token;
    kefir_bool_t map_keywords;
};

kefir_result_t kefir_lexer_scan_identifier_or_keyword(struct kefir_mem *mem, struct kefir_lexer_source_cursor *cursor,
                                                      kefir_lexer_mode_t mode, struct kefir_string_pool *symbols,
                                                      const struct kefir_trie *keywords, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source cursor"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));

    struct kefir_source_location identifier_location = cursor->location;
#define MB_IDENTIFIER_SIZE (MB_LEN_MAX * MAX_IDENTIFIER_LENGTH)
    char mb_identifier[MB_IDENTIFIER_SIZE + 1];
    kefir_size_t mb_identifier_available = MB_IDENTIFIER_SIZE;
    REQUIRE_OK(scan_identifier_nondigit(cursor, &mb_identifier[0], &mb_identifier_available));

    kefir_bool_t scan_identifier =
        mode != KEFIR_LEXER_ASSEMBLY_MODE || (mb_identifier[0] != U'$' && mb_identifier[0] != U'@');
    for (; scan_identifier;) {
        if (kefir_lexer_source_cursor_at(cursor, 0) == '@' && mode == KEFIR_LEXER_ASSEMBLY_MODE) {
            break;
        }

        kefir_result_t res = scan_identifier_digit(cursor, &mb_identifier[MB_IDENTIFIER_SIZE - mb_identifier_available], &mb_identifier_available);
        if (res == KEFIR_NO_MATCH) {
            scan_identifier = false;
        } else if (res == KEFIR_OUT_OF_BOUNDS) {
            return KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &identifier_location,
                                                                           "Identifier exceeds maximum length");
        } else {
            REQUIRE_OK(res);
        }
    }
    mb_identifier[MB_IDENTIFIER_SIZE - mb_identifier_available] = '\0';
#undef MB_IDENTIFIER_SIZE

    kefir_keyword_token_t keyword;
    kefir_result_t res = keywords != NULL ? kefir_lexer_get_keyword(keywords, mb_identifier, &keyword) : KEFIR_NO_MATCH;
    if (res == KEFIR_NO_MATCH) {
        REQUIRE_OK(kefir_token_new_identifier(mem, symbols, mb_identifier, token));
    } else {
        REQUIRE_OK(res);
        const char *keyword_spelling = kefir_string_pool_insert(mem, symbols, mb_identifier, NULL);
        REQUIRE(keyword_spelling != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to insert identifier into symbol table"));
        REQUIRE_OK(kefir_token_new_keyword_with_spelling(keyword, keyword_spelling, token));
    }
    return KEFIR_OK;
}

static kefir_result_t match_impl(struct kefir_mem *mem, struct kefir_lexer *lexer, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(lexer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct match_payload *, param, payload);

    REQUIRE_OK(kefir_lexer_scan_identifier_or_keyword(mem, lexer->cursor, lexer->mode, lexer->symbols,
                                                      param->map_keywords ? &lexer->keywords : NULL, param->token));
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_match_identifier_or_keyword(struct kefir_mem *mem, struct kefir_lexer *lexer,
                                                       struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(lexer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token"));

    struct match_payload param = {.token = token, .map_keywords = true};
    REQUIRE_OK(kefir_lexer_apply(mem, lexer, match_impl, &param));
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_match_identifier(struct kefir_mem *mem, struct kefir_lexer *lexer,
                                            struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(lexer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token"));

    struct match_payload param = {.token = token, .map_keywords = false};
    REQUIRE_OK(kefir_lexer_apply(mem, lexer, match_impl, &param));
    return KEFIR_OK;
}
