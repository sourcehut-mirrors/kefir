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

#include "kefir/lexer/string_literal_impl.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/util/char32.h"
#include "kefir/core/source_error.h"

static kefir_result_t kefir_lexer_next_string_literal_impl(struct kefir_mem *mem, struct kefir_lexer *lexer,
                                                           const kefir_char32_t *prefix,
                                                           struct kefir_string_buffer *buffer) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(lexer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer"));
    REQUIRE(prefix != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid prefix"));
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string buffer"));

    kefir_result_t res = kefir_lexer_cursor_match_string(lexer->cursor, prefix);
    if (res == KEFIR_NO_MATCH) {
        return KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match string literal");
    } else {
        REQUIRE_OK(res);
    }
    REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, kefir_strlen32(prefix)));

    for (kefir_char32_t chr = kefir_lexer_source_cursor_at(lexer->cursor, 0); chr != U'\"';
         chr = kefir_lexer_source_cursor_at(lexer->cursor, 0)) {

        struct kefir_source_location char_location = lexer->cursor->location;

        REQUIRE(chr != KEFIR_LEXER_SOURCE_CURSOR_EOF,
                KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &lexer->cursor->location, "Unexpected end of input"));
        REQUIRE(chr != U'\n',
                KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &lexer->cursor->location, "Unexpected newline character"));
        if (chr == U'\\') {
            kefir_char32_t next_chr = kefir_lexer_source_cursor_at(lexer->cursor, 1);
            kefir_bool_t hex_oct_sequence =
                next_chr == U'x' || kefir_isoctdigit32(next_chr) || kefir_ishexdigit32(next_chr);

            kefir_char32_t result;
            REQUIRE_OK(kefir_lexer_cursor_next_escape_sequence(lexer->cursor, &result));
            if (hex_oct_sequence) {
                kefir_result_t res = kefir_string_buffer_append_literal(mem, buffer, result);
                if (res == KEFIR_OUT_OF_BOUNDS) {
                    kefir_clear_error();
                    res = KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &char_location,
                                                 "Escape sequence exceeded maximum character value");
                }
                REQUIRE_OK(res);
            } else {
                REQUIRE_OK(kefir_string_buffer_append(mem, buffer, result));
            }
        } else {
            REQUIRE_OK(kefir_string_buffer_append(mem, buffer, chr));
            REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 1));
        }
    }
    REQUIRE_OK(kefir_lexer_source_cursor_next(lexer->cursor, 1));
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_next_string_literal_sequence_impl(struct kefir_mem *mem, struct kefir_lexer *lexer,
                                                             const kefir_char32_t *prefix,
                                                             struct kefir_string_buffer *buffer,
                                                             kefir_bool_t merge_adjacent) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(lexer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer"));
    REQUIRE(prefix != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid prefix"));
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string buffer"));

    REQUIRE_OK(kefir_lexer_next_string_literal_impl(mem, lexer, prefix, buffer));
    kefir_bool_t scan_literals = merge_adjacent;
    while (scan_literals) {
        REQUIRE_OK(kefir_lexer_cursor_match_whitespace(mem, lexer, NULL));
        kefir_result_t res = kefir_lexer_next_string_literal_impl(mem, lexer, prefix, buffer);
        if (res == KEFIR_NO_MATCH) {
            res = kefir_lexer_next_string_literal_impl(mem, lexer, U"\"", buffer);
        }
        if (res == KEFIR_NO_MATCH) {
            scan_literals = false;
        } else {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}
