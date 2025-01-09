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

#ifndef KEFIR_LEXER_LEXEM_H_
#define KEFIR_LEXER_LEXEM_H_

#include "kefir/lexer/base.h"
#include "kefir/core/mem.h"
#include "kefir/core/basic-types.h"
#include "kefir/core/string_pool.h"
#include "kefir/core/source_location.h"
#include <stdio.h>

typedef struct kefir_token kefir_token_t;
typedef struct kefir_json_output kefir_json_output_t;

typedef enum kefir_token_class {
    KEFIR_TOKEN_SENTINEL,
    KEFIR_TOKEN_KEYWORD,
    KEFIR_TOKEN_IDENTIFIER,
    KEFIR_TOKEN_CONSTANT,
    KEFIR_TOKEN_STRING_LITERAL,
    KEFIR_TOKEN_PUNCTUATOR,

    // Preprocessing tokens
    KEFIR_TOKEN_PP_WHITESPACE,
    KEFIR_TOKEN_PP_NUMBER,
    KEFIR_TOKEN_PP_HEADER_NAME,
    KEFIR_TOKEN_PP_PLACEMAKER,

    // Extension tokens
    KEFIR_TOKEN_EXTENSION
} kefir_token_class_t;

typedef enum kefir_keyword_token {
    KEFIR_KEYWORD_AUTO,
    KEFIR_KEYWORD_BREAK,
    KEFIR_KEYWORD_CASE,
    KEFIR_KEYWORD_CHAR,
    KEFIR_KEYWORD_CONST,
    KEFIR_KEYWORD_CONTINUE,
    KEFIR_KEYWORD_DEFAULT,
    KEFIR_KEYWORD_DO,
    KEFIR_KEYWORD_DOUBLE,
    KEFIR_KEYWORD_ELSE,
    KEFIR_KEYWORD_ENUM,
    KEFIR_KEYWORD_EXTERN,
    KEFIR_KEYWORD_FLOAT,
    KEFIR_KEYWORD_FOR,
    KEFIR_KEYWORD_GOTO,
    KEFIR_KEYWORD_IF,
    KEFIR_KEYWORD_INLINE,
    KEFIR_KEYWORD_INT,
    KEFIR_KEYWORD_LONG,
    KEFIR_KEYWORD_REGISTER,
    KEFIR_KEYWORD_RESTRICT,
    KEFIR_KEYWORD_RETURN,
    KEFIR_KEYWORD_SHORT,
    KEFIR_KEYWORD_SIGNED,
    KEFIR_KEYWORD_SIZEOF,
    KEFIR_KEYWORD_STATIC,
    KEFIR_KEYWORD_STRUCT,
    KEFIR_KEYWORD_SWITCH,
    KEFIR_KEYWORD_TYPEDEF,
    KEFIR_KEYWORD_UNION,
    KEFIR_KEYWORD_UNSIGNED,
    KEFIR_KEYWORD_VOID,
    KEFIR_KEYWORD_VOLATILE,
    KEFIR_KEYWORD_WHILE,
    KEFIR_KEYWORD_ALIGNAS,
    KEFIR_KEYWORD_ALIGNOF,
    KEFIR_KEYWORD_ATOMIC,
    KEFIR_KEYWORD_BOOL,
    KEFIR_KEYWORD_COMPLEX,
    KEFIR_KEYWORD_GENERIC,
    KEFIR_KEYWORD_IMAGINARY,
    KEFIR_KEYWORD_NORETURN,
    KEFIR_KEYWORD_STATIC_ASSERT,
    KEFIR_KEYWORD_THREAD_LOCAL,
    KEFIR_KEYWORD_TYPEOF,
    KEFIR_KEYWORD_TYPEOF_UNQUAL,
    KEFIR_KEYWORD_AUTO_TYPE,

    // Extensions
    KEFIR_KEYWORD_ATTRIBUTE,
    KEFIR_KEYWORD_ASM
} kefir_keyword_token_t;

