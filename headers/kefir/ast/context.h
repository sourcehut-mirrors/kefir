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

#ifndef KEFIR_AST_CONTEXT_H_
#define KEFIR_AST_CONTEXT_H_

#include "kefir/core/mem.h"
#include "kefir/core/standard_version.h"
#include "kefir/ast/base.h"
#include "kefir/ast/scope.h"
#include "kefir/ast/target_environment.h"
#include "kefir/ast/temporaries.h"
#include "kefir/ast/flow_control.h"
#include "kefir/ast/type/struct.h"
#include "kefir/ast/initializer.h"
#include "kefir/ast/constants.h"
#include "kefir/ast/declarator.h"
#include "kefir/ast/cache.h"
#include "kefir/util/bigint.h"
#include <stdio.h>

typedef struct kefir_ast_identifier_flat_scope kefir_ast_identifier_flat_scope_t;

typedef struct kefir_ast_context_configuration {
    kefir_c_language_standard_version_t standard_version;
    struct {
        kefir_bool_t permissive_pointer_conv;
        kefir_bool_t non_strict_qualifiers;
        kefir_bool_t fixed_enum_type;
        kefir_bool_t implicit_function_declaration;
        kefir_bool_t ext_pointer_arithmetics;
        kefir_bool_t missing_braces_subobj;
        kefir_bool_t int_to_pointer;
    } analysis;

    FILE *warning_output;
} kefir_ast_context_configuration_t;

kefir_result_t kefir_ast_context_configuration_defaults(struct kefir_ast_context_configuration *);

typedef struct kefir_ast_context_extensions {
    kefir_result_t (*on_init)(struct kefir_mem *, struct kefir_ast_context *);
    kefir_result_t (*on_free)(struct kefir_mem *, struct kefir_ast_context *);
    kefir_result_t (*analyze_extension_node)(struct kefir_mem *, const struct kefir_ast_context *,
                                             struct kefir_ast_node_base *);
    kefir_result_t (*evaluate_constant_extension_node)(struct kefir_mem *, const struct kefir_ast_context *,
                                                       const struct kefir_ast_node_base *,
                                                       struct kefir_ast_constant_expression_value *);
    kefir_result_t (*evaluate_constant_pointer_extension_node)(struct kefir_mem *, const struct kefir_ast_context *,
                                                               const struct kefir_ast_node_base *,
                                                               struct kefir_ast_constant_expression_pointer *);
    kefir_result_t (*before_node_analysis)(struct kefir_mem *, const struct kefir_ast_context *,
                                           struct kefir_ast_node_base *, struct kefir_ast_visitor *);
    kefir_result_t (*after_node_analysis)(struct kefir_mem *, const struct kefir_ast_context *,
                                          struct kefir_ast_node_base *);
    void *payload;
} kefir_ast_context_extensions_t;

typedef struct kefir_ast_context {
    kefir_result_t (*resolve_ordinary_identifier)(const struct kefir_ast_context *, const char *,
                                                  const struct kefir_ast_scoped_identifier **);
    kefir_result_t (*resolve_tag_identifier)(const struct kefir_ast_context *, const char *,
                                             const struct kefir_ast_scoped_identifier **);
    kefir_result_t (*resolve_label_identifier)(const struct kefir_ast_context *, const char *,
                                               const struct kefir_ast_scoped_identifier **);
    kefir_result_t (*allocate_temporary_value)(struct kefir_mem *, const struct kefir_ast_context *,
                                               const struct kefir_ast_type *, kefir_ast_scoped_identifier_storage_t,
                                               struct kefir_ast_initializer *, const struct kefir_source_location *,
                                               struct kefir_ast_temporary_identifier *);

    kefir_result_t (*define_tag)(struct kefir_mem *, const struct kefir_ast_context *, const struct kefir_ast_type *,
                                 const struct kefir_ast_declarator_attributes *, const struct kefir_source_location *);
    kefir_result_t (*define_constant)(struct kefir_mem *, const struct kefir_ast_context *, const char *,
                                      const struct kefir_ast_constant_expression_value *, const struct kefir_ast_type *,
                                      const struct kefir_ast_declarator_attributes *,
                                      const struct kefir_source_location *);
    kefir_result_t (*refine_constant_type)(struct kefir_mem *, const struct kefir_ast_context *, const char *,
                                           const struct kefir_ast_type *, const struct kefir_source_location *);
    kefir_result_t (*define_identifier)(struct kefir_mem *, const struct kefir_ast_context *, kefir_bool_t,
                                        const char *, const struct kefir_ast_type *,
                                        kefir_ast_scoped_identifier_storage_t, kefir_ast_function_specifier_t,
                                        struct kefir_ast_alignment *, struct kefir_ast_initializer *,
                                        const struct kefir_ast_declarator_attributes *,
                                        const struct kefir_source_location *,
                                        const struct kefir_ast_scoped_identifier **);
    kefir_result_t (*reference_label)(struct kefir_mem *, const struct kefir_ast_context *, const char *,
                                      struct kefir_ast_flow_control_structure *, const struct kefir_source_location *,
                                      const struct kefir_ast_scoped_identifier **);
    kefir_result_t (*reference_public_label)(struct kefir_mem *, const struct kefir_ast_context *, const char *,
                                             struct kefir_ast_flow_control_structure *,
                                             const struct kefir_source_location *,
                                             const struct kefir_ast_scoped_identifier **);
    kefir_result_t (*push_block)(struct kefir_mem *, const struct kefir_ast_context *,
                                 const struct kefir_ast_identifier_flat_scope **,
                                 const struct kefir_ast_identifier_flat_scope **);
    kefir_result_t (*pop_block)(struct kefir_mem *, const struct kefir_ast_context *);
    kefir_result_t (*push_external_ordinary_scope)(struct kefir_mem *, struct kefir_ast_identifier_flat_scope *,
                                                   const struct kefir_ast_context *);
    kefir_result_t (*pop_external_oridnary_scope)(struct kefir_mem *, const struct kefir_ast_context *);
    kefir_result_t (*current_flow_control_point)(struct kefir_mem *, const struct kefir_ast_context *,
                                                 struct kefir_ast_flow_control_point **);

    struct kefir_string_pool *symbols;
    const struct kefir_ast_type_traits *type_traits;
    struct kefir_ast_type_bundle *type_bundle;
    struct kefir_bigint_pool *bigint_pool;
    const struct kefir_ast_target_environment *target_env;
    struct kefir_ast_context_type_cache *cache;
    kefir_ast_type_analysis_context_t type_analysis_context;
    struct kefir_ast_flow_control_tree *flow_control_tree;
    struct kefir_ast_global_context *global_context;
    struct kefir_list *function_decl_contexts;
    const struct kefir_ast_scoped_identifier *surrounding_function;
    const char *surrounding_function_name;
    const struct kefir_ast_context_configuration *configuration;

    const struct kefir_ast_context_extensions *extensions;
    void *extensions_payload;

    void *payload;
} kefir_ast_context_t;

#endif
