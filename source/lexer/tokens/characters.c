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

static kefir_uint32_t oct_to_digit(kefir_lexer_char_t chr) {
    if (chr >= '0' && chr <= '7') {
        return chr - '0';
    } else {
        return ~((kefir_uint32_t) 0);
    }
}

kefir_result_t kefir_lexer_cursor_next_universal_character(struct kefir_lexer_source_cursor *cursor,
                                                           kefir_char32_t *target) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer source cursor"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to character"));

    kefir_lexer_char_t chr = kefir_lexer_source_cursor_at(cursor, 0);
    kefir_lexer_char_t chr2 = kefir_lexer_source_cursor_at(cursor, 1);

    if (chr == '\\' && chr2 == 'u') {
        kefir_lexer_char_t hex1 = kefir_lexer_source_cursor_at(cursor, 2), hex2 = kefir_lexer_source_cursor_at(cursor, 3),
                       hex3 = kefir_lexer_source_cursor_at(cursor, 4), hex4 = kefir_lexer_source_cursor_at(cursor, 5);
        REQUIRE(kefir_lexer_char_ishexdigit(hex1) && kefir_lexer_char_ishexdigit(hex2) && kefir_lexer_char_ishexdigit(hex3) &&
                    kefir_lexer_char_ishexdigit(hex4),
                KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &cursor->location, "Incomplete universal character"));
        *target = kefir_lexer_char_hex2dec(hex4) | (kefir_lexer_char_hex2dec(hex3) << 4) | (kefir_lexer_char_hex2dec(hex2) << 8) |
                  (kefir_lexer_char_hex2dec(hex1) << 12);
        REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 6));
    } else if (chr == '\\' && chr2 == 'U') {
        kefir_lexer_char_t hex1 = kefir_lexer_source_cursor_at(cursor, 2), hex2 = kefir_lexer_source_cursor_at(cursor, 3),
                       hex3 = kefir_lexer_source_cursor_at(cursor, 4), hex4 = kefir_lexer_source_cursor_at(cursor, 5),
                       hex5 = kefir_lexer_source_cursor_at(cursor, 6), hex6 = kefir_lexer_source_cursor_at(cursor, 7),
                       hex7 = kefir_lexer_source_cursor_at(cursor, 8), hex8 = kefir_lexer_source_cursor_at(cursor, 9);
        REQUIRE(kefir_lexer_char_ishexdigit(hex1) && kefir_lexer_char_ishexdigit(hex2) && kefir_lexer_char_ishexdigit(hex3) &&
                    kefir_lexer_char_ishexdigit(hex4) && kefir_lexer_char_ishexdigit(hex5) && kefir_lexer_char_ishexdigit(hex6) &&
                    kefir_lexer_char_ishexdigit(hex7) && kefir_lexer_char_ishexdigit(hex8),
                KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &cursor->location, "Incomplete universal character"));
        *target = kefir_lexer_char_hex2dec(hex8) | (kefir_lexer_char_hex2dec(hex7) << 4) | (kefir_lexer_char_hex2dec(hex6) << 8) |
                  (kefir_lexer_char_hex2dec(hex5) << 12) | (kefir_lexer_char_hex2dec(hex4) << 16) | (kefir_lexer_char_hex2dec(hex3) << 20) |
                  (kefir_lexer_char_hex2dec(hex2) << 24) | (kefir_lexer_char_hex2dec(hex1) << 28);
        REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 10));
    } else {
        return KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match universal character");
    }
    return KEFIR_OK;
}