typedef enum kefir_constant_token_type {
    KEFIR_CONSTANT_TOKEN_INTEGER,
    KEFIR_CONSTANT_TOKEN_LONG_INTEGER,
    KEFIR_CONSTANT_TOKEN_LONG_LONG_INTEGER,
    KEFIR_CONSTANT_TOKEN_UNSIGNED_INTEGER,
    KEFIR_CONSTANT_TOKEN_UNSIGNED_LONG_INTEGER,
    KEFIR_CONSTANT_TOKEN_UNSIGNED_LONG_LONG_INTEGER,
    KEFIR_CONSTANT_TOKEN_FLOAT,
    KEFIR_CONSTANT_TOKEN_DOUBLE,
    KEFIR_CONSTANT_TOKEN_LONG_DOUBLE,
    KEFIR_CONSTANT_TOKEN_COMPLEX_FLOAT,
    KEFIR_CONSTANT_TOKEN_COMPLEX_DOUBLE,
    KEFIR_CONSTANT_TOKEN_COMPLEX_LONG_DOUBLE,
    KEFIR_CONSTANT_TOKEN_CHAR,
    KEFIR_CONSTANT_TOKEN_WIDE_CHAR,
    KEFIR_CONSTANT_TOKEN_UNICODE16_CHAR,
    KEFIR_CONSTANT_TOKEN_UNICODE32_CHAR
} kefir_constant_token_type_t;

typedef enum kefir_string_literal_token_type {
    KEFIR_STRING_LITERAL_TOKEN_MULTIBYTE,
    KEFIR_STRING_LITERAL_TOKEN_UNICODE8,
    KEFIR_STRING_LITERAL_TOKEN_UNICODE16,
    KEFIR_STRING_LITERAL_TOKEN_UNICODE32,
    KEFIR_STRING_LITERAL_TOKEN_WIDE
} kefir_string_literal_token_type_t;

typedef struct kefir_constant_token {
    kefir_constant_token_type_t type;
    union {
        kefir_int64_t integer;
        kefir_uint64_t uinteger;
        kefir_float32_t float32;
        kefir_float64_t float64;
        kefir_long_double_t long_double;
        struct {
            kefir_float32_t real;
            kefir_float32_t imaginary;
        } complex_float32;
        struct {
            kefir_float64_t real;
            kefir_float64_t imaginary;
        } complex_float64;
        struct {
            kefir_long_double_t real;
            kefir_long_double_t imaginary;
        } complex_long_double;
        kefir_int_t character;
        kefir_wchar_t wide_char;
        kefir_char16_t unicode16_char;
        kefir_char32_t unicode32_char;
    };
} kefir_constant_token_t;

typedef struct kefir_string_literal_token {
    kefir_bool_t raw_literal;
    kefir_string_literal_token_type_t type;
    void *literal;
    kefir_size_t length;
} kefir_string_literal_token_t;

typedef enum kefir_punctuator_token {
    KEFIR_PUNCTUATOR_LEFT_BRACKET,
    KEFIR_PUNCTUATOR_RIGHT_BRACKET,
    KEFIR_PUNCTUATOR_LEFT_PARENTHESE,
    KEFIR_PUNCTUATOR_RIGHT_PARENTHESE,
    KEFIR_PUNCTUATOR_LEFT_BRACE,
    KEFIR_PUNCTUATOR_RIGHT_BRACE,
    KEFIR_PUNCTUATOR_DOT,
    KEFIR_PUNCTUATOR_RIGHT_ARROW,
    KEFIR_PUNCTUATOR_DOUBLE_PLUS,
    KEFIR_PUNCTUATOR_DOUBLE_MINUS,
    KEFIR_PUNCTUATOR_AMPERSAND,
    KEFIR_PUNCTUATOR_STAR,
    KEFIR_PUNCTUATOR_PLUS,
    KEFIR_PUNCTUATOR_MINUS,
    KEFIR_PUNCTUATOR_TILDE,
    KEFIR_PUNCTUATOR_EXCLAMATION_MARK,
    KEFIR_PUNCTUATOR_SLASH,
    KEFIR_PUNCTUATOR_PERCENT,
    KEFIR_PUNCTUATOR_LEFT_SHIFT,
    KEFIR_PUNCTUATOR_RIGHT_SHIFT,
    KEFIR_PUNCTUATOR_LESS_THAN,
    KEFIR_PUNCTUATOR_GREATER_THAN,
    KEFIR_PUNCTUATOR_LESS_OR_EQUAL,
    KEFIR_PUNCTUATOR_GREATER_OR_EQUAL,
    KEFIR_PUNCTUATOR_EQUAL,
    KEFIR_PUNCTUATOR_NOT_EQUAL,
    KEFIR_PUNCTUATOR_CARET,
    KEFIR_PUNCTUATOR_VBAR,
    KEFIR_PUNCTUATOR_DOUBLE_AMPERSAND,
    KEFIR_PUNCTUATOR_DOUBLE_VBAR,
    KEFIR_PUNCTUATOR_QUESTION_MARK,
    KEFIR_PUNCTUATOR_COLON,
    KEFIR_PUNCTUATOR_SEMICOLON,
    KEFIR_PUNCTUATOR_ELLIPSIS,
    KEFIR_PUNCTUATOR_ASSIGN,
    KEFIR_PUNCTUATOR_ASSIGN_MULTIPLY,
    KEFIR_PUNCTUATOR_ASSIGN_DIVIDE,
    KEFIR_PUNCTUATOR_ASSIGN_MODULO,
    KEFIR_PUNCTUATOR_ASSIGN_ADD,
    KEFIR_PUNCTUATOR_ASSIGN_SUBTRACT,
    KEFIR_PUNCTUATOR_ASSIGN_SHIFT_LEFT,
    KEFIR_PUNCTUATOR_ASSIGN_SHIFT_RIGHT,
    KEFIR_PUNCTUATOR_ASSIGN_AND,
    KEFIR_PUNCTUATOR_ASSIGN_XOR,
    KEFIR_PUNCTUATOR_ASSIGN_OR,
    KEFIR_PUNCTUATOR_COMMA,
    KEFIR_PUNCTUATOR_HASH,
    KEFIR_PUNCTUATOR_BACKSLASH,
    KEFIR_PUNCTUATOR_DOUBLE_HASH,
    KEFIR_PUNCTUATOR_DIGRAPH_LEFT_BRACKET,
    KEFIR_PUNCTUATOR_DIGRAPH_RIGHT_BRACKET,
    KEFIR_PUNCTUATOR_DIGRAPH_LEFT_BRACE,
    KEFIR_PUNCTUATOR_DIGRAPH_RIGHT_BRACE,
    KEFIR_PUNCTUATOR_DIGRAPH_HASH,
    KEFIR_PUNCTUATOR_DIGRAPH_DOUBLE_HASH
} kefir_punctuator_token_t;

