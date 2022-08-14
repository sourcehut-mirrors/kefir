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

#ifndef KEFIR_PREPROCESSOR_PREPROCESSOR_H_
#define KEFIR_PREPROCESSOR_PREPROCESSOR_H_

#include "kefir/core/extensions.h"
#include "kefir/lexer/lexer.h"
#include "kefir/preprocessor/user_macro.h"
#include "kefir/preprocessor/predefined_macro.h"
#include "kefir/preprocessor/macro_scope.h"
#include "kefir/preprocessor/source_file.h"
#include "kefir/preprocessor/directives.h"
#include "kefir/ast/context.h"
#include "kefir/core/data_model.h"
#include "kefir/core/hashtree.h"
#include <time.h>

typedef struct kefir_preprocessor_context kefir_preprocessor_context_t;
typedef struct kefir_preprocessor kefir_preprocessor_t;

typedef struct kefir_preprocessor_extensions {
    kefir_result_t (*on_init)(struct kefir_mem *, struct kefir_preprocessor *);
    kefir_result_t (*on_free)(struct kefir_mem *, struct kefir_preprocessor *);
    kefir_result_t (*on_next_directive)(struct kefir_mem *, struct kefir_preprocessor *,
                                        struct kefir_preprocessor_directive *);
    kefir_result_t (*before_run)(struct kefir_mem *, struct kefir_preprocessor *, struct kefir_token_buffer *);
    kefir_result_t (*after_run)(struct kefir_mem *, struct kefir_preprocessor *, struct kefir_token_buffer *);
    const struct kefir_lexer_extensions *lexer_extensions;
    const struct kefir_parser_extensions *parser;
    void *payload;
} kefir_preprocessor_extensions_t;

typedef struct kefir_preprocessor_context_extensions {
    kefir_result_t (*on_init)(struct kefir_mem *, struct kefir_preprocessor_context *);
    kefir_result_t (*on_free)(struct kefir_mem *, struct kefir_preprocessor_context *);
    void *payload;
} kefir_preprocessor_context_extensions_t;

typedef struct kefir_preprocessor_environment {
    time_t timestamp;
    kefir_bool_t hosted;
    kefir_ulong_t version;
    kefir_ulong_t stdc_iso10646;
    kefir_bool_t stdc_mb_might_neq_wc;
    kefir_bool_t stdc_utf16;
    kefir_bool_t stdc_utf32;
    kefir_bool_t stdc_analyzable;
    kefir_bool_t stdc_iec559;
    kefir_bool_t stdc_iec559_complex;
    kefir_ulong_t stdc_lib_ext1;
    kefir_bool_t stdc_no_atomics;
    kefir_bool_t stdc_no_complex;
    kefir_bool_t stdc_no_threads;
    kefir_bool_t stdc_no_vla;
    const struct kefir_data_model_descriptor *data_model;
} kefir_preprocessor_environment_t;

typedef struct kefir_preprocessor_configuration {
    kefir_bool_t named_macro_vararg;
    kefir_bool_t include_next;
} kefir_preprocessor_configuration_t;

kefir_result_t kefir_preprocessor_configuration_default(struct kefir_preprocessor_configuration *);

typedef struct kefir_preprocessor_context {
    struct kefir_preprocessor_user_macro_scope user_macros;
    struct kefir_hashtree undefined_macros;
    const struct kefir_preprocessor_source_locator *source_locator;
    struct kefir_ast_context *ast_context;
    const struct kefir_preprocessor_configuration *preprocessor_config;
    struct kefir_preprocessor_environment environment;
    const struct kefir_preprocessor_context_extensions *extensions;
    struct {
        kefir_uint_t counter;
    } state;
    void *extensions_payload;
} kefir_preprocessor_context_t;

kefir_result_t kefir_preprocessor_context_init(struct kefir_mem *, struct kefir_preprocessor_context *,
                                               const struct kefir_preprocessor_source_locator *,
                                               struct kefir_ast_context *,
                                               const struct kefir_preprocessor_context_extensions *);
kefir_result_t kefir_preprocessor_context_free(struct kefir_mem *, struct kefir_preprocessor_context *);

typedef struct kefir_preprocessor {
    const struct kefir_preprocessor_source_file_info *current_file;
    struct kefir_lexer lexer;
    struct kefir_preprocessor_context *context;
    struct kefir_preprocessor_directive_scanner directive_scanner;
    struct kefir_preprocessor_predefined_macro_scope predefined_macros;
    struct kefir_preprocessor_overlay_macro_scope macro_overlay;
    struct kefir_preprocessor_macro_scope *macros;
    const struct kefir_preprocessor *parent;
    const struct kefir_preprocessor_extensions *extensions;
    void *extension_payload;
} kefir_preprocessor_t;

typedef enum kefir_preprocessor_substitution_context {
    KEFIR_PREPROCESSOR_SUBSTITUTION_NORMAL,
    KEFIR_PREPROCESSOR_SUBSTITUTION_IF_CONDITION
} kefir_preprocessor_substitution_context_t;

kefir_result_t kefir_preprocessor_init(struct kefir_mem *, struct kefir_preprocessor *, struct kefir_symbol_table *,
                                       struct kefir_lexer_source_cursor *, const struct kefir_lexer_context *,
                                       struct kefir_preprocessor_context *,
                                       const struct kefir_preprocessor_source_file_info *,
                                       const struct kefir_preprocessor_extensions *);
kefir_result_t kefir_preprocessor_free(struct kefir_mem *, struct kefir_preprocessor *);

kefir_result_t kefir_preprocessor_skip_group(struct kefir_mem *, struct kefir_preprocessor *);
kefir_result_t kefir_preprocessor_run_group(struct kefir_mem *, struct kefir_preprocessor *,
                                            struct kefir_token_buffer *);
kefir_result_t kefir_preprocessor_run_substitutions(struct kefir_mem *, struct kefir_preprocessor *,
                                                    struct kefir_token_buffer *,
                                                    kefir_preprocessor_substitution_context_t);
kefir_result_t kefir_preprocessor_run(struct kefir_mem *, struct kefir_preprocessor *, struct kefir_token_buffer *);

kefir_result_t kefir_preprocessor_token_convert(struct kefir_mem *, struct kefir_preprocessor *, struct kefir_token *,
                                                const struct kefir_token *);
kefir_result_t kefir_preprocessor_token_convert_buffer(struct kefir_mem *, struct kefir_preprocessor *,
                                                       struct kefir_token_buffer *, const struct kefir_token_buffer *);

#endif
