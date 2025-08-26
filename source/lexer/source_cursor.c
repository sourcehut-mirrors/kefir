/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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
#include "kefir/util/uchar.h"
#include <string.h>

static kefir_result_t next_impl(struct kefir_lexer_source_cursor *, kefir_size_t, kefir_char32_t *);

kefir_result_t kefir_lexer_source_cursor_init(struct kefir_lexer_source_cursor *cursor, const char *content,
                                              kefir_size_t length, const char *source_id) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer source cursor"));
    REQUIRE(content != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid content"));
    REQUIRE(source_id != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source identifier"));

    *cursor = (struct kefir_lexer_source_cursor) {0};
    cursor->content = content;
    cursor->length = length;
    cursor->carriage_return_char = U'\r';
    cursor->newline_char = U'\n';
    REQUIRE_OK(kefir_source_location_init(&cursor->location, source_id, 1, 1));
    REQUIRE_OK(kefir_source_location_init(&cursor->current_location, source_id, 1, 1));

    for (kefir_size_t i = 0; i < KEFIR_LEXER_SOURCE_CURSOR_LOOKAHEAD; i++) {
        cursor->lookahead[i].location.line = cursor->current_location.line;
        cursor->lookahead[i].location.column = cursor->current_location.column;
        REQUIRE_OK(next_impl(cursor, 1, &cursor->lookahead[i].character));
    }
    cursor->location.line = cursor->lookahead[0].location.line;
    cursor->location.column = cursor->lookahead[0].location.column;
    return KEFIR_OK;
}

static kefir_char32_t at_impl(const struct kefir_lexer_source_cursor *cursor, kefir_size_t count) {
    kefir_char32_t character = KEFIR_LEXER_SOURCE_CURSOR_EOF;
    kefir_size_t index = cursor->index;
    mbstate_t mbstate = cursor->mbstate;
    do {
        if (index == cursor->length) {
            character = KEFIR_LEXER_SOURCE_CURSOR_EOF;
            break;
        }

        size_t rc = mbrtoc32(&character, cursor->content + index, cursor->length - index, &mbstate);
        switch (rc) {
            case (size_t) -1:
            case (size_t) -2:
            case (size_t) -3:
                REQUIRE(index < cursor->length,
                        KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected source cursor index"));
                character = (kefir_int32_t) (*(cursor->content + index));
                break;

            case 0:
                character = U'\0';
                index++;
                break;

            default:
                index += rc;
                if (character == U'\\' && index < cursor->length) {  // Convert physical line to logical
                    kefir_char32_t character2;
                    mbstate_t mbstate2 = {0};
                    rc = mbrtoc32(&character2, cursor->content + index, cursor->length - index, &mbstate2);
                    switch (rc) {
                        case (size_t) -1:
                        case (size_t) -2:
                        case (size_t) -3:
                        case 0:
                            break;

                        default:
                            if (character2 == cursor->newline_char) {  // Skip line break
                                index += rc;
                                count++;
                            } else if (character2 == cursor->carriage_return_char &&
                                       cursor->index + rc < cursor->length) {
                                mbstate_t mbstate2 = {0};
                                kefir_size_t rc2 = mbrtoc32(&character2, cursor->content + index + rc,
                                                            cursor->length - index - rc, &mbstate2);
                                switch (rc2) {
                                    case (size_t) -1:
                                    case (size_t) -2:
                                    case (size_t) -3:
                                    case 0:
                                        break;

                                    default:
                                        if (character2 == cursor->newline_char) {  // Skip line break
                                            index += rc + rc2;
                                            count++;
                                        }
                                        break;
                                }
                            }
                            break;
                    }
                }
                break;
        }
    } while (count--);
    return character;
}

kefir_char32_t kefir_lexer_source_cursor_at(const struct kefir_lexer_source_cursor *cursor, kefir_size_t count) {
    REQUIRE(cursor != NULL, KEFIR_LEXER_SOURCE_CURSOR_EOF);
    if (count < KEFIR_LEXER_SOURCE_CURSOR_LOOKAHEAD) {
        return cursor->lookahead[count].character;
    } else {
        return at_impl(cursor, count - KEFIR_LEXER_SOURCE_CURSOR_LOOKAHEAD);
    }
}

static kefir_result_t next_impl(struct kefir_lexer_source_cursor *cursor, kefir_size_t count,
                                kefir_char32_t *last_char) {
    kefir_char32_t chr = KEFIR_LEXER_SOURCE_CURSOR_EOF;
    while (count--) {
        if (cursor->length == cursor->index) {
            break;
        }
        size_t rc = mbrtoc32(&chr, cursor->content + cursor->index, cursor->length - cursor->index, &cursor->mbstate);
        switch (rc) {
            case (size_t) -1:
            case (size_t) -2:
            case (size_t) -3:
                if (cursor->index < cursor->length) {
                    chr = (kefir_char32_t) * (cursor->content + cursor->index);
                    cursor->index++;
                    cursor->mbstate = (mbstate_t) {0};
                } else {
                    chr = U'\0';
                }
                break;

            case 0:
                chr = U'\0';
                cursor->index++;
                cursor->mbstate = (mbstate_t) {0};
                break;

            default:
                cursor->index += rc;
                if (chr == U'\n') {
                    cursor->current_location.column = 1;
                    cursor->current_location.line++;
                } else {
                    kefir_bool_t skip_line_break = false;
                    if (chr == U'\\' && cursor->index < cursor->length) {  // Convert physical line to logical
                        kefir_char32_t character2;
                        mbstate_t mbstate2 = {0};
                        rc = mbrtoc32(&character2, cursor->content + cursor->index, cursor->length - cursor->index,
                                      &mbstate2);
                        switch (rc) {
                            case (size_t) -1:
                            case (size_t) -2:
                            case (size_t) -3:
                            case 0:
                                break;

                            default:
                                if (character2 == cursor->newline_char) {  // Skip line break
                                    cursor->index += rc;
                                    count++;
                                    skip_line_break = true;
                                    cursor->current_location.column = 1;
                                    cursor->current_location.line++;
                                } else if (character2 == cursor->carriage_return_char &&
                                           cursor->index + rc < cursor->length) {
                                    mbstate_t mbstate2 = {0};
                                    kefir_size_t rc2 = mbrtoc32(&character2, cursor->content + cursor->index + rc,
                                                                cursor->length - cursor->index, &mbstate2);
                                    switch (rc2) {
                                        case (size_t) -1:
                                        case (size_t) -2:
                                        case (size_t) -3:
                                        case 0:
                                            break;

                                        default:
                                            if (character2 == cursor->newline_char) {  // Skip line break
                                                cursor->index += rc + rc2;
                                                count++;
                                                skip_line_break = true;
                                                cursor->current_location.column = 1;
                                                cursor->current_location.line++;
                                            }
                                            break;
                                    }
                                }
                                break;
                        }
                    }

                    if (!skip_line_break) {
                        cursor->current_location.column++;
                    }
                }
                break;
        }
    }
    ASSIGN_PTR(last_char, chr);
    return KEFIR_OK;
}

static kefir_result_t do_next(struct kefir_lexer_source_cursor *cursor, kefir_size_t count) {
    if (count < KEFIR_LEXER_SOURCE_CURSOR_LOOKAHEAD) {
        memmove(
            &cursor->lookahead, &cursor->lookahead[count],
            sizeof(struct kefir_lexer_source_cursor_lookahead_entry) * (KEFIR_LEXER_SOURCE_CURSOR_LOOKAHEAD - count));
        for (kefir_size_t i = KEFIR_LEXER_SOURCE_CURSOR_LOOKAHEAD - count; i < KEFIR_LEXER_SOURCE_CURSOR_LOOKAHEAD;
             i++) {
            cursor->lookahead[i].location.line = cursor->current_location.line;
            cursor->lookahead[i].location.column = cursor->current_location.column;
            REQUIRE_OK(next_impl(cursor, 1, &cursor->lookahead[i].character));
        }
    } else {
        REQUIRE_OK(next_impl(cursor, count - KEFIR_LEXER_SOURCE_CURSOR_LOOKAHEAD, NULL));
        for (kefir_size_t i = 0; i < KEFIR_LEXER_SOURCE_CURSOR_LOOKAHEAD; i++) {
            cursor->lookahead[i].location.line = cursor->current_location.line;
            cursor->lookahead[i].location.column = cursor->current_location.column;
            REQUIRE_OK(next_impl(cursor, 1, &cursor->lookahead[i].character));
        }
    }
    cursor->location.line = cursor->lookahead[0].location.line;
    cursor->location.column = cursor->lookahead[0].location.column;
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_source_cursor_next(struct kefir_lexer_source_cursor *cursor, kefir_size_t count) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer source cursor"));
    REQUIRE_OK(do_next(cursor, count));
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_source_cursor_skip(struct kefir_lexer_source_cursor *cursor, kefir_size_t count) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer source cursor"));
    REQUIRE_OK(do_next(cursor, count));
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_source_cursor_save(const struct kefir_lexer_source_cursor *cursor,
                                              struct kefir_lexer_source_cursor_state *state) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer source cursor"));
    REQUIRE(state != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer lexer source cursor state"));

    state->index = cursor->index;
    state->mbstate = cursor->mbstate;
    state->location = cursor->location;
    state->current_location = cursor->current_location;
    memcpy(&state->lookahead, &cursor->lookahead,
           sizeof(struct kefir_lexer_source_cursor_lookahead_entry) * KEFIR_LEXER_SOURCE_CURSOR_LOOKAHEAD);
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_source_cursor_restore(struct kefir_lexer_source_cursor *cursor,
                                                 const struct kefir_lexer_source_cursor_state *state) {
    REQUIRE(cursor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer source cursor"));
    REQUIRE(state != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer lexer source cursor state"));

    cursor->index = state->index;
    cursor->mbstate = state->mbstate;
    cursor->location = state->location;
    cursor->current_location = state->current_location;
    memcpy(&cursor->lookahead, &state->lookahead,
           sizeof(struct kefir_lexer_source_cursor_lookahead_entry) * KEFIR_LEXER_SOURCE_CURSOR_LOOKAHEAD);
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_cursor_match_string(const struct kefir_lexer_source_cursor *cursor,
                                               const kefir_char32_t *string) {
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

    const kefir_source_location_line_t original_line = cursor->location.line;
    kefir_source_location_line_t line_diff = source_location->line - cursor->location.line;
    kefir_source_location_line_t column_diff = source_location->column - cursor->location.column;
    cursor->location = *source_location;
    for (kefir_size_t i = 0; i < KEFIR_LEXER_SOURCE_CURSOR_LOOKAHEAD; i++) {
        if (cursor->lookahead[i].location.line == original_line) {
            cursor->lookahead[i].location.column += column_diff;
        }
        cursor->lookahead[i].location.line += line_diff;
    }
    cursor->current_location.source = source_location->source;
    if (cursor->current_location.line == original_line) {
        cursor->current_location.column += column_diff;
    }
    cursor->current_location.line += line_diff;
    return KEFIR_OK;
}