typedef struct kefir_pptoken_pp_whitespace {
    kefir_bool_t newline;
} kefir_pptoken_pp_whitespace_t;

typedef struct kefir_pptoken_pp_number {
    const char *number_literal;
} kefir_pptoken_pp_number_t;

typedef struct kefir_pptoken_pp_header_name {
    kefir_bool_t system;
    const char *header_name;
} kefir_pptoken_pp_header_name_t;

typedef struct kefir_token_extension_class {
    kefir_result_t (*free)(struct kefir_mem *, struct kefir_token *);
    kefir_result_t (*copy)(struct kefir_mem *, struct kefir_token *, const struct kefir_token *);
    kefir_result_t (*format_json)(struct kefir_json_output *, const struct kefir_token *, kefir_bool_t);
    kefir_result_t (*format)(FILE *, const struct kefir_token *);
    kefir_result_t (*concat)(struct kefir_mem *, const struct kefir_token *, const struct kefir_token *,
                             struct kefir_token *);
    void *payload;
} kefir_token_extension_class_t;

typedef struct kefir_token_extension {
    const struct kefir_token_extension_class *klass;
    void *payload;
} kefir_token_extension_t;

typedef struct kefir_token_macro_expansions {
    struct kefir_hashtree macro_expansions;
} kefir_token_macro_expansions_t;

typedef struct kefir_token {
    kefir_token_class_t klass;
    struct kefir_token_macro_expansions *macro_expansions;
    union {
        kefir_keyword_token_t keyword;
        const char *identifier;
        struct kefir_constant_token constant;
        struct kefir_string_literal_token string_literal;
        kefir_punctuator_token_t punctuator;
        struct kefir_pptoken_pp_whitespace pp_whitespace;
        struct kefir_pptoken_pp_number pp_number;
        struct kefir_pptoken_pp_header_name pp_header_name;
        struct kefir_token_extension extension;
    };

    struct kefir_source_location source_location;
} kefir_token_t;

kefir_result_t kefir_token_macro_expansions_init(struct kefir_token_macro_expansions *);
kefir_result_t kefir_token_macro_expansions_free(struct kefir_mem *, struct kefir_token_macro_expansions *);
kefir_result_t kefir_token_macro_expansions_add(struct kefir_mem *, struct kefir_token_macro_expansions **,
                                                const char *);
kefir_result_t kefir_token_macro_expansions_merge(struct kefir_mem *, struct kefir_token_macro_expansions **,
                                                  const struct kefir_token_macro_expansions *);
kefir_bool_t kefir_token_macro_expansions_has(const struct kefir_token_macro_expansions *, const char *);

