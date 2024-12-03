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

#include "kefir/lexer/buffer.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include <string.h>

static kefir_result_t chunk_free(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                 kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct kefir_token_buffer_chunk *, chunk, value);

    if (chunk != NULL) {
        for (kefir_size_t i = 0; i < chunk->length; i++) {
            REQUIRE_OK(kefir_token_free(mem, &chunk->content[i]));
        }
        KEFIR_FREE(mem, chunk);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_init(struct kefir_token_buffer *buffer) {
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));

    REQUIRE_OK(kefir_hashtree_init(&buffer->chunks, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&buffer->chunks, chunk_free, NULL));
    buffer->length = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_free(struct kefir_mem *mem, struct kefir_token_buffer *buffer) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));

    REQUIRE_OK(kefir_hashtree_free(mem, &buffer->chunks));
    buffer->length = 0;
    return KEFIR_OK;
}

#define INIT_CHUNK_CAPACITY 32
#define MAX_CHUNK_LENGTH 4096
#define CHUNK_SIZEOF(_len) (sizeof(struct kefir_token_buffer_chunk) + (_len) * sizeof(struct kefir_token))

static struct kefir_token_buffer_chunk *last_chunk(struct kefir_token_buffer *buffer) {
    struct kefir_hashtree_node *last_node = NULL;
    kefir_result_t res = kefir_hashtree_max(&buffer->chunks, &last_node);
    if (res == KEFIR_OK && last_node != NULL) {
        return (struct kefir_token_buffer_chunk *) last_node->value;
    } else {
        return NULL;
    }
}

static kefir_result_t ensure_capacity(struct kefir_mem *mem, struct kefir_token_buffer *buffer) {
    struct kefir_hashtree_node *last_node = NULL;
    struct kefir_token_buffer_chunk *chunk = NULL;
    kefir_result_t res = kefir_hashtree_max(&buffer->chunks, &last_node);
    if (res != KEFIR_NOT_FOUND && last_node != NULL) {
        REQUIRE_OK(res);
        chunk = (struct kefir_token_buffer_chunk *) last_node->value;
    }

    if (chunk == NULL || chunk->length >= MAX_CHUNK_LENGTH) {
        chunk = KEFIR_MALLOC(mem, CHUNK_SIZEOF(INIT_CHUNK_CAPACITY));
        REQUIRE(chunk != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate token buffer chunk"));

        chunk->capacity = INIT_CHUNK_CAPACITY;
        chunk->length = 0;

        res = kefir_hashtree_insert(mem, &buffer->chunks, (kefir_hashtree_key_t) buffer->length,
                                    (kefir_hashtree_value_t) chunk);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(mem, chunk);
            return res;
        });
    } else if (chunk->length == chunk->capacity) {
        const kefir_size_t new_capacity = chunk->capacity * 2;
        struct kefir_token_buffer_chunk *new_chunk = KEFIR_REALLOC(mem, chunk, CHUNK_SIZEOF(new_capacity));
        REQUIRE(new_chunk != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to reallocate token buffer chunk"));

        new_chunk->capacity = new_capacity;
        last_node->value = (kefir_hashtree_value_t) new_chunk;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_emplace(struct kefir_mem *mem, struct kefir_token_buffer *buffer,
                                          struct kefir_token *token) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token"));

    REQUIRE_OK(ensure_capacity(mem, buffer));
    struct kefir_token_buffer_chunk *chunk = last_chunk(buffer);
    REQUIRE_OK(kefir_token_move(&chunk->content[chunk->length], token));
    chunk->length++;
    buffer->length++;
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

kefir_result_t kefir_token_buffer_insert(struct kefir_mem *mem, struct kefir_token_buffer *dst,
                                         struct kefir_token_buffer *src) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination token buffer"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source token buffer"));

    struct kefir_hashtree_node *last_node = NULL;
    struct kefir_token_buffer_chunk *last_chunk = NULL;
    kefir_result_t res = kefir_hashtree_max(&dst->chunks, &last_node);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        if (last_node != NULL) {
            last_chunk = (struct kefir_token_buffer_chunk *) last_node->value;
        }
    }

    struct kefir_hashtree_node_iterator iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&src->chunks, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_token_buffer_chunk *, chunk, node->value);

        if (last_chunk != NULL && last_chunk->length + chunk->length < MAX_CHUNK_LENGTH) {
            const kefir_size_t acc_length = last_chunk->length + chunk->length;
            if (acc_length > last_chunk->capacity) {
                const kefir_size_t new_capacity = round_capacity_up(acc_length);
                struct kefir_token_buffer_chunk *new_chunk = KEFIR_REALLOC(mem, last_chunk, CHUNK_SIZEOF(new_capacity));
                REQUIRE(new_chunk != NULL,
                        KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to reallocate token buffer chunk"));
                new_chunk->capacity = new_capacity;
                last_chunk = new_chunk;
                last_node->value = (kefir_hashtree_value_t) new_chunk;
            }

            memcpy(&last_chunk->content[last_chunk->length], chunk->content,
                   sizeof(struct kefir_token) * chunk->length);
            last_chunk->length += chunk->length;
            dst->length += chunk->length;
            KEFIR_FREE(mem, chunk);
            node->value = (kefir_hashtree_value_t) NULL;
            continue;
        }

        if (chunk->length < chunk->capacity) {
            const kefir_size_t new_capacity = round_capacity_up(chunk->length);
            struct kefir_token_buffer_chunk *new_chunk = KEFIR_REALLOC(mem, chunk, CHUNK_SIZEOF(new_capacity));
            REQUIRE(new_chunk != NULL,
                    KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to reallocate token buffer chunk"));
            new_chunk->capacity = new_capacity;
            chunk = new_chunk;
        }

        REQUIRE_OK(kefir_hashtree_insert(mem, &dst->chunks, (kefir_hashtree_key_t) dst->length,
                                         (kefir_hashtree_value_t) chunk));
        dst->length += chunk->length;
        node->value = (kefir_hashtree_value_t) NULL;

        REQUIRE_OK(kefir_hashtree_max(&dst->chunks, &last_node));
        last_chunk = (struct kefir_token_buffer_chunk *) last_node->value;
    }
    REQUIRE_OK(kefir_hashtree_clean(mem, &src->chunks));
    src->length = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_pop(struct kefir_mem *mem, struct kefir_token_buffer *buffer) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));
    REQUIRE(buffer->length > 0, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Cannot pop token from empty buffer"));

    struct kefir_hashtree_node *last_node = NULL;
    REQUIRE_OK(kefir_hashtree_max(&buffer->chunks, &last_node));
    ASSIGN_DECL_CAST(struct kefir_token_buffer_chunk *, chunk, last_node->value);

    REQUIRE_OK(kefir_token_free(mem, &chunk->content[chunk->length - 1]));
    chunk->length--;
    if (chunk->length == 0) {
        REQUIRE_OK(kefir_hashtree_delete(mem, &buffer->chunks, last_node->key));
    }
    buffer->length--;
    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_copy(struct kefir_mem *mem, struct kefir_token_buffer *dst,
                                       const struct kefir_token_buffer *src) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dst != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination token buffer"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source token buffer"));

    for (kefir_size_t i = 0; i < kefir_token_buffer_length(src); i++) {
        struct kefir_token token_copy;
        REQUIRE_OK(kefir_token_copy(mem, &token_copy, kefir_token_buffer_at(src, i)));
        kefir_result_t res = kefir_token_buffer_emplace(mem, dst, &token_copy);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_token_free(mem, &token_copy);
            return res;
        });
    }
    return KEFIR_OK;
}

