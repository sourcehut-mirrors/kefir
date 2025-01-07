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

#ifndef KEFIR_AST_TRANSLATOR_LVALUE_H_
#define KEFIR_AST_TRANSLATOR_LVALUE_H_

#include "kefir/core/mem.h"
#include "kefir/ast-translator/context.h"
#include "kefir/ir/builder.h"
#include "kefir/ast/scope.h"

kefir_result_t kefir_ast_translator_object_lvalue(struct kefir_mem *, struct kefir_ast_translator_context *,
                                                  struct kefir_irbuilder_block *, const char *,
                                                  const struct kefir_ast_scoped_identifier *);

kefir_result_t kefir_ast_translator_function_lvalue(struct kefir_mem *, struct kefir_ast_translator_context *,
                                                    struct kefir_irbuilder_block *, const char *);

kefir_result_t kefir_ast_translate_array_subscript_lvalue(struct kefir_mem *, struct kefir_ast_translator_context *,
                                                          struct kefir_irbuilder_block *,
                                                          const struct kefir_ast_array_subscript *);

kefir_result_t kefir_ast_translate_struct_member_lvalue(struct kefir_mem *, struct kefir_ast_translator_context *,
                                                        struct kefir_irbuilder_block *,
                                                        const struct kefir_ast_struct_member *);

kefir_result_t kefir_ast_translate_compound_literal_lvalue(struct kefir_mem *, struct kefir_ast_translator_context *,
                                                           struct kefir_irbuilder_block *,
                                                           const struct kefir_ast_compound_literal *);

#endif
