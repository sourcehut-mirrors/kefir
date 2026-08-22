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

static kefir_result_t scan_char32(struct kefir_lexer_source_cursor *cursor, kefir_char32_t *char32, kefir_bool_t *success) {       
    mbstate_t mbstate = {0};
    *success = false;
    for (kefir_size_t i = 0; !*success && kefir_lexer_source_cursor_at(cursor, i) != KEFIR_LEXER_SOURCE_CURSOR_EOF; i++) {
        char c = kefir_lexer_source_cursor_at(cursor, i);
        size_t rc = mbrtoc32(char32, &c, 1, &mbstate);
        switch (rc) {
            case (size_t) -3:
            case (size_t) -2:
                // Intentionally left blank
                break;

            case (size_t) -1:
                *success = true;
                break;

            case 0:
                REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 1));
                *char32 = U'\0';
                *success = true;
                break;

            default:
                REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, i + 1));
                *success = true;
                break;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t next_character(struct kefir_lexer_source_cursor *cursor, kefir_uint_t *value,
                                     kefir_bool_t *continueScan) {
    struct kefir_source_location char_location = cursor->location;
    kefir_char32_t char32;
    kefir_lexer_char_t current_chr = kefir_lexer_source_cursor_at(cursor, 0);
    kefir_bool_t hex_oct_escape = false;
    if (current_chr == '\\') {
        *continueScan = true;
        kefir_lexer_char_t next_chr = kefir_lexer_source_cursor_at(cursor, 1);
        hex_oct_escape = next_chr == 'x' || kefir_lexer_char_isoctdigit(next_chr) || kefir_lexer_char_ishexdigit(next_chr);
        REQUIRE_OK(kefir_lexer_cursor_next_escape_sequence(cursor, &char32));
    } else if (current_chr == '\'') {
        *continueScan = false;
    } else {
        REQUIRE(current_chr != KEFIR_LEXER_SOURCE_CURSOR_EOF && current_chr != cursor->newline_char,
                KEFIR_SET_ERROR(KEFIR_LEXER_ERROR, "Unable to match character constant"));
        *continueScan = true;
        
        kefir_bool_t success = false;
        REQUIRE_OK(scan_char32(cursor, &char32, &success));
        if (!success) {
            char32 = kefir_lexer_source_cursor_at(cursor, 0);
            REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 1));
        }
    }

    if (*continueScan) {
        char multibyte[MB_LEN_MAX];
        size_t sz = 1;
        if (hex_oct_escape) {
            REQUIRE(char32 <= KEFIR_UCHAR_MAX, KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &char_location,
                                                                "Escape sequence exceeds maximum character value"));
            multibyte[0] = (char) char32;
        } else {
            mbstate_t mbstate = {0};
            sz = c32rtomb(multibyte, char32, &mbstate);
            if (sz == (size_t) -1) {
                *multibyte = (char) char32;
                sz = 1;
            }
        }
        char *iter = multibyte;
        while (sz--) {
            *value <<= 8;
            *value += *iter;
            iter++;
        }
    }
    return KEFIR_OK;
}
static kefir_result_t next_wide_character(struct kefir_lexer_source_cursor *cursor, kefir_char32_t *value,
                                          kefir_bool_t *continueScan) {
    kefir_lexer_char_t chr = kefir_lexer_source_cursor_at(cursor, 0);
    if (chr == '\\') {
        *continueScan = true;
        REQUIRE_OK(kefir_lexer_cursor_next_escape_sequence(cursor, value));
    } else if (chr == '\'') {
        *continueScan = false;
    } else {
        *continueScan = true;
        kefir_bool_t success = false;
        REQUIRE_OK(scan_char32(cursor, value, &success));
        if (!success) {
            *value = kefir_lexer_source_cursor_at(cursor, 0);
            REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 1));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t match_narrow_character(struct kefir_mem *mem, struct kefir_lexer *lexer,
                                             struct kefir_token *token) {
    REQUIRE(kefir_lexer_source_cursor_at(lexer->cursor, 0) == '\'',
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match character constant"));
    REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 1));

    kefir_uint_t character_value = 0;
    kefir_lexer_char_t chr = kefir_lexer_source_cursor_at(lexer->cursor, 0);
    REQUIRE(chr != '\'', KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &lexer->cursor->location,
                                                 "Empty character constant is not permitted"));
    for (kefir_bool_t scan = true; scan;) {
        REQUIRE_OK(next_character(lexer->cursor, &character_value, &scan));
    }
    chr = kefir_lexer_source_cursor_at(lexer->cursor, 0);
    REQUIRE(chr == '\'', KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &lexer->cursor->location,
                                                 "Character constant shall terminate with single quote"));
    REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 1));
    REQUIRE_OK(kefir_token_new_constant_char(mem, (kefir_int_t) character_value, token));
    return KEFIR_OK;
}

