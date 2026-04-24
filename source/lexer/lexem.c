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

kefir_result_t kefir_token_macro_expansions_add(struct kefir_mem *mem, struct kefir_token_macro_expansions **expansions,
                                                const char *identifier) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(expansions != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token macro expansions"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));

    if (*expansions == NULL) {
        *expansions = KEFIR_MALLOC(mem, sizeof(struct kefir_token_macro_expansions));
        kefir_result_t res = kefir_token_macro_expansions_init(*expansions);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, *expansions);
            return res;
        });
    }

    kefir_result_t res = kefir_hashtree_insert(mem, &(*expansions)->macro_expansions, (kefir_hashtree_key_t) identifier,
                                               (kefir_hashtree_value_t) 0);
    if (res != KEFIR_ALREADY_EXISTS) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_token_macro_expansions_merge(struct kefir_mem *mem, struct kefir_token_macro_expansions **dst,
                                                  const struct kefir_token_macro_expansions *src) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination token macro expansions"));

    if (src != NULL) {
        struct kefir_hashtree_node_iterator iter;
        for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&src->macro_expansions, &iter); node != NULL;
             node = kefir_hashtree_next(&iter)) {
            REQUIRE_OK(kefir_token_macro_expansions_add(mem, dst, (const char *) node->key));
        }
    }
    return KEFIR_OK;
}

kefir_bool_t kefir_token_macro_expansions_has(const struct kefir_token_macro_expansions *expansions,
                                              const char *identifier) {
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));

    return expansions != NULL && kefir_hashtree_has(&expansions->macro_expansions, (kefir_hashtree_key_t) identifier);
}

