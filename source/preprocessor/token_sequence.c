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

#include "kefir/preprocessor/token_sequence.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

struct buffer_element {
    struct kefir_token_buffer buffer;
    kefir_size_t index;
    kefir_preprocessor_token_destination_t token_destination;
};

static kefir_result_t free_token_buffer(struct kefir_mem *mem, struct kefir_list *list, struct kefir_list_entry *entry,
                                        void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));
    ASSIGN_DECL_CAST(struct buffer_element *, elt, entry->value);

    REQUIRE_OK(kefir_token_buffer_free(mem, &elt->buffer));
    KEFIR_FREE(mem, elt);
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_token_sequence_init(struct kefir_preprocessor_token_sequence *seq,
                                                      const struct kefir_preprocessor_token_sequence_source *source) {
    REQUIRE(seq != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token sequence"));

    REQUIRE_OK(kefir_list_init(&seq->buffer_stack));
    REQUIRE_OK(kefir_list_on_remove(&seq->buffer_stack, free_token_buffer, NULL));
    seq->source = source;
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_token_sequence_free(struct kefir_mem *mem,
                                                      struct kefir_preprocessor_token_sequence *seq) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(seq != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token sequence"));

    REQUIRE_OK(kefir_list_free(mem, &seq->buffer_stack));
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_token_sequence_push_front(struct kefir_mem *mem,
                                                            struct kefir_preprocessor_token_sequence *seq,
                                                            struct kefir_token_buffer *src,
                                                            kefir_preprocessor_token_destination_t token_destination) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(seq != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token sequence"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));

    struct buffer_element *buffer_elt = KEFIR_MALLOC(mem, sizeof(struct buffer_element));
    REQUIRE(buffer_elt != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate token buffer element"));
    buffer_elt->buffer = *src;
    REQUIRE_OK(kefir_token_buffer_init(src));
    buffer_elt->index = 0;
    buffer_elt->token_destination = token_destination;

    kefir_result_t res =
        kefir_list_insert_after(mem, &seq->buffer_stack, kefir_list_tail(&seq->buffer_stack), buffer_elt);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_token_buffer_free(mem, &buffer_elt->buffer);
        KEFIR_FREE(mem, buffer_elt);
        return res;
    });
    return KEFIR_OK;
}

static kefir_result_t next_buffer_elt(struct kefir_mem *mem, struct kefir_preprocessor_token_sequence *seq,
                                      struct buffer_element **buffer_elt) {
    *buffer_elt = NULL;
    while (*buffer_elt == NULL) {
        struct kefir_list_entry *entry = kefir_list_tail(&seq->buffer_stack);
        if (entry == NULL && seq->source != NULL) {
            REQUIRE_OK(seq->source->next_buffer(mem, seq, seq->source->payload));
            entry = kefir_list_tail(&seq->buffer_stack);
        }
        REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_ITERATOR_END, "Token sequence is empty"));
        *buffer_elt = entry->value;
        if ((*buffer_elt)->index == kefir_token_buffer_length(&(*buffer_elt)->buffer)) {
            REQUIRE_OK(kefir_list_pop(mem, &seq->buffer_stack, entry));
            *buffer_elt = NULL;
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_token_sequence_next(struct kefir_mem *mem,
                                                      struct kefir_preprocessor_token_sequence *seq,
                                                      const struct kefir_token **token_ptr,
                                                      kefir_preprocessor_token_destination_t *token_destination) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(seq != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token sequence"));

    struct buffer_element *buffer_elt = NULL;
    REQUIRE_OK(next_buffer_elt(mem, seq, &buffer_elt));
    if (token_ptr != NULL) {
        *token_ptr = kefir_token_buffer_at(&buffer_elt->buffer, buffer_elt->index);
        ASSIGN_PTR(token_destination, buffer_elt->token_destination);
    }
    buffer_elt->index++;
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_token_sequence_shift(struct kefir_mem *mem,
                                                       struct kefir_preprocessor_token_sequence *seq,
                                                       struct kefir_token_buffer *buffer) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(seq != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token sequence"));
    REQUIRE(buffer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token buffer"));

    const struct kefir_token *token;
    REQUIRE_OK(kefir_preprocessor_token_sequence_next(mem, seq, &token, NULL));
    REQUIRE_OK(kefir_token_buffer_emplace(mem, buffer, token));
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_token_sequence_current(struct kefir_mem *mem,
                                                         struct kefir_preprocessor_token_sequence *seq,
                                                         const struct kefir_token **token_ptr,
                                                         kefir_preprocessor_token_destination_t *token_destination) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(seq != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token sequence"));
    REQUIRE(token_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));

    struct buffer_element *buffer_elt = NULL;
    REQUIRE_OK(next_buffer_elt(mem, seq, &buffer_elt));
    *token_ptr = kefir_token_buffer_at(&buffer_elt->buffer, buffer_elt->index);
    ASSIGN_PTR(token_destination, buffer_elt->token_destination);
    return KEFIR_OK;
}

kefir_result_t kefir_preprocessor_token_sequence_skip_whitespaces(struct kefir_mem *mem,
                                                                  struct kefir_preprocessor_token_sequence *seq,
                                                                  const struct kefir_token **token_ptr,
                                                                  struct kefir_token_buffer *buffer) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(seq != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token sequence"));

    const struct kefir_token *current_token = NULL;
    kefir_bool_t skip_whitespaces = true;
    while (skip_whitespaces) {
        kefir_result_t res = kefir_preprocessor_token_sequence_current(mem, seq, &current_token, NULL);
        if (res == KEFIR_ITERATOR_END) {
            skip_whitespaces = false;
            current_token = NULL;
        } else {
            REQUIRE_OK(res);
            if (current_token->klass != KEFIR_TOKEN_PP_WHITESPACE) {
                skip_whitespaces = false;
            } else {
                if (buffer != NULL) {
                    REQUIRE_OK(kefir_preprocessor_token_sequence_shift(mem, seq, buffer));
                } else {
                    REQUIRE_OK(kefir_preprocessor_token_sequence_next(mem, seq, NULL, NULL));
                }
            }
        }
    }
    ASSIGN_PTR(token_ptr, current_token);
    return KEFIR_OK;
}
