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

#include "kefir/lexer/buffer.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include <string.h>

#define CHUNK_CAPACITY 4096
#define CHUNK_COUNT(_len) (((_len) + CHUNK_CAPACITY - 1) / CHUNK_CAPACITY)
#define CHUNK_INDEX(_idx) ((_idx) / CHUNK_CAPACITY)
#define CHUNK_OFFSET(_idx) ((_idx) % CHUNK_CAPACITY)

kefir_result_t kefir_token_buffer_init(struct kefir_token_buffer *buffer) {
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));

    buffer->token_chunks = NULL;
    buffer->length = 0;
    buffer->chunk_count = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_free(struct kefir_mem *mem, struct kefir_token_buffer *buffer) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));

    for (kefir_size_t i = 0; i < CHUNK_COUNT(buffer->length); i++) {
        KEFIR_FREE(mem, buffer->token_chunks[i]);
    }
    KEFIR_FREE(mem, buffer->token_chunks);
    memset(buffer, 0, sizeof(struct kefir_token_buffer));
    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_reset(struct kefir_mem *mem, struct kefir_token_buffer *buffer) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));

    for (kefir_size_t i = 0; i < buffer->chunk_count; i++) {
        KEFIR_FREE(mem, buffer->token_chunks[i]);
    }
    KEFIR_FREE(mem, buffer->token_chunks);
    memset(buffer, 0, sizeof(struct kefir_token_buffer));
    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_emplace(struct kefir_mem *mem, struct kefir_token_buffer *buffer,
                                          const struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token"));

    if (CHUNK_COUNT(buffer->length + 1) >= buffer->chunk_count) {
        const kefir_size_t new_count = buffer->chunk_count + 1;
        const struct kefir_token ***new_chunks = KEFIR_REALLOC(mem, buffer->token_chunks, sizeof(struct kefir_token **) * new_count);
        REQUIRE(new_chunks != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate token buffer"));
        new_chunks[buffer->chunk_count] = NULL;

        buffer->token_chunks = new_chunks;
        buffer->chunk_count = new_count;
    }

    const kefir_size_t index = CHUNK_INDEX(buffer->length);
    const kefir_size_t offset = CHUNK_OFFSET(buffer->length);
    if (offset == 0) {
        const struct kefir_token **chunk = KEFIR_MALLOC(mem, sizeof(struct kefir_token *) * CHUNK_CAPACITY);
        REQUIRE(chunk != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate token buffer"));

        buffer->token_chunks[index] = chunk;
    }
    
    buffer->token_chunks[index][offset] = token;
    buffer->length++;
    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_insert(struct kefir_mem *mem, struct kefir_token_buffer *dst,
                                         struct kefir_token_buffer *src) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination token buffer"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source token buffer"));

    for (kefir_size_t i = 0; i < kefir_token_buffer_length(src); i++) {
        REQUIRE_OK(kefir_token_buffer_emplace(mem, dst, kefir_token_buffer_at(src, i)));
    }
    REQUIRE_OK(kefir_token_buffer_reset(mem, src));
    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_pop(struct kefir_mem *mem, struct kefir_token_buffer *buffer) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));
    REQUIRE(buffer->length > 0, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Cannot pop token from empty buffer"));

    if (buffer->length % CHUNK_CAPACITY == 1) {
        KEFIR_FREE(mem, buffer->token_chunks[CHUNK_INDEX(buffer->length)]);
    }
    buffer->length--;
    return KEFIR_OK;
}

#define FLUSH_UNIT 4096

kefir_result_t kefir_token_buffer_flush_front(struct kefir_mem *mem, struct kefir_token_buffer *buffer,
                                              kefir_size_t length, kefir_size_t *flushed_length_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));

    const kefir_size_t flush_chunks = MIN(buffer->length, length) / CHUNK_CAPACITY;
    const kefir_size_t flush = flush_chunks * CHUNK_CAPACITY;
    ASSIGN_PTR(flushed_length_ptr, flush);
    REQUIRE(flush_chunks > 0, KEFIR_OK);

    for (kefir_size_t i = 0; i < flush_chunks; i++) {
        KEFIR_FREE(mem, buffer->token_chunks[i]);
    }
    const kefir_size_t move_chunks = CHUNK_COUNT(buffer->length) - flush_chunks;
    if (move_chunks > 0) {
        memmove(&buffer->token_chunks[0], &buffer->token_chunks[flush_chunks], sizeof(struct kefir_token **) * move_chunks);
    }
    buffer->length -= flush;

    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_copy(struct kefir_mem *mem, struct kefir_token_buffer *dst,
                                       const struct kefir_token_buffer *src) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination token buffer"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source token buffer"));

    for (kefir_size_t i = 0; i < kefir_token_buffer_length(src); i++) {
        REQUIRE_OK(kefir_token_buffer_emplace(mem, dst, kefir_token_buffer_at(src, i)));
    }
    return KEFIR_OK;
}

kefir_size_t kefir_token_buffer_length(const struct kefir_token_buffer *buffer) {
    REQUIRE(buffer != NULL, 0);
    
    return buffer->length;
}

const struct kefir_token *kefir_token_buffer_at(const struct kefir_token_buffer *buffer, kefir_size_t index) {
    REQUIRE(buffer != NULL, NULL);
    REQUIRE(index < buffer->length, NULL);

    return buffer->token_chunks[CHUNK_INDEX(index)][CHUNK_OFFSET(index)];
}

static kefir_result_t token_cursor_get_token(kefir_size_t index, const struct kefir_token **token_ptr,
                                             const struct kefir_token_cursor_handle *handle) {
    REQUIRE(token_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE(handle != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token cursor handle"));

    ASSIGN_DECL_CAST(const struct kefir_token_buffer *, tokens, handle->payload[0]);
    *token_ptr = kefir_token_buffer_at(tokens, index);
    return KEFIR_OK;
}

static kefir_result_t token_cursor_flush(const struct kefir_token_cursor_handle *handle, kefir_size_t length) {
    UNUSED(length);
    REQUIRE(handle != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token cursor handle"));

    // Intentionally left blank
    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_cursor_handle(const struct kefir_token_buffer *buffer,
                                                struct kefir_token_cursor_handle *handle_ptr) {
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));
    REQUIRE(handle_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token cursor handle"));

    *handle_ptr = (struct kefir_token_cursor_handle) {
        .get_token = token_cursor_get_token, .flush = token_cursor_flush, .payload = {(kefir_uptr_t) buffer}};
    return KEFIR_OK;
}
