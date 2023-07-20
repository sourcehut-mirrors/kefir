/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#include "kefir/lexer/lexem.h"
#include "kefir/test/unit_test.h"
#include "kefir/test/util.h"
#include "kefir/util/char32.h"

DEFINE_CASE(parser_lexem_construction_sentinel, "Parser - sentinel tokens") {
    struct kefir_token token;
    ASSERT_OK(kefir_token_new_sentinel(&token));
    ASSERT(token.klass == KEFIR_TOKEN_SENTINEL);
}
END_CASE

DEFINE_CASE(parser_lexem_construction_keyword, "Parser - keyword tokens") {
#define ASSERT_KEYWORD(_keyword)                                \
    do {                                                        \
        struct kefir_token token;                               \
        ASSERT_OK(kefir_token_new_keyword((_keyword), &token)); \
        ASSERT(token.klass == KEFIR_TOKEN_KEYWORD);             \
        ASSERT(token.keyword == (_keyword));                    \
        ASSERT_OK(kefir_token_free(&kft_mem, &token));          \
    } while (0);
    ASSERT_KEYWORD(KEFIR_KEYWORD_AUTO);
    ASSERT_KEYWORD(KEFIR_KEYWORD_BREAK);
    ASSERT_KEYWORD(KEFIR_KEYWORD_CASE);
    ASSERT_KEYWORD(KEFIR_KEYWORD_CHAR);
    ASSERT_KEYWORD(KEFIR_KEYWORD_CONST);
    ASSERT_KEYWORD(KEFIR_KEYWORD_CONTINUE);
    ASSERT_KEYWORD(KEFIR_KEYWORD_DEFAULT);
    ASSERT_KEYWORD(KEFIR_KEYWORD_DO);
    ASSERT_KEYWORD(KEFIR_KEYWORD_DOUBLE);
    ASSERT_KEYWORD(KEFIR_KEYWORD_ELSE);
    ASSERT_KEYWORD(KEFIR_KEYWORD_ENUM);
    ASSERT_KEYWORD(KEFIR_KEYWORD_EXTERN);
    ASSERT_KEYWORD(KEFIR_KEYWORD_FLOAT);
    ASSERT_KEYWORD(KEFIR_KEYWORD_FOR);
    ASSERT_KEYWORD(KEFIR_KEYWORD_GOTO);
    ASSERT_KEYWORD(KEFIR_KEYWORD_IF);
    ASSERT_KEYWORD(KEFIR_KEYWORD_INLINE);
    ASSERT_KEYWORD(KEFIR_KEYWORD_INT);
    ASSERT_KEYWORD(KEFIR_KEYWORD_LONG);
    ASSERT_KEYWORD(KEFIR_KEYWORD_REGISTER);
    ASSERT_KEYWORD(KEFIR_KEYWORD_RESTRICT);
    ASSERT_KEYWORD(KEFIR_KEYWORD_RETURN);
    ASSERT_KEYWORD(KEFIR_KEYWORD_SHORT);
    ASSERT_KEYWORD(KEFIR_KEYWORD_SIGNED);
    ASSERT_KEYWORD(KEFIR_KEYWORD_SIZEOF);
    ASSERT_KEYWORD(KEFIR_KEYWORD_STATIC);
    ASSERT_KEYWORD(KEFIR_KEYWORD_STRUCT);
    ASSERT_KEYWORD(KEFIR_KEYWORD_SWITCH);
    ASSERT_KEYWORD(KEFIR_KEYWORD_TYPEDEF);
    ASSERT_KEYWORD(KEFIR_KEYWORD_UNION);
    ASSERT_KEYWORD(KEFIR_KEYWORD_UNSIGNED);
    ASSERT_KEYWORD(KEFIR_KEYWORD_VOID);
    ASSERT_KEYWORD(KEFIR_KEYWORD_VOLATILE);
    ASSERT_KEYWORD(KEFIR_KEYWORD_WHILE);
    ASSERT_KEYWORD(KEFIR_KEYWORD_ALIGNAS);
    ASSERT_KEYWORD(KEFIR_KEYWORD_ALIGNOF);
    ASSERT_KEYWORD(KEFIR_KEYWORD_ATOMIC);
    ASSERT_KEYWORD(KEFIR_KEYWORD_BOOL);
    ASSERT_KEYWORD(KEFIR_KEYWORD_COMPLEX);
    ASSERT_KEYWORD(KEFIR_KEYWORD_GENERIC);
    ASSERT_KEYWORD(KEFIR_KEYWORD_IMAGINARY);
    ASSERT_KEYWORD(KEFIR_KEYWORD_NORETURN);
    ASSERT_KEYWORD(KEFIR_KEYWORD_STATIC_ASSERT);
    ASSERT_KEYWORD(KEFIR_KEYWORD_THREAD_LOCAL);
#undef ASSERT_KEYWORD
}
END_CASE