static kefir_result_t next_simple_escape_sequence(struct kefir_lexer_source_cursor *cursor, kefir_char32_t *target) {
    switch (kefir_lexer_source_cursor_at(cursor, 1)) {
        case '\'':
            *target = U'\'';
            REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));
            break;

        case '\"':
            *target = U'\"';
            REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));
            break;

        case '?':
            *target = U'\?';
            REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));
            break;

        case '\\':
            *target = U'\\';
            REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));
            break;

        case 'a':
            *target = U'\a';
            REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));
            break;

        case 'b':
            *target = U'\b';
            REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));
            break;

        case 'f':
            *target = U'\f';
            REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));
            break;

        case 'n':
            *target = U'\n';
            REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));
            break;

        case 'r':
            *target = U'\r';
            REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));
            break;

        case 't':
            *target = U'\t';
            REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));
            break;

        case 'v':
            *target = U'\v';
            REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));
            break;

        case 'e':
            *target = U'\x1B';
            REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Cannot match simple escape sequence");
    }
    return KEFIR_OK;
}

static kefir_result_t next_octal_escape_sequence(struct kefir_lexer_source_cursor *cursor, kefir_char32_t *target) {
    kefir_lexer_char_t chr1 = kefir_lexer_source_cursor_at(cursor, 1),
        chr2 = kefir_lexer_source_cursor_at(cursor, 2),
        chr3 = kefir_lexer_source_cursor_at(cursor, 3);

    if (kefir_lexer_char_isoctdigit(chr1) && kefir_lexer_char_isoctdigit(chr2) && kefir_lexer_char_isoctdigit(chr3)) {
        *target = (oct_to_digit(chr1) << 6) | (oct_to_digit(chr2) << 3) | oct_to_digit(chr3);
        REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 4));
    } else if (kefir_lexer_char_isoctdigit(chr1) && kefir_lexer_char_isoctdigit(chr2)) {
        *target = (oct_to_digit(chr1) << 3) | oct_to_digit(chr2);
        REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 3));
    } else if (kefir_lexer_char_isoctdigit(chr1)) {
        *target = oct_to_digit(chr1);
        REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));
    } else {
        return KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match octal escape sequence");
    }
    return KEFIR_OK;
}

static kefir_result_t next_hexadecimal_escape_sequence(struct kefir_lexer_source_cursor *cursor,
                                                       kefir_char32_t *target) {
    REQUIRE(kefir_lexer_source_cursor_at(cursor, 1) == 'x',
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match hexadecimal escape sequence"));

    REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));
    kefir_lexer_char_t chr = kefir_lexer_source_cursor_at(cursor, 0);
    REQUIRE(kefir_lexer_char_ishexdigit(chr),
            KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, &cursor->location, "Expected hexadecimal digit"));
    *target = 0;
    for (; kefir_lexer_char_ishexdigit(chr);
         kefir_lexer_source_cursor_next(cursor, 1), chr = kefir_lexer_source_cursor_at(cursor, 0)) {
        *target <<= 4;
        *target += kefir_lexer_char_hex2dec(chr);
    }
    return KEFIR_OK;
}

static kefir_result_t next_unknown_sequence(struct kefir_lexer_source_cursor *cursor, kefir_char32_t *target) {
    *target = kefir_lexer_source_cursor_at(cursor, 1);
    REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_cursor_next_escape_sequence(struct kefir_lexer_source_cursor *cursor,
                                                       kefir_char32_t *target) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer source cursor"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to character"));
    REQUIRE(kefir_lexer_source_cursor_at(cursor, 0) == '\\',
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match escape sequence"));

    kefir_result_t res = next_simple_escape_sequence(cursor, target);
    REQUIRE(res == KEFIR_NO_MATCH, res);
    res = next_octal_escape_sequence(cursor, target);
    REQUIRE(res == KEFIR_NO_MATCH, res);
    res = next_hexadecimal_escape_sequence(cursor, target);
    REQUIRE(res == KEFIR_NO_MATCH, res);
    res = kefir_lexer_cursor_next_universal_character(cursor, target);
    REQUIRE(res == KEFIR_NO_MATCH, res);
    REQUIRE_OK(next_unknown_sequence(cursor, target));
    return KEFIR_OK;
}
