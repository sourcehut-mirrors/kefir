/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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
#include "kefir/util/char32.h"
#include "kefir/core/string_buffer.h"
#include "kefir/lexer/string_literal_impl.h"

kefir_result_t kefir_lexer_next_wide_string_literal(struct kefir_mem *mem, struct kefir_lexer *lexer,
                                                    struct kefir_token *token, kefir_bool_t merge_adjacent) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(lexer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer"));
    REQUIRE(token != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid token"));

    REQUIRE(kefir_lexer_source_cursor_at(lexer->cursor, 0) == U'L' &&
                kefir_lexer_source_cursor_at(lexer->cursor, 1) == U'\"',
            KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match wide string literal"));

    struct kefir_string_buffer string_buffer;
    REQUIRE_OK(kefir_string_buffer_init(mem, &string_buffer, KEFIR_STRING_BUFFER_WIDE));

    kefir_result_t res =
        kefir_lexer_next_string_literal_sequence_impl(mem, lexer, U"L\"", &string_buffer, merge_adjacent);

    kefir_size_t literal_length;
    const kefir_wchar_t *literal = kefir_string_buffer_value(&string_buffer, &literal_length);
    REQUIRE_CHAIN(&res, kefir_token_new_string_literal_wide(mem, literal, literal_length, token));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_string_buffer_free(mem, &string_buffer);
        return res;
    });

    res = kefir_string_buffer_free(mem, &string_buffer);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_token_free(mem, token);
        return res;
    });
    return KEFIR_OK;
}