DEFINE_CASE(parser_lexem_construction_identifier, "Parser - identifier tokens") {
    struct kefir_string_pool symbols;
    ASSERT_OK(kefir_string_pool_init(&symbols));

    struct kefir_token token;
    ASSERT_NOK(kefir_token_new_identifier(&kft_mem, NULL, "abc", &token));
    ASSERT_NOK(kefir_token_new_identifier(NULL, &symbols, "abc", &token));
    ASSERT_NOK(kefir_token_new_identifier(NULL, NULL, NULL, &token));
    ASSERT_NOK(kefir_token_new_identifier(NULL, NULL, "", &token));

#define ASSERT_IDENTIFIER(_id)                                                    \
    do {                                                                          \
        ASSERT_OK(kefir_token_new_identifier(&kft_mem, &symbols, (_id), &token)); \
        ASSERT(token.klass == KEFIR_TOKEN_IDENTIFIER);                            \
        ASSERT(token.identifier != NULL);                                         \
        ASSERT(strcmp(token.identifier, (_id)) == 0);                             \
        ASSERT_OK(kefir_token_free(&kft_mem, &token));                            \
    } while (0)

    ASSERT_IDENTIFIER("a");
    ASSERT_IDENTIFIER("abc");
    ASSERT_IDENTIFIER("ABC123abc");
    ASSERT_IDENTIFIER("___abc_test_156TESTtest_TEST");

#undef ASSERT_IDENTIFIER

#define ASSERT_IDENTIFIER(_id)                                            \
    do {                                                                  \
        ASSERT_OK(kefir_token_new_identifier(NULL, NULL, (_id), &token)); \
        ASSERT(token.klass == KEFIR_TOKEN_IDENTIFIER);                    \
        ASSERT(token.identifier != NULL);                                 \
        ASSERT(strcmp(token.identifier, (_id)) == 0);                     \
        ASSERT_OK(kefir_token_free(&kft_mem, &token));                    \
    } while (0)

    ASSERT_IDENTIFIER("a");
    ASSERT_IDENTIFIER("abc");
    ASSERT_IDENTIFIER("ABC123abc");
    ASSERT_IDENTIFIER("___abc_test_156TESTtest_TEST");

#undef ASSERT_IDENTIFIER

    ASSERT_OK(kefir_string_pool_free(&kft_mem, &symbols));
}
END_CASE

DEFINE_CASE(parser_lexem_construction_constants, "Parser - constant tokens") {
    struct kefir_token token;
#define ASSERT_CONSTANT(_fn, _type, _field, _value)    \
    do {                                               \
        ASSERT_OK(_fn((_value), &token));              \
        ASSERT(token.klass == KEFIR_TOKEN_CONSTANT);   \
        ASSERT(token.constant.type == (_type));        \
        ASSERT(token.constant._field == (_value));     \
        ASSERT_OK(kefir_token_free(&kft_mem, &token)); \
    } while (0)

    for (kefir_int64_t i = -10; i < 10; i++) {
        ASSERT_CONSTANT(kefir_token_new_constant_int, KEFIR_CONSTANT_TOKEN_INTEGER, integer, 1 + i);
        ASSERT_CONSTANT(kefir_token_new_constant_uint, KEFIR_CONSTANT_TOKEN_UNSIGNED_INTEGER, uinteger,
                        (kefir_uint64_t) 11 + i);
        ASSERT_CONSTANT(kefir_token_new_constant_long, KEFIR_CONSTANT_TOKEN_LONG_INTEGER, integer, 2 + i);
        ASSERT_CONSTANT(kefir_token_new_constant_ulong, KEFIR_CONSTANT_TOKEN_UNSIGNED_LONG_INTEGER, uinteger,
                        (kefir_uint64_t) 13 + i);
        ASSERT_CONSTANT(kefir_token_new_constant_long_long, KEFIR_CONSTANT_TOKEN_LONG_LONG_INTEGER, integer, 4 + i);
        ASSERT_CONSTANT(kefir_token_new_constant_ulong_long, KEFIR_CONSTANT_TOKEN_UNSIGNED_LONG_LONG_INTEGER, uinteger,
                        (kefir_uint64_t) 15 + i);
        ASSERT_CONSTANT(kefir_token_new_constant_char, KEFIR_CONSTANT_TOKEN_CHAR, character, 'A' + i);
        ASSERT_CONSTANT(kefir_token_new_constant_wide_char, KEFIR_CONSTANT_TOKEN_WIDE_CHAR, wide_char, U'B' + i);
        ASSERT_CONSTANT(kefir_token_new_constant_unicode16_char, KEFIR_CONSTANT_TOKEN_UNICODE16_CHAR, unicode16_char,
                        U'C' + i);
        ASSERT_CONSTANT(kefir_token_new_constant_unicode32_char, KEFIR_CONSTANT_TOKEN_UNICODE32_CHAR, unicode32_char,
                        U'D' + i);

        ASSERT_OK(kefir_token_new_constant_float(((kefir_float32_t) i) / 10, &token));
        ASSERT(token.klass == KEFIR_TOKEN_CONSTANT);
        ASSERT(token.constant.type == KEFIR_CONSTANT_TOKEN_FLOAT);
        ASSERT(FLOAT_EQUALS(token.constant.float32, ((kefir_float32_t) i) / 10, FLOAT_EPSILON));
        ASSERT_OK(kefir_token_free(&kft_mem, &token));

        ASSERT_OK(kefir_token_new_constant_double(((kefir_float64_t) i) / 10, &token));
        ASSERT(token.klass == KEFIR_TOKEN_CONSTANT);
        ASSERT(token.constant.type == KEFIR_CONSTANT_TOKEN_DOUBLE);
        ASSERT(DOUBLE_EQUALS(token.constant.float64, ((kefir_float64_t) i) / 10, DOUBLE_EPSILON));
        ASSERT_OK(kefir_token_free(&kft_mem, &token));
    }

#undef ASSERT_CONSTANT
}
END_CASE

