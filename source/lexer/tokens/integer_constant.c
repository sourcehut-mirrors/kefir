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

#include "kefir/lexer/lexer.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"
#include "kefir/core/string_buffer.h"
#include "kefir/util/char32.h"
#include <string.h>

enum integer_constant_type {
    CONSTANT_INT,
    CONSTANT_UNSIGNED_INT,
    CONSTANT_LONG,
    CONSTANT_UNSIGNED_LONG,
    CONSTANT_LONG_LONG,
    CONSTANT_UNSIGNED_LONG_LONG,
    CONSTANT_BIT_PRECISE,
    CONSTANT_UNSIGNED_BIT_PRECISE
};

static kefir_result_t get_permitted_constant_types(enum integer_constant_type original, kefir_bool_t decimal,
                                                   const enum integer_constant_type **permitted, kefir_size_t *length) {
    switch (original) {
        case CONSTANT_INT:
            if (decimal) {
                static const enum integer_constant_type types[] = {CONSTANT_INT, CONSTANT_LONG, CONSTANT_LONG_LONG};
                *permitted = types;
                *length = sizeof(types) / sizeof(types[0]);
            } else {
                static const enum integer_constant_type types[] = {CONSTANT_INT,       CONSTANT_UNSIGNED_INT,
                                                                   CONSTANT_LONG,      CONSTANT_UNSIGNED_LONG,
                                                                   CONSTANT_LONG_LONG, CONSTANT_UNSIGNED_LONG_LONG};
                *permitted = types;
                *length = sizeof(types) / sizeof(types[0]);
            }
            break;

        case CONSTANT_UNSIGNED_INT: {
            static const enum integer_constant_type types[] = {CONSTANT_UNSIGNED_INT, CONSTANT_UNSIGNED_LONG,
                                                               CONSTANT_UNSIGNED_LONG_LONG};
            *permitted = types;
            *length = sizeof(types) / sizeof(types[0]);
        } break;

        case CONSTANT_LONG:
            if (decimal) {
                static const enum integer_constant_type types[] = {CONSTANT_LONG, CONSTANT_LONG_LONG};
                *permitted = types;
                *length = sizeof(types) / sizeof(types[0]);
            } else {
                static const enum integer_constant_type types[] = {CONSTANT_LONG, CONSTANT_UNSIGNED_LONG,
                                                                   CONSTANT_LONG_LONG, CONSTANT_UNSIGNED_LONG_LONG};
                *permitted = types;
                *length = sizeof(types) / sizeof(types[0]);
            }
            break;

        case CONSTANT_UNSIGNED_LONG: {
            static const enum integer_constant_type types[] = {CONSTANT_UNSIGNED_LONG, CONSTANT_UNSIGNED_LONG_LONG};
            *permitted = types;
            *length = sizeof(types) / sizeof(types[0]);
        } break;

        case CONSTANT_LONG_LONG:
            if (decimal) {
                static const enum integer_constant_type types[] = {CONSTANT_LONG_LONG};
                *permitted = types;
                *length = sizeof(types) / sizeof(types[0]);
            } else {
                static const enum integer_constant_type types[] = {CONSTANT_LONG_LONG, CONSTANT_UNSIGNED_LONG_LONG};
                *permitted = types;
                *length = sizeof(types) / sizeof(types[0]);
            }
            break;

        case CONSTANT_UNSIGNED_LONG_LONG: {
            static const enum integer_constant_type types[] = {CONSTANT_UNSIGNED_LONG_LONG};
            *permitted = types;
            *length = sizeof(types) / sizeof(types[0]);
        } break;

        case CONSTANT_UNSIGNED_BIT_PRECISE: {
            static const enum integer_constant_type types[] = {CONSTANT_UNSIGNED_BIT_PRECISE};
            *permitted = types;
            *length = sizeof(types) / sizeof(types[0]);
        } break;

        case CONSTANT_BIT_PRECISE: {
            static const enum integer_constant_type types[] = {CONSTANT_BIT_PRECISE};
            *permitted = types;
            *length = sizeof(types) / sizeof(types[0]);
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t parse_integer_constant(const char *literal, kefir_size_t base, kefir_uint64_t *value) {
    *value = 0;
    switch (base) {
        case 10:
            for (; *literal != '\0'; literal++) {
                *value *= 10;
                *value += (*literal) - '0';
            }
            break;

        case 16:
            for (; *literal != '\0'; literal++) {
                *value <<= 4;
                switch (*literal) {
                    case 'a':
                    case 'A':
                        *value += 0xa;
                        break;

                    case 'b':
                    case 'B':
                        *value += 0xb;
                        break;

                    case 'c':
                    case 'C':
                        *value += 0xc;
                        break;

                    case 'd':
                    case 'D':
                        *value += 0xd;
                        break;

                    case 'e':
                    case 'E':
                        *value += 0xe;
                        break;

                    case 'f':
                    case 'F':
                        *value += 0xf;
                        break;

                    default:
                        *value += (*literal) - '0';
                        break;
                }
            }
            break;

        case 8:
            for (; *literal != '\0'; literal++) {
                *value <<= 3;
                *value += (*literal) - '0';
            }
            break;

        case 2:
            for (; *literal != '\0'; literal++) {
                *value <<= 1;
                *value += (*literal) - '0';
            }
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected integral constant base");
    }

    return KEFIR_OK;
}

static kefir_result_t make_integral_constant(struct kefir_mem *mem, const struct kefir_lexer_context *context,
                                             enum integer_constant_type type, const char *literal, kefir_size_t base,
                                             struct kefir_token *token) {
    kefir_uint64_t value = 0;
    switch (type) {
        case CONSTANT_INT:
            REQUIRE_OK(parse_integer_constant(literal, base, &value));
            REQUIRE(value <= context->integer_max_value,
                    KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Provided constant exceeds maximum value of its type"));
            REQUIRE_OK(kefir_token_new_constant_int((kefir_int64_t) value, token));
            break;

        case CONSTANT_UNSIGNED_INT:
            REQUIRE_OK(parse_integer_constant(literal, base, &value));
            REQUIRE(value <= context->uinteger_max_value,
                    KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Provided constant exceeds maximum value of its type"));
            REQUIRE_OK(kefir_token_new_constant_uint(value, token));
            break;

        case CONSTANT_LONG:
            REQUIRE_OK(parse_integer_constant(literal, base, &value));
            REQUIRE(value <= context->long_max_value,
                    KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Provided constant exceeds maximum value of its type"));
            REQUIRE_OK(kefir_token_new_constant_long((kefir_int64_t) value, token));
            break;

        case CONSTANT_UNSIGNED_LONG:
            REQUIRE_OK(parse_integer_constant(literal, base, &value));
            REQUIRE(value <= context->ulong_max_value,
                    KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Provided constant exceeds maximum value of its type"));
            REQUIRE_OK(kefir_token_new_constant_ulong(value, token));
            break;

        case CONSTANT_LONG_LONG:
            REQUIRE_OK(parse_integer_constant(literal, base, &value));
            REQUIRE(value <= context->long_long_max_value,
                    KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Provided constant exceeds maximum value of its type"));
            REQUIRE_OK(kefir_token_new_constant_long_long((kefir_int64_t) value, token));
            break;

        case CONSTANT_UNSIGNED_LONG_LONG:
            REQUIRE_OK(parse_integer_constant(literal, base, &value));
            REQUIRE(value <= context->ulong_long_max_value,
                    KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Provided constant exceeds maximum value of its type"));
            REQUIRE_OK(kefir_token_new_constant_ulong_long(value, token));
            break;

        case CONSTANT_UNSIGNED_BIT_PRECISE: {
            struct kefir_bigint bigint;
            REQUIRE_OK(kefir_bigint_init(&bigint));
            kefir_result_t res;
            switch (base) {
                case 10:
                    res = kefir_bigint_unsigned_parse10(mem, &bigint, literal, strlen(literal));
                    break;

                case 16:
                    res = kefir_bigint_unsigned_parse16(mem, &bigint, literal, strlen(literal));
                    break;

                case 8:
                    res = kefir_bigint_unsigned_parse8(mem, &bigint, literal, strlen(literal));
                    break;

                case 2:
                    res = kefir_bigint_unsigned_parse2(mem, &bigint, literal, strlen(literal));
                    break;

                default:
                    res = KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected integer constant base");
                    break;
            }
            REQUIRE_CHAIN(&res,
                          kefir_bigint_resize_cast_unsigned(mem, &bigint, kefir_bigint_min_unsigned_width(&bigint)));
            REQUIRE_CHAIN(&res, kefir_token_new_constant_unsigned_bit_precise(&bigint, token));
            REQUIRE_ELSE(res == KEFIR_OK, { kefir_bigint_free(mem, &bigint); });
        } break;

        case CONSTANT_BIT_PRECISE: {
            struct kefir_bigint bigint;
            REQUIRE_OK(kefir_bigint_init(&bigint));
            kefir_result_t res;
            switch (base) {
                case 10:
                    res = kefir_bigint_unsigned_parse10(mem, &bigint, literal, strlen(literal));
                    break;

                case 16:
                    res = kefir_bigint_unsigned_parse16(mem, &bigint, literal, strlen(literal));
                    break;

                case 8:
                    res = kefir_bigint_unsigned_parse8(mem, &bigint, literal, strlen(literal));
                    break;

                case 2:
                    res = kefir_bigint_unsigned_parse2(mem, &bigint, literal, strlen(literal));
                    break;

                default:
                    res = KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected integer constant base");
                    break;
            }
            REQUIRE_CHAIN(
                &res, kefir_bigint_resize_cast_unsigned(mem, &bigint, kefir_bigint_min_unsigned_width(&bigint) + 1));
            REQUIRE_CHAIN(&res, kefir_token_new_constant_bit_precise(&bigint, token));
            REQUIRE_ELSE(res == KEFIR_OK, { kefir_bigint_free(mem, &bigint); });
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t build_integral_constant(struct kefir_mem *mem, const struct kefir_lexer_context *context,
                                              enum integer_constant_type type, const char *literal, kefir_size_t base,
                                              struct kefir_token *token,
                                              const struct kefir_source_location *source_location) {
    const enum integer_constant_type *permitted_types = NULL;
    kefir_size_t permitted_types_length = 0;
    REQUIRE_OK(get_permitted_constant_types(type, base == 10, &permitted_types, &permitted_types_length));
    for (kefir_size_t i = 0; i < permitted_types_length; i++) {
        kefir_result_t res = make_integral_constant(mem, context, permitted_types[i], literal, base, token);
        if (res == KEFIR_OK) {
            return KEFIR_OK;
        } else {
            REQUIRE(res == KEFIR_NO_MATCH, res);
        }
    }

    return KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location,
                                  "Provided constant exceeds maximum value of its type");
}

static kefir_result_t next_decimal_constant(struct kefir_mem *mem, struct kefir_lexer_source_cursor *cursor,
                                            struct kefir_string_buffer *strbuf, kefir_size_t *base) {
    kefir_char32_t chr = kefir_lexer_source_cursor_at(cursor, 0);
    REQUIRE(kefir_isdigit32(chr) && chr != U'0',
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match decimal integer constant"));

    for (; kefir_isdigit32(chr) || chr == U'\'';
         kefir_lexer_source_cursor_next(cursor, 1), chr = kefir_lexer_source_cursor_at(cursor, 0)) {
        if (chr == U'\'') {
            if (!kefir_isdigit32(kefir_lexer_source_cursor_at(cursor, 1))) {
                break;
            }
        } else {
            REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, chr));
        }
    }
    *base = 10;
    return KEFIR_OK;
}

static kefir_result_t next_hexadecimal_constant(struct kefir_mem *mem, struct kefir_lexer_source_cursor *cursor,
                                                struct kefir_string_buffer *strbuf, kefir_size_t *base) {
    kefir_char32_t init_chr = kefir_lexer_source_cursor_at(cursor, 0);
    kefir_char32_t init_chr2 = kefir_lexer_source_cursor_at(cursor, 1);
    kefir_char32_t chr = kefir_lexer_source_cursor_at(cursor, 2);
    REQUIRE(init_chr == U'0' && (init_chr2 == U'x' || init_chr2 == U'X') && kefir_ishexdigit32(chr),
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match hexadecimal integer constant"));
    REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));

    for (; kefir_ishexdigit32(chr) || chr == U'\'';
         kefir_lexer_source_cursor_next(cursor, 1), chr = kefir_lexer_source_cursor_at(cursor, 0)) {
        if (chr == U'\'') {
            if (!kefir_ishexdigit32(kefir_lexer_source_cursor_at(cursor, 1))) {
                break;
            }
        } else {
            REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, chr));
        }
    }
    *base = 16;
    return KEFIR_OK;
}

static kefir_result_t next_binary_constant(struct kefir_mem *mem, struct kefir_lexer_source_cursor *cursor,
                                           struct kefir_string_buffer *strbuf, kefir_size_t *base) {
    kefir_char32_t init_chr = kefir_lexer_source_cursor_at(cursor, 0);
    kefir_char32_t init_chr2 = kefir_lexer_source_cursor_at(cursor, 1);
    kefir_char32_t chr = kefir_lexer_source_cursor_at(cursor, 2);
    REQUIRE(init_chr == U'0' && (init_chr2 == U'b' || init_chr2 == U'B') && (chr == U'0' || chr == U'1'),
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match binary integer constant"));
    REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));

    for (; chr == U'0' || chr == U'1' || chr == U'\'';
         kefir_lexer_source_cursor_next(cursor, 1), chr = kefir_lexer_source_cursor_at(cursor, 0)) {
        if (chr == U'\'') {
            const kefir_char32_t next_chr = kefir_lexer_source_cursor_at(cursor, 1);
            if (next_chr != U'0' && next_chr != U'1') {
                break;
            }
        } else {
            REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, chr));
        }
    }
    *base = 2;
    return KEFIR_OK;
}

static kefir_result_t next_octal_constant(struct kefir_mem *mem, struct kefir_lexer_source_cursor *cursor,
                                          struct kefir_string_buffer *strbuf, kefir_size_t *base) {
    kefir_char32_t chr = kefir_lexer_source_cursor_at(cursor, 0);
    REQUIRE(chr == U'0', KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match octal integer constant"));

    for (; kefir_isoctdigit32(chr) || chr == U'\'';
         kefir_lexer_source_cursor_next(cursor, 1), chr = kefir_lexer_source_cursor_at(cursor, 0)) {
        if (chr == U'\'') {
            if (!kefir_isoctdigit32(kefir_lexer_source_cursor_at(cursor, 1))) {
                break;
            }
        } else {
            REQUIRE_OK(kefir_string_buffer_append(mem, strbuf, chr));
        }
    }
    *base = 8;
    return KEFIR_OK;
}

static kefir_result_t scan_suffix(struct kefir_mem *mem, struct kefir_lexer_source_cursor *cursor,
                                  const struct kefir_lexer_context *lexer_context, const char *literal,
                                  kefir_size_t base, struct kefir_token *token,
                                  const struct kefir_source_location *source_location) {
    UNUSED(literal);
    static const struct Suffix {
        const kefir_char32_t *suffix;
        enum integer_constant_type type;
    } SUFFIXES[] = {
        {U"uLL", CONSTANT_UNSIGNED_LONG_LONG},
        {U"ull", CONSTANT_UNSIGNED_LONG_LONG},
        {U"LLu", CONSTANT_UNSIGNED_LONG_LONG},
        {U"llu", CONSTANT_UNSIGNED_LONG_LONG},
        {U"ULL", CONSTANT_UNSIGNED_LONG_LONG},
        {U"Ull", CONSTANT_UNSIGNED_LONG_LONG},
        {U"LLU", CONSTANT_UNSIGNED_LONG_LONG},
        {U"llU", CONSTANT_UNSIGNED_LONG_LONG},
        {U"wbu", CONSTANT_UNSIGNED_BIT_PRECISE},
        {U"wbU", CONSTANT_UNSIGNED_BIT_PRECISE},
        {U"uwb", CONSTANT_UNSIGNED_BIT_PRECISE},
        {U"Uwb", CONSTANT_UNSIGNED_BIT_PRECISE},
        {U"WBu", CONSTANT_UNSIGNED_BIT_PRECISE},
        {U"WBU", CONSTANT_UNSIGNED_BIT_PRECISE},
        {U"uWB", CONSTANT_UNSIGNED_BIT_PRECISE},
        {U"UWB", CONSTANT_UNSIGNED_BIT_PRECISE},
        {U"wb", CONSTANT_BIT_PRECISE},
        {U"WB", CONSTANT_BIT_PRECISE},
        {U"uL", CONSTANT_UNSIGNED_LONG},
        {U"ul", CONSTANT_UNSIGNED_LONG},
        {U"Lu", CONSTANT_UNSIGNED_LONG},
        {U"lu", CONSTANT_UNSIGNED_LONG},
        {U"UL", CONSTANT_UNSIGNED_LONG},
        {U"Ul", CONSTANT_UNSIGNED_LONG},
        {U"LU", CONSTANT_UNSIGNED_LONG},
        {U"lU", CONSTANT_UNSIGNED_LONG},
        {U"u", CONSTANT_UNSIGNED_INT},
        {U"U", CONSTANT_UNSIGNED_INT},
        {U"LL", CONSTANT_LONG_LONG},
        {U"ll", CONSTANT_LONG_LONG},
        {U"L", CONSTANT_LONG},
        {U"l", CONSTANT_LONG},
    };
    static const kefir_size_t SUFFIXES_LENGTH = sizeof(SUFFIXES) / sizeof(SUFFIXES[0]);

    const struct Suffix *matchedSuffix = NULL;
    for (kefir_size_t i = 0; matchedSuffix == NULL && i < SUFFIXES_LENGTH; i++) {
        const struct Suffix *suffix = &SUFFIXES[i];
        kefir_result_t res = kefir_lexer_cursor_match_string(cursor, suffix->suffix);
        if (res == KEFIR_OK) {
            matchedSuffix = suffix;
            REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, kefir_strlen32(suffix->suffix)));
        } else {
            REQUIRE(res == KEFIR_NO_MATCH, res);
        }
    }

    if (matchedSuffix == NULL) {
        REQUIRE_OK(build_integral_constant(mem, lexer_context, CONSTANT_INT, literal, base, token, source_location));
    } else {
        REQUIRE_OK(
            build_integral_constant(mem, lexer_context, matchedSuffix->type, literal, base, token, source_location));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_scan_integral_constant(struct kefir_mem *mem, struct kefir_lexer_source_cursor *cursor,
                                                  const struct kefir_lexer_context *lexer_context,
                                                  struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source cursor"));
    REQUIRE(lexer_context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer context"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));

    struct kefir_source_location source_location = cursor->location;
    kefir_size_t base = 0;
    struct kefir_string_buffer strbuf;
    REQUIRE_OK(kefir_string_buffer_init(mem, &strbuf, KEFIR_STRING_BUFFER_MULTIBYTE));
    kefir_result_t res = next_decimal_constant(mem, cursor, &strbuf, &base);
    if (res == KEFIR_NO_MATCH) {
        res = next_hexadecimal_constant(mem, cursor, &strbuf, &base);
    }
    if (res == KEFIR_NO_MATCH) {
        res = next_binary_constant(mem, cursor, &strbuf, &base);
    }
    if (res == KEFIR_NO_MATCH) {
        res = next_octal_constant(mem, cursor, &strbuf, &base);
    }
    if (res == KEFIR_NO_MATCH) {
        res = KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match integer constant");
    }
    REQUIRE_CHAIN(&res, scan_suffix(mem, cursor, lexer_context, kefir_string_buffer_value(&strbuf, NULL), base, token,
                                    &source_location));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_string_buffer_free(mem, &strbuf);
        return res;
    });
    REQUIRE_OK(kefir_string_buffer_free(mem, &strbuf));
    return KEFIR_OK;
}

static kefir_result_t match_impl(struct kefir_mem *mem, struct kefir_lexer *lexer, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(lexer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct kefir_token *, token, payload);

    REQUIRE_OK(kefir_lexer_scan_integral_constant(mem, lexer->cursor, lexer->context, token));
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_match_integer_constant(struct kefir_mem *mem, struct kefir_lexer *lexer,
                                                  struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(lexer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token"));

    REQUIRE_OK(kefir_lexer_apply(mem, lexer, match_impl, token));
    return KEFIR_OK;
}
