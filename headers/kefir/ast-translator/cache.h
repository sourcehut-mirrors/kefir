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

#ifndef KEFIR_AST_TRANSLATOR_CACHE_H_
#define KEFIR_AST_TRANSLATOR_CACHE_H_

#include "kefir/ast-translator/base.h"
#include "kefir/ast-translator/type.h"
#include "kefir/ast/type.h"
#include "kefir/core/hashtree.h"
#include "kefir/core/source_location.h"

typedef struct kefir_ast_translator_context_type_cache {
    const struct kefir_ast_translator_context *context;
    struct kefir_hashtree types;
} kefir_ast_translator_context_type_cache_t;

kefir_result_t kefir_ast_translator_context_type_cache_init(struct kefir_ast_translator_context_type_cache *,
                                                            const struct kefir_ast_translator_context *);
kefir_result_t kefir_ast_translator_context_type_cache_free(struct kefir_mem *,
                                                            struct kefir_ast_translator_context_type_cache *);

kefir_result_t kefir_ast_translator_context_type_cache_get_type(struct kefir_mem *,
                                                                struct kefir_ast_translator_context_type_cache *,
                                                                const struct kefir_ast_type *,
                                                                const struct kefir_ast_translator_type **,
                                                                const struct kefir_source_location *);

#endif