DEFINE_CASE(parser_lexem_construction_string_literals, "Parser - string literal tokens") {
    struct kefir_token token;
#define ASSERT_STRING_LITERAL(_literal, _length)                                                      \
    do {                                                                                              \
        ASSERT_OK(kefir_token_new_string_literal_multibyte(&kft_mem, (_literal), (_length), &token)); \
        ASSERT(token.klass == KEFIR_TOKEN_STRING_LITERAL);                                            \
        ASSERT(token.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE);                    \
        ASSERT(token.string_literal.literal != NULL);                                                 \
        ASSERT(token.string_literal.literal != (_literal));                                           \
        ASSERT(token.string_literal.length == (_length));                                             \
        ASSERT(memcmp(token.string_literal.literal, (_literal), (_length)) == 0);                     \
        ASSERT_OK(kefir_token_free(&kft_mem, &token));                                                \
    } while (0)

    const char *MSG[] = {"", "abc", "test test test", "One two three\n\n\n\t    Test...test...test...123",
                         "Hello, cruel-cruel-cruel world!"};
    kefir_size_t MSG_LEN = sizeof(MSG) / sizeof(MSG[0]);
    for (kefir_size_t i = 0; i < MSG_LEN; i++) {
        ASSERT_STRING_LITERAL(MSG[i], strlen(MSG[i]));
    }

    const char line[] = " \0\0\0\n\t\0";
    ASSERT_STRING_LITERAL(line, sizeof(line));

#undef ASSERT_STRING_LITERAL
}
END_CASE

DEFINE_CASE(parser_lexem_construction_unicode8_string_literals, "Parser - unicode8 string literal tokens") {
    struct kefir_token token;
#define ASSERT_STRING_LITERAL(_literal, _length)                                                     \
    do {                                                                                             \
        ASSERT_OK(kefir_token_new_string_literal_unicode8(&kft_mem, (_literal), (_length), &token)); \
        ASSERT(token.klass == KEFIR_TOKEN_STRING_LITERAL);                                           \
        ASSERT(token.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_UNICODE8);                    \
        ASSERT(token.string_literal.literal != NULL);                                                \
        ASSERT(token.string_literal.literal != (_literal));                                          \
        ASSERT(token.string_literal.length == (_length));                                            \
        ASSERT(memcmp(token.string_literal.literal, (_literal), (_length)) == 0);                    \
        ASSERT_OK(kefir_token_free(&kft_mem, &token));                                               \
    } while (0)

    const char *MSG[] = {u8"", u8"abc", u8"test test test", u8"One two three\n\n\n\t    Test...test...test...123",
                         u8"Hello, cruel-cruel-cruel world!"};
    kefir_size_t MSG_LEN = sizeof(MSG) / sizeof(MSG[0]);
    for (kefir_size_t i = 0; i < MSG_LEN; i++) {
        ASSERT_STRING_LITERAL(MSG[i], strlen(MSG[i]));
    }

    const char line[] = u8" \0\0\0\n\t\0";
    ASSERT_STRING_LITERAL(line, sizeof(line));

#undef ASSERT_STRING_LITERAL
}
END_CASE

static kefir_size_t strlen16(const kefir_char16_t *string) {
    kefir_size_t length = 0;
    for (; *string != u'\0'; length++, string++) {}
    return length;
}

DEFINE_CASE(parser_lexem_construction_unicode16_string_literals, "Parser - unicode16 string literal tokens") {
    struct kefir_token token;
#define ASSERT_STRING_LITERAL(_literal, _length)                                                           \
    do {                                                                                                   \
        ASSERT_OK(kefir_token_new_string_literal_unicode16(&kft_mem, (_literal), (_length), &token));      \
        ASSERT(token.klass == KEFIR_TOKEN_STRING_LITERAL);                                                 \
        ASSERT(token.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_UNICODE16);                         \
        ASSERT(token.string_literal.literal != NULL);                                                      \
        ASSERT(token.string_literal.literal != (_literal));                                                \
        ASSERT(token.string_literal.length == (_length));                                                  \
        ASSERT(memcmp(token.string_literal.literal, (_literal), (_length) * sizeof(kefir_char16_t)) == 0); \
        ASSERT_OK(kefir_token_free(&kft_mem, &token));                                                     \
    } while (0)

    const kefir_char16_t *MSG[] = {u"", u"abc", u"test test test", u"One two three\n\n\n\t    Test...test...test...123",
                                   u"Hello, cruel-cruel-cruel world!"};
    kefir_size_t MSG_LEN = sizeof(MSG) / sizeof(MSG[0]);
    for (kefir_size_t i = 0; i < MSG_LEN; i++) {
        ASSERT_STRING_LITERAL(MSG[i], strlen16(MSG[i]));
    }

    const kefir_char16_t line[] = u" \0\0\0\n\t\0";
    ASSERT_STRING_LITERAL(line, sizeof(line) / sizeof(line[0]));

#undef ASSERT_STRING_LITERAL
}
END_CASE

