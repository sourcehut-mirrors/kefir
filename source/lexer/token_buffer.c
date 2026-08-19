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

kefir_result_t kefir_token_buffer_init(struct kefir_token_buffer *buffer) {
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));

    buffer->tokens = NULL;
    buffer->length = 0;
    buffer->capacity = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_free(struct kefir_mem *mem, struct kefir_token_buffer *buffer) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));

    KEFIR_FREE(mem, buffer->tokens);
    memset(buffer, 0, sizeof(struct kefir_token_buffer));
    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_reset(struct kefir_mem *mem, struct kefir_token_buffer *buffer) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));

    KEFIR_FREE(mem, buffer->tokens);
    memset(buffer, 0, sizeof(struct kefir_token_buffer));
    return KEFIR_OK;
}

static kefir_uint64_t round_capacity_up(kefir_uint64_t n) {
    if (n <= 1) {
        return n;
    }
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    n++;
    return n;
}


static kefir_result_t ensure_capacity(struct kefir_mem *mem, struct kefir_token_buffer *buffer, kefir_size_t extra) {
    if (buffer->length + extra > buffer->capacity) {
        kefir_size_t new_capacity = round_capacity_up(buffer->capacity + extra);
        new_capacity = MAX(new_capacity, 128);
        const struct kefir_token **new_tokens = KEFIR_REALLOC(mem, buffer->tokens, sizeof(struct kefir_token *) * new_capacity);
        REQUIRE(new_tokens != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate token buffer"));

        buffer->tokens = new_tokens;
        buffer->capacity = new_capacity;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_emplace(struct kefir_mem *mem, struct kefir_token_buffer *buffer,
                                          const struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token"));

    REQUIRE_OK(ensure_capacity(mem, buffer, 1));
    buffer->tokens[buffer->length++] = token;
    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_insert(struct kefir_mem *mem, struct kefir_token_buffer *dst,
                                         struct kefir_token_buffer *src) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination token buffer"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source token buffer"));

    if (src->length > 0) {
        REQUIRE_OK(ensure_capacity(mem, dst, src->length));
        memcpy(&dst->tokens[dst->length], src->tokens, sizeof(struct kefir_token *) * src->length);
        dst->length += src->length;
    }

    KEFIR_FREE(mem, src->tokens);
    memset(src, 0, sizeof(struct kefir_token_buffer));
    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_pop(struct kefir_mem *mem, struct kefir_token_buffer *buffer) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));
    REQUIRE(buffer->length > 0, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Cannot pop token from empty buffer"));

    buffer->length--;
    return KEFIR_OK;
}

#define FLUSH_UNIT 4096

kefir_result_t kefir_token_buffer_flush_front(struct kefir_mem *mem, struct kefir_token_buffer *buffer,
                                              kefir_size_t length, kefir_size_t *flushed_length_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));

    kefir_size_t flush = MIN(buffer->length, length) / FLUSH_UNIT * FLUSH_UNIT;
    ASSIGN_PTR(flushed_length_ptr, flush);
    REQUIRE(flush > 0, KEFIR_OK);
    if (flush < buffer->length) {
        memmove(&buffer->tokens[0], &buffer->tokens[flush], sizeof(struct kefir_token *) * (buffer->length - flush));
    }
    buffer->length -= flush;

    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_copy(struct kefir_mem *mem, struct kefir_token_buffer *dst,
                                       const struct kefir_token_buffer *src) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination token buffer"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source token buffer"));

    if (src->length > 0) {
        REQUIRE_OK(ensure_capacity(mem, dst, src->length));
        memcpy(&dst->tokens[dst->length], src->tokens, sizeof(struct kefir_token *) * src->length);
        dst->length += src->length;
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

    return buffer->tokens[index];
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