static kefir_result_t match_unicode8_character(struct kefir_mem *mem, struct kefir_lexer *lexer,
                                               struct kefir_token *token) {
    REQUIRE(kefir_lexer_source_cursor_at(lexer->cursor, 0) == 'u' &&
                kefir_lexer_source_cursor_at(lexer->cursor, 1) == '8' &&
                kefir_lexer_source_cursor_at(lexer->cursor, 2) == '\'',
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match unicode8 character constant"));
    REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 3));

    kefir_uint_t character_value = 0;
    kefir_lexer_char_t chr = kefir_lexer_source_cursor_at(lexer->cursor, 0);
    REQUIRE(chr != '\'', KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &lexer->cursor->location,
                                                 "Empty character constant is not permitted"));
    for (kefir_bool_t scan = true; scan;) {
        REQUIRE_OK(next_character(lexer->cursor, &character_value, &scan));
    }
    chr = kefir_lexer_source_cursor_at(lexer->cursor, 0);
    REQUIRE(chr == '\'', KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &lexer->cursor->location,
                                                 "Character constant shall terminate with single quote"));
    REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 1));
    REQUIRE_OK(kefir_token_new_constant_unicode8_char(mem, (kefir_int_t) character_value, token));
    return KEFIR_OK;
}

static kefir_result_t scan_wide_character(struct kefir_lexer *lexer, kefir_char32_t *value) {
    kefir_lexer_char_t chr = kefir_lexer_source_cursor_at(lexer->cursor, 0);
    REQUIRE(chr != '\'', KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &lexer->cursor->location,
                                                 "Empty character constant is not permitted"));
    for (kefir_bool_t scan = true; scan;) {
        REQUIRE_OK(next_wide_character(lexer->cursor, value, &scan));
    }
    chr = kefir_lexer_source_cursor_at(lexer->cursor, 0);
    REQUIRE(chr == '\'', KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &lexer->cursor->location,
                                                 "Character constant shall terminate with single quote"));
    REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 1));
    return KEFIR_OK;
}

static kefir_result_t match_wide_character(struct kefir_mem *mem, struct kefir_lexer *lexer,
                                           struct kefir_token *token) {
    UNUSED(token);
    REQUIRE(kefir_lexer_source_cursor_at(lexer->cursor, 0) == 'L' &&
                kefir_lexer_source_cursor_at(lexer->cursor, 1) == '\'',
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match wide character constant"));
    REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 2));

    kefir_char32_t character_value = 0;
    REQUIRE_OK(scan_wide_character(lexer, &character_value));
    REQUIRE_OK(kefir_token_new_constant_wide_char(mem, (kefir_wchar_t) character_value, token));
    return KEFIR_OK;
}

static kefir_result_t match_unicode16_character(struct kefir_mem *mem, struct kefir_lexer *lexer,
                                                struct kefir_token *token) {
    UNUSED(token);
    REQUIRE(kefir_lexer_source_cursor_at(lexer->cursor, 0) == 'u' &&
                kefir_lexer_source_cursor_at(lexer->cursor, 1) == '\'',
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match unicode character constant"));
    REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 2));

    kefir_char32_t character_value = 0;
    REQUIRE_OK(scan_wide_character(lexer, &character_value));
    REQUIRE_OK(kefir_token_new_constant_unicode16_char(mem, (kefir_char16_t) character_value, token));
    return KEFIR_OK;
}

static kefir_result_t match_unicode32_character(struct kefir_mem *mem, struct kefir_lexer *lexer,
                                                struct kefir_token *token) {
    UNUSED(token);
    REQUIRE(kefir_lexer_source_cursor_at(lexer->cursor, 0) == 'U' &&
                kefir_lexer_source_cursor_at(lexer->cursor, 1) == '\'',
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match unicode character constant"));
    REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 2));

    kefir_char32_t character_value = 0;
    REQUIRE_OK(scan_wide_character(lexer, &character_value));
    REQUIRE_OK(kefir_token_new_constant_unicode32_char(mem, (kefir_char32_t) character_value, token));
    return KEFIR_OK;
}

static kefir_result_t match_impl(struct kefir_mem *mem, struct kefir_lexer *lexer, void *payload) {
    UNUSED(mem);
    REQUIRE(lexer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct kefir_token *, token, payload);

    kefir_result_t res = match_narrow_character(mem, lexer, token);
    REQUIRE(res == KEFIR_NO_MATCH, res);
    res = match_unicode8_character(mem, lexer, token);
    REQUIRE(res == KEFIR_NO_MATCH, res);
    res = match_wide_character(mem, lexer, token);
    REQUIRE(res == KEFIR_NO_MATCH, res);
    res = match_unicode16_character(mem, lexer, token);
    REQUIRE(res == KEFIR_NO_MATCH, res);
    res = match_unicode32_character(mem, lexer, token);
    REQUIRE(res == KEFIR_NO_MATCH, res);
    return KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match character constant");
}

kefir_result_t kefir_lexer_match_character_constant(struct kefir_mem *mem, struct kefir_lexer *lexer,
                                                    struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(lexer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token"));

    REQUIRE_OK(kefir_lexer_apply(mem, lexer, match_impl, token));
    return KEFIR_OK;
}