DEFINE_CASE(parser_lexem_construction_unicode32_string_literals, "Parser - unicode32 string literal tokens") {
    struct kefir_token token;
#define ASSERT_STRING_LITERAL(_literal, _length)                                                           \
    do {                                                                                                   \
        ASSERT_OK(kefir_token_new_string_literal_unicode32(&kft_mem, (_literal), (_length), &token));      \
        ASSERT(token.klass == KEFIR_TOKEN_STRING_LITERAL);                                                 \
        ASSERT(token.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_UNICODE32);                         \
        ASSERT(token.string_literal.literal != NULL);                                                      \
        ASSERT(token.string_literal.literal != (_literal));                                                \
        ASSERT(token.string_literal.length == (_length));                                                  \
        ASSERT(memcmp(token.string_literal.literal, (_literal), (_length) * sizeof(kefir_char32_t)) == 0); \
        ASSERT_OK(kefir_token_free(&kft_mem, &token));                                                     \
    } while (0)

    const kefir_char32_t *MSG[] = {U"", U"abc", U"test test test", U"One two three\n\n\n\t    Test...test...test...123",
                                   U"Hello, cruel-cruel-cruel world!"};
    kefir_size_t MSG_LEN = sizeof(MSG) / sizeof(MSG[0]);
    for (kefir_size_t i = 0; i < MSG_LEN; i++) {
        ASSERT_STRING_LITERAL(MSG[i], kefir_strlen32(MSG[i]));
    }

    const kefir_char32_t line[] = U" \0\0\0\n\t\0";
    ASSERT_STRING_LITERAL(line, sizeof(line) / sizeof(line[0]));

#undef ASSERT_STRING_LITERAL
}
END_CASE

DEFINE_CASE(parser_lexem_construction_wide_string_literals, "Parser - wide string literal tokens") {
    struct kefir_token token;
#define ASSERT_STRING_LITERAL(_literal, _length)                                                          \
    do {                                                                                                  \
        ASSERT_OK(kefir_token_new_string_literal_wide(&kft_mem, (_literal), (_length), &token));          \
        ASSERT(token.klass == KEFIR_TOKEN_STRING_LITERAL);                                                \
        ASSERT(token.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_WIDE);                             \
        ASSERT(token.string_literal.literal != NULL);                                                     \
        ASSERT(token.string_literal.literal != (_literal));                                               \
        ASSERT(token.string_literal.length == (_length));                                                 \
        ASSERT(memcmp(token.string_literal.literal, (_literal), (_length) * sizeof(kefir_wchar_t)) == 0); \
        ASSERT_OK(kefir_token_free(&kft_mem, &token));                                                    \
    } while (0)

    const kefir_wchar_t *MSG[] = {L"", L"abc", L"test test test", L"One two three\n\n\n\t    Test...test...test...123",
                                  L"Hello, cruel-cruel-cruel world!"};
    kefir_size_t MSG_LEN = sizeof(MSG) / sizeof(MSG[0]);
    for (kefir_size_t i = 0; i < MSG_LEN; i++) {
        ASSERT_STRING_LITERAL(MSG[i], wcslen(MSG[i]));
    }

    const kefir_wchar_t line[] = L" \0\0\0\n\t\0";
    ASSERT_STRING_LITERAL(line, sizeof(line) / sizeof(line[0]));

#undef ASSERT_STRING_LITERAL
}
END_CASE

DEFINE_CASE(parser_lexem_construction_punctuator, "Parser - punctuator tokens") {
#define ASSERT_PUNCTUATOR(_punctuator)                                \
    do {                                                              \
        struct kefir_token token;                                     \
        ASSERT_OK(kefir_token_new_punctuator((_punctuator), &token)); \
        ASSERT(token.klass == KEFIR_TOKEN_PUNCTUATOR);                \
        ASSERT(token.punctuator == (_punctuator));                    \
        ASSERT_OK(kefir_token_free(&kft_mem, &token));                \
    } while (0);

    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_LEFT_BRACKET);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_RIGHT_BRACKET);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_LEFT_PARENTHESE);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_RIGHT_PARENTHESE);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_LEFT_BRACE);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_RIGHT_BRACE);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_DOT);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_RIGHT_ARROW);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_DOUBLE_PLUS);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_DOUBLE_MINUS);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_AMPERSAND);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_STAR);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_MINUS);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_TILDE);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_EXCLAMATION_MARK);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_SLASH);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_PERCENT);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_LEFT_SHIFT);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_RIGHT_SHIFT);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_LESS_THAN);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_GREATER_THAN);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_LESS_OR_EQUAL);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_GREATER_OR_EQUAL);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_EQUAL);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_NOT_EQUAL);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_CARET);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_VBAR);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_DOUBLE_AMPERSAND);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_DOUBLE_VBAR);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_QUESTION_MARK);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_COLON);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_SEMICOLON);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_ELLIPSIS);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_ASSIGN);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_ASSIGN_MULTIPLY);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_ASSIGN_DIVIDE);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_ASSIGN_MODULO);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_ASSIGN_ADD);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_ASSIGN_SUBTRACT);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_ASSIGN_SHIFT_LEFT);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_ASSIGN_SHIFT_RIGHT);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_ASSIGN_AND);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_ASSIGN_XOR);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_ASSIGN_OR);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_COMMA);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_HASH);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_DOUBLE_HASH);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_DIGRAPH_LEFT_BRACKET);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_DIGRAPH_RIGHT_BRACKET);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_DIGRAPH_LEFT_BRACE);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_DIGRAPH_RIGHT_BRACE);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_DIGRAPH_HASH);
    ASSERT_PUNCTUATOR(KEFIR_PUNCTUATOR_DIGRAPH_DOUBLE_HASH);

