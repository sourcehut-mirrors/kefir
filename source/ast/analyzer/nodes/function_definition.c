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

#include <string.h>
#include "kefir/ast/analyzer/nodes.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/ast/analyzer/declarator.h"
#include "kefir/ast/local_context.h"
#include "kefir/ast/type_conv.h"
#include "kefir/ast/downcast.h"
#include "kefir/ast/runtime.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

static kefir_result_t analyze_function_parameters(const struct kefir_ast_function_definition *node,
                                                  const struct kefir_ast_declarator_function *decl_func,
                                                  kefir_bool_t *has_long_double_params) {
    REQUIRE(
        kefir_list_length(&node->declarations) == 0,
        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->base.source_location,
                               "Function definition with non-empty parameter list shall not contain any declarations"));

    kefir_result_t res;
    for (const struct kefir_list_entry *iter = kefir_list_head(&decl_func->parameters); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_node_base *, param, iter->value);
        REQUIRE(param->properties.category == KEFIR_AST_NODE_CATEGORY_DECLARATION,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected function parameter to be declaration"));

        struct kefir_ast_declaration *param_decl = NULL;
        REQUIRE_MATCH_OK(&res, kefir_ast_downcast_declaration(param, &param_decl, false),
                         KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &param->source_location,
                                                "Expected function parameter to be a declaration"));
        REQUIRE(kefir_list_length(&param_decl->init_declarators) == 1,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected function parameter to have exactly one declarator"));

        ASSIGN_DECL_CAST(struct kefir_ast_init_declarator *, init_decl,
                         kefir_list_head(&param_decl->init_declarators)->value);
        struct kefir_ast_declarator_identifier *param_identifier = NULL;
        REQUIRE_OK(kefir_ast_declarator_unpack_identifier(init_decl->declarator, &param_identifier));
        REQUIRE(init_decl->base.properties.type != NULL,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Function definition parameters shall have definite types"));

        if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(kefir_ast_unqualified_type(init_decl->base.properties.type))) {
            *has_long_double_params = true;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t analyze_function_parameter_identifiers_impl(
    struct kefir_mem *mem, const struct kefir_ast_context *context, const struct kefir_ast_function_definition *node,
    const struct kefir_ast_declarator_function *decl_func, struct kefir_ast_local_context *local_context,
    const struct kefir_hashtree *argtree, kefir_bool_t *has_long_double_params) {
    kefir_result_t res;
    for (const struct kefir_list_entry *iter = kefir_list_head(&node->declarations); iter != NULL;
         kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(struct kefir_ast_node_base *, decl_node, iter->value);
        struct kefir_ast_declaration *decl_list = NULL;
        REQUIRE_MATCH_OK(
            &res, kefir_ast_downcast_declaration(decl_node, &decl_list, false),
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_node->source_location,
                                   "Function definition declaration list shall contain exclusively declarations"));

        for (const struct kefir_list_entry *iter = kefir_list_head(&decl_list->init_declarators); iter != NULL;
             kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(struct kefir_ast_init_declarator *, decl, iter->value);

            const char *identifier = NULL;
            const struct kefir_ast_type *type = NULL, *original_type = NULL;
            kefir_ast_scoped_identifier_storage_t storage = KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN;
            kefir_ast_function_specifier_t function_specifier = KEFIR_AST_FUNCTION_SPECIFIER_NONE;
            kefir_size_t alignment = 0;
            REQUIRE_OK(kefir_ast_analyze_declaration(
                mem, &local_context->context, &decl_list->specifiers, decl->declarator, &identifier, &original_type,
                &storage, &function_specifier, &alignment, KEFIR_AST_DECLARATION_ANALYSIS_NORMAL, NULL,
                &decl->base.source_location));
            REQUIRE(!KEFIR_AST_TYPE_IS_AUTO(original_type),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl->base.source_location,
                                           "Unexpected auto type specifier"));

            if (identifier != NULL) {
                identifier = kefir_string_pool_insert(mem, context->symbols, identifier, NULL);
                REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                                            "Failed to insert parameter identifier into symbol table"));
            }

            type = kefir_ast_type_conv_adjust_function_parameter(mem, context->type_bundle, original_type);

            REQUIRE(kefir_hashtree_has(argtree, (kefir_hashtree_key_t) identifier),
                    KEFIR_SET_SOURCE_ERROR(
                        KEFIR_ANALYSIS_ERROR, &decl_node->source_location,
                        "Function definition declaration list declarations shall refer to identifier list"));
            REQUIRE(storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN ||
                        storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_REGISTER,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &decl_node->source_location,
                                           "Function definition declaration list shall not contain "
                                           "storage class specifiers other than register"));
            REQUIRE(alignment == 0, KEFIR_SET_SOURCE_ERROR(
                                        KEFIR_ANALYSIS_ERROR, &decl_node->source_location,
                                        "Function definition declaration list shall not contain alignment specifiers"));

            const struct kefir_ast_scoped_identifier *param_scoped_id = NULL;
            REQUIRE_OK(local_context->context.define_identifier(mem, &local_context->context, true, identifier, type,
                                                                storage, function_specifier, NULL, NULL, NULL,
                                                                &decl_node->source_location, &param_scoped_id));

            decl->base.properties.category = KEFIR_AST_NODE_CATEGORY_INIT_DECLARATOR;
            decl->base.properties.declaration_props.alignment = 0;
            decl->base.properties.declaration_props.function = KEFIR_AST_FUNCTION_SPECIFIER_NONE;
            decl->base.properties.declaration_props.identifier = identifier;
            decl->base.properties.declaration_props.scoped_id = param_scoped_id;
            decl->base.properties.declaration_props.static_assertion = false;
            decl->base.properties.declaration_props.storage = storage;
            decl->base.properties.declaration_props.original_type = original_type;
            decl->base.properties.type = type;

            if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(kefir_ast_unqualified_type(type))) {
                *has_long_double_params = true;
            }
        }

        decl_list->base.properties.category = KEFIR_AST_NODE_CATEGORY_DECLARATION;
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(&decl_func->parameters); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_node_base *, param, iter->value);

        struct kefir_ast_identifier *id_node = NULL;
        REQUIRE_MATCH_OK(&res, kefir_ast_downcast_identifier(param, &id_node, false),
                         KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected parameter to be AST identifier"));

        REQUIRE_OK(kefir_ast_try_analyze_identifier(mem, &local_context->context, id_node, param));
    }
    return KEFIR_OK;
}

