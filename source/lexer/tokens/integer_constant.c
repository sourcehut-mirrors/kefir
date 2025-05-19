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
#include "kefir/util/char32.h"

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

static kefir_result_t make_integral_constant(const struct kefir_lexer_context *context, enum integer_constant_type type,
                                             kefir_uint64_t value, struct kefir_token *token) {
    switch (type) {
        case CONSTANT_INT:
            REQUIRE(value <= context->integer_max_value,
                    KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Provided constant exceeds maximum value of its type"));
            REQUIRE_OK(kefir_token_new_constant_int((kefir_int64_t) value, token));
            break;

        case CONSTANT_UNSIGNED_INT:
            REQUIRE(value <= context->uinteger_max_value,
                    KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Provided constant exceeds maximum value of its type"));
            REQUIRE_OK(kefir_token_new_constant_uint(value, token));
            break;

        case CONSTANT_LONG:
            REQUIRE(value <= context->long_max_value,
                    KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Provided constant exceeds maximum value of its type"));
            REQUIRE_OK(kefir_token_new_constant_long((kefir_int64_t) value, token));
            break;

        case CONSTANT_UNSIGNED_LONG:
            REQUIRE(value <= context->ulong_max_value,
                    KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Provided constant exceeds maximum value of its type"));
            REQUIRE_OK(kefir_token_new_constant_ulong(value, token));
            break;

        case CONSTANT_LONG_LONG:
            REQUIRE(value <= context->long_long_max_value,
                    KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Provided constant exceeds maximum value of its type"));
            REQUIRE_OK(kefir_token_new_constant_long_long((kefir_int64_t) value, token));
            break;

        case CONSTANT_UNSIGNED_LONG_LONG:
            REQUIRE(value <= context->ulong_long_max_value,
                    KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Provided constant exceeds maximum value of its type"));
            REQUIRE_OK(kefir_token_new_constant_ulong_long(value, token));
            break;

        case CONSTANT_UNSIGNED_BIT_PRECISE: {
            REQUIRE(value <= context->ulong_long_max_value,
                KEFIR_SET_ERROR(KEFIR_NOT_IMPLEMENTED, "Bit-precise integers exceeding widest integer type are not implemented yet"));

            kefir_size_t width = 1;
            const kefir_size_t max_width = sizeof(kefir_uint64_t) * CHAR_BIT;
            for (; width <= max_width; width++) {
                const kefir_uint64_t max_value = width < max_width
                    ? (1ull << width) - 1
                    : ~0ull;
                if (value <= max_value) {
                    break;
                }
            }
            REQUIRE(width <= max_width, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unable to determine unsigned bit-precise integer width"));
            REQUIRE_OK(kefir_token_new_constant_unsigned_bit_precise(value, width, token));
        } break;

        case CONSTANT_BIT_PRECISE: {
            REQUIRE((kefir_int64_t) value <= (kefir_int64_t) context->long_long_max_value && (kefir_int64_t) value >= -((kefir_int64_t) context->long_long_max_value) - 1,
                KEFIR_SET_ERROR(KEFIR_NOT_IMPLEMENTED, "Bit-precise integers exceeding widest integer type are not implemented yet"));

            kefir_size_t width = 2;
            const kefir_size_t max_width = sizeof(kefir_uint64_t) * CHAR_BIT;
            for (; width <= max_width; width++) {
                const kefir_int64_t max_value = (1ull << (width - 1)) - 1;
                const kefir_int64_t min_value = -max_value - 1;
                if ((kefir_int64_t) value >= min_value && (kefir_int64_t) value <= max_value) {
                    break;
                }
            }
            REQUIRE(width <= max_width, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unable to determine bit-precise integer width"));
            REQUIRE_OK(kefir_token_new_constant_bit_precise(value, width, token));
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t build_integral_constant(const struct kefir_lexer_context *context,
                                              enum integer_constant_type type, kefir_bool_t decimal,
                                              kefir_uint64_t value, struct kefir_token *token,
                                              const struct kefir_source_location *source_location) {
    const enum integer_constant_type *permitted_types = NULL;
    kefir_size_t permitted_types_length = 0;
    REQUIRE_OK(get_permitted_constant_types(type, decimal, &permitted_types, &permitted_types_length));
    for (kefir_size_t i = 0; i < permitted_types_length; i++) {
        kefir_result_t res = make_integral_constant(context, permitted_types[i], value, token);
        if (res == KEFIR_OK) {
            return KEFIR_OK;
        } else {
            REQUIRE(res == KEFIR_NO_MATCH, res);
        }
    }

    return KEFIR_SET_SOURCE_ERROR(KEFIR_LEXER_ERROR, source_location,
                                  "Provided constant exceeds maximum value of its type");
}

static kefir_result_t next_decimal_constant(struct kefir_lexer_source_cursor *cursor, kefir_uint64_t *value) {
    kefir_char32_t chr = kefir_lexer_source_cursor_at(cursor, 0);
    REQUIRE(kefir_isdigit32(chr) && chr != U'0',
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match decimal integer constant"));

    *value = 0;
    for (; kefir_isdigit32(chr);
         kefir_lexer_source_cursor_next(cursor, 1), chr = kefir_lexer_source_cursor_at(cursor, 0)) {
        *value *= 10;
        *value += chr - U'0';
    }
    return KEFIR_OK;
}

static kefir_result_t next_hexadecimal_constant(struct kefir_lexer_source_cursor *cursor, kefir_uint64_t *value) {
    kefir_char32_t init_chr = kefir_lexer_source_cursor_at(cursor, 0);
    kefir_char32_t init_chr2 = kefir_lexer_source_cursor_at(cursor, 1);
    kefir_char32_t chr = kefir_lexer_source_cursor_at(cursor, 2);
    REQUIRE(init_chr == U'0' && (init_chr2 == U'x' || init_chr2 == U'X') && kefir_ishexdigit32(chr),
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match hexadecimal integer constant"));
    REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));

    *value = 0;
    for (; kefir_ishexdigit32(chr);
         kefir_lexer_source_cursor_next(cursor, 1), chr = kefir_lexer_source_cursor_at(cursor, 0)) {
        *value <<= 4;
        *value += kefir_hex32todec(chr);
    }
    return KEFIR_OK;
}

static kefir_result_t next_binary_constant(struct kefir_lexer_source_cursor *cursor, kefir_uint64_t *value) {
    kefir_char32_t init_chr = kefir_lexer_source_cursor_at(cursor, 0);
    kefir_char32_t init_chr2 = kefir_lexer_source_cursor_at(cursor, 1);
    kefir_char32_t chr = kefir_lexer_source_cursor_at(cursor, 2);
    REQUIRE(init_chr == U'0' && (init_chr2 == U'b' || init_chr2 == U'B') && (chr == U'0' || chr == U'1'),
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match binary integer constant"));
    REQUIRE_OK(kefir_lexer_source_cursor_next(cursor, 2));

    *value = 0;
    for (; chr == U'0' || chr == U'1';
         kefir_lexer_source_cursor_next(cursor, 1), chr = kefir_lexer_source_cursor_at(cursor, 0)) {
        *value <<= 1;
        *value += chr - U'0';
    }
    return KEFIR_OK;
}

static kefir_result_t next_octal_constant(struct kefir_lexer_source_cursor *cursor, kefir_uint64_t *value) {
    kefir_char32_t chr = kefir_lexer_source_cursor_at(cursor, 0);
    REQUIRE(chr == U'0', KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match octal integer constant"));

    *value = 0;
    for (; kefir_isoctdigit32(chr);
         kefir_lexer_source_cursor_next(cursor, 1), chr = kefir_lexer_source_cursor_at(cursor, 0)) {
        *value <<= 3;
        *value += chr - U'0';
    }
    return KEFIR_OK;
}

static kefir_result_t scan_suffix(struct kefir_lexer_source_cursor *cursor,
                                  const struct kefir_lexer_context *lexer_context, kefir_bool_t decimal,
                                  kefir_uint64_t value, struct kefir_token *token,
                                  const struct kefir_source_location *source_location) {
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
        REQUIRE_OK(build_integral_constant(lexer_context, CONSTANT_INT, decimal, value, token, source_location));
    } else {
        REQUIRE_OK(build_integral_constant(lexer_context, matchedSuffix->type, decimal, value, token, source_location));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_scan_integral_constant(struct kefir_lexer_source_cursor *cursor,
                                                  const struct kefir_lexer_context *lexer_context,
                                                  struct kefir_token *token) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source cursor"));
    REQUIRE(lexer_context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer context"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));

    struct kefir_source_location source_location = cursor->location;
    kefir_uint64_t value = 0;
    kefir_bool_t decimal = true;
    kefir_result_t res = next_decimal_constant(cursor, &value);
    if (res == KEFIR_NO_MATCH) {
        decimal = false;
        res = next_hexadecimal_constant(cursor, &value);
    }
    if (res == KEFIR_NO_MATCH) {
        decimal = false;
        res = next_binary_constant(cursor, &value);
    }
    if (res == KEFIR_NO_MATCH) {
        res = next_octal_constant(cursor, &value);
    }
    if (res == KEFIR_NO_MATCH) {
        return KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match integer constant");
    }
    REQUIRE_OK(res);
    REQUIRE_OK(scan_suffix(cursor, lexer_context, decimal, value, token, &source_location));
    return KEFIR_OK;
}

static kefir_result_t match_impl(struct kefir_mem *mem, struct kefir_lexer *lexer, void *payload) {
    UNUSED(mem);
    REQUIRE(lexer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct kefir_token *, token, payload);

    REQUIRE_OK(kefir_lexer_scan_integral_constant(lexer->cursor, lexer->context, token));
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
