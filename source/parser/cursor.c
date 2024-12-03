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

#include "kefir/parser/cursor.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

static kefir_result_t direct_cursor_get_token(kefir_size_t index, const struct kefir_token **token_ptr,
                                              const struct kefir_token_cursor_handle *handle) {
    REQUIRE(token_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to token"));
    REQUIRE(handle != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token cursor handle"));

    ASSIGN_DECL_CAST(struct kefir_token *, tokens, handle->payload[0]);
    ASSIGN_DECL_CAST(kefir_size_t, length, handle->payload[1]);
    if (index < length) {
        *token_ptr = &tokens[index];
    } else {
        *token_ptr = NULL;
    }
    return KEFIR_OK;
}

static kefir_result_t direct_cursor_length(kefir_size_t *length_ptr, const struct kefir_token_cursor_handle *handle) {
    REQUIRE(length_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to length"));
    REQUIRE(handle != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token cursor handle"));

    *length_ptr = (kefir_size_t) handle->payload[1];
    return KEFIR_OK;
}

kefir_result_t kefir_parser_token_cursor_init(struct kefir_parser_token_cursor *cursor,
                                              const struct kefir_token_cursor_handle *handle) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected value token cursor"));
    REQUIRE(handle != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected value token cursor handle"));

    cursor->handle = handle;
    cursor->index = 0;
    REQUIRE_OK(kefir_token_new_sentinel(&cursor->sentinel));
    return KEFIR_OK;
}

kefir_result_t kefir_parser_token_cursor_init_direct(struct kefir_parser_token_cursor *cursor,
                                                     struct kefir_token *tokens, kefir_size_t length) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected value token cursor"));
    REQUIRE(tokens != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token array"));

    cursor->direct_cursor.handle.payload[0] = (kefir_uptr_t) tokens;
    cursor->direct_cursor.handle.payload[1] = (kefir_uptr_t) length;
    cursor->direct_cursor.handle.get_token = direct_cursor_get_token;
    cursor->direct_cursor.handle.length = direct_cursor_length;

    cursor->handle = &cursor->direct_cursor.handle;
    cursor->index = 0;
    REQUIRE_OK(kefir_token_new_sentinel(&cursor->sentinel));
    return KEFIR_OK;
}

const struct kefir_token *kefir_parser_token_cursor_at(const struct kefir_parser_token_cursor *cursor,
                                                       kefir_size_t offset) {
    REQUIRE(cursor != NULL, NULL);

    const kefir_size_t index = cursor->index + offset;
    const struct kefir_token *token;
    kefir_result_t res = cursor->handle->get_token(index, &token, cursor->handle);
    REQUIRE(res == KEFIR_OK, &cursor->sentinel);
    if (token != NULL) {
        return token;
    } else {
        return &cursor->sentinel;
    }
}

kefir_result_t kefir_parser_token_cursor_reset(struct kefir_parser_token_cursor *cursor) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected value token cursor"));
    cursor->index = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_parser_token_cursor_next(struct kefir_parser_token_cursor *cursor) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected value token cursor"));
    kefir_size_t length;
    REQUIRE_OK(cursor->handle->length(&length, cursor->handle));
    if (cursor->index < length) {
        cursor->index++;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_parser_token_cursor_save(struct kefir_parser_token_cursor *cursor, kefir_size_t *value) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected value token cursor"));
    REQUIRE(value != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to index"));
    *value = cursor->index;
    return KEFIR_OK;
}

kefir_result_t kefir_parser_token_cursor_restore(struct kefir_parser_token_cursor *cursor, kefir_size_t index) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected value token cursor"));

    kefir_size_t length;
    REQUIRE_OK(cursor->handle->length(&length, cursor->handle));
    REQUIRE(index <= length, KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Expected valid index"));
    cursor->index = index;
    return KEFIR_OK;
}
