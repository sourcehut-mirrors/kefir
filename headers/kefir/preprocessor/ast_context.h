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

#ifndef KEFIR_PREPROCESSOR_AST_CONTEXT_H_
#define KEFIR_PREPROCESSOR_AST_CONTEXT_H_

#include "kefir/ast/context.h"

typedef struct kefir_preprocessor_ast_context {
    struct kefir_ast_context context;
    struct kefir_ast_type_bundle type_bundle;
    struct kefir_ast_context_type_cache cache;
    struct kefir_bigint_pool bigint_pool;
    struct kefir_ast_context_configuration configuration;
} kefir_preprocessor_ast_context_t;

kefir_result_t kefir_preprocessor_ast_context_init(struct kefir_mem *, struct kefir_preprocessor_ast_context *,
                                                   struct kefir_string_pool *, const struct kefir_ast_type_traits *,
                                                   const struct kefir_ast_target_environment *,
                                                   const struct kefir_ast_context_extensions *);

kefir_result_t kefir_preprocessor_ast_context_free(struct kefir_mem *, struct kefir_preprocessor_ast_context *);

#endif
