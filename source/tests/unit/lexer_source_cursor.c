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

#include "kefir/test/unit_test.h"
#include "kefir/lexer/source_cursor.h"

DEFINE_CASE(lexer_source_cursor1, "Lexer - source cursor #1") {
    const char CONTENT[] = "123 {{(Hello\nworld\t,,,x!";
    kefir_size_t LENGTH = sizeof(CONTENT) - 1;

    struct kefir_lexer_source_cursor cursor;
    ASSERT_OK(kefir_lexer_source_cursor_init(&cursor, CONTENT, LENGTH, ""));

    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == '1');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 1) == '2');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 2) == '3');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 3) == ' ');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 4) == '{');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 5) == '{');

    struct kefir_lexer_source_cursor_state state, state2;
    ASSERT_OK(kefir_lexer_source_cursor_save(&cursor, &state));
    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 1));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == '2');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 1) == '3');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 2) == ' ');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 3) == '{');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 4) == '{');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 5) == '(');

    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 3));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == '{');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 1) == '{');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 2) == '(');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 3) == 'H');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 4) == 'e');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 5) == 'l');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 6) == 'l');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 7) == 'o');

    ASSERT_OK(kefir_lexer_source_cursor_save(&cursor, &state2));
    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 9));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == 'w');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 1) == 'o');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 2) == 'r');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 3) == 'l');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 4) == 'd');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 5) == '\t');

    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 4));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == 'd');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 1) == '\t');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 2) == ',');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 3) == ',');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 4) == ',');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 5) == 'x');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 6) == '!');

    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 3));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == ',');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 1) == ',');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 2) == 'x');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 3) == '!');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 4) == KEFIR_LEXER_SOURCE_CURSOR_EOF);
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 5) == KEFIR_LEXER_SOURCE_CURSOR_EOF);
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 6) == KEFIR_LEXER_SOURCE_CURSOR_EOF);

    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 1));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == ',');
    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 1));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == 'x');
    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 1));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == '!');
    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 1));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == KEFIR_LEXER_SOURCE_CURSOR_EOF);
    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 1));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == KEFIR_LEXER_SOURCE_CURSOR_EOF);
    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 10));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == KEFIR_LEXER_SOURCE_CURSOR_EOF);

    ASSERT_OK(kefir_lexer_source_cursor_restore(&cursor, &state2));
    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 3));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == 'H');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 1) == 'e');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 2) == 'l');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 3) == 'l');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 4) == 'o');

    ASSERT_OK(kefir_lexer_source_cursor_restore(&cursor, &state));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == '1');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 1) == '2');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 2) == '3');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 3) == ' ');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 4) == '{');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 5) == '{');
}
END_CASE

DEFINE_CASE(lexer_source_cursor_newlines, "Lexer - source cursor newlines") {
    const char CONTENT[] = "abc\\\ndef\\\n\\\n\\\n\\\nghi\njk\\\\l\n\n\n\\";
    kefir_size_t LENGTH = sizeof(CONTENT) - 1;

    struct kefir_lexer_source_cursor cursor;
    ASSERT_OK(kefir_lexer_source_cursor_init(&cursor, CONTENT, LENGTH, ""));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == 'a');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 1) == 'b');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 2) == 'c');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 3) == 'd');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 4) == 'e');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 5) == 'f');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 6) == 'g');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 7) == 'h');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 8) == 'i');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 9) == '\n');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 10) == 'j');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 11) == 'k');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 12) == '\\');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 13) == '\\');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 14) == 'l');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 15) == '\n');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 16) == '\n');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 17) == '\n');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 18) == '\\');
    ASSERT(cursor.location.line == 1);
    ASSERT(cursor.location.column == 1);

    struct kefir_lexer_source_cursor_state state;
    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 2));
    ASSERT_OK(kefir_lexer_source_cursor_save(&cursor, &state));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == 'c');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 1) == 'd');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 2) == 'e');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 3) == 'f');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 4) == 'g');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 5) == 'h');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 6) == 'i');
    ASSERT(cursor.location.line == 1);
    ASSERT(cursor.location.column == 3);

    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 1));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == 'd');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 1) == 'e');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 2) == 'f');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 3) == 'g');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 4) == 'h');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 5) == 'i');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 6) == '\n');
    ASSERT(cursor.location.line == 1);
    ASSERT(cursor.location.column == 4);

    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 1));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == 'e');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 1) == 'f');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 2) == 'g');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 3) == 'h');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 4) == 'i');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 5) == '\n');
    ASSERT(cursor.location.line == 2);
    ASSERT(cursor.location.column == 2);

    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 3));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == 'h');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 1) == 'i');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 2) == '\n');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 3) == 'j');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 4) == 'k');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 5) == '\\');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 6) == '\\');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 7) == 'l');
    ASSERT(cursor.location.line == 6);
    ASSERT(cursor.location.column == 2);

    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 3));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == 'j');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 1) == 'k');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 2) == '\\');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 3) == '\\');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 4) == 'l');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 5) == '\n');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 6) == '\n');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 7) == '\n');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 8) == '\\');
    ASSERT(cursor.location.line == 7);
    ASSERT(cursor.location.column == 1);

    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 5));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == '\n');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 1) == '\n');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 2) == '\n');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 3) == '\\');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 4) == KEFIR_LEXER_SOURCE_CURSOR_EOF);
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 5) == KEFIR_LEXER_SOURCE_CURSOR_EOF);
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 6) == KEFIR_LEXER_SOURCE_CURSOR_EOF);
    ASSERT(cursor.location.line == 7);
    ASSERT(cursor.location.column == 6);

    ASSERT_OK(kefir_lexer_source_cursor_next(&cursor, 5));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == KEFIR_LEXER_SOURCE_CURSOR_EOF);
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 1) == KEFIR_LEXER_SOURCE_CURSOR_EOF);

    ASSERT_OK(kefir_lexer_source_cursor_restore(&cursor, &state));
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 0) == 'c');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 1) == 'd');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 2) == 'e');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 3) == 'f');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 4) == 'g');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 5) == 'h');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 6) == 'i');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 7) == '\n');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 8) == 'j');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 9) == 'k');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 10) == '\\');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 11) == '\\');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 12) == 'l');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 13) == '\n');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 14) == '\n');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 15) == '\n');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 16) == '\\');
    ASSERT(kefir_lexer_source_cursor_at(&cursor, 17) == KEFIR_LEXER_SOURCE_CURSOR_EOF);
    ASSERT(cursor.location.line == 1);
    ASSERT(cursor.location.column == 3);
}
END_CASE
