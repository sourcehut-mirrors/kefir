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

#ifndef KEFIR_PARSER_BUILDER_H_
#define KEFIR_PARSER_BUILDER_H_

#include "kefir/ast/node.h"
#include "kefir/parser/parser.h"

typedef struct kefir_parser_ast_builder {
    struct kefir_parser *parser;
    struct kefir_list stack;
} kefir_parser_ast_builder_t;

kefir_result_t kefir_parser_ast_builder_init(struct kefir_parser_ast_builder *, struct kefir_parser *);
kefir_result_t kefir_parser_ast_builder_free(struct kefir_mem *, struct kefir_parser_ast_builder *);
kefir_result_t kefir_parser_ast_builder_push(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                             struct kefir_ast_node_base *);
kefir_result_t kefir_parser_ast_builder_pop(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                            struct kefir_ast_node_base **);
kefir_result_t kefir_parser_ast_builder_peek(struct kefir_parser_ast_builder *, struct kefir_ast_node_base **);
kefir_result_t kefir_parser_ast_builder_scan(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                             kefir_parser_rule_fn_t, void *);

typedef kefir_result_t kefir_parser_ast_builder_callback_t(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                           void *);
kefir_result_t kefir_parser_ast_builder_wrap(struct kefir_mem *, struct kefir_parser *, struct kefir_ast_node_base **,
                                             kefir_parser_ast_builder_callback_t, void *);

kefir_result_t kefir_parser_ast_builder_set_source_location(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                            const struct kefir_source_location *);
kefir_result_t kefir_parser_ast_builder_array_subscript(struct kefir_mem *, struct kefir_parser_ast_builder *);
kefir_result_t kefir_parser_ast_builder_function_call(struct kefir_mem *, struct kefir_parser_ast_builder *);
kefir_result_t kefir_parser_ast_builder_function_call_append(struct kefir_mem *, struct kefir_parser_ast_builder *);
kefir_result_t kefir_parser_ast_builder_struct_member(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                      kefir_bool_t, const char *);
kefir_result_t kefir_parser_ast_builder_unary_operation(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                        kefir_ast_unary_operation_type_t);
kefir_result_t kefir_parser_ast_builder_binary_operation(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                         kefir_ast_binary_operation_type_t);
kefir_result_t kefir_parser_ast_builder_cast(struct kefir_mem *, struct kefir_parser_ast_builder *);
kefir_result_t kefir_parser_ast_builder_conditional_operator_ommited1(struct kefir_mem *,
                                                                      struct kefir_parser_ast_builder *);
kefir_result_t kefir_parser_ast_builder_conditional_operator(struct kefir_mem *, struct kefir_parser_ast_builder *);
kefir_result_t kefir_parser_ast_builder_assignment_operator(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                            kefir_ast_assignment_operation_t);
kefir_result_t kefir_parser_ast_builder_comma_operator(struct kefir_mem *, struct kefir_parser_ast_builder *);
kefir_result_t kefir_parser_ast_builder_static_assertion(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                         kefir_bool_t);
kefir_result_t kefir_parser_ast_builder_generic_selection(struct kefir_mem *, struct kefir_parser_ast_builder *);
kefir_result_t kefir_parser_ast_builder_generic_selection_append(struct kefir_mem *, struct kefir_parser_ast_builder *);
kefir_result_t kefir_parser_ast_builder_generic_selection_append_default(struct kefir_mem *,
                                                                         struct kefir_parser_ast_builder *);
kefir_result_t kefir_parser_ast_builder_compound_literal(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                         struct kefir_ast_initializer *);
kefir_result_t kefir_parser_ast_builder_declaration(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                    struct kefir_ast_declarator_specifier_list *);
kefir_result_t kefir_parser_ast_builder_init_declarator(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                        struct kefir_ast_declarator *, struct kefir_ast_initializer *,
                                                        struct kefir_ast_init_declarator **);
kefir_result_t kefir_parser_ast_builder_compound_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                           struct kefir_ast_node_attributes *,
                                                           const struct kefir_ast_pragma_state *);
kefir_result_t kefir_parser_ast_builder_compound_statement_append(struct kefir_mem *,
                                                                  struct kefir_parser_ast_builder *);
kefir_result_t kefir_parser_ast_builder_empty_labeled_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                                const char *, struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_labeled_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                          const char *, struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_case_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                       struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_empty_case_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                             struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_range_case_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                             struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_empty_range_case_statement(struct kefir_mem *,
                                                                   struct kefir_parser_ast_builder *,
                                                                   struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_default_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                          struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_empty_default_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                                struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_if_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                     struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_if_else_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                          struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_switch_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                         struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_while_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                        struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_do_while_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                           struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_for_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                      kefir_bool_t, kefir_bool_t, kefir_bool_t,
                                                      struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_return_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                         struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_return_value_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                               struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_goto_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                       const char *, struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_continue_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                           struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_break_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                        struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_translation_unit(struct kefir_mem *, struct kefir_parser_ast_builder *);
kefir_result_t kefir_parser_ast_builder_translation_unit_append(struct kefir_mem *, struct kefir_parser_ast_builder *);

kefir_result_t kefir_parser_ast_builder_builtin(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                kefir_ast_builtin_operator_t);
kefir_result_t kefir_parser_ast_builder_builtin_append(struct kefir_mem *, struct kefir_parser_ast_builder *);

kefir_result_t kefir_parser_ast_builder_label_address(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                      const char *);
kefir_result_t kefir_parser_ast_builder_goto_address_statement(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                               struct kefir_ast_node_attributes *);

kefir_result_t kefir_parser_ast_builder_statement_expression(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                             struct kefir_ast_node_attributes *);
kefir_result_t kefir_parser_ast_builder_statement_expression_append(struct kefir_mem *,
                                                                    struct kefir_parser_ast_builder *);

kefir_result_t kefir_parser_ast_builder_attribute_list(struct kefir_mem *, struct kefir_parser_ast_builder *);
kefir_result_t kefir_parser_ast_builder_attribute(struct kefir_mem *, struct kefir_parser_ast_builder *, const char *,
                                                  const char *);
kefir_result_t kefir_parser_ast_builder_attribute_parameter(struct kefir_mem *, struct kefir_parser_ast_builder *);
kefir_result_t kefir_parser_ast_builder_attribute_unstructured_parameter(struct kefir_mem *,
                                                                         struct kefir_parser_ast_builder *,
                                                                         const struct kefir_token *);

kefir_result_t kefir_parser_ast_builder_inline_assembly(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                        struct kefir_ast_inline_assembly_qualifiers, const char *);
kefir_result_t kefir_parser_ast_builder_inline_assembly_add_output(struct kefir_mem *,
                                                                   struct kefir_parser_ast_builder *, const char *,
                                                                   const char *);
kefir_result_t kefir_parser_ast_builder_inline_assembly_add_input(struct kefir_mem *, struct kefir_parser_ast_builder *,
                                                                  const char *, const char *);
kefir_result_t kefir_parser_ast_builder_inline_assembly_add_clobber(struct kefir_mem *,
                                                                    struct kefir_parser_ast_builder *, const char *);
kefir_result_t kefir_parser_ast_builder_inline_assembly_add_jump_target(struct kefir_mem *,
                                                                        struct kefir_parser_ast_builder *,
                                                                        const char *);

#endif
