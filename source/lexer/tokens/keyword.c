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

#include "kefir/lexer/lexer.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/standard_version.h"
#include "kefir/util/char32.h"

static const struct KeywordEntry {
    const kefir_char32_t *literal;
    kefir_keyword_token_t keyword;
    kefir_c_language_standard_version_t min_standard_version;
} KEYWORDS[] = {{U"auto", KEFIR_KEYWORD_AUTO, KEFIR_C17_STANDARD_VERSION},
                {U"break", KEFIR_KEYWORD_BREAK, KEFIR_C17_STANDARD_VERSION},
                {U"case", KEFIR_KEYWORD_CASE, KEFIR_C17_STANDARD_VERSION},
                {U"char", KEFIR_KEYWORD_CHAR, KEFIR_C17_STANDARD_VERSION},
                {U"__const", KEFIR_KEYWORD_CONST, KEFIR_C17_STANDARD_VERSION},
                {U"const", KEFIR_KEYWORD_CONST, KEFIR_C17_STANDARD_VERSION},
                {U"continue", KEFIR_KEYWORD_CONTINUE, KEFIR_C17_STANDARD_VERSION},
                {U"default", KEFIR_KEYWORD_DEFAULT, KEFIR_C17_STANDARD_VERSION},
                {U"do", KEFIR_KEYWORD_DO, KEFIR_C17_STANDARD_VERSION},
                {U"double", KEFIR_KEYWORD_DOUBLE, KEFIR_C17_STANDARD_VERSION},
                {U"else", KEFIR_KEYWORD_ELSE, KEFIR_C17_STANDARD_VERSION},
                {U"enum", KEFIR_KEYWORD_ENUM, KEFIR_C17_STANDARD_VERSION},
                {U"extern", KEFIR_KEYWORD_EXTERN, KEFIR_C17_STANDARD_VERSION},
                {U"float", KEFIR_KEYWORD_FLOAT, KEFIR_C17_STANDARD_VERSION},
                {U"for", KEFIR_KEYWORD_FOR, KEFIR_C17_STANDARD_VERSION},
                {U"goto", KEFIR_KEYWORD_GOTO, KEFIR_C17_STANDARD_VERSION},
                {U"if", KEFIR_KEYWORD_IF, KEFIR_C17_STANDARD_VERSION},
                {U"inline", KEFIR_KEYWORD_INLINE, KEFIR_C17_STANDARD_VERSION},
                {U"__inline__", KEFIR_KEYWORD_INLINE, KEFIR_C17_STANDARD_VERSION},
                {U"__inline", KEFIR_KEYWORD_INLINE, KEFIR_C17_STANDARD_VERSION},
                {U"int", KEFIR_KEYWORD_INT, KEFIR_C17_STANDARD_VERSION},
                {U"long", KEFIR_KEYWORD_LONG, KEFIR_C17_STANDARD_VERSION},
                {U"register", KEFIR_KEYWORD_REGISTER, KEFIR_C17_STANDARD_VERSION},
                {U"restrict", KEFIR_KEYWORD_RESTRICT, KEFIR_C17_STANDARD_VERSION},
                {U"__restrict__", KEFIR_KEYWORD_RESTRICT, KEFIR_C17_STANDARD_VERSION},
                {U"__restrict", KEFIR_KEYWORD_RESTRICT, KEFIR_C17_STANDARD_VERSION},
                {U"return", KEFIR_KEYWORD_RETURN, KEFIR_C17_STANDARD_VERSION},
                {U"short", KEFIR_KEYWORD_SHORT, KEFIR_C17_STANDARD_VERSION},
                {U"signed", KEFIR_KEYWORD_SIGNED, KEFIR_C17_STANDARD_VERSION},
                {U"__signed__", KEFIR_KEYWORD_SIGNED, KEFIR_C17_STANDARD_VERSION},
                {U"__signed", KEFIR_KEYWORD_SIGNED, KEFIR_C17_STANDARD_VERSION},
                {U"sizeof", KEFIR_KEYWORD_SIZEOF, KEFIR_C17_STANDARD_VERSION},
                {U"static", KEFIR_KEYWORD_STATIC, KEFIR_C17_STANDARD_VERSION},
                {U"struct", KEFIR_KEYWORD_STRUCT, KEFIR_C17_STANDARD_VERSION},
                {U"switch", KEFIR_KEYWORD_SWITCH, KEFIR_C17_STANDARD_VERSION},
                {U"typedef", KEFIR_KEYWORD_TYPEDEF, KEFIR_C17_STANDARD_VERSION},
                {U"union", KEFIR_KEYWORD_UNION, KEFIR_C17_STANDARD_VERSION},
                {U"unsigned", KEFIR_KEYWORD_UNSIGNED, KEFIR_C17_STANDARD_VERSION},
                {U"void", KEFIR_KEYWORD_VOID, KEFIR_C17_STANDARD_VERSION},
                {U"volatile", KEFIR_KEYWORD_VOLATILE, KEFIR_C17_STANDARD_VERSION},
                {U"__volatile__", KEFIR_KEYWORD_VOLATILE, KEFIR_C17_STANDARD_VERSION},
                {U"__volatile", KEFIR_KEYWORD_VOLATILE, KEFIR_C17_STANDARD_VERSION},
                {U"while", KEFIR_KEYWORD_WHILE, KEFIR_C17_STANDARD_VERSION},
                {U"_Alignas", KEFIR_KEYWORD_ALIGNAS, KEFIR_C17_STANDARD_VERSION},
                {U"alignas", KEFIR_KEYWORD_ALIGNAS, KEFIR_C23_STANDARD_VERSION},
                {U"_Alignof", KEFIR_KEYWORD_ALIGNOF, KEFIR_C17_STANDARD_VERSION},
                {U"__alignof__", KEFIR_KEYWORD_ALIGNOF, KEFIR_C17_STANDARD_VERSION},
                {U"__alignof", KEFIR_KEYWORD_ALIGNOF, KEFIR_C17_STANDARD_VERSION},
                {U"alignof", KEFIR_KEYWORD_ALIGNOF, KEFIR_C23_STANDARD_VERSION},
                {U"_Atomic", KEFIR_KEYWORD_ATOMIC, KEFIR_C17_STANDARD_VERSION},
                {U"_Bool", KEFIR_KEYWORD_BOOL, KEFIR_C17_STANDARD_VERSION},
                {U"bool", KEFIR_KEYWORD_BOOL, KEFIR_C23_STANDARD_VERSION},
                {U"true", KEFIR_KEYWORD_TRUE, KEFIR_C23_STANDARD_VERSION},
                {U"false", KEFIR_KEYWORD_FALSE, KEFIR_C23_STANDARD_VERSION},
                {U"_Complex", KEFIR_KEYWORD_COMPLEX, KEFIR_C17_STANDARD_VERSION},
                {U"__complex__", KEFIR_KEYWORD_COMPLEX, KEFIR_C17_STANDARD_VERSION},
                {U"_Generic", KEFIR_KEYWORD_GENERIC, KEFIR_C17_STANDARD_VERSION},
                {U"_Imaginary", KEFIR_KEYWORD_IMAGINARY, KEFIR_C17_STANDARD_VERSION},
                {U"_Noreturn", KEFIR_KEYWORD_NORETURN, KEFIR_C17_STANDARD_VERSION},
                {U"_Static_assert", KEFIR_KEYWORD_STATIC_ASSERT, KEFIR_C17_STANDARD_VERSION},
                {U"static_assert", KEFIR_KEYWORD_STATIC_ASSERT, KEFIR_C23_STANDARD_VERSION},
                {U"_Thread_local", KEFIR_KEYWORD_THREAD_LOCAL, KEFIR_C17_STANDARD_VERSION},
                {U"thread_local", KEFIR_KEYWORD_THREAD_LOCAL, KEFIR_C23_STANDARD_VERSION},
                {U"__attribute__", KEFIR_KEYWORD_ATTRIBUTE, KEFIR_C17_STANDARD_VERSION},
                {U"__attribute", KEFIR_KEYWORD_ATTRIBUTE, KEFIR_C17_STANDARD_VERSION},
                {U"asm", KEFIR_KEYWORD_ASM, KEFIR_C17_STANDARD_VERSION},
                {U"__asm__", KEFIR_KEYWORD_ASM, KEFIR_C17_STANDARD_VERSION},
                {U"__asm", KEFIR_KEYWORD_ASM, KEFIR_C17_STANDARD_VERSION},
                {U"__typeof__", KEFIR_KEYWORD_TYPEOF, KEFIR_C17_STANDARD_VERSION},
                {U"__typeof", KEFIR_KEYWORD_TYPEOF, KEFIR_C17_STANDARD_VERSION},
                {U"typeof", KEFIR_KEYWORD_TYPEOF, KEFIR_C23_STANDARD_VERSION},
                {U"__typeof_unqual__", KEFIR_KEYWORD_TYPEOF_UNQUAL, KEFIR_C17_STANDARD_VERSION},
                {U"__typeof_unqual", KEFIR_KEYWORD_TYPEOF_UNQUAL, KEFIR_C17_STANDARD_VERSION},
                {U"typeof_unqual", KEFIR_KEYWORD_TYPEOF_UNQUAL, KEFIR_C23_STANDARD_VERSION},
                {U"__auto_type", KEFIR_KEYWORD_AUTO_TYPE, KEFIR_C17_STANDARD_VERSION},
                {U"nullptr", KEFIR_KEYWORD_NULLPTR, KEFIR_C23_STANDARD_VERSION},
                {U"_BitInt", KEFIR_KEYWORD_BITINT, KEFIR_C17_STANDARD_VERSION}};