#undef ASSERT_PUNCTUATOR
}
END_CASE

DEFINE_CASE(parser_lexem_construction_pp_whitespace, "Parser - pp whitespaces") {
#define ASSERT_WHITESPACE(_newline)                                   \
    do {                                                              \
        struct kefir_token token;                                     \
        ASSERT_OK(kefir_token_new_pp_whitespace((_newline), &token)); \
        ASSERT(token.klass == KEFIR_TOKEN_PP_WHITESPACE);             \
        ASSERT(token.pp_whitespace.newline == (_newline));            \
        ASSERT_OK(kefir_token_free(&kft_mem, &token));                \
    } while (0)

    ASSERT_WHITESPACE(false);
    ASSERT_WHITESPACE(true);

#undef ASSERT_WHITESPACE
}
END_CASE

DEFINE_CASE(parser_lexem_construction_pp_numbers, "Parser - pp numbers") {
#define ASSERT_PP_NUMBER(_literal)                                                        \
    do {                                                                                  \
        const char LITERAL[] = _literal;                                                  \
        struct kefir_token token;                                                         \
        ASSERT_OK(kefir_token_new_pp_number(&kft_mem, LITERAL, sizeof(LITERAL), &token)); \
        ASSERT(token.klass == KEFIR_TOKEN_PP_NUMBER);                                     \
        ASSERT(memcmp(token.pp_number.number_literal, LITERAL, sizeof(LITERAL)) == 0);    \
        ASSERT_OK(kefir_token_free(&kft_mem, &token));                                    \
    } while (0)

    ASSERT_PP_NUMBER("123123.454e54");
    ASSERT_PP_NUMBER("1");
    ASSERT_PP_NUMBER("148u2ie-829");

    do {
        struct kefir_token token;
        ASSERT_NOK(kefir_token_new_pp_number(&kft_mem, "", 0, &token));
    } while (0);

#undef ASSERT_PP_NUMBER
}
END_CASE

DEFINE_CASE(parser_lexem_construction_pp_header_name, "Parser - pp header names") {
#define ASSERT_PP_HEADER_NAME(_system, _literal)                                                          \
    do {                                                                                                  \
        const char LITERAL[] = _literal;                                                                  \
        struct kefir_token token;                                                                         \
        ASSERT_OK(kefir_token_new_pp_header_name(&kft_mem, (_system), LITERAL, sizeof(LITERAL), &token)); \
        ASSERT(token.klass == KEFIR_TOKEN_PP_HEADER_NAME);                                                \
        ASSERT(token.pp_header_name.system == (_system));                                                 \
        ASSERT(memcmp(token.pp_header_name.header_name, LITERAL, sizeof(LITERAL)) == 0);                  \
        ASSERT_OK(kefir_token_free(&kft_mem, &token));                                                    \
    } while (0)

    ASSERT_PP_HEADER_NAME(true, "stdlib.h");
    ASSERT_PP_HEADER_NAME(false, "kefir.h");
    ASSERT_PP_HEADER_NAME(true, "test...test...test...h");
    ASSERT_PP_HEADER_NAME(false, ".h");

    do {
        struct kefir_token token;
        ASSERT_NOK(kefir_token_new_pp_header_name(&kft_mem, true, "", 0, &token));
    } while (0);

#undef ASSERT_PP_HEADER_NAME
}
END_CASE