static kefir_result_t analyze_function_parameter_identifiers(struct kefir_mem *mem,
                                                             const struct kefir_ast_context *context,
                                                             const struct kefir_ast_function_definition *node,
                                                             const struct kefir_ast_declarator_function *decl_func,
                                                             struct kefir_ast_local_context *local_context,
                                                             kefir_bool_t *has_long_double_params) {
    struct kefir_hashtree argtree;
    REQUIRE_OK(kefir_hashtree_init(&argtree, &kefir_hashtree_str_ops));

    kefir_result_t res = KEFIR_OK;
    for (const struct kefir_list_entry *iter = kefir_list_head(&decl_func->parameters); iter != NULL && res == KEFIR_OK;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_node_base *, param, iter->value);
        struct kefir_ast_identifier *id_node = NULL;
        REQUIRE_MATCH(&res, kefir_ast_downcast_identifier(param, &id_node, false),
                      KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected parameter to be AST identifier"));
        REQUIRE_CHAIN(&res, kefir_hashtree_insert(mem, &argtree, (kefir_hashtree_key_t) id_node->identifier,
                                                  (kefir_hashtree_value_t) 0));
    }
    REQUIRE_CHAIN(&res, analyze_function_parameter_identifiers_impl(mem, context, node, decl_func, local_context,
                                                                    &argtree, has_long_double_params));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &argtree);
        return res;
    });
    REQUIRE_OK(kefir_hashtree_free(mem, &argtree));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_analyze_function_definition_node(struct kefir_mem *mem,
                                                          const struct kefir_ast_context *context,
                                                          const struct kefir_ast_function_definition *node,
                                                          struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST function definition"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST base node"));

    kefir_result_t res = KEFIR_OK;

    REQUIRE_OK(kefir_ast_node_properties_init(&base->properties));
    base->properties.category = KEFIR_AST_NODE_CATEGORY_FUNCTION_DEFINITION;

    struct kefir_ast_local_context *local_context = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_local_context));
    REQUIRE(local_context != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST local context"));
    res = kefir_ast_local_context_init(mem, context->global_context, local_context);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, local_context);
        return res;
    });

    struct kefir_ast_flow_control_structure_associated_scopes associated_scopes;
    REQUIRE_OK(local_context->context.push_block(mem, &local_context->context, &associated_scopes.ordinary_scope,
                                                 &associated_scopes.tag_scope));

    const struct kefir_ast_type *type = NULL;
    kefir_ast_scoped_identifier_storage_t storage = KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN;
    kefir_size_t alignment = 0;
    const char *function_identifier = NULL;
    struct kefir_ast_declarator_attributes attributes;
    REQUIRE_CHAIN(&res, kefir_ast_analyze_declaration(mem, &local_context->context, &node->specifiers, node->declarator,
                                                      &function_identifier, &type, &storage,
                                                      &base->properties.function_definition.function, &alignment,
                                                      KEFIR_AST_DECLARATION_ANALYSIS_FUNCTION_DEFINITION_CONTEXT,
                                                      &attributes, &node->base.source_location));
    REQUIRE_CHAIN_SET(
        &res, !KEFIR_AST_TYPE_IS_AUTO(type),
        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->base.source_location, "Unexpected auto type specifier"));
    REQUIRE_CHAIN(
        &res, kefir_ast_analyze_type(mem, context, context->type_analysis_context, type, &node->base.source_location));

    REQUIRE_CHAIN_SET(&res, type->tag == KEFIR_AST_TYPE_FUNCTION,
                      KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->base.source_location,
                                             "Function definition declarator shall have function type"));
    REQUIRE_CHAIN_SET(&res, function_identifier != NULL,
                      KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->base.source_location,
                                             "Function definition shall have non-empty identifier"));
    REQUIRE_CHAIN_SET(&res, alignment == 0,
                      KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->base.source_location,
                                             "Function definition cannot have non-zero alignment"));
    if (res == KEFIR_OK) {
        base->properties.function_definition.identifier =
            kefir_string_pool_insert(mem, context->symbols, function_identifier, NULL);
        REQUIRE_CHAIN_SET(
            &res, base->properties.function_definition.identifier != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert function identifier into symbol table"));
    }

    if (res == KEFIR_OK) {
        switch (storage) {
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_TYPEDEF:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_THREAD_LOCAL:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN_THREAD_LOCAL:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC_THREAD_LOCAL:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_AUTO:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_REGISTER:
                res = KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->base.source_location,
                                             "Invalid function definition storage specifier");

            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN:
                // Intentionally left blank
                break;
        }
    }

    const struct kefir_ast_scoped_identifier *scoped_id = NULL;
    REQUIRE_CHAIN(&res, context->define_identifier(mem, context, false, base->properties.function_definition.identifier,
                                                   type, storage, base->properties.function_definition.function, NULL,
                                                   NULL, &attributes, &node->base.source_location, &scoped_id));
    REQUIRE_CHAIN_SET(
        &res, scoped_id->klass == KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION,
        KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Scoped identifier does not correspond to function definition type"));

    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_local_context_free(mem, local_context);
        KEFIR_FREE(mem, local_context);
        return res;
    });

    base->properties.type = scoped_id->function.type;
    base->properties.function_definition.storage = scoped_id->function.storage;
    base->properties.function_definition.scoped_id = scoped_id;

    local_context->context.surrounding_function = scoped_id;
    local_context->context.surrounding_function_name = base->properties.function_definition.identifier;
    *scoped_id->function.local_context_ptr = local_context;

    REQUIRE_OK(kefir_ast_node_properties_init(&node->body->base.properties));
    node->body->base.properties.category = KEFIR_AST_NODE_CATEGORY_STATEMENT;

    REQUIRE(local_context->context.flow_control_tree != NULL,
            KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR,
                            "Expected function definition local context to have valid flow control tree"));
    REQUIRE_OK(kefir_ast_flow_control_tree_push(mem, local_context->context.flow_control_tree,
                                                KEFIR_AST_FLOW_CONTROL_STRUCTURE_BLOCK, &associated_scopes,
                                                &node->body->base.properties.statement_props.flow_control_statement));

    const struct kefir_ast_declarator_function *decl_func = NULL;
    REQUIRE_OK(kefir_ast_declarator_unpack_function(node->declarator, &decl_func));
    REQUIRE(decl_func != NULL,
            KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected function definition to have function declarator"));
    kefir_bool_t hash_long_double_params = false;
    switch (type->function_type.mode) {
        case KEFIR_AST_FUNCTION_TYPE_PARAMETERS:
            REQUIRE_OK(analyze_function_parameters(node, decl_func, &hash_long_double_params));
            break;

        case KEFIR_AST_FUNCTION_TYPE_PARAM_IDENTIFIERS:
            REQUIRE_OK(analyze_function_parameter_identifiers(mem, context, node, decl_func, local_context,
                                                              &hash_long_double_params));
            break;

        case KEFIR_AST_FUNCTION_TYPE_PARAM_EMPTY:
            REQUIRE(kefir_list_length(&node->declarations) == 0,
                    KEFIR_SET_SOURCE_ERROR(
                        KEFIR_ANALYSIS_ERROR, &node->base.source_location,
                        "Function definition with empty parameter list shall not contain any declarations"));
            break;
    }

    if (hash_long_double_params) {
        REQUIRE_OK(local_context->context.allocate_temporary_value(
            mem, &local_context->context, kefir_ast_type_long_double(), NULL, &base->source_location,
            &base->properties.function_definition.temp_identifier));
    }

    for (const struct kefir_list_entry *iter = kefir_list_head(&node->body->block_items); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_node_base *, item, iter->value);
        REQUIRE_OK(kefir_ast_analyze_node(mem, &local_context->context, item));
        REQUIRE(item->properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT ||
                    item->properties.category == KEFIR_AST_NODE_CATEGORY_DECLARATION ||
                    item->properties.category == KEFIR_AST_NODE_CATEGORY_INIT_DECLARATOR ||
                    item->properties.category == KEFIR_AST_NODE_CATEGORY_INLINE_ASSEMBLY,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &item->source_location,
                                       "Compound statement items shall be either statements or declarations"));
    }
    REQUIRE_OK(kefir_ast_flow_control_tree_pop(local_context->context.flow_control_tree));
    REQUIRE_OK(local_context->context.pop_block(mem, &local_context->context));

    if (local_context->vl_arrays.next_id > 0) {
        struct kefir_ast_constant_expression *array_length =
            kefir_ast_constant_expression_integer(mem, local_context->vl_arrays.next_id);
        REQUIRE(array_length != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate AST array length constant"));

        const struct kefir_ast_type *array_type = kefir_ast_type_array(
            mem, context->type_bundle, kefir_ast_type_pointer(mem, context->type_bundle, kefir_ast_type_void()),
            array_length, NULL);
        REQUIRE_ELSE(array_type != NULL, {
            kefir_ast_constant_expression_free(mem, array_length);
            return KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate AST array type");
        });

        const struct kefir_ast_scoped_identifier *scoped_id = NULL;
        REQUIRE_OK(local_context->context.define_identifier(
            mem, &local_context->context, true, KEFIR_AST_TRANSLATOR_VLA_ELEMENTS_IDENTIFIER, array_type,
            KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN, KEFIR_AST_FUNCTION_SPECIFIER_NONE, NULL, NULL, NULL, NULL,
            &scoped_id));
    }
    return KEFIR_OK;
}
