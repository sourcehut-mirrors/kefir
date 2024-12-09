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

#include "kefir/lexer/allocator.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include <string.h>

#define ALLOCATOR_CHUNK_CAPACITY 4096
#define CHUNK_SIZEOF(_capacity) (sizeof(struct kefir_token_allocator_chunk) + (_capacity) * sizeof(struct kefir_token))

kefir_result_t kefir_token_allocator_init(struct kefir_token_allocator *allocator) {
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token allocator"));

    allocator->chunk_capacity = ALLOCATOR_CHUNK_CAPACITY;
    allocator->last_chunk = NULL;
    allocator->last_token_index = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_token_allocator_free(struct kefir_mem *mem, struct kefir_token_allocator *allocator) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token allocator"));

    for (struct kefir_token_allocator_chunk *chunk = allocator->last_chunk; chunk != NULL;) {

        const kefir_size_t chunk_length =
            chunk == allocator->last_chunk ? allocator->last_token_index : allocator->chunk_capacity;
        for (kefir_size_t i = 0; i < chunk_length; i++) {
            REQUIRE_OK(kefir_token_free(mem, &chunk->tokens[i]));
        }

        struct kefir_token_allocator_chunk *const prev_chunk = chunk->prev_chunk;
        memset(chunk, 0, sizeof(struct kefir_token_allocator_chunk));
        KEFIR_FREE(mem, chunk);
        chunk = prev_chunk;
    }

    memset(allocator, 0, sizeof(struct kefir_token_allocator));
    return KEFIR_OK;
}

static kefir_result_t allocator_ensure_capacity(struct kefir_mem *mem, struct kefir_token_allocator *allocator) {
    if (allocator->last_chunk == NULL || allocator->last_token_index == allocator->chunk_capacity) {
        struct kefir_token_allocator_chunk *new_chunk = KEFIR_MALLOC(mem, CHUNK_SIZEOF(allocator->chunk_capacity));
        REQUIRE(new_chunk != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate token chunk"));

        new_chunk->prev_chunk = allocator->last_chunk;
        allocator->last_chunk = new_chunk;
        allocator->last_token_index = 0;
    }

    return KEFIR_OK;
}

kefir_result_t kefir_token_allocator_emplace(struct kefir_mem *mem, struct kefir_token_allocator *allocator,
                                              struct kefir_token *token, const struct kefir_token **token_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token allocator"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token"));
    REQUIRE(token_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to allocated token"));

    REQUIRE_OK(allocator_ensure_capacity(mem, allocator));

    REQUIRE_OK(kefir_token_move(&allocator->last_chunk->tokens[allocator->last_token_index], token));
    *token_ptr = &allocator->last_chunk->tokens[allocator->last_token_index++];
    return KEFIR_OK;
}

kefir_result_t kefir_token_allocator_allocate_empty(struct kefir_mem *mem, struct kefir_token_allocator *allocator, struct kefir_token **token_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token allocator"));
    REQUIRE(token_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to allocated token"));

    REQUIRE_OK(allocator_ensure_capacity(mem, allocator));

    REQUIRE_OK(kefir_token_new_sentinel(&allocator->last_chunk->tokens[allocator->last_token_index]));
    *token_ptr = &allocator->last_chunk->tokens[allocator->last_token_index++];
    return KEFIR_OK;
}
