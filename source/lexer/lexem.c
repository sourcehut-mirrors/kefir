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

#include "kefir/lexer/lexem.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/string_buffer.h"
#include "kefir/util/char32.h"
#include <string.h>

kefir_result_t kefir_token_macro_expansions_init(struct kefir_token_macro_expansions *expansions) {
    REQUIRE(expansions != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token macro expansions"));

    REQUIRE_OK(kefir_hashtree_init(&expansions->macro_expansions, &kefir_hashtree_str_ops));
    return KEFIR_OK;
}

kefir_result_t kefir_token_macro_expansions_free(struct kefir_mem *mem,
                                                 struct kefir_token_macro_expansions *expansions) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(expansions != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected token macro expansions"));

    REQUIRE_OK(kefir_hashtree_free(mem, &expansions->macro_expansions));
    return KEFIR_OK;
}

kefir_result_t kefir_token_macro_expansions_add(struct kefir_mem *mem, struct kefir_token_macro_expansions *expansions,
                                                const char *identifier) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(expansions != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token macro expansions"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));

    kefir_result_t res = kefir_hashtree_insert(mem, &expansions->macro_expansions, (kefir_hashtree_key_t) identifier,
                                               (kefir_hashtree_value_t) 0);
    if (res != KEFIR_ALREADY_EXISTS) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_token_macro_expansions_merge(struct kefir_mem *mem, struct kefir_token_macro_expansions *dst,
                                                  const struct kefir_token_macro_expansions *src) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination token macro expansions"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source token macro expansions"));

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&src->macro_expansions, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        REQUIRE_OK(kefir_token_macro_expansions_add(mem, dst, (const char *) node->key));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_token_macro_expansions_move(struct kefir_token_macro_expansions *dst,
                                                 struct kefir_token_macro_expansions *src) {
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination token macro expansions"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source token macro expansions"));

    dst->macro_expansions = src->macro_expansions;
    src->macro_expansions = (struct kefir_hashtree){0};
    return KEFIR_OK;
}