kefir_size_t kefir_token_buffer_length(const struct kefir_token_buffer *buffer) {
    REQUIRE(buffer != NULL, 0);
    return buffer->length;
}

struct kefir_token *kefir_token_buffer_at(const struct kefir_token_buffer *buffer, kefir_size_t index) {
    REQUIRE(buffer != NULL, NULL);
    REQUIRE(index < buffer->length, NULL);

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_lower_bound(&buffer->chunks, (kefir_hashtree_key_t) index, &node);
    REQUIRE(res == KEFIR_OK, NULL);
    ASSIGN_DECL_CAST(struct kefir_token_buffer_chunk *, chunk, node->value);

    return &chunk->content[index - (kefir_size_t) node->key];
}

static kefir_result_t token_cursor_get_token(kefir_size_t index, const struct kefir_token **token_ptr,
                                             const struct kefir_token_cursor_handle *handle) {
    REQUIRE(token_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE(handle != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token cursor handle"));

    ASSIGN_DECL_CAST(const struct kefir_token_buffer *, tokens, handle->payload[0]);
    *token_ptr = kefir_token_buffer_at(tokens, index);
    return KEFIR_OK;
}

static kefir_result_t token_cursor_length(kefir_size_t *length_ptr, const struct kefir_token_cursor_handle *handle) {
    REQUIRE(length_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to length"));
    REQUIRE(handle != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token cursor handle"));

    ASSIGN_DECL_CAST(const struct kefir_token_buffer *, tokens, handle->payload[0]);
    *length_ptr = kefir_token_buffer_length(tokens);
    return KEFIR_OK;
}

kefir_result_t kefir_token_buffer_cursor_handle(const struct kefir_token_buffer *buffer,
                                                struct kefir_token_cursor_handle *handle_ptr) {
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));
    REQUIRE(handle_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token cursor handle"));

    *handle_ptr = (struct kefir_token_cursor_handle){
        .get_token = token_cursor_get_token, .length = token_cursor_length, .payload = {(kefir_uptr_t) buffer}};
    return KEFIR_OK;
}
