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

#ifndef KEFIR_AST_ANALYZER_ANALYZER_H_
#define KEFIR_AST_ANALYZER_ANALYZER_H_

#include "kefir/core/mem.h"
#include "kefir/ast/node.h"
#include "kefir/ast/context.h"
#include "kefir/ast/alignment.h"
#include "kefir/core/source_location.h"

typedef struct kefir_ast_analysis_parameters {
    struct kefir_mem *mem;
    struct kefir_ast_node_base *base;
    const struct kefir_ast_context *context;
} kefir_ast_analysis_parameters_t;

kefir_result_t kefir_ast_analyze_constant_expression(struct kefir_mem *, const struct kefir_ast_context *,
                                                     struct kefir_ast_constant_expression *);

kefir_result_t kefir_ast_analyze_alignment(struct kefir_mem *, const struct kefir_ast_context *,
                                           struct kefir_ast_alignment *);

kefir_result_t kefir_ast_node_is_lvalue_reference_constant(const struct kefir_ast_context *,
                                                           const struct kefir_ast_node_base *, kefir_bool_t *);

kefir_result_t kefir_ast_type_assignable(struct kefir_mem *, const struct kefir_ast_context *,
                                         const struct kefir_ast_type *, kefir_bool_t, const struct kefir_ast_type *);

kefir_result_t kefir_ast_node_assignable(struct kefir_mem *, const struct kefir_ast_context *,
                                         const struct kefir_ast_node_base *, const struct kefir_ast_type *);

kefir_result_t kefir_ast_analyze_node(struct kefir_mem *, const struct kefir_ast_context *,
                                      struct kefir_ast_node_base *);

kefir_result_t kefir_ast_analyze_type(struct kefir_mem *, const struct kefir_ast_context *,
                                      kefir_ast_type_analysis_context_t, const struct kefir_ast_type *,
                                      const struct kefir_source_location *);

kefir_result_t kefir_ast_try_analyze_identifier(struct kefir_mem *, const struct kefir_ast_context *,
                                                const struct kefir_ast_identifier *, struct kefir_ast_node_base *);

#endif
