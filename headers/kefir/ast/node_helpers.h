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

#ifndef KEFIR_AST_NODE_HELPERS_H_
#define KEFIR_AST_NODE_HELPERS_H_

#include "kefir/ast/node.h"

#ifndef KEFIR_AST_NODE_INTERNAL_DEF
#error "This file should only be included from kefir/ast/node.h"
#endif

kefir_result_t kefir_ast_generic_selection_append(struct kefir_mem *, struct kefir_ast_generic_selection *,
                                                  struct kefir_ast_type_name *, struct kefir_ast_node_base *);

kefir_result_t kefir_ast_function_call_append(struct kefir_mem *, struct kefir_ast_function_call *,
                                              struct kefir_ast_node_base *);

kefir_result_t kefir_ast_builtin_append(struct kefir_mem *, struct kefir_ast_builtin *, struct kefir_ast_node_base *);

kefir_result_t kefir_ast_comma_append(struct kefir_mem *, struct kefir_ast_comma_operator *,
                                      struct kefir_ast_node_base *);

kefir_result_t kefir_ast_compound_literal_set_initializer(struct kefir_mem *, struct kefir_ast_compound_literal *,
                                                          struct kefir_ast_initializer *);
struct kefir_ast_declaration *kefir_ast_new_single_declaration(struct kefir_mem *, struct kefir_ast_declarator *,
                                                               struct kefir_ast_initializer *,
                                                               struct kefir_ast_init_declarator **);

kefir_result_t kefir_ast_declaration_unpack_single(struct kefir_ast_declaration *, struct kefir_ast_init_declarator **);

kefir_result_t kefir_ast_attribute_list_append(struct kefir_mem *, struct kefir_string_pool *, const char *,
                                               const char *, struct kefir_ast_attribute_list *,
                                               struct kefir_ast_attribute **);

kefir_result_t kefir_ast_inline_assembly_add_output(struct kefir_mem *, struct kefir_string_pool *,
                                                    struct kefir_ast_inline_assembly *, const char *, const char *,
                                                    struct kefir_ast_node_base *);
kefir_result_t kefir_ast_inline_assembly_add_input(struct kefir_mem *, struct kefir_string_pool *,
                                                   struct kefir_ast_inline_assembly *, const char *, const char *,
                                                   struct kefir_ast_node_base *);
kefir_result_t kefir_ast_inline_assembly_add_clobber(struct kefir_mem *, struct kefir_string_pool *,
                                                     struct kefir_ast_inline_assembly *, const char *);
kefir_result_t kefir_ast_inline_assembly_add_jump_label(struct kefir_mem *, struct kefir_string_pool *,
                                                        struct kefir_ast_inline_assembly *, const char *);

#endif