kefir_result_t kefir_token_new_sentinel(struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_SENTINEL;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_keyword(kefir_keyword_token_t keyword, struct kefir_token *token) {
    return kefir_token_new_keyword_with_spelling(keyword, NULL, token);
}

kefir_result_t kefir_token_new_keyword_with_spelling(kefir_keyword_token_t keyword, const char *spelling,
                                                     struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_KEYWORD;
    token->keyword = keyword;
    token->keyword_spelling = spelling;
    token->macro_expansions = NULL;
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
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    if (symbols != NULL) {
        const char *copy = kefir_string_pool_insert(mem, symbols, identifier, NULL);
        REQUIRE(copy != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to insert identifier into symbol table"));
        identifier = copy;
    }
    token->klass = KEFIR_TOKEN_IDENTIFIER;
    token->identifier = identifier;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_int(struct kefir_mem *mem, kefir_int64_t value, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_INTEGER;
    token->constant->integer = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_uint(struct kefir_mem *mem, kefir_uint64_t value, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_UNSIGNED_INTEGER;
    token->constant->uinteger = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_long(struct kefir_mem *mem, kefir_int64_t value, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_LONG_INTEGER;
    token->constant->integer = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_ulong(struct kefir_mem *mem, kefir_uint64_t value, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_UNSIGNED_LONG_INTEGER;
    token->constant->uinteger = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_long_long(struct kefir_mem *mem, kefir_int64_t value,
                                                  struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_LONG_LONG_INTEGER;
    token->constant->integer = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_ulong_long(struct kefir_mem *mem, kefir_uint64_t value,
                                                   struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_UNSIGNED_LONG_LONG_INTEGER;
    token->constant->integer = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_bit_precise(struct kefir_mem *mem, struct kefir_bigint *bigint,
                                                    struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_BIT_PRECISE;
    REQUIRE_OK(kefir_bigint_move(&token->constant->bitprecise, bigint));
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_unsigned_bit_precise(struct kefir_mem *mem, struct kefir_bigint *bigint,
                                                             struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_UNSIGNED_BIT_PRECISE;
    REQUIRE_OK(kefir_bigint_move(&token->constant->bitprecise, bigint));
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_char(struct kefir_mem *mem, kefir_int_t value, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_CHAR;
    token->constant->character = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_unicode8_char(struct kefir_mem *mem, kefir_int_t value,
                                                      struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_UNICODE8_CHAR;
    token->constant->character = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_wide_char(struct kefir_mem *mem, kefir_wchar_t value,
                                                  struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_WIDE_CHAR;
    token->constant->wide_char = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_unicode16_char(struct kefir_mem *mem, kefir_char16_t value,
                                                       struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_UNICODE16_CHAR;
    token->constant->unicode16_char = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_unicode32_char(struct kefir_mem *mem, kefir_char32_t value,
                                                       struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_UNICODE32_CHAR;
    token->constant->unicode32_char = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_float(struct kefir_mem *mem, kefir_float32_t value, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_FLOAT;
    token->constant->float32 = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_float32(struct kefir_mem *mem, kefir_float32_t value,
                                                struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_FLOAT32;
    token->constant->float32 = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_float32x(struct kefir_mem *mem, kefir_float64_t value,
                                                 struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_FLOAT32X;
    token->constant->float64 = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_double(struct kefir_mem *mem, kefir_float64_t value,
                                               struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_DOUBLE;
    token->constant->float64 = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_float64(struct kefir_mem *mem, kefir_float64_t value,
                                                struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_FLOAT64;
    token->constant->float64 = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_float64x(struct kefir_mem *mem, kefir_long_double_t value,
                                                 struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_FLOAT64X;
    token->constant->long_double = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_long_double(struct kefir_mem *mem, kefir_long_double_t value,
                                                    struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_LONG_DOUBLE;
    token->constant->long_double = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_float80(struct kefir_mem *mem, kefir_long_double_t value,
                                                struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_FLOAT80;
    token->constant->long_double = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_decimal32(struct kefir_mem *mem, kefir_dfp_decimal32_t value,
                                                  struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_DECIMAL32;
    token->constant->decimal32 = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_decimal64(struct kefir_mem *mem, kefir_dfp_decimal64_t value,
                                                  struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_DECIMAL64;
    token->constant->decimal64 = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_decimal128(struct kefir_mem *mem, kefir_dfp_decimal128_t value,
                                                   struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_DECIMAL128;
    token->constant->decimal128 = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_decimal64x(struct kefir_mem *mem, kefir_dfp_decimal128_t value,
                                                   struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_DECIMAL64X;
    token->constant->decimal128 = value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_complex_float(struct kefir_mem *mem, kefir_float32_t real_value,
                                                      kefir_float32_t imaginary_value, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_COMPLEX_FLOAT;
    token->constant->complex_float32.real = real_value;
    token->constant->complex_float32.imaginary = imaginary_value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_complex_float32(struct kefir_mem *mem, kefir_float32_t real_value,
                                                        kefir_float32_t imaginary_value, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_COMPLEX_FLOAT32;
    token->constant->complex_float32.real = real_value;
    token->constant->complex_float32.imaginary = imaginary_value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_complex_double(struct kefir_mem *mem, kefir_float64_t real_value,
                                                       kefir_float64_t imaginary_value, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_COMPLEX_DOUBLE;
    token->constant->complex_float64.real = real_value;
    token->constant->complex_float64.imaginary = imaginary_value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_complex_float32x(struct kefir_mem *mem, kefir_float64_t real_value,
                                                         kefir_float64_t imaginary_value, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_COMPLEX_FLOAT32X;
    token->constant->complex_float64.real = real_value;
    token->constant->complex_float64.imaginary = imaginary_value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_complex_float64(struct kefir_mem *mem, kefir_float64_t real_value,
                                                        kefir_float64_t imaginary_value, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_COMPLEX_FLOAT64;
    token->constant->complex_float64.real = real_value;
    token->constant->complex_float64.imaginary = imaginary_value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_complex_long_double(struct kefir_mem *mem, kefir_long_double_t real_value,
                                                            kefir_long_double_t imaginary_value,
                                                            struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_COMPLEX_LONG_DOUBLE;
    token->constant->complex_long_double.real = real_value;
    token->constant->complex_long_double.imaginary = imaginary_value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_complex_float64x(struct kefir_mem *mem, kefir_long_double_t real_value,
                                                         kefir_long_double_t imaginary_value,
                                                         struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_COMPLEX_FLOAT64X;
    token->constant->complex_long_double.real = real_value;
    token->constant->complex_long_double.imaginary = imaginary_value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_constant_complex_float80(struct kefir_mem *mem, kefir_long_double_t real_value,
                                                        kefir_long_double_t imaginary_value,
                                                        struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memoyr allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
    REQUIRE(token->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
    token->klass = KEFIR_TOKEN_CONSTANT;
    token->constant->type = KEFIR_CONSTANT_TOKEN_COMPLEX_FLOAT80;
    token->constant->complex_long_double.real = real_value;
    token->constant->complex_long_double.imaginary = imaginary_value;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_string_literal_multibyte(struct kefir_mem *mem, const char *content, kefir_size_t length,
                                                        struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->string_literal = KEFIR_MALLOC(mem, sizeof(struct kefir_string_literal_token) + length);
    REQUIRE(token->string_literal != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate string literal token"));
    memcpy(token->string_literal->literal, content, length);

    token->klass = KEFIR_TOKEN_STRING_LITERAL;
    token->string_literal->raw_literal = false;
    token->string_literal->type = KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE;
    token->string_literal->length = length;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_string_literal_unicode8(struct kefir_mem *mem, const char *content, kefir_size_t length,
                                                       struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->string_literal = KEFIR_MALLOC(mem, sizeof(struct kefir_string_literal_token) + length);
    REQUIRE(token->string_literal != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate string literal token"));
    memcpy(token->string_literal->literal, content, length);

    token->klass = KEFIR_TOKEN_STRING_LITERAL;
    token->string_literal->raw_literal = false;
    token->string_literal->type = KEFIR_STRING_LITERAL_TOKEN_UNICODE8;
    token->string_literal->length = length;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_string_literal_unicode16(struct kefir_mem *mem, const kefir_char16_t *content,
                                                        kefir_size_t length, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    kefir_size_t sz = sizeof(kefir_char16_t) * length;
    token->string_literal = KEFIR_MALLOC(mem, sizeof(struct kefir_string_literal_token) + sz);
    REQUIRE(token->string_literal != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate string literal token"));
    memcpy(token->string_literal->literal, content, sz);

    token->klass = KEFIR_TOKEN_STRING_LITERAL;
    token->string_literal->raw_literal = false;
    token->string_literal->type = KEFIR_STRING_LITERAL_TOKEN_UNICODE16;
    token->string_literal->length = length;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_string_literal_unicode32(struct kefir_mem *mem, const kefir_char32_t *content,
                                                        kefir_size_t length, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    kefir_size_t sz = sizeof(kefir_char32_t) * length;
    token->string_literal = KEFIR_MALLOC(mem, sizeof(struct kefir_string_literal_token) + sz);
    REQUIRE(token->string_literal != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate string literal token"));
    memcpy(token->string_literal->literal, content, sz);

    token->klass = KEFIR_TOKEN_STRING_LITERAL;
    token->string_literal->raw_literal = false;
    token->string_literal->type = KEFIR_STRING_LITERAL_TOKEN_UNICODE32;
    token->string_literal->length = length;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_string_literal_wide(struct kefir_mem *mem, const kefir_wchar_t *content,
                                                   kefir_size_t length, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    kefir_size_t sz = sizeof(kefir_wchar_t) * length;
    token->string_literal = KEFIR_MALLOC(mem, sizeof(struct kefir_string_literal_token) + sz);
    REQUIRE(token->string_literal != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate string literal token"));
    memcpy(token->string_literal->literal, content, sz);

    token->klass = KEFIR_TOKEN_STRING_LITERAL;
    token->string_literal->raw_literal = false;
    token->string_literal->type = KEFIR_STRING_LITERAL_TOKEN_WIDE;
    token->string_literal->length = length;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_string_literal_raw(struct kefir_mem *mem, kefir_string_literal_token_type_t type,
                                                  const kefir_char32_t *content, kefir_size_t length,
                                                  struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string literal"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    kefir_size_t sz = sizeof(kefir_char32_t) * length;
    token->string_literal = KEFIR_MALLOC(mem, sizeof(struct kefir_string_literal_token) + sz);
    REQUIRE(token->string_literal != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate string literal token"));
    memcpy(token->string_literal->literal, content, sz);

    token->klass = KEFIR_TOKEN_STRING_LITERAL;
    token->string_literal->raw_literal = true;
    token->string_literal->type = type;
    token->string_literal->length = length;
    token->macro_expansions = NULL;
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
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_PUNCTUATOR;
    token->punctuator = punctuator;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_pragma(struct kefir_mem *mem, kefir_pragma_token_type_t pragma,
                                      kefir_pragma_token_parameter_t param, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->pragma = KEFIR_MALLOC(mem, sizeof(struct kefir_pragma_token));
    REQUIRE_ELSE(token->pragma != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate pragma token"));

    token->klass = KEFIR_TOKEN_PRAGMA;
    token->pragma->pragma = pragma;
    token->pragma->pragma_param = param;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_pp_whitespace(kefir_bool_t newline, struct kefir_token *token) {
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));
    token->klass = KEFIR_TOKEN_PP_WHITESPACE;
    token->pp_whitespace.newline = newline;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_pp_number(struct kefir_mem *mem, const char *number_literal, kefir_size_t length,
                                         struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(number_literal != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid number literal"));
    REQUIRE(length > 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid non-zero length of the literal"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    char *clone_number_literal = KEFIR_MALLOC(mem, length + 1);
    REQUIRE(clone_number_literal != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate pp number literal"));
    memcpy(clone_number_literal, number_literal, length);
    clone_number_literal[length] = '\0';
    token->klass = KEFIR_TOKEN_PP_NUMBER;
    token->pp_number.number_literal = clone_number_literal;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_pp_header_name(struct kefir_mem *mem, kefir_bool_t system, const char *header_name,
                                              kefir_size_t length, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(header_name != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid header name"));
    REQUIRE(length > 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid non-zero length of the literal"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->pp_header_name = KEFIR_MALLOC(mem, sizeof(struct kefir_pptoken_pp_header_name) + length + 1);
    REQUIRE(token->pp_header_name != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate pp header name"));
    memcpy(token->pp_header_name->header_name, header_name, length);
    token->pp_header_name->header_name[length] = '\0';

    token->klass = KEFIR_TOKEN_PP_HEADER_NAME;
    token->pp_header_name->system = system;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_new_extension(struct kefir_mem *mem,
                                         const struct kefir_token_extension_class *extension_class, void *payload,
                                         struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(extension_class != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid extension class"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE_OK(kefir_source_location_empty(&token->source_location));

    token->extension = KEFIR_MALLOC(mem, sizeof(struct kefir_token_extension));
    REQUIRE(token->extension != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate extension token"));

    token->klass = KEFIR_TOKEN_EXTENSION;
    token->extension->klass = extension_class;
    token->extension->payload = payload;
    token->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_move(struct kefir_token *dst, struct kefir_token *src) {
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to destination token"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to source token"));
    memcpy(dst, src, sizeof(struct kefir_token));
    src->klass = KEFIR_TOKEN_SENTINEL;
    src->macro_expansions = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_token_copy(struct kefir_mem *mem, struct kefir_token *dst, const struct kefir_token *src) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to destination token"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to source token"));

    memcpy(dst, src, sizeof(struct kefir_token));
    if (src->klass == KEFIR_TOKEN_STRING_LITERAL) {
        kefir_size_t sz = 0;
        if (src->string_literal->raw_literal) {
            sz = sizeof(kefir_char32_t) * src->string_literal->length;
        } else {
            switch (src->string_literal->type) {
                case KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE:
                case KEFIR_STRING_LITERAL_TOKEN_UNICODE8:
                    sz = src->string_literal->length;
                    break;

                case KEFIR_STRING_LITERAL_TOKEN_UNICODE16:
                    sz = sizeof(kefir_char16_t) * src->string_literal->length;
                    break;

                case KEFIR_STRING_LITERAL_TOKEN_UNICODE32:
                    sz = sizeof(kefir_char32_t) * src->string_literal->length;
                    break;

                case KEFIR_STRING_LITERAL_TOKEN_WIDE:
                    sz = sizeof(kefir_wchar_t) * src->string_literal->length;
                    break;
            }
        }

        dst->string_literal = KEFIR_MALLOC(mem, sizeof(struct kefir_string_literal_token) + sz);
        REQUIRE(dst->string_literal != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate string literal"));
        memcpy(dst->string_literal->literal, src->string_literal->literal, sz);

        dst->string_literal->raw_literal = src->string_literal->raw_literal;
        dst->string_literal->type = src->string_literal->type;
        dst->string_literal->length = src->string_literal->length;
    } else if (src->klass == KEFIR_TOKEN_PP_NUMBER) {
        char *clone_number_literal = KEFIR_MALLOC(mem, strlen(src->pp_number.number_literal) + 1);
        REQUIRE(clone_number_literal != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate pp number literal"));
        strcpy(clone_number_literal, src->pp_number.number_literal);
        dst->pp_number.number_literal = clone_number_literal;
    } else if (src->klass == KEFIR_TOKEN_PP_HEADER_NAME) {
        dst->pp_header_name = KEFIR_MALLOC(
            mem, sizeof(struct kefir_pptoken_pp_header_name) + strlen(src->pp_header_name->header_name) + 1);
        REQUIRE(dst->pp_header_name != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate pp header name"));
        strcpy(dst->pp_header_name->header_name, src->pp_header_name->header_name);

        dst->pp_header_name->system = src->pp_header_name->system;
    } else if (src->klass == KEFIR_TOKEN_EXTENSION) {
        REQUIRE(src->extension->klass != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Invalid extension token"));
        REQUIRE_OK(src->extension->klass->copy(mem, dst, src));
    } else if (src->klass == KEFIR_TOKEN_CONSTANT) {
        dst->constant = KEFIR_MALLOC(mem, sizeof(struct kefir_constant_token));
        REQUIRE(dst->constant != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate constant token"));
        memcpy(dst->constant, src->constant, sizeof(struct kefir_constant_token));
    } else if (src->klass == KEFIR_TOKEN_PRAGMA) {
        dst->pragma = KEFIR_MALLOC(mem, sizeof(struct kefir_pragma_token));
        REQUIRE(dst->pragma != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate pragma token"));
        memcpy(dst->pragma, src->pragma, sizeof(struct kefir_pragma_token));
    }

    if (src->macro_expansions != NULL) {
        dst->macro_expansions = KEFIR_MALLOC(mem, sizeof(struct kefir_token_macro_expansions));
        REQUIRE(dst->macro_expansions != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate token macro expansions"));

        kefir_result_t res = kefir_token_macro_expansions_init(dst->macro_expansions);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, dst->macro_expansions);
            return res;
        });
        res = kefir_token_macro_expansions_merge(mem, &dst->macro_expansions, src->macro_expansions);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_token_macro_expansions_free(mem, dst->macro_expansions);
            KEFIR_FREE(mem, dst->macro_expansions);
            return res;
        });
    } else {
        dst->macro_expansions = NULL;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_token_free(struct kefir_mem *mem, struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    if (token->macro_expansions != NULL) {
        REQUIRE_OK(kefir_token_macro_expansions_free(mem, token->macro_expansions));
        KEFIR_FREE(mem, token->macro_expansions);
        token->macro_expansions = NULL;
    }
    switch (token->klass) {
        case KEFIR_TOKEN_STRING_LITERAL:
            token->string_literal->length = 0;
            KEFIR_FREE(mem, token->string_literal);
            token->string_literal = NULL;
            break;

        case KEFIR_TOKEN_PP_NUMBER:
            KEFIR_FREE(mem, (void *) token->pp_number.number_literal);
            token->pp_number.number_literal = NULL;
            break;

        case KEFIR_TOKEN_PP_HEADER_NAME:
            KEFIR_FREE(mem, token->pp_header_name);
            break;

        case KEFIR_TOKEN_EXTENSION:
            if (token->extension->klass != NULL) {
                REQUIRE_OK(token->extension->klass->free(mem, token));
                token->extension->klass = NULL;
                token->extension->payload = NULL;
            }
            KEFIR_FREE(mem, token->extension);
            break;

        case KEFIR_TOKEN_CONSTANT:
            if (token->constant->type == KEFIR_CONSTANT_TOKEN_BIT_PRECISE ||
                token->constant->type == KEFIR_CONSTANT_TOKEN_UNSIGNED_BIT_PRECISE) {
                REQUIRE_OK(kefir_bigint_free(mem, &token->constant->bitprecise));
            }
            KEFIR_FREE(mem, token->constant);
            token->constant = NULL;
            break;

        case KEFIR_TOKEN_IDENTIFIER:
        case KEFIR_TOKEN_SENTINEL:
        case KEFIR_TOKEN_KEYWORD:
        case KEFIR_TOKEN_PUNCTUATOR:
        case KEFIR_TOKEN_PRAGMA:
        case KEFIR_TOKEN_PP_WHITESPACE:
        case KEFIR_TOKEN_PP_PLACEMAKER:
            break;
    }
    return KEFIR_OK;
}