kefir_result_t kefir_token_new_sentinel(struct kefir_token *);
kefir_result_t kefir_token_new_keyword(kefir_keyword_token_t, struct kefir_token *);
kefir_result_t kefir_token_new_identifier(struct kefir_mem *, struct kefir_string_pool *, const char *,
                                          struct kefir_token *);
kefir_result_t kefir_token_new_constant_int(kefir_int64_t, struct kefir_token *);
kefir_result_t kefir_token_new_constant_uint(kefir_uint64_t, struct kefir_token *);
kefir_result_t kefir_token_new_constant_long(kefir_int64_t, struct kefir_token *);
kefir_result_t kefir_token_new_constant_ulong(kefir_uint64_t, struct kefir_token *);
kefir_result_t kefir_token_new_constant_long_long(kefir_int64_t, struct kefir_token *);
kefir_result_t kefir_token_new_constant_ulong_long(kefir_uint64_t, struct kefir_token *);
kefir_result_t kefir_token_new_constant_char(kefir_int_t, struct kefir_token *);
kefir_result_t kefir_token_new_constant_wide_char(kefir_wchar_t, struct kefir_token *);
kefir_result_t kefir_token_new_constant_unicode16_char(kefir_char16_t, struct kefir_token *);
kefir_result_t kefir_token_new_constant_unicode32_char(kefir_char32_t, struct kefir_token *);
kefir_result_t kefir_token_new_constant_float(kefir_float32_t, struct kefir_token *);
kefir_result_t kefir_token_new_constant_double(kefir_float64_t, struct kefir_token *);
kefir_result_t kefir_token_new_constant_long_double(kefir_long_double_t, struct kefir_token *);
kefir_result_t kefir_token_new_constant_complex_float(kefir_float32_t, kefir_float32_t, struct kefir_token *);
kefir_result_t kefir_token_new_constant_complex_double(kefir_float64_t, kefir_float64_t, struct kefir_token *);
kefir_result_t kefir_token_new_constant_complex_long_double(kefir_long_double_t, kefir_long_double_t,
                                                            struct kefir_token *);
kefir_result_t kefir_token_new_string_literal_multibyte(struct kefir_mem *, const char *, kefir_size_t,
                                                        struct kefir_token *);
kefir_result_t kefir_token_new_string_literal_unicode8(struct kefir_mem *, const char *, kefir_size_t,
                                                       struct kefir_token *);
kefir_result_t kefir_token_new_string_literal_unicode16(struct kefir_mem *, const kefir_char16_t *, kefir_size_t,
                                                        struct kefir_token *);
kefir_result_t kefir_token_new_string_literal_unicode32(struct kefir_mem *, const kefir_char32_t *, kefir_size_t,
                                                        struct kefir_token *);
kefir_result_t kefir_token_new_string_literal_wide(struct kefir_mem *, const kefir_wchar_t *, kefir_size_t,
                                                   struct kefir_token *);
kefir_result_t kefir_token_new_string_literal_raw(struct kefir_mem *, kefir_string_literal_token_type_t,
                                                  const kefir_char32_t *, kefir_size_t, struct kefir_token *);
kefir_bool_t kefir_token_string_literal_type_concat(kefir_string_literal_token_type_t,
                                                    kefir_string_literal_token_type_t,
                                                    kefir_string_literal_token_type_t *);
kefir_result_t kefir_token_new_punctuator(kefir_punctuator_token_t, struct kefir_token *);
kefir_result_t kefir_token_new_pp_whitespace(kefir_bool_t, struct kefir_token *);
kefir_result_t kefir_token_new_pp_number(struct kefir_mem *, const char *, kefir_size_t, struct kefir_token *);
kefir_result_t kefir_token_new_pp_header_name(struct kefir_mem *, kefir_bool_t, const char *, kefir_size_t,
                                              struct kefir_token *);
kefir_result_t kefir_token_new_extension(const struct kefir_token_extension_class *, void *, struct kefir_token *);

kefir_result_t kefir_token_move(struct kefir_token *, struct kefir_token *);
kefir_result_t kefir_token_copy(struct kefir_mem *, struct kefir_token *, const struct kefir_token *);
kefir_result_t kefir_token_free(struct kefir_mem *, struct kefir_token *);

typedef struct kefir_token_cursor_handle {
    kefir_result_t (*get_token)(kefir_size_t, const struct kefir_token **, const struct kefir_token_cursor_handle *);
    kefir_result_t (*length)(kefir_size_t *, const struct kefir_token_cursor_handle *);
    kefir_uptr_t payload[2];
} kefir_token_cursor_handle_t;

#endif
