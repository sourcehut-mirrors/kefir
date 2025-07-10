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

#ifndef KEFIR_PREPROCESSOR_DIRECTIVES_H_
#define KEFIR_PREPROCESSOR_DIRECTIVES_H_

#include "kefir/core/source_location.h"
#include "kefir/lexer/source_cursor.h"
#include "kefir/core/list.h"
#include "kefir/lexer/buffer.h"
#include "kefir/lexer/allocator.h"
#include "kefir/preprocessor/tokenizer.h"

typedef struct kefir_preprocessor_context kefir_preprocessor_context_t;

typedef enum kefir_preprocessor_directive_type {
    KEFIR_PREPROCESSOR_DIRECTIVE_IF,
    KEFIR_PREPROCESSOR_DIRECTIVE_IFDEF,
    KEFIR_PREPROCESSOR_DIRECTIVE_IFNDEF,
    KEFIR_PREPROCESSOR_DIRECTIVE_ELIFDEF,
    KEFIR_PREPROCESSOR_DIRECTIVE_ELIFNDEF,
    KEFIR_PREPROCESSOR_DIRECTIVE_ELIF,
    KEFIR_PREPROCESSOR_DIRECTIVE_ELSE,
    KEFIR_PREPROCESSOR_DIRECTIVE_ENDIF,
    KEFIR_PREPROCESSOR_DIRECTIVE_INCLUDE,
    KEFIR_PREPROCESSOR_DIRECTIVE_INCLUDE_NEXT,
    KEFIR_PREPROCESSOR_DIRECTIVE_DEFINE,
    KEFIR_PREPROCESSOR_DIRECTIVE_UNDEF,
    KEFIR_PREPROCESSOR_DIRECTIVE_LINE,
    KEFIR_PREPROCESSOR_DIRECTIVE_ERROR,
    KEFIR_PREPROCESSOR_DIRECTIVE_PRAGMA,
    KEFIR_PREPROCESSOR_DIRECTIVE_EMPTY,
    KEFIR_PREPROCESSOR_DIRECTIVE_NON,
    KEFIR_PREPROCESSOR_DIRECTIVE_PP_TOKEN,
    KEFIR_PREPROCESSOR_DIRECTIVE_SENTINEL
} kefir_preprocessor_directive_type_t;

typedef struct kefir_preprocessor_if_directive {
    struct kefir_token_buffer condition;
} kefir_preprocessor_if_directive_t;

typedef struct kefir_preprocessor_ifdef_directive {
    const char *identifier;
} kefir_preprocessor_ifdef_directive_t;

typedef struct kefir_preprocessor_define_directive {
    kefir_bool_t object;
    const char *identifier;
    struct kefir_list parameters;
    kefir_bool_t vararg;
    const char *vararg_parameter;
    struct kefir_token_buffer replacement;
} kefir_preprocessor_define_directive_t;

typedef struct kefir_preprocessor_undef_directive {
    const char *identifier;
} kefir_preprocessor_undef_directive_t;

typedef struct kefir_preprocessor_directive {
    kefir_preprocessor_directive_type_t type;
    union {
        struct kefir_preprocessor_if_directive if_directive;
        struct kefir_preprocessor_ifdef_directive ifdef_directive;
        struct kefir_preprocessor_define_directive define_directive;
        struct kefir_preprocessor_undef_directive undef_directive;
        struct kefir_token_buffer pp_tokens;
        struct kefir_token pp_token;
    };
    struct kefir_source_location source_location;
} kefir_preprocessor_directive_t;

typedef struct kefir_preprocessor_directive_scanner {
    struct kefir_lexer *lexer;
    struct kefir_preprocessor_tokenizer_context tokenizer_context;
    kefir_bool_t newline_flag;
    const struct kefir_preprocessor_context *context;
} kefir_preprocessor_directive_scanner_t;

typedef struct kefir_preprocessor_directive_scanner_state {
    struct kefir_lexer_source_cursor_state cursor_state;
    kefir_bool_t newline_flag;
} kefir_preprocessor_directive_scanner_state_t;

kefir_result_t kefir_preprocessor_directive_scanner_init(struct kefir_preprocessor_directive_scanner *,
                                                         struct kefir_lexer *,
                                                         const struct kefir_preprocessor_context *);

kefir_result_t kefir_preprocessor_directive_scanner_save(const struct kefir_preprocessor_directive_scanner *,
                                                         struct kefir_preprocessor_directive_scanner_state *);
kefir_result_t kefir_preprocessor_directive_scanner_restore(struct kefir_preprocessor_directive_scanner *,
                                                            const struct kefir_preprocessor_directive_scanner_state *);
kefir_result_t kefir_preprocessor_directive_scanner_skip_line(struct kefir_mem *,
                                                              struct kefir_preprocessor_directive_scanner *);
kefir_result_t kefir_preprocessor_directive_scanner_match(struct kefir_mem *,
                                                          struct kefir_preprocessor_directive_scanner *,
                                                          kefir_preprocessor_directive_type_t *);
kefir_result_t kefir_preprocessor_directive_scanner_next(struct kefir_mem *,
                                                         struct kefir_preprocessor_directive_scanner *,
                                                         struct kefir_token_allocator *,
                                                         struct kefir_preprocessor_directive *);
kefir_result_t kefir_preprocessor_directive_free(struct kefir_mem *, struct kefir_preprocessor_directive *);

#endif