kefir_bool_t kefir_token_macro_expansions_has(const struct kefir_token_macro_expansions *expansions,
                                              const char *identifier) {
    REQUIRE(expansions != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token macro expansions"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));

    return kefir_hashtree_has(&expansions->macro_expansions, (kefir_hashtree_key_t) identifier);
}

kefir_result_t kefir_token_new_sentinel(struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_SENTINEL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_keyword(kefir_keyword_token_t keyword, struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_KEYWORD;
    token->keyword = keyword;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_identifier(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                          const char *identifier, struct kefir_token *token) {
    REQUIRE(identifier != NULL && strlen(identifier) > 0,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE((mem == NULL && symbols == NULL) || (mem != NULL && symbols != NULL),
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                            "Expected both memory allocator and symbol table to be either valid or not"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    if (symbols != NULL) {
        const char *copy = kefir_string_pool_insert(mem, symbols, identifier, NULL);
        REQUIRE(copy != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to insert identifier into symbol table"));
        identifier = copy;
    }
    token->klass = KEFIR_TOKEN_IDENTIFIER;
    token->identifier = identifier;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_int(kefir_int64_t value, struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant.type = KEFIR_CONSTANT_TOKEN_INTEGER;
    token->constant.integer = value;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_uint(kefir_uint64_t value, struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant.type = KEFIR_CONSTANT_TOKEN_UNSIGNED_INTEGER;
    token->constant.uinteger = value;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_long(kefir_int64_t value, struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant.type = KEFIR_CONSTANT_TOKEN_LONG_INTEGER;
    token->constant.integer = value;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_ulong(kefir_uint64_t value, struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant.type = KEFIR_CONSTANT_TOKEN_UNSIGNED_LONG_INTEGER;
    token->constant.uinteger = value;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_long_long(kefir_int64_t value, struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant.type = KEFIR_CONSTANT_TOKEN_LONG_LONG_INTEGER;
    token->constant.integer = value;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_ulong_long(kefir_uint64_t value, struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant.type = KEFIR_CONSTANT_TOKEN_UNSIGNED_LONG_LONG_INTEGER;
    token->constant.integer = value;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_char(kefir_int_t value, struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant.type = KEFIR_CONSTANT_TOKEN_CHAR;
    token->constant.character = value;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_wide_char(kefir_wchar_t value, struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant.type = KEFIR_CONSTANT_TOKEN_WIDE_CHAR;
    token->constant.wide_char = value;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_unicode16_char(kefir_char16_t value, struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant.type = KEFIR_CONSTANT_TOKEN_UNICODE16_CHAR;
    token->constant.unicode16_char = value;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_unicode32_char(kefir_char32_t value, struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant.type = KEFIR_CONSTANT_TOKEN_UNICODE32_CHAR;
    token->constant.unicode32_char = value;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_float(kefir_float32_t value, struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant.type = KEFIR_CONSTANT_TOKEN_FLOAT;
    token->constant.float32 = value;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_double(kefir_float64_t value, struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant.type = KEFIR_CONSTANT_TOKEN_DOUBLE;
    token->constant.float64 = value;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_long_double(kefir_long_double_t value, struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant.type = KEFIR_CONSTANT_TOKEN_LONG_DOUBLE;
    token->constant.long_double = value;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_string_literal_multibyte(struct kefir_mem *mem, const char *content, kefir_size_t length,
                                                        struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    char *dest_content = KEFIR_MALLOC(mem, length);
    REQUIRE(dest_content != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate string literal"));
    memcpy(dest_content, content, length);
    token->klass = KEFIR_TOKEN_STRING_LITERAL;
    token->string_literal.raw_literal = false;
    token->string_literal.type = KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE;
    token->string_literal.literal = dest_content;
    token->string_literal.length = length;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_string_literal_unicode8(struct kefir_mem *mem, const char *content, kefir_size_t length,
                                                       struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    char *dest_content = KEFIR_MALLOC(mem, length);
    REQUIRE(dest_content != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate unicode8 string literal"));
    memcpy(dest_content, content, length);
    token->klass = KEFIR_TOKEN_STRING_LITERAL;
    token->string_literal.raw_literal = false;
    token->string_literal.type = KEFIR_STRING_LITERAL_TOKEN_UNICODE8;
    token->string_literal.literal = dest_content;
    token->string_literal.length = length;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_string_literal_unicode16(struct kefir_mem *mem, const kefir_char16_t *content,
                                                        kefir_size_t length, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    kefir_size_t sz = sizeof(kefir_char16_t) * length;
    char *dest_content = KEFIR_MALLOC(mem, sz);
    REQUIRE(dest_content != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate unicode16 string literal"));
    memcpy(dest_content, content, sz);
    token->klass = KEFIR_TOKEN_STRING_LITERAL;
    token->string_literal.raw_literal = false;
    token->string_literal.type = KEFIR_STRING_LITERAL_TOKEN_UNICODE16;
    token->string_literal.literal = dest_content;
    token->string_literal.length = length;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_string_literal_unicode32(struct kefir_mem *mem, const kefir_char32_t *content,
                                                        kefir_size_t length, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    kefir_size_t sz = sizeof(kefir_char32_t) * length;
    char *dest_content = KEFIR_MALLOC(mem, sz);
    REQUIRE(dest_content != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate unicode32 string literal"));
    memcpy(dest_content, content, sz);
    token->klass = KEFIR_TOKEN_STRING_LITERAL;
    token->string_literal.raw_literal = false;
    token->string_literal.type = KEFIR_STRING_LITERAL_TOKEN_UNICODE32;
    token->string_literal.literal = dest_content;
    token->string_literal.length = length;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_string_literal_wide(struct kefir_mem *mem, const kefir_wchar_t *content,
                                                   kefir_size_t length, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    kefir_size_t sz = sizeof(kefir_wchar_t) * length;
    char *dest_content = KEFIR_MALLOC(mem, sz);
    REQUIRE(dest_content != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate wide string literal"));
    memcpy(dest_content, content, sz);
    token->klass = KEFIR_TOKEN_STRING_LITERAL;
    token->string_literal.raw_literal = false;
    token->string_literal.type = KEFIR_STRING_LITERAL_TOKEN_WIDE;
    token->string_literal.literal = dest_content;
    token->string_literal.length = length;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_string_literal_raw(struct kefir_mem *mem, kefir_string_literal_token_type_t type,
                                                  const kefir_char32_t *content, kefir_size_t length,
                                                  struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    kefir_size_t sz = sizeof(kefir_char32_t) * length;
    char *dest_content = KEFIR_MALLOC(mem, sz);
    REQUIRE(dest_content != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate unicode32 string literal"));
    memcpy(dest_content, content, sz);
    token->klass = KEFIR_TOKEN_STRING_LITERAL;
    token->string_literal.raw_literal = true;
    token->string_literal.type = type;
    token->string_literal.literal = dest_content;
    token->string_literal.length = length;
    return KEFIR_OK;
}

kefir_bool_t kefir_token_string_literal_type_concat(kefir_string_literal_token_type_t type1,
                                                    kefir_string_literal_token_type_t type2,
                                                    kefir_string_literal_token_type_t *result) {
    switch (type2) {
        case KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE:
            *result = type1;
            break;

        case KEFIR_STRING_LITERAL_TOKEN_UNICODE8:
            if (type1 != KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE && type1 != KEFIR_STRING_LITERAL_TOKEN_UNICODE8) {
                return false;
            }
            *result = type2;
            break;

        case KEFIR_STRING_LITERAL_TOKEN_UNICODE16:
            if (type1 != KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE && type1 != KEFIR_STRING_LITERAL_TOKEN_UNICODE16) {
                return false;
            }
            *result = type2;
            break;

        case KEFIR_STRING_LITERAL_TOKEN_UNICODE32:
            if (type1 != KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE && type1 != KEFIR_STRING_LITERAL_TOKEN_UNICODE32) {
                return false;
            }
            *result = type2;
            break;

        case KEFIR_STRING_LITERAL_TOKEN_WIDE:
            if (type1 != KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE && type1 != KEFIR_STRING_LITERAL_TOKEN_WIDE) {
                return false;
            }
            *result = type2;
            break;
    }
    return true;
}

kefir_result_t kefir_token_new_punctuator(kefir_punctuator_token_t punctuator, struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_PUNCTUATOR;
    token->punctuator = punctuator;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_pp_whitespace(kefir_bool_t newline, struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_PP_WHITESPACE;
    token->pp_whitespace.newline = newline;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_pp_number(struct kefir_mem *mem, const char *number_literal, kefir_size_t length,
                                         struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(number_literal != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid number literal"));
    REQUIRE(length > 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid non-zero length of the literal"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    char *clone_number_literal = KEFIR_MALLOC(mem, length + 1);
    REQUIRE(clone_number_literal != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate pp number literal"));
    memcpy(clone_number_literal, number_literal, length);
    clone_number_literal[length] = '\0';
    token->klass = KEFIR_TOKEN_PP_NUMBER;
    token->pp_number.number_literal = clone_number_literal;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_pp_header_name(struct kefir_mem *mem, kefir_bool_t system, const char *header_name,
                                              kefir_size_t length, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(header_name != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid header name"));
    REQUIRE(length > 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid non-zero length of the literal"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    char *clone_header_name = KEFIR_MALLOC(mem, length + 1);
    REQUIRE(clone_header_name != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate pp header name"));
    memcpy(clone_header_name, header_name, length);
    clone_header_name[length] = '\0';
    token->klass = KEFIR_TOKEN_PP_HEADER_NAME;
    token->pp_header_name.system = system;
    token->pp_header_name.header_name = clone_header_name;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_extension(const struct kefir_token_extension_class *extension_class, void *payload,
                                         struct kefir_token *token) {
    REQUIRE(extension_class != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid extension class"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_init(&token->macro_expansions));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->klass = KEFIR_TOKEN_EXTENSION;
    token->extension.klass = extension_class;
    token->extension.payload = payload;
    return KEFIR_OK;
}

kefir_result_t kefir_token_move(struct kefir_token *dst, struct kefir_token *src) {
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to destination token"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to source token"));
    *dst = *src;
    if (src->klass == KEFIR_TOKEN_STRING_LITERAL) {
        src->string_literal.literal = NULL;
        src->string_literal.length = 0;
    } else if (src->klass == KEFIR_TOKEN_PP_NUMBER) {
        src->pp_number.number_literal = NULL;
    } else if (src->klass == KEFIR_TOKEN_PP_HEADER_NAME) {
        src->pp_header_name.header_name = NULL;
    } else if (src->klass == KEFIR_TOKEN_EXTENSION) {
        src->extension.klass = NULL;
        src->extension.payload = NULL;
    }
    REQUIRE_OK(kefir_token_macro_expansions_move(&dst->macro_expansions, &src->macro_expansions));
    return KEFIR_OK;
}

kefir_result_t kefir_token_copy(struct kefir_mem *mem, struct kefir_token *dst, const struct kefir_token *src) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to destination token"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to source token"));

    *dst = *src;
    if (src->klass == KEFIR_TOKEN_STRING_LITERAL) {
        kefir_size_t sz = 0;
        if (src->string_literal.raw_literal) {
            sz = sizeof(kefir_char32_t) * src->string_literal.length;
        } else {
            switch (src->string_literal.type) {
                case KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE:
                case KEFIR_STRING_LITERAL_TOKEN_UNICODE8:
                    sz = src->string_literal.length;
                    break;

                case KEFIR_STRING_LITERAL_TOKEN_UNICODE16:
                    sz = sizeof(kefir_char16_t) * src->string_literal.length;
                    break;

                case KEFIR_STRING_LITERAL_TOKEN_UNICODE32:
                    sz = sizeof(kefir_char32_t) * src->string_literal.length;
                    break;

                case KEFIR_STRING_LITERAL_TOKEN_WIDE:
                    sz = sizeof(kefir_wchar_t) * src->string_literal.length;
                    break;
            }
        }

        char *content = KEFIR_MALLOC(mem, sz);
        REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate string literal"));
        memcpy(content, src->string_literal.literal, sz);
        dst->string_literal.raw_literal = src->string_literal.raw_literal;
        dst->string_literal.type = src->string_literal.type;
        dst->string_literal.literal = content;
        dst->string_literal.length = src->string_literal.length;
    } else if (src->klass == KEFIR_TOKEN_PP_NUMBER) {
        char *clone_number_literal = KEFIR_MALLOC(mem, strlen(src->pp_number.number_literal) + 1);
        REQUIRE(clone_number_literal != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate pp number literal"));
        strcpy(clone_number_literal, src->pp_number.number_literal);
        dst->pp_number.number_literal = clone_number_literal;
    } else if (src->klass == KEFIR_TOKEN_PP_HEADER_NAME) {
        char *clone_header_name = KEFIR_MALLOC(mem, strlen(src->pp_header_name.header_name) + 1);
        REQUIRE(clone_header_name != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate pp header name"));
        strcpy(clone_header_name, src->pp_header_name.header_name);
        dst->pp_header_name.header_name = clone_header_name;
    } else if (src->klass == KEFIR_TOKEN_EXTENSION) {
        REQUIRE(src->extension.klass != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Invalid extension token"));
        REQUIRE_OK(src->extension.klass->copy(mem, dst, src));
    }

    REQUIRE_OK(kefir_token_macro_expansions_init(&dst->macro_expansions));
    kefir_result_t res = kefir_token_macro_expansions_merge(mem, &dst->macro_expansions, &src->macro_expansions);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_token_macro_expansions_free(mem, &dst->macro_expansions);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_token_free(struct kefir_mem *mem, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_token_macro_expansions_free(mem, &token->macro_expansions));
    switch (token->klass) {
        case KEFIR_TOKEN_STRING_LITERAL:
            KEFIR_FREE(mem, token->string_literal.literal);
            token->string_literal.literal = NULL;
            token->string_literal.length = 0;
            break;

        case KEFIR_TOKEN_PP_NUMBER:
            KEFIR_FREE(mem, (void *) token->pp_number.number_literal);
            token->pp_number.number_literal = NULL;
            break;

        case KEFIR_TOKEN_PP_HEADER_NAME:
            KEFIR_FREE(mem, (void *) token->pp_header_name.header_name);
            token->pp_header_name.header_name = NULL;
            break;

        case KEFIR_TOKEN_EXTENSION:
            if (token->extension.klass != NULL) {
                REQUIRE_OK(token->extension.klass->free(mem, token));
                token->extension.klass = NULL;
                token->extension.payload = NULL;
            }
            break;

        case KEFIR_TOKEN_IDENTIFIER:
        case KEFIR_TOKEN_SENTINEL:
        case KEFIR_TOKEN_KEYWORD:
        case KEFIR_TOKEN_CONSTANT:
        case KEFIR_TOKEN_PUNCTUATOR:
        case KEFIR_TOKEN_PP_WHITESPACE:
        case KEFIR_TOKEN_PP_PLACEMAKER:
            break;
    }
    return KEFIR_OK;
}
