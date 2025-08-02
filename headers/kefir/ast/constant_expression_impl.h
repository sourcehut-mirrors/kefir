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

#ifndef KEFIR_AST_CONSTANT_EXPRESSION_IMPL_H_
#define KEFIR_AST_CONSTANT_EXPRESSION_IMPL_H_

#include "kefir/ast/constant_expression.h"
#include "kefir/ast/node.h"
#include "kefir/ast/context.h"

#define EVAL(_id, _type)                                                                                 \
    kefir_result_t kefir_ast_evaluate_##_id##_node(struct kefir_mem *, const struct kefir_ast_context *, \
                                                   const _type *, struct kefir_ast_constant_expression_value *)

EVAL(scalar, struct kefir_ast_constant);
EVAL(identifier, struct kefir_ast_identifier);
EVAL(struct_member, struct kefir_ast_struct_member);
EVAL(array_subscript, struct kefir_ast_array_subscript);
EVAL(string_literal, struct kefir_ast_string_literal);
EVAL(compound_literal, struct kefir_ast_compound_literal);
EVAL(label_address, struct kefir_ast_label_address);
EVAL(generic_selection, struct kefir_ast_generic_selection);
EVAL(unary_operation, struct kefir_ast_unary_operation);
EVAL(binary_operation, struct kefir_ast_binary_operation);
EVAL(conditional_operator, struct kefir_ast_conditional_operator);
EVAL(cast_operator, struct kefir_ast_cast_operator);
EVAL(builtin, struct kefir_ast_builtin);
EVAL(comma_operator, struct kefir_ast_comma_operator);
EVAL(function_call, struct kefir_ast_function_call);
#undef EVAL

kefir_result_t kefir_ast_constant_expression_value_evaluate_lvalue_reference(
    struct kefir_mem *, const struct kefir_ast_context *, const struct kefir_ast_node_base *,
    struct kefir_ast_constant_expression_pointer *);

kefir_result_t kefir_ast_evaluate_builtin_ffs_constant_expression_value(kefir_uint64_t arg, kefir_size_t bits,
                                                                        struct kefir_ast_constant_expression_value *);
kefir_result_t kefir_ast_evaluate_builtin_clz_constant_expression_value(kefir_uint64_t, kefir_size_t, kefir_int64_t *,
                                                                        struct kefir_ast_constant_expression_value *,
                                                                        const struct kefir_source_location *);
kefir_result_t kefir_ast_evaluate_builtin_ctz_constant_expression_value(kefir_uint64_t, kefir_size_t, kefir_int64_t *,
                                                                        struct kefir_ast_constant_expression_value *,
                                                                        const struct kefir_source_location *);
kefir_result_t kefir_ast_evaluate_builtin_clrsb_constant_expression_value(kefir_uint64_t, kefir_size_t,
                                                                          struct kefir_ast_constant_expression_value *);
kefir_result_t kefir_ast_evaluate_builtin_popcount_constant_expression_value(
    kefir_uint64_t, kefir_size_t, struct kefir_ast_constant_expression_value *);
kefir_result_t kefir_ast_evaluate_builtin_parity_constant_expression_value(
    kefir_uint64_t, kefir_size_t, struct kefir_ast_constant_expression_value *);

#endif