static const kefir_size_t KEYWORDS_LENGTH = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]);

#define KEYWORD_NONE (~((kefir_trie_value_t) 0))

static kefir_result_t insert_keyword(struct kefir_mem *mem, struct kefir_trie *trie, const kefir_char32_t *literal,
                                     kefir_keyword_token_t keyword) {
    struct kefir_trie_vertex *vertex = NULL;
    if (literal[1] == U'\0') {
        kefir_result_t res = kefir_trie_at(trie, (kefir_trie_key_t) literal[0], &vertex);
        if (res == KEFIR_NOT_FOUND) {
            REQUIRE_OK(kefir_trie_insert_vertex(mem, trie, (kefir_trie_key_t) literal[0], (kefir_trie_value_t) keyword,
                                                &vertex));
        } else {
            REQUIRE_OK(res);
            REQUIRE(vertex->node.value == KEYWORD_NONE,
                    KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Keyword clash in the trie"));
            vertex->node.value = (kefir_trie_value_t) keyword;
        }
    } else {
        kefir_result_t res = kefir_trie_at(trie, (kefir_trie_key_t) literal[0], &vertex);
        if (res == KEFIR_NOT_FOUND) {
            REQUIRE_OK(kefir_trie_insert_vertex(mem, trie, (kefir_trie_key_t) literal[0], KEYWORD_NONE, &vertex));
        } else {
            REQUIRE_OK(res);
        }
        REQUIRE_OK(insert_keyword(mem, &vertex->node, literal + 1, keyword));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_init_keywords(struct kefir_mem *mem, struct kefir_lexer *lexer) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(lexer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid lexer"));

    REQUIRE_OK(kefir_trie_init(&lexer->keywords, KEYWORD_NONE));
    for (kefir_size_t i = 0; i < KEYWORDS_LENGTH; i++) {
        if (KEFIR_STANDARD_VERSION_AT_LEAST(lexer->standard_version, KEYWORDS[i].min_standard_version)) {
            kefir_result_t res = insert_keyword(mem, &lexer->keywords, KEYWORDS[i].literal, KEYWORDS[i].keyword);
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_trie_free(mem, &lexer->keywords);
                return res;
            });
        }
    }
    return KEFIR_OK;
}

static kefir_result_t match_keyword(const kefir_char32_t *string, const struct kefir_trie *trie,
                                    kefir_keyword_token_t *keyword) {
    if (*string == U'\0') {
        REQUIRE(trie->value != KEYWORD_NONE, KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to match keyword"));
        *keyword = (kefir_keyword_token_t) trie->value;
    } else {
        struct kefir_trie_vertex *vertex = NULL;
        REQUIRE_OK(kefir_trie_at(trie, (kefir_trie_key_t) *string, &vertex));
        string++;
        REQUIRE_OK(match_keyword(string, &vertex->node, keyword));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_lexer_get_keyword(const struct kefir_trie *keywords, const kefir_char32_t *string,
                                       kefir_keyword_token_t *keyword) {
    REQUIRE(keywords != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid keyword trie"));
    REQUIRE(string != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid string"));
    REQUIRE(keyword != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to keyword"));

    kefir_result_t res = match_keyword(string, keywords, keyword);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match keyword");
    }
    REQUIRE_OK(res);
    return KEFIR_OK;
}