DEFINE_CASE(parser_lexem_move, "Parser - moving tokens") {
    struct kefir_string_pool symbols;
    ASSERT_OK(kefir_string_pool_init(&symbols));

    struct kefir_token src, dst;
    ASSERT_NOK(kefir_token_move(NULL, NULL));
    ASSERT_NOK(kefir_token_move(&dst, NULL));
    ASSERT_NOK(kefir_token_move(NULL, &src));

    ASSERT_OK(kefir_token_new_sentinel(&src));
    ASSERT_OK(kefir_token_move(&dst, &src));
    ASSERT(dst.klass == KEFIR_TOKEN_SENTINEL);
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    ASSERT_OK(kefir_token_new_keyword(KEFIR_KEYWORD_DO, &src));
    ASSERT_OK(kefir_token_move(&dst, &src));
    ASSERT(dst.klass == KEFIR_TOKEN_KEYWORD);
    ASSERT(dst.keyword == KEFIR_KEYWORD_DO);
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    ASSERT_OK(kefir_token_new_identifier(&kft_mem, &symbols, "testTEST", &src));
    ASSERT_OK(kefir_token_move(&dst, &src));
    ASSERT(dst.klass == KEFIR_TOKEN_IDENTIFIER);
    ASSERT(strcmp(dst.identifier, "testTEST") == 0);
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    ASSERT_OK(kefir_token_new_constant_double(7.5926, &src));
    ASSERT_OK(kefir_token_move(&dst, &src));
    ASSERT(dst.klass == KEFIR_TOKEN_CONSTANT);
    ASSERT(dst.constant.type == KEFIR_CONSTANT_TOKEN_DOUBLE);
    ASSERT(DOUBLE_EQUALS(dst.constant.float64, 7.5926, DOUBLE_EPSILON));
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    const char MSG[] = "\0\0\0TEST...TEST...TEST...HELLO!!!!\0";
    ASSERT_OK(kefir_token_new_string_literal_multibyte(&kft_mem, MSG, sizeof(MSG), &src));
    ASSERT_OK(kefir_token_move(&dst, &src));
    ASSERT(dst.klass == KEFIR_TOKEN_STRING_LITERAL);
    ASSERT(dst.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE);
    ASSERT(dst.string_literal.length == sizeof(MSG));
    ASSERT(memcmp(MSG, dst.string_literal.literal, sizeof(MSG)) == 0);
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    const char MSG2[] = u8"\\t\\nTeST test one-twoTHREE\\r\\'\\\"\0";
    ASSERT_OK(kefir_token_new_string_literal_unicode8(&kft_mem, MSG2, sizeof(MSG2), &src));
    ASSERT_OK(kefir_token_move(&dst, &src));
    ASSERT(dst.klass == KEFIR_TOKEN_STRING_LITERAL);
    ASSERT(dst.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_UNICODE8);
    ASSERT(dst.string_literal.length == sizeof(MSG2));
    ASSERT(memcmp(MSG2, dst.string_literal.literal, sizeof(MSG2)) == 0);
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    const kefir_char16_t MSG3[] = u"Another test string, n00000th1ng sp3c1al!\n\r\0\t";
    ASSERT_OK(kefir_token_new_string_literal_unicode16(&kft_mem, MSG3, sizeof(MSG3) / sizeof(MSG3[0]), &src));
    ASSERT_OK(kefir_token_move(&dst, &src));
    ASSERT(dst.klass == KEFIR_TOKEN_STRING_LITERAL);
    ASSERT(dst.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_UNICODE16);
    ASSERT(dst.string_literal.length == sizeof(MSG3) / sizeof(MSG3[0]));
    ASSERT(memcmp(MSG3, dst.string_literal.literal, sizeof(MSG3)) == 0);
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    const kefir_char32_t MSG4[] = U"\0\0rSTRING-string-testing-123\v";
    ASSERT_OK(kefir_token_new_string_literal_unicode32(&kft_mem, MSG4, sizeof(MSG4) / sizeof(MSG4[0]), &src));
    ASSERT_OK(kefir_token_move(&dst, &src));
    ASSERT(dst.klass == KEFIR_TOKEN_STRING_LITERAL);
    ASSERT(dst.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_UNICODE32);
    ASSERT(dst.string_literal.length == sizeof(MSG4) / sizeof(MSG4[0]));
    ASSERT(memcmp(MSG4, dst.string_literal.literal, sizeof(MSG4)) == 0);
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    const kefir_wchar_t MSG5[] = L"\"\"\\Th3 l@st w1d3 $tring!!!\v\t";
    ASSERT_OK(kefir_token_new_string_literal_wide(&kft_mem, MSG5, sizeof(MSG5) / sizeof(MSG5[0]), &src));
    ASSERT_OK(kefir_token_move(&dst, &src));
    ASSERT(dst.klass == KEFIR_TOKEN_STRING_LITERAL);
    ASSERT(dst.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_WIDE);
    ASSERT(dst.string_literal.length == sizeof(MSG5) / sizeof(MSG5[0]));
    ASSERT(memcmp(MSG5, dst.string_literal.literal, sizeof(MSG5)) == 0);
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    ASSERT_OK(kefir_token_new_pp_whitespace(true, &src));
    ASSERT_OK(kefir_token_move(&dst, &src));
    ASSERT(dst.klass == KEFIR_TOKEN_PP_WHITESPACE);
    ASSERT(dst.pp_whitespace.newline);
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    const char LITERAL1[] = "18336228P-81728e8h7";
    ASSERT_OK(kefir_token_new_pp_number(&kft_mem, LITERAL1, sizeof(LITERAL1), &src));
    ASSERT_OK(kefir_token_move(&dst, &src));
    ASSERT(dst.klass == KEFIR_TOKEN_PP_NUMBER);
    ASSERT(strcmp(LITERAL1, dst.pp_number.number_literal) == 0);
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    const char LITERAL2[] = "some/header.h";
    ASSERT_OK(kefir_token_new_pp_header_name(&kft_mem, true, LITERAL2, sizeof(LITERAL2), &src));
    ASSERT_OK(kefir_token_move(&dst, &src));
    ASSERT(dst.klass == KEFIR_TOKEN_PP_HEADER_NAME);
    ASSERT(dst.pp_header_name.system);
    ASSERT(strcmp(LITERAL2, dst.pp_header_name.header_name) == 0);
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    ASSERT_OK(kefir_string_pool_free(&kft_mem, &symbols));
}
END_CASE

