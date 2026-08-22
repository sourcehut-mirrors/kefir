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

#include "kefir/lexer/source_cursor.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include <string.h>

kefir_result_t kefir_lexer_source_cursor_init(struct kefir_lexer_source_cursor *cursor, const char *content,
                                              kefir_size_t length, const char *source_id) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer source cursor"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid content"));
    REQUIRE(source_id != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source identifier"));

    cursor->content = content;
    cursor->length = length;
    cursor->index = 0;
    cursor->carriage_return_char = '\r';
    cursor->newline_char = '\n';
    REQUIRE_OK(kefir_source_location_init(&cursor->location, source_id, 1, 1));
    return KEFIR_OK;
}

#define TRIGRAPH_SEQS(_trigraph, _separator)                                                                \
    _trigraph('=', '#') _separator \
    _trigraph('(', '[') _separator \
    _trigraph('/', '\\') _separator \
    _trigraph(')', ']') _separator \
    _trigraph('\'', '^') _separator \
    _trigraph('<', '{') _separator \
    _trigraph('!', '|') _separator \
    _trigraph('>', '}') _separator \
    _trigraph('-', '~')

static kefir_lexer_char_t at_impl_physical(const struct kefir_lexer_source_cursor *cursor, kefir_size_t *index,
                                       kefir_source_location_column_t *column) {
    REQUIRE(*index < cursor->length, KEFIR_LEXER_SOURCE_CURSOR_EOF);

    kefir_lexer_char_t character = (unsigned char) cursor->content[(*index)++];
    
    if (character != '\0' && column != NULL) {
        (*column)++;
    }

    if (character == '?' && *index + 1 < cursor->length && cursor->content[*index] == '?') {
#define DEF_TRIGRAPH(_in, _out)           \
    if (cursor->content[*index + 1] == (_in)) { \
        character = (_out);               \
        (*index) += 2; \
    }
        TRIGRAPH_SEQS(DEF_TRIGRAPH, else)
#undef DEF_TRIGRAPH
    }
    return character;
}

static kefir_lexer_char_t at_impl(const struct kefir_lexer_source_cursor *cursor, kefir_size_t count) {
    kefir_lexer_char_t character = KEFIR_LEXER_SOURCE_CURSOR_EOF;
    kefir_size_t index = cursor->index;
    do {
        character = at_impl_physical(cursor, &index, NULL);
        kefir_size_t next_index = index;
        if (character == KEFIR_LEXER_SOURCE_CURSOR_EOF) {
            break;
        } else if (character == '\\' &&
                   at_impl_physical(cursor, &next_index, NULL) == cursor->newline_char) {
            index = next_index;
            count++;
        } else {
            next_index = index;
            if (character == '\\' &&
                at_impl_physical(cursor, &next_index, NULL) == cursor->carriage_return_char &&
                at_impl_physical(cursor, &next_index, NULL) == cursor->newline_char) {
                index = next_index;
                count++;
            }
        }
    } while (count--);
    return character;
}

kefir_lexer_char_t kefir_lexer_source_cursor_at(const struct kefir_lexer_source_cursor *cursor, kefir_size_t count) {
    REQUIRE(cursor != NULL, KEFIR_LEXER_SOURCE_CURSOR_EOF);
    return at_impl(cursor, count);
}

static kefir_result_t next_impl(struct kefir_lexer_source_cursor *cursor, kefir_size_t count,
                                kefir_lexer_char_t *last_char) {
    kefir_lexer_char_t chr = KEFIR_LEXER_SOURCE_CURSOR_EOF;
    while (count--) {
        chr = at_impl_physical(cursor, &cursor->index, &cursor->location.column);
        if (chr == KEFIR_LEXER_SOURCE_CURSOR_EOF) {
            break;
        }

        if (chr == cursor->newline_char) {
            cursor->location.column = 1;
            cursor->location.line++;
        } else {
            kefir_size_t next_index = cursor->index;
            if (chr == '\\' && at_impl_physical(cursor, &next_index, NULL) == cursor->newline_char) {
                cursor->index = next_index;
                count++;
                cursor->location.column = 1;
                cursor->location.line++;
            } else {
                next_index = cursor->index;
                if (chr == '\\' &&
                    at_impl_physical(cursor, &next_index, NULL) == cursor->carriage_return_char &&
                    at_impl_physical(cursor, &next_index, NULL) == cursor->newline_char) {
                    cursor->index = next_index;
                    count++;
                    cursor->location.column = 1;
                    cursor->location.line++;
                }
            }
        }
    }
    ASSIGN_PTR(last_char, chr);
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_source_cursor_next(struct kefir_lexer_source_cursor *cursor, kefir_size_t count) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer source cursor"));
    REQUIRE_OK(next_impl(cursor, count, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_source_cursor_skip(struct kefir_lexer_source_cursor *cursor, kefir_size_t count) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer source cursor"));
    REQUIRE_OK(next_impl(cursor, count, NULL));
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_source_cursor_save(const struct kefir_lexer_source_cursor *cursor,
                                              struct kefir_lexer_source_cursor_state *state) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer source cursor"));
    REQUIRE(state != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer lexer source cursor state"));

    state->index = cursor->index;
    state->location = cursor->location;
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_source_cursor_restore(struct kefir_lexer_source_cursor *cursor,
                                                 const struct kefir_lexer_source_cursor_state *state) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer source cursor"));
    REQUIRE(state != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer lexer source cursor state"));

    cursor->index = state->index;
    cursor->location = state->location;
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_cursor_match_string(const struct kefir_lexer_source_cursor *cursor,
                                               const char *string) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer source cursor"));
    REQUIRE(string != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string"));

    for (kefir_size_t index = 0; string[index] != '\0'; index++) {
        if (kefir_lexer_source_cursor_at(cursor, index) != string[index]) {
            return KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Cannot match provided string");
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_cursor_set_source_location(struct kefir_lexer_source_cursor *cursor,
                                                      const struct kefir_source_location *source_location) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer source cursor"));
    REQUIRE(source_location != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source location"));

    cursor->location = *source_location;
    return KEFIR_OK;
}