DEFINE_CASE(parser_lexem_copy, "Parser - copying tokens") {
    struct kefir_string_pool symbols;
    ASSERT_OK(kefir_string_pool_init(&symbols));

    struct kefir_token src, dst;
    ASSERT_NOK(kefir_token_copy(&kft_mem, NULL, NULL));
    ASSERT_NOK(kefir_token_copy(&kft_mem, &dst, NULL));
    ASSERT_NOK(kefir_token_copy(&kft_mem, NULL, &src));
    ASSERT_NOK(kefir_token_copy(NULL, &dst, &src));

    ASSERT_OK(kefir_token_new_sentinel(&src));
    ASSERT_OK(kefir_token_copy(&kft_mem, &dst, &src));
    ASSERT(src.klass == KEFIR_TOKEN_SENTINEL);
    ASSERT(dst.klass == KEFIR_TOKEN_SENTINEL);
    ASSERT_OK(kefir_token_free(&kft_mem, &src));
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    ASSERT_OK(kefir_token_new_keyword(KEFIR_KEYWORD_DO, &src));
    ASSERT_OK(kefir_token_copy(&kft_mem, &dst, &src));
    ASSERT(src.klass == KEFIR_TOKEN_KEYWORD);
    ASSERT(src.keyword == KEFIR_KEYWORD_DO);
    ASSERT(dst.klass == KEFIR_TOKEN_KEYWORD);
    ASSERT(dst.keyword == KEFIR_KEYWORD_DO);
    ASSERT_OK(kefir_token_free(&kft_mem, &src));
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    ASSERT_OK(kefir_token_new_identifier(&kft_mem, &symbols, "testTEST", &src));
    ASSERT_OK(kefir_token_copy(&kft_mem, &dst, &src));
    ASSERT(src.klass == KEFIR_TOKEN_IDENTIFIER);
    ASSERT(strcmp(src.identifier, "testTEST") == 0);
    ASSERT(dst.klass == KEFIR_TOKEN_IDENTIFIER);
    ASSERT(strcmp(dst.identifier, "testTEST") == 0);
    ASSERT_OK(kefir_token_free(&kft_mem, &src));
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    ASSERT_OK(kefir_token_new_constant_double(7.5926, &src));
    ASSERT_OK(kefir_token_copy(&kft_mem, &dst, &src));
    ASSERT(src.klass == KEFIR_TOKEN_CONSTANT);
    ASSERT(src.constant.type == KEFIR_CONSTANT_TOKEN_DOUBLE);
    ASSERT(DOUBLE_EQUALS(src.constant.float64, 7.5926, DOUBLE_EPSILON));
    ASSERT(dst.klass == KEFIR_TOKEN_CONSTANT);
    ASSERT(dst.constant.type == KEFIR_CONSTANT_TOKEN_DOUBLE);
    ASSERT(DOUBLE_EQUALS(dst.constant.float64, 7.5926, DOUBLE_EPSILON));
    ASSERT_OK(kefir_token_free(&kft_mem, &src));
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    const char MSG[] = "\0\0\0TEST...TEST...TEST...HELLO!!!!\0";
    ASSERT_OK(kefir_token_new_string_literal_multibyte(&kft_mem, MSG, sizeof(MSG), &src));
    ASSERT_OK(kefir_token_copy(&kft_mem, &dst, &src));
    ASSERT(src.klass == KEFIR_TOKEN_STRING_LITERAL);
    ASSERT(src.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE);
    ASSERT(src.string_literal.length == sizeof(MSG));
    ASSERT(memcmp(MSG, src.string_literal.literal, sizeof(MSG)) == 0);
    ASSERT(src.string_literal.literal != MSG);
    ASSERT(dst.klass == KEFIR_TOKEN_STRING_LITERAL);
    ASSERT(dst.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE);
    ASSERT(dst.string_literal.length == sizeof(MSG));
    ASSERT(memcmp(MSG, dst.string_literal.literal, sizeof(MSG)) == 0);
    ASSERT(dst.string_literal.literal != MSG);
    ASSERT_OK(kefir_token_free(&kft_mem, &src));
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    const char MSG2[] = u8"Message #number2 (two)\t\v\ryes!\0.";
    ASSERT_OK(kefir_token_new_string_literal_unicode8(&kft_mem, MSG2, sizeof(MSG2), &src));
    ASSERT_OK(kefir_token_copy(&kft_mem, &dst, &src));
    ASSERT(src.klass == KEFIR_TOKEN_STRING_LITERAL);
    ASSERT(src.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_UNICODE8);
    ASSERT(src.string_literal.length == sizeof(MSG2));
    ASSERT(memcmp(MSG2, src.string_literal.literal, sizeof(MSG2)) == 0);
    ASSERT(src.string_literal.literal != MSG2);
    ASSERT(dst.klass == KEFIR_TOKEN_STRING_LITERAL);
    ASSERT(dst.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_UNICODE8);
    ASSERT(dst.string_literal.length == sizeof(MSG2));
    ASSERT(memcmp(MSG2, dst.string_literal.literal, sizeof(MSG2)) == 0);
    ASSERT(dst.string_literal.literal != MSG2);
    ASSERT_OK(kefir_token_free(&kft_mem, &src));
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    const kefir_char16_t MSG3[] = u"Message #number2 (two)\t\v\ryes!\0.";
    ASSERT_OK(kefir_token_new_string_literal_unicode16(&kft_mem, MSG3, sizeof(MSG3) / sizeof(MSG3[0]), &src));
    ASSERT_OK(kefir_token_copy(&kft_mem, &dst, &src));
    ASSERT(src.klass == KEFIR_TOKEN_STRING_LITERAL);
    ASSERT(src.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_UNICODE16);
    ASSERT(src.string_literal.length == sizeof(MSG3) / sizeof(MSG3[0]));
    ASSERT(memcmp(MSG3, src.string_literal.literal, sizeof(MSG3)) == 0);
    ASSERT(src.string_literal.literal != MSG3);
    ASSERT(dst.klass == KEFIR_TOKEN_STRING_LITERAL);
    ASSERT(dst.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_UNICODE16);
    ASSERT(dst.string_literal.length == sizeof(MSG3) / sizeof(MSG3[0]));
    ASSERT(memcmp(MSG3, dst.string_literal.literal, sizeof(MSG3)) == 0);
    ASSERT(dst.string_literal.literal != MSG3);
    ASSERT_OK(kefir_token_free(&kft_mem, &src));
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    const kefir_char32_t MSG4[] = U"Yet another silly boring messgage\n!";
    ASSERT_OK(kefir_token_new_string_literal_unicode32(&kft_mem, MSG4, sizeof(MSG4) / sizeof(MSG4[0]), &src));
    ASSERT_OK(kefir_token_copy(&kft_mem, &dst, &src));
    ASSERT(src.klass == KEFIR_TOKEN_STRING_LITERAL);
    ASSERT(src.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_UNICODE32);
    ASSERT(src.string_literal.length == sizeof(MSG4) / sizeof(MSG4[0]));
    ASSERT(memcmp(MSG4, src.string_literal.literal, sizeof(MSG4)) == 0);
    ASSERT(src.string_literal.literal != MSG4);
    ASSERT(dst.klass == KEFIR_TOKEN_STRING_LITERAL);
    ASSERT(dst.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_UNICODE32);
    ASSERT(dst.string_literal.length == sizeof(MSG4) / sizeof(MSG4[0]));
    ASSERT(memcmp(MSG4, dst.string_literal.literal, sizeof(MSG4)) == 0);
    ASSERT(dst.string_literal.literal != MSG4);
    ASSERT_OK(kefir_token_free(&kft_mem, &src));
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    const kefir_wchar_t MSG5[] = L"\0\nTHE last MSGe?0\"";
    ASSERT_OK(kefir_token_new_string_literal_wide(&kft_mem, MSG5, sizeof(MSG5) / sizeof(MSG5[0]), &src));
    ASSERT_OK(kefir_token_copy(&kft_mem, &dst, &src));
    ASSERT(src.klass == KEFIR_TOKEN_STRING_LITERAL);
    ASSERT(src.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_WIDE);
    ASSERT(src.string_literal.length == sizeof(MSG5) / sizeof(MSG5[0]));
    ASSERT(memcmp(MSG5, src.string_literal.literal, sizeof(MSG5)) == 0);
    ASSERT(src.string_literal.literal != MSG5);
    ASSERT(dst.klass == KEFIR_TOKEN_STRING_LITERAL);
    ASSERT(dst.string_literal.type == KEFIR_STRING_LITERAL_TOKEN_WIDE);
    ASSERT(dst.string_literal.length == sizeof(MSG5) / sizeof(MSG5[0]));
    ASSERT(memcmp(MSG5, dst.string_literal.literal, sizeof(MSG5)) == 0);
    ASSERT(dst.string_literal.literal != MSG5);
    ASSERT_OK(kefir_token_free(&kft_mem, &src));
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    ASSERT_OK(kefir_token_new_pp_whitespace(false, &src));
    ASSERT_OK(kefir_token_copy(&kft_mem, &dst, &src));
    ASSERT(src.klass == KEFIR_TOKEN_PP_WHITESPACE);
    ASSERT(!src.pp_whitespace.newline);
    ASSERT(dst.klass == KEFIR_TOKEN_PP_WHITESPACE);
    ASSERT(!dst.pp_whitespace.newline);
    ASSERT_OK(kefir_token_free(&kft_mem, &src));
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    const char LITERAL1[] = "18373.u83hje82P-1";
    ASSERT_OK(kefir_token_new_pp_number(&kft_mem, LITERAL1, sizeof(LITERAL1), &src));
    ASSERT_OK(kefir_token_copy(&kft_mem, &dst, &src));
    ASSERT(src.klass == KEFIR_TOKEN_PP_NUMBER);
    ASSERT(strcmp(LITERAL1, src.pp_number.number_literal) == 0);
    ASSERT(src.pp_number.number_literal != LITERAL1);
    ASSERT(dst.klass == KEFIR_TOKEN_PP_NUMBER);
    ASSERT(strcmp(LITERAL1, dst.pp_number.number_literal) == 0);
    ASSERT(dst.pp_number.number_literal != LITERAL1);
    ASSERT(dst.pp_number.number_literal != src.pp_number.number_literal);
    ASSERT_OK(kefir_token_free(&kft_mem, &src));
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    const char LITERAL2[] = "directory/subdirectory1/header.ext";
    ASSERT_OK(kefir_token_new_pp_header_name(&kft_mem, false, LITERAL2, sizeof(LITERAL2), &src));
    ASSERT_OK(kefir_token_copy(&kft_mem, &dst, &src));
    ASSERT(src.klass == KEFIR_TOKEN_PP_HEADER_NAME);
    ASSERT(!src.pp_header_name.system);
    ASSERT(strcmp(LITERAL2, src.pp_header_name.header_name) == 0);
    ASSERT(src.pp_header_name.header_name != LITERAL2);
    ASSERT(dst.klass == KEFIR_TOKEN_PP_HEADER_NAME);
    ASSERT(!dst.pp_header_name.system);
    ASSERT(strcmp(LITERAL2, dst.pp_header_name.header_name) == 0);
    ASSERT(dst.pp_header_name.header_name != LITERAL2);
    ASSERT(dst.pp_header_name.header_name != src.pp_header_name.header_name);
    ASSERT_OK(kefir_token_free(&kft_mem, &src));
    ASSERT_OK(kefir_token_free(&kft_mem, &dst));

    ASSERT_OK(kefir_string_pool_free(&kft_mem, &symbols));
}
END_CASE
