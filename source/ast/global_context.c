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

#include <string.h>
#include <stdio.h>
#include "kefir/ast/global_context.h"
#include "kefir/ast/runtime.h"
#include "kefir/ast/context_impl.h"
#include "kefir/ast/analyzer/initializer.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/ast/function_declaration_context.h"
#include "kefir/core/source_error.h"
#include "kefir/core/extensions.h"
#include "kefir/ast/declarator.h"

static kefir_result_t context_resolve_ordinary_identifier(const struct kefir_ast_context *context,
                                                          const char *identifier,
                                                          const struct kefir_ast_scoped_identifier **scoped_id) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    ASSIGN_DECL_CAST(struct kefir_ast_global_context *, global_ctx, context->payload);
    return kefir_ast_global_context_resolve_scoped_ordinary_identifier(global_ctx, identifier, scoped_id);
}

static kefir_result_t context_resolve_tag_identifier(const struct kefir_ast_context *context, const char *identifier,
                                                     const struct kefir_ast_scoped_identifier **scoped_id) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    ASSIGN_DECL_CAST(struct kefir_ast_global_context *, global_ctx, context->payload);
    return kefir_ast_global_context_resolve_scoped_tag_identifier(global_ctx, identifier, scoped_id);
}

static kefir_result_t context_resolve_label_identifier(const struct kefir_ast_context *context, const char *identifier,
                                                       const struct kefir_ast_scoped_identifier **scoped_id) {
    UNUSED(context);
    UNUSED(identifier);
    UNUSED(scoped_id);
    return KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Label resolving is not possible in the global scope");
}

static kefir_result_t context_allocate_temporary_value(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                       const struct kefir_ast_type *type,
                                                       struct kefir_ast_initializer *initializer,
                                                       const struct kefir_source_location *location,
                                                       struct kefir_ast_temporary_identifier *temp_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(temp_id != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid temporary identifier pointer"));

    ASSIGN_DECL_CAST(struct kefir_ast_global_context *, global_ctx, context->payload);
    UNUSED(kefir_ast_temporaries_init(mem, context->type_bundle, false, context->temporaries));
    REQUIRE_OK(kefir_ast_temporaries_new_temporary(mem, context, type, temp_id));
    REQUIRE(!temp_id->nested,
            KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected AST global context temporaries to have flat structure"));

#define BUFSIZE 128
    char buf[BUFSIZE] = {0};
    snprintf(buf, sizeof(buf) - 1, KEFIR_AST_TRANSLATOR_TEMPORARY_GLOBAL_IDENTIFIER, temp_id->identifier,
             temp_id->field);
    REQUIRE_OK(
        kefir_ast_global_context_define_static(mem, global_ctx, buf, type, NULL, initializer, NULL, location, NULL));
#undef BUFSIZE
    return KEFIR_OK;
}

static kefir_result_t context_define_tag(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                         const struct kefir_ast_type *type,
                                         const struct kefir_source_location *location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));

    ASSIGN_DECL_CAST(struct kefir_ast_global_context *, global_ctx, context->payload);
    REQUIRE_OK(kefir_ast_global_context_define_tag(mem, global_ctx, type, location, NULL));
    return KEFIR_OK;
}

static kefir_result_t context_define_constant(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                              const char *identifier, struct kefir_ast_constant_expression *value,
                                              const struct kefir_ast_type *type,
                                              const struct kefir_source_location *location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translatation context"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));
    REQUIRE(value != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant expression"));

    ASSIGN_DECL_CAST(struct kefir_ast_global_context *, global_ctx, context->payload);
    REQUIRE_OK(kefir_ast_global_context_define_constant(mem, global_ctx, identifier, value, type, location, NULL));
    return KEFIR_OK;
}

static kefir_result_t context_define_identifier(
    struct kefir_mem *mem, const struct kefir_ast_context *context, kefir_bool_t declaration, const char *identifier,
    const struct kefir_ast_type *type, kefir_ast_scoped_identifier_storage_t storage_class,
    kefir_ast_function_specifier_t function_specifier, struct kefir_ast_alignment *alignment,
    struct kefir_ast_initializer *initializer, const struct kefir_ast_declarator_attributes *attributes,
    const struct kefir_source_location *location, const struct kefir_ast_scoped_identifier **scoped_id) {

    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(identifier != NULL, KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                                       "Empty identifiers are not permitted in the global scope"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));

    ASSIGN_DECL_CAST(struct kefir_ast_global_context *, global_ctx, context->payload);

    const struct kefir_ast_type *unqualified_type = kefir_ast_unqualified_type(type);
    kefir_bool_t is_function = unqualified_type->tag == KEFIR_AST_TYPE_FUNCTION;
    if (is_function) {
        REQUIRE(alignment == NULL,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Alignment specifier is not permitted in the declaration of function"));
        REQUIRE(initializer == NULL,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location, "Function cannot be defined with initializer"));
        switch (storage_class) {
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_TYPEDEF:
                REQUIRE(function_specifier == KEFIR_AST_FUNCTION_SPECIFIER_NONE,
                        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                               "Functin specifiers may only be used in function declarations"));
                REQUIRE(declaration,
                        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                               "Typedef specifier cannot be used for function definition"));
                REQUIRE_OK(
                    kefir_ast_global_context_define_type(mem, global_ctx, identifier, type, NULL, location, scoped_id));
                break;

            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN:
                if (declaration) {
                    REQUIRE_OK(kefir_ast_global_context_declare_function(
                        mem, global_ctx, function_specifier, storage_class == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN,
                        identifier, unqualified_type, attributes, location, scoped_id));
                } else {
                    REQUIRE_OK(kefir_ast_global_context_define_function(
                        mem, global_ctx, function_specifier, storage_class == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN,
                        identifier, unqualified_type, attributes, location, scoped_id));
                }
                break;

            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC:
                REQUIRE_OK(kefir_ast_global_context_define_static_function(mem, global_ctx, function_specifier,
                                                                           identifier, declaration, unqualified_type,
                                                                           attributes, location, scoped_id));
                break;

            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_THREAD_LOCAL:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN_THREAD_LOCAL:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC_THREAD_LOCAL:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_AUTO:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_REGISTER:
                return KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                              "Illegal function storage-class specifier");
        }
    } else {
        REQUIRE(function_specifier == KEFIR_AST_FUNCTION_SPECIFIER_NONE,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Function specifiers cannot be used for non-function types"));
        REQUIRE(declaration || initializer != NULL,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Initializer must be provided to for identifier definition"));
        switch (storage_class) {
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_TYPEDEF:
                REQUIRE_OK(kefir_ast_global_context_define_type(mem, global_ctx, identifier, type, alignment, location,
                                                                scoped_id));
                break;

            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN:
                if (declaration) {
                    REQUIRE_OK(kefir_ast_global_context_declare_external(mem, global_ctx, identifier, type, alignment,
                                                                         attributes, location, scoped_id));
                } else {
                    REQUIRE_OK(kefir_ast_global_context_define_external(mem, global_ctx, identifier, type, alignment,
                                                                        initializer, attributes, location, scoped_id));
                }
                break;

            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC:
                REQUIRE_OK(kefir_ast_global_context_define_static(mem, global_ctx, identifier, type, alignment,
                                                                  initializer, attributes, location, scoped_id));
                break;

            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_THREAD_LOCAL:
                REQUIRE_OK(kefir_ast_global_context_define_external_thread_local(
                    mem, global_ctx, identifier, type, alignment, initializer, attributes, location, scoped_id));
                break;

            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN_THREAD_LOCAL:
                if (declaration) {
                    REQUIRE_OK(kefir_ast_global_context_declare_external_thread_local(
                        mem, global_ctx, identifier, type, alignment, attributes, location, scoped_id));
                } else {
                    REQUIRE_OK(kefir_ast_global_context_define_external_thread_local(
                        mem, global_ctx, identifier, type, alignment, initializer, attributes, location, scoped_id));
                }
                break;

            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC_THREAD_LOCAL:
                REQUIRE_OK(kefir_ast_global_context_define_static_thread_local(
                    mem, global_ctx, identifier, type, alignment, initializer, attributes, location, scoped_id));
                break;

            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN:
                REQUIRE_OK(kefir_ast_global_context_define_external(mem, global_ctx, identifier, type, alignment,
                                                                    initializer, attributes, location, scoped_id));
                break;

            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_AUTO:
            case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_REGISTER:
                return KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                              "Illegal file-scope identifier storage class");
        }
    }

    return KEFIR_OK;
}

static kefir_result_t context_reference_label(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                              const char *label, struct kefir_ast_flow_control_structure *parent,
                                              const struct kefir_source_location *location,
                                              const struct kefir_ast_scoped_identifier **scoped_id) {
    UNUSED(mem);
    UNUSED(context);
    UNUSED(label);
    UNUSED(parent);
    UNUSED(scoped_id);

    return KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                  "Labels cannot be defined or referenced in a global context");
}

static kefir_result_t context_push_block(struct kefir_mem *mem, const struct kefir_ast_context *context) {
    UNUSED(mem);
    UNUSED(context);

    return KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Blocks cannot be pushed in a global context");
}

static kefir_result_t context_pop_block(struct kefir_mem *mem, const struct kefir_ast_context *context) {
    UNUSED(mem);
    UNUSED(context);

    return KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Blocks cannot be popped in a global context");
}

static kefir_result_t free_func_decl_context(struct kefir_mem *mem, struct kefir_list *list,
                                             struct kefir_list_entry *entry, void *payload) {
    UNUSED(list);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid list entry"));

    ASSIGN_DECL_CAST(struct kefir_ast_function_declaration_context *, context, entry->value);
    REQUIRE_OK(kefir_ast_function_declaration_context_free(mem, context));
    KEFIR_FREE(mem, context);
    return KEFIR_OK;
}

static kefir_result_t context_current_flow_control_point(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                         struct kefir_ast_flow_control_point **point) {
    UNUSED(mem);
    UNUSED(context);
    UNUSED(point);

    return KEFIR_SET_ERROR(KEFIR_INVALID_CHANGE, "Control flow cannot be referenced in a global context");
}

kefir_result_t kefir_ast_global_context_init(struct kefir_mem *mem, const struct kefir_ast_type_traits *type_traits,
                                             const struct kefir_ast_target_environment *target_env,
                                             struct kefir_ast_global_context *context,
                                             const struct kefir_ast_context_extensions *extensions) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(type_traits != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type traits"));
    REQUIRE(target_env != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST target environment"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translatation context"));
    REQUIRE_OK(kefir_symbol_table_init(&context->symbols));
    REQUIRE_OK(kefir_ast_type_bundle_init(&context->type_bundle, &context->symbols));
    context->type_traits = type_traits;
    context->target_env = target_env;
    context->temporaries = (struct kefir_ast_context_temporaries){0};
    REQUIRE_OK(kefir_ast_identifier_flat_scope_init(&context->tag_scope));
    REQUIRE_OK(kefir_ast_identifier_flat_scope_on_removal(&context->tag_scope, kefir_ast_context_free_scoped_identifier,
                                                          NULL));
    REQUIRE_OK(kefir_list_init(&context->function_decl_contexts));
    REQUIRE_OK(kefir_list_on_remove(&context->function_decl_contexts, free_func_decl_context, NULL));

    REQUIRE_OK(kefir_ast_identifier_flat_scope_init(&context->object_identifiers));
    REQUIRE_OK(kefir_ast_identifier_flat_scope_on_removal(&context->object_identifiers,
                                                          kefir_ast_context_free_scoped_identifier, NULL));
    REQUIRE_OK(kefir_ast_identifier_flat_scope_init(&context->constant_identifiers));
    REQUIRE_OK(kefir_ast_identifier_flat_scope_on_removal(&context->constant_identifiers,
                                                          kefir_ast_context_free_scoped_identifier, NULL));
    REQUIRE_OK(kefir_ast_identifier_flat_scope_init(&context->type_identifiers));
    REQUIRE_OK(kefir_ast_identifier_flat_scope_on_removal(&context->type_identifiers,
                                                          kefir_ast_context_free_scoped_identifier, NULL));
    REQUIRE_OK(kefir_ast_identifier_flat_scope_init(&context->function_identifiers));
    REQUIRE_OK(kefir_ast_identifier_flat_scope_on_removal(&context->function_identifiers,
                                                          kefir_ast_context_free_scoped_identifier, NULL));
    REQUIRE_OK(kefir_ast_identifier_flat_scope_init(&context->ordinary_scope));
    REQUIRE_OK(kefir_ast_context_configuration_defaults(&context->configuration));

    context->context.resolve_ordinary_identifier = context_resolve_ordinary_identifier;
    context->context.resolve_tag_identifier = context_resolve_tag_identifier;
    context->context.resolve_label_identifier = context_resolve_label_identifier;
    context->context.allocate_temporary_value = context_allocate_temporary_value;
    context->context.define_tag = context_define_tag;
    context->context.define_constant = context_define_constant;
    context->context.define_identifier = context_define_identifier;
    context->context.reference_label = context_reference_label;
    context->context.push_block = context_push_block;
    context->context.pop_block = context_pop_block;
    context->context.current_flow_control_point = context_current_flow_control_point;
    context->context.symbols = &context->symbols;
    context->context.type_bundle = &context->type_bundle;
    context->context.type_traits = context->type_traits;
    context->context.target_env = context->target_env;
    context->context.temporaries = &context->temporaries;
    context->context.type_analysis_context = KEFIR_AST_TYPE_ANALYSIS_DEFAULT;
    context->context.flow_control_tree = NULL;
    context->context.global_context = context;
    context->context.function_decl_contexts = &context->function_decl_contexts;
    context->context.surrounding_function = NULL;
    context->context.surrounding_function_name = NULL;
    context->context.configuration = &context->configuration;
    context->context.payload = context;

    context->context.extensions = extensions;
    context->context.extensions_payload = NULL;
    kefir_result_t res;
    KEFIR_RUN_EXTENSION0(&res, mem, &context->context, on_init);
    REQUIRE_OK(res);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_global_context_free(struct kefir_mem *mem, struct kefir_ast_global_context *context) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translatation context"));

    kefir_result_t res;
    KEFIR_RUN_EXTENSION0(&res, mem, &context->context, on_free);
    REQUIRE_OK(res);
    context->context.extensions = NULL;
    context->context.extensions_payload = NULL;

    REQUIRE_OK(kefir_ast_identifier_flat_scope_free(mem, &context->tag_scope));
    REQUIRE_OK(kefir_ast_identifier_flat_scope_free(mem, &context->ordinary_scope));
    REQUIRE_OK(kefir_ast_identifier_flat_scope_free(mem, &context->constant_identifiers));
    REQUIRE_OK(kefir_ast_identifier_flat_scope_free(mem, &context->type_identifiers));
    REQUIRE_OK(kefir_ast_identifier_flat_scope_free(mem, &context->function_identifiers));
    REQUIRE_OK(kefir_ast_identifier_flat_scope_free(mem, &context->object_identifiers));
    REQUIRE_OK(kefir_list_free(mem, &context->function_decl_contexts));
    REQUIRE_OK(kefir_ast_type_bundle_free(mem, &context->type_bundle));
    REQUIRE_OK(kefir_symbol_table_free(mem, &context->symbols));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_global_context_resolve_scoped_ordinary_identifier(
    const struct kefir_ast_global_context *context, const char *identifier,
    const struct kefir_ast_scoped_identifier **scoped_id) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translatation context"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));
    REQUIRE(scoped_id != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST scoped identifier pointer"));
    struct kefir_ast_scoped_identifier *result = NULL;
    REQUIRE_OK(kefir_ast_identifier_flat_scope_at(&context->ordinary_scope, identifier, &result));
    *scoped_id = result;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_global_context_resolve_scoped_tag_identifier(
    const struct kefir_ast_global_context *context, const char *identifier,
    const struct kefir_ast_scoped_identifier **scoped_id) {
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translatation context"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));
    REQUIRE(scoped_id != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST scoped identifier pointer"));
    struct kefir_ast_scoped_identifier *result = NULL;
    REQUIRE_OK(kefir_ast_identifier_flat_scope_at(&context->tag_scope, identifier, &result));
    *scoped_id = result;
    return KEFIR_OK;
}

static kefir_result_t insert_ordinary_identifier(struct kefir_mem *mem, struct kefir_ast_global_context *context,
                                                 const char *identifier,
                                                 struct kefir_ast_scoped_identifier *ordinary_id) {
    struct kefir_ast_scoped_identifier *scoped_id = NULL;
    kefir_result_t res = kefir_ast_identifier_flat_scope_at(&context->ordinary_scope, identifier, &scoped_id);
    if (res == KEFIR_NOT_FOUND) {
        res = kefir_ast_identifier_flat_scope_insert(mem, &context->ordinary_scope, identifier, ordinary_id);
    }
    return res;
}

static kefir_result_t require_ordinary_identifier_type(struct kefir_ast_global_context *context, const char *identifier,
                                                       kefir_ast_scoped_identifier_class_t klass,
                                                       const struct kefir_source_location *location) {
    struct kefir_ast_scoped_identifier *scoped_id = NULL;
    kefir_result_t res = kefir_ast_identifier_flat_scope_at(&context->ordinary_scope, identifier, &scoped_id);
    if (res == KEFIR_OK) {
        REQUIRE(scoped_id->klass == klass,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Cannot redefine identifier with different kind of symbol"));
    } else {
        REQUIRE(res == KEFIR_NOT_FOUND, res);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_global_context_declare_external(struct kefir_mem *mem,
                                                         struct kefir_ast_global_context *context,
                                                         const char *identifier, const struct kefir_ast_type *type,
                                                         struct kefir_ast_alignment *alignment,
                                                         const struct kefir_ast_declarator_attributes *attributes,
                                                         const struct kefir_source_location *location,
                                                         const struct kefir_ast_scoped_identifier **scoped_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translatation context"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(!kefir_ast_type_is_variably_modified(type),
            KEFIR_SET_SOURCE_ERROR(
                KEFIR_ANALYSIS_ERROR, location,
                "Declarations with variably-modified types shall have block or function prototype scope"));

    REQUIRE_OK(require_ordinary_identifier_type(context, identifier, KEFIR_AST_SCOPE_IDENTIFIER_OBJECT, location));

    identifier = kefir_symbol_table_insert(mem, &context->symbols, identifier, NULL);
    REQUIRE(identifier != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert identifier into symbol table"));

    struct kefir_ast_scoped_identifier *ordinary_id = NULL;
    kefir_result_t res = kefir_ast_identifier_flat_scope_at(&context->object_identifiers, identifier, &ordinary_id);
    if (res == KEFIR_OK) {
        REQUIRE(ordinary_id->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN ||
                    ordinary_id->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Cannot declare the same identifier with different storage class"));
        REQUIRE(KEFIR_AST_TYPE_COMPATIBLE(context->type_traits, ordinary_id->object.type, type),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "All declarations of the same identifier shall have compatible types"));
        REQUIRE_OK(kefir_ast_context_merge_alignment(mem, &ordinary_id->object.alignment, alignment));
        ordinary_id->object.type =
            KEFIR_AST_TYPE_COMPOSITE(mem, &context->type_bundle, context->type_traits, ordinary_id->object.type, type);
        if (attributes != NULL) {
            if (ordinary_id->object.asm_label == NULL) {
                ordinary_id->object.asm_label = attributes->asm_label;
            } else {
                REQUIRE(
                    attributes->asm_label == NULL || strcmp(attributes->asm_label, ordinary_id->object.asm_label) == 0,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                           "Assembly label mismatch with previous declaration"));
            }
        }
    } else {
        REQUIRE(res == KEFIR_NOT_FOUND, res);
        ordinary_id = kefir_ast_context_allocate_scoped_object_identifier(
            mem, type, KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN, alignment,
            KEFIR_AST_SCOPED_IDENTIFIER_EXTERNAL_LINKAGE, true, NULL, attributes != NULL ? attributes->asm_label : NULL,
            location);
        res = kefir_ast_identifier_flat_scope_insert(mem, &context->object_identifiers, identifier, ordinary_id);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_context_free_scoped_identifier(mem, ordinary_id, NULL);
            return res;
        });
    }

    REQUIRE_OK(insert_ordinary_identifier(mem, context, identifier, ordinary_id));
    ASSIGN_PTR(scoped_id, ordinary_id);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_global_context_declare_external_thread_local(
    struct kefir_mem *mem, struct kefir_ast_global_context *context, const char *identifier,
    const struct kefir_ast_type *type, struct kefir_ast_alignment *alignment,
    const struct kefir_ast_declarator_attributes *attributes, const struct kefir_source_location *location,
    const struct kefir_ast_scoped_identifier **scoped_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translatation context"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(!kefir_ast_type_is_variably_modified(type),
            KEFIR_SET_SOURCE_ERROR(
                KEFIR_ANALYSIS_ERROR, location,
                "Declarations with variably-modified types shall have block or function prototype scope"));

    REQUIRE_OK(require_ordinary_identifier_type(context, identifier, KEFIR_AST_SCOPE_IDENTIFIER_OBJECT, location));

    identifier = kefir_symbol_table_insert(mem, &context->symbols, identifier, NULL);
    REQUIRE(identifier != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert identifier into symbol table"));

    struct kefir_ast_scoped_identifier *ordinary_id = NULL;
    kefir_result_t res = kefir_ast_identifier_flat_scope_at(&context->object_identifiers, identifier, &ordinary_id);
    if (res == KEFIR_OK) {
        REQUIRE(ordinary_id->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN_THREAD_LOCAL ||
                    ordinary_id->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC_THREAD_LOCAL,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Cannot declare the same identifier with different storage class"));
        REQUIRE(KEFIR_AST_TYPE_COMPATIBLE(context->type_traits, ordinary_id->object.type, type),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "All declarations of the same identifier shall have compatible types"));
        REQUIRE_OK(kefir_ast_context_merge_alignment(mem, &ordinary_id->object.alignment, alignment));
        ordinary_id->object.type =
            KEFIR_AST_TYPE_COMPOSITE(mem, &context->type_bundle, context->type_traits, ordinary_id->object.type, type);
        if (attributes != NULL) {
            if (ordinary_id->object.asm_label == NULL) {
                ordinary_id->object.asm_label = attributes->asm_label;
            } else {
                REQUIRE(
                    attributes->asm_label == NULL || strcmp(attributes->asm_label, ordinary_id->object.asm_label) == 0,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                           "Assembly label mismatch with previous declaration"));
            }
        }
    } else {
        REQUIRE(res == KEFIR_NOT_FOUND, res);
        ordinary_id = kefir_ast_context_allocate_scoped_object_identifier(
            mem, type, KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN_THREAD_LOCAL, alignment,
            KEFIR_AST_SCOPED_IDENTIFIER_EXTERNAL_LINKAGE, true, NULL, attributes != NULL ? attributes->asm_label : NULL,
            location);
        res = kefir_ast_identifier_flat_scope_insert(mem, &context->object_identifiers, identifier, ordinary_id);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_context_free_scoped_identifier(mem, ordinary_id, NULL);
            return res;
        });
    }

    REQUIRE_OK(insert_ordinary_identifier(mem, context, identifier, ordinary_id));
    ASSIGN_PTR(scoped_id, ordinary_id);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_global_context_define_external(struct kefir_mem *mem, struct kefir_ast_global_context *context,
                                                        const char *identifier, const struct kefir_ast_type *type,
                                                        struct kefir_ast_alignment *alignment,
                                                        struct kefir_ast_initializer *initializer,
                                                        const struct kefir_ast_declarator_attributes *attributes,
                                                        const struct kefir_source_location *location,
                                                        const struct kefir_ast_scoped_identifier **scoped_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translatation context"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(!kefir_ast_type_is_variably_modified(type),
            KEFIR_SET_SOURCE_ERROR(
                KEFIR_ANALYSIS_ERROR, location,
                "Declarations with variably-modified types shall have block or function prototype scope"));

    REQUIRE_OK(require_ordinary_identifier_type(context, identifier, KEFIR_AST_SCOPE_IDENTIFIER_OBJECT, location));

    identifier = kefir_symbol_table_insert(mem, &context->symbols, identifier, NULL);
    REQUIRE(identifier != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert identifier into symbol table"));

    struct kefir_ast_scoped_identifier *ordinary_id = NULL;
    kefir_result_t res = kefir_ast_identifier_flat_scope_at(&context->object_identifiers, identifier, &ordinary_id);
    if (res == KEFIR_OK) {
        REQUIRE(ordinary_id->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Cannot declare the same identifier with different storage class"));
        REQUIRE(KEFIR_AST_TYPE_COMPATIBLE(context->type_traits, ordinary_id->object.type, type),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "All declarations of the same identifier shall have compatible types"));
        REQUIRE(initializer == NULL || ordinary_id->object.initializer == NULL,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location, "Redefinition of identifier is not permitted"));
        REQUIRE_OK(kefir_ast_context_merge_alignment(mem, &ordinary_id->object.alignment, alignment));
        ordinary_id->object.type =
            KEFIR_AST_TYPE_COMPOSITE(mem, &context->type_bundle, context->type_traits, ordinary_id->object.type, type);
        ordinary_id->object.external = false;
        if (attributes != NULL) {
            if (ordinary_id->object.asm_label == NULL) {
                ordinary_id->object.asm_label = attributes->asm_label;
            } else {
                REQUIRE(
                    attributes->asm_label == NULL || strcmp(attributes->asm_label, ordinary_id->object.asm_label) == 0,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                           "Assembly label mismatch with previous declaration"));
            }
        }
    } else {
        REQUIRE(res == KEFIR_NOT_FOUND, res);
        ordinary_id = kefir_ast_context_allocate_scoped_object_identifier(
            mem, type, KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN, alignment,
            KEFIR_AST_SCOPED_IDENTIFIER_EXTERNAL_LINKAGE, false, initializer,
            attributes != NULL ? attributes->asm_label : NULL, location);
        res = kefir_ast_identifier_flat_scope_insert(mem, &context->object_identifiers, identifier, ordinary_id);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_context_free_scoped_identifier(mem, ordinary_id, NULL);
            return res;
        });
    }

    REQUIRE_OK(insert_ordinary_identifier(mem, context, identifier, ordinary_id));

    if (initializer != NULL) {
        struct kefir_ast_type_qualification qualifications;
        REQUIRE_OK(kefir_ast_type_retrieve_qualifications(&qualifications, type));
        struct kefir_ast_initializer_properties props;
        REQUIRE_OK(kefir_ast_analyze_initializer(mem, &context->context, type, initializer, &props));
        type = props.type;
        REQUIRE(props.constant,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Initializers of object with static storage duration shall be constant"));

        if (!KEFIR_AST_TYPE_IS_ZERO_QUALIFICATION(&qualifications)) {
            type = kefir_ast_type_qualified(mem, &context->type_bundle, type, qualifications);
        }

        ordinary_id->type = type;
        ordinary_id->object.initializer = initializer;
    }
    ASSIGN_PTR(scoped_id, ordinary_id);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_global_context_define_external_thread_local(
    struct kefir_mem *mem, struct kefir_ast_global_context *context, const char *identifier,
    const struct kefir_ast_type *type, struct kefir_ast_alignment *alignment, struct kefir_ast_initializer *initializer,
    const struct kefir_ast_declarator_attributes *attributes, const struct kefir_source_location *location,
    const struct kefir_ast_scoped_identifier **scoped_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translatation context"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(!kefir_ast_type_is_variably_modified(type),
            KEFIR_SET_SOURCE_ERROR(
                KEFIR_ANALYSIS_ERROR, location,
                "Declarations with variably-modified types shall have block or function prototype scope"));

    REQUIRE_OK(require_ordinary_identifier_type(context, identifier, KEFIR_AST_SCOPE_IDENTIFIER_OBJECT, location));

    identifier = kefir_symbol_table_insert(mem, &context->symbols, identifier, NULL);
    REQUIRE(identifier != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert identifier into symbol table"));

    struct kefir_ast_scoped_identifier *ordinary_id = NULL;
    kefir_result_t res = kefir_ast_identifier_flat_scope_at(&context->object_identifiers, identifier, &ordinary_id);
    if (res == KEFIR_OK) {
        REQUIRE(ordinary_id->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN_THREAD_LOCAL,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Cannot declare the same identifier with different storage class"));
        REQUIRE(KEFIR_AST_TYPE_COMPATIBLE(context->type_traits, ordinary_id->object.type, type),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "All declarations of the same identifier shall have compatible types"));
        REQUIRE(initializer == NULL || ordinary_id->object.initializer == NULL,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location, "Redefinition of identifier is not permitted"));
        REQUIRE_OK(kefir_ast_context_merge_alignment(mem, &ordinary_id->object.alignment, alignment));
        ordinary_id->object.type =
            KEFIR_AST_TYPE_COMPOSITE(mem, &context->type_bundle, context->type_traits, ordinary_id->object.type, type);
        ordinary_id->object.external = false;
        if (attributes != NULL) {
            if (ordinary_id->object.asm_label == NULL) {
                ordinary_id->object.asm_label = attributes->asm_label;
            } else {
                REQUIRE(
                    attributes->asm_label == NULL || strcmp(attributes->asm_label, ordinary_id->object.asm_label) == 0,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                           "Assembly label mismatch with previous declaration"));
            }
        }
    } else {
        REQUIRE(res == KEFIR_NOT_FOUND, res);
        ordinary_id = kefir_ast_context_allocate_scoped_object_identifier(
            mem, type, KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN_THREAD_LOCAL, alignment,
            KEFIR_AST_SCOPED_IDENTIFIER_EXTERNAL_LINKAGE, false, initializer,
            attributes != NULL ? attributes->asm_label : NULL, location);
        res = kefir_ast_identifier_flat_scope_insert(mem, &context->object_identifiers, identifier, ordinary_id);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_context_free_scoped_identifier(mem, ordinary_id, NULL);
            return res;
        });
    }

    REQUIRE_OK(insert_ordinary_identifier(mem, context, identifier, ordinary_id));

    if (initializer != NULL) {
        struct kefir_ast_type_qualification qualifications;
        REQUIRE_OK(kefir_ast_type_retrieve_qualifications(&qualifications, type));
        struct kefir_ast_initializer_properties props;
        REQUIRE_OK(kefir_ast_analyze_initializer(mem, &context->context, type, initializer, &props));
        type = props.type;
        REQUIRE(props.constant,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Initializers of object with thread local storage duration shall be constant"));

        if (!KEFIR_AST_TYPE_IS_ZERO_QUALIFICATION(&qualifications)) {
            type = kefir_ast_type_qualified(mem, &context->type_bundle, type, qualifications);
        }
        ordinary_id->type = type;
        ordinary_id->object.initializer = initializer;
    }
    ASSIGN_PTR(scoped_id, ordinary_id);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_global_context_define_static(struct kefir_mem *mem, struct kefir_ast_global_context *context,
                                                      const char *identifier, const struct kefir_ast_type *type,
                                                      struct kefir_ast_alignment *alignment,
                                                      struct kefir_ast_initializer *initializer,
                                                      const struct kefir_ast_declarator_attributes *attributes,
                                                      const struct kefir_source_location *location,
                                                      const struct kefir_ast_scoped_identifier **scoped_id) {
    UNUSED(attributes);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translatation context"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(!kefir_ast_type_is_variably_modified(type),
            KEFIR_SET_SOURCE_ERROR(
                KEFIR_ANALYSIS_ERROR, location,
                "Declarations with variably-modified types shall have block or function prototype scope"));

    REQUIRE_OK(require_ordinary_identifier_type(context, identifier, KEFIR_AST_SCOPE_IDENTIFIER_OBJECT, location));

    identifier = kefir_symbol_table_insert(mem, &context->symbols, identifier, NULL);
    REQUIRE(identifier != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert identifier into symbol table"));

    struct kefir_ast_scoped_identifier *ordinary_id = NULL;
    kefir_result_t res = kefir_ast_identifier_flat_scope_at(&context->object_identifiers, identifier, &ordinary_id);
    if (res == KEFIR_OK) {
        REQUIRE(ordinary_id->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Cannot declare the same identifier with different storage class"));
        REQUIRE(KEFIR_AST_TYPE_COMPATIBLE(context->type_traits, ordinary_id->object.type, type),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "All declarations of the same identifier shall have compatible types"));
        REQUIRE(!ordinary_id->object.external,
                KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Identifier with static storage duration cannot be external"));
        REQUIRE(initializer == NULL || ordinary_id->object.initializer == NULL,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location, "Redefinition of identifier is not permitted"));
        REQUIRE_OK(kefir_ast_context_merge_alignment(mem, &ordinary_id->object.alignment, alignment));
        ordinary_id->object.type =
            KEFIR_AST_TYPE_COMPOSITE(mem, &context->type_bundle, context->type_traits, ordinary_id->object.type, type);
    } else {
        REQUIRE(res == KEFIR_NOT_FOUND, res);
        if (initializer == NULL) {
            REQUIRE(!KEFIR_AST_TYPE_IS_INCOMPLETE(type),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                           "Tentative definition with internal linkage shall have complete type"));
        }
        ordinary_id = kefir_ast_context_allocate_scoped_object_identifier(
            mem, type, KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC, alignment,
            KEFIR_AST_SCOPED_IDENTIFIER_INTERNAL_LINKAGE, false, initializer, NULL, location);
        res = kefir_ast_identifier_flat_scope_insert(mem, &context->object_identifiers, identifier, ordinary_id);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_context_free_scoped_identifier(mem, ordinary_id, NULL);
            return res;
        });
    }

    REQUIRE_OK(insert_ordinary_identifier(mem, context, identifier, ordinary_id));

    if (initializer != NULL) {
        struct kefir_ast_type_qualification qualifications;
        REQUIRE_OK(kefir_ast_type_retrieve_qualifications(&qualifications, type));
        struct kefir_ast_initializer_properties props;
        REQUIRE_OK(kefir_ast_analyze_initializer(mem, &context->context, type, initializer, &props));
        type = props.type;
        REQUIRE(props.constant,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Initializers of object with static storage duration shall be constant"));

        if (!KEFIR_AST_TYPE_IS_ZERO_QUALIFICATION(&qualifications)) {
            type = kefir_ast_type_qualified(mem, &context->type_bundle, type, qualifications);
        }
        ordinary_id->type = type;
        ordinary_id->object.initializer = initializer;
    }
    ASSIGN_PTR(scoped_id, ordinary_id);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_global_context_define_static_thread_local(
    struct kefir_mem *mem, struct kefir_ast_global_context *context, const char *identifier,
    const struct kefir_ast_type *type, struct kefir_ast_alignment *alignment, struct kefir_ast_initializer *initializer,
    const struct kefir_ast_declarator_attributes *attributes, const struct kefir_source_location *location,
    const struct kefir_ast_scoped_identifier **scoped_id) {
    UNUSED(attributes);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translatation context"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(!kefir_ast_type_is_variably_modified(type),
            KEFIR_SET_SOURCE_ERROR(
                KEFIR_ANALYSIS_ERROR, location,
                "Declarations with variably-modified types shall have block or function prototype scope"));

    REQUIRE_OK(require_ordinary_identifier_type(context, identifier, KEFIR_AST_SCOPE_IDENTIFIER_OBJECT, location));

    identifier = kefir_symbol_table_insert(mem, &context->symbols, identifier, NULL);
    REQUIRE(identifier != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert identifier into symbol table"));

    struct kefir_ast_scoped_identifier *ordinary_id = NULL;
    kefir_result_t res = kefir_ast_identifier_flat_scope_at(&context->object_identifiers, identifier, &ordinary_id);
    if (res == KEFIR_OK) {
        REQUIRE(ordinary_id->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC_THREAD_LOCAL,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Cannot declare the same identifier with different storage class"));
        REQUIRE(KEFIR_AST_TYPE_COMPATIBLE(context->type_traits, ordinary_id->object.type, type),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "All declarations of the same identifier shall have compatible types"));
        REQUIRE(!ordinary_id->object.external,
                KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Identifier with static storage duration cannot be external"));
        REQUIRE(initializer == NULL || ordinary_id->object.initializer == NULL,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location, "Redefinition of identifier is not permitted"));
        REQUIRE_OK(kefir_ast_context_merge_alignment(mem, &ordinary_id->object.alignment, alignment));
        ordinary_id->object.type =
            KEFIR_AST_TYPE_COMPOSITE(mem, &context->type_bundle, context->type_traits, ordinary_id->object.type, type);
    } else {
        REQUIRE(res == KEFIR_NOT_FOUND, res);
        if (initializer == NULL) {
            REQUIRE(!KEFIR_AST_TYPE_IS_INCOMPLETE(type),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                           "Tentative definition with internal linkage shall have complete type"));
        }
        ordinary_id = kefir_ast_context_allocate_scoped_object_identifier(
            mem, type, KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC_THREAD_LOCAL, alignment,
            KEFIR_AST_SCOPED_IDENTIFIER_INTERNAL_LINKAGE, false, initializer, NULL, location);
        res = kefir_ast_identifier_flat_scope_insert(mem, &context->object_identifiers, identifier, ordinary_id);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_context_free_scoped_identifier(mem, ordinary_id, NULL);
            return res;
        });
    }

    REQUIRE_OK(insert_ordinary_identifier(mem, context, identifier, ordinary_id));

    if (initializer != NULL) {
        struct kefir_ast_type_qualification qualifications;
        REQUIRE_OK(kefir_ast_type_retrieve_qualifications(&qualifications, type));
        struct kefir_ast_initializer_properties props;
        REQUIRE_OK(kefir_ast_analyze_initializer(mem, &context->context, type, initializer, &props));
        type = props.type;
        REQUIRE(props.constant,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Initializers of object with thread local storage duration shall be constant"));

        if (!KEFIR_AST_TYPE_IS_ZERO_QUALIFICATION(&qualifications)) {
            type = kefir_ast_type_qualified(mem, &context->type_bundle, type, qualifications);
        }
        ordinary_id->type = type;
        ordinary_id->object.initializer = initializer;
    }
    ASSIGN_PTR(scoped_id, ordinary_id);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_global_context_define_constant(struct kefir_mem *mem, struct kefir_ast_global_context *context,
                                                        const char *identifier,
                                                        struct kefir_ast_constant_expression *value,
                                                        const struct kefir_ast_type *type,
                                                        const struct kefir_source_location *location,
                                                        const struct kefir_ast_scoped_identifier **scoped_id_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translatation context"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));
    REQUIRE(value != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant expression"));
    REQUIRE(!kefir_ast_type_is_variably_modified(type),
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                   "Constant definition cannot have variably-modified type"));

    identifier = kefir_symbol_table_insert(mem, &context->symbols, identifier, NULL);
    REQUIRE(identifier != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert identifier into symbol table"));

    struct kefir_ast_scoped_identifier *scoped_id = NULL;
    kefir_result_t res = kefir_ast_identifier_flat_scope_at(&context->ordinary_scope, identifier, &scoped_id);
    if (res == KEFIR_OK) {
        return KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location, "Cannot redefine constant");
    } else {
        REQUIRE(res == KEFIR_NOT_FOUND, res);
        scoped_id = kefir_ast_context_allocate_scoped_constant(mem, value, type, location);
        REQUIRE(scoped_id != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocted AST scoped identifier"));
        res = kefir_ast_identifier_flat_scope_insert(mem, &context->constant_identifiers, identifier, scoped_id);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_context_free_scoped_identifier(mem, scoped_id, NULL);
            return res;
        });
        REQUIRE_OK(kefir_ast_identifier_flat_scope_insert(mem, &context->ordinary_scope, identifier, scoped_id));
    }
    ASSIGN_PTR(scoped_id_ptr, scoped_id);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_global_context_define_tag(struct kefir_mem *mem, struct kefir_ast_global_context *context,
                                                   const struct kefir_ast_type *type,
                                                   const struct kefir_source_location *location,
                                                   const struct kefir_ast_scoped_identifier **scoped_id_ptr) {
    UNUSED(location);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translatation context"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(!kefir_ast_type_is_variably_modified(type),
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                   "Tag definition cannot refer to variably-modified type"));

    const char *identifier = NULL;
    REQUIRE_OK(kefir_ast_context_type_retrieve_tag(type, &identifier));

    struct kefir_ast_scoped_identifier *scoped_id = NULL;
    kefir_result_t res = kefir_ast_identifier_flat_scope_at(&context->tag_scope, identifier,
                                                            (struct kefir_ast_scoped_identifier **) &scoped_id);
    if (res == KEFIR_OK) {
        REQUIRE_OK(kefir_ast_context_update_existing_scoped_type_tag(scoped_id, type));
    } else {
        REQUIRE(res == KEFIR_NOT_FOUND, res);
        scoped_id = kefir_ast_context_allocate_scoped_type_tag(mem, type, location);
        REQUIRE(scoped_id != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocted AST scoped identifier"));
        const char *id = kefir_symbol_table_insert(mem, &context->symbols, identifier, NULL);
        REQUIRE(id != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert identifier into symbol table"));
        res = kefir_ast_identifier_flat_scope_insert(mem, &context->tag_scope, id, scoped_id);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_context_free_scoped_identifier(mem, scoped_id, NULL);
            return res;
        });
    }
    ASSIGN_PTR(scoped_id_ptr, scoped_id);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_global_context_define_type(struct kefir_mem *mem, struct kefir_ast_global_context *context,
                                                    const char *identifier, const struct kefir_ast_type *type,
                                                    struct kefir_ast_alignment *alignment,
                                                    const struct kefir_source_location *location,
                                                    const struct kefir_ast_scoped_identifier **scoped_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translatation context"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid identifier"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(!kefir_ast_type_is_variably_modified(type),
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                   "Type definition in file scope cannot be variably-modified"));

    REQUIRE_OK(
        require_ordinary_identifier_type(context, identifier, KEFIR_AST_SCOPE_IDENTIFIER_TYPE_DEFINITION, location));

    identifier = kefir_symbol_table_insert(mem, &context->symbols, identifier, NULL);
    REQUIRE(identifier != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert identifier into symbol table"));

    struct kefir_ast_scoped_identifier *ordinary_id = NULL;
    kefir_result_t res = kefir_ast_identifier_flat_scope_at(&context->type_identifiers, identifier, &ordinary_id);
    if (res == KEFIR_OK) {
        REQUIRE(ordinary_id->klass == KEFIR_AST_SCOPE_IDENTIFIER_TYPE_DEFINITION &&
                    KEFIR_AST_TYPE_COMPATIBLE(context->type_traits, ordinary_id->type_definition.type, type),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Unable to redefine different type with the same identifier"));
        if (KEFIR_AST_TYPE_IS_INCOMPLETE(ordinary_id->type_definition.type) && !KEFIR_AST_TYPE_IS_INCOMPLETE(type)) {
            ordinary_id->type = type;
        }
        REQUIRE_OK(kefir_ast_context_allocate_scoped_type_definition_update_alignment(mem, ordinary_id, alignment));
    } else {
        REQUIRE(res == KEFIR_NOT_FOUND, res);
        ordinary_id = kefir_ast_context_allocate_scoped_type_definition(mem, type, alignment, location);
        REQUIRE(ordinary_id != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocted AST scoped identifier"));
        res = kefir_ast_identifier_flat_scope_insert(mem, &context->type_identifiers, identifier, ordinary_id);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_context_free_scoped_identifier(mem, ordinary_id, NULL);
            return res;
        });
        REQUIRE_OK(insert_ordinary_identifier(mem, context, identifier, ordinary_id));
    }

    ASSIGN_PTR(scoped_id, ordinary_id);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_global_context_declare_function(
    struct kefir_mem *mem, struct kefir_ast_global_context *context, kefir_ast_function_specifier_t specifier,
    kefir_bool_t external_linkage, const char *identifier, const struct kefir_ast_type *function,
    const struct kefir_ast_declarator_attributes *attributes, const struct kefir_source_location *location,
    const struct kefir_ast_scoped_identifier **scoped_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translatation context"));
    REQUIRE(function != NULL && function->tag == KEFIR_AST_TYPE_FUNCTION,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST function type"));
    REQUIRE(identifier != NULL && strlen(identifier) > 0,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected function with non-empty identifier"));
    REQUIRE(!kefir_ast_type_is_variably_modified(function),
            KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Function type cannot be variably-modified"));

    REQUIRE_OK(require_ordinary_identifier_type(context, identifier, KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION, location));

    struct kefir_ast_scoped_identifier *ordinary_id = NULL;
    kefir_result_t res = kefir_ast_identifier_flat_scope_at(&context->function_identifiers, identifier, &ordinary_id);
    if (res == KEFIR_OK) {
        REQUIRE(ordinary_id->function.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN ||
                    ordinary_id->function.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Cannot declare the same identifier with different storage class"));
        REQUIRE(KEFIR_AST_TYPE_COMPATIBLE(context->type_traits, ordinary_id->function.type, function),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "All declarations of the same identifier shall have compatible types"));
        ordinary_id->function.type = KEFIR_AST_TYPE_COMPOSITE(mem, &context->type_bundle, context->type_traits,
                                                              ordinary_id->function.type, function);
        ordinary_id->function.specifier =
            kefir_ast_context_merge_function_specifiers(ordinary_id->function.specifier, specifier);
        ordinary_id->function.inline_definition = ordinary_id->function.inline_definition && !external_linkage &&
                                                  kefir_ast_function_specifier_is_inline(specifier);
        if (attributes != NULL) {
            ordinary_id->function.flags.gnu_inline = ordinary_id->function.flags.gnu_inline || attributes->gnu_inline;
            if (ordinary_id->function.asm_label == NULL) {
                ordinary_id->function.asm_label = attributes->asm_label;
            } else {
                REQUIRE(attributes->asm_label == NULL ||
                            strcmp(attributes->asm_label, ordinary_id->function.asm_label) == 0,
                        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                               "Assembly label does not match with previous declaration"));
            }
        }
    } else {
        REQUIRE(res == KEFIR_NOT_FOUND, res);
        ordinary_id = kefir_ast_context_allocate_scoped_function_identifier(
            mem, function, specifier, KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN, true, false,
            !external_linkage && kefir_ast_function_specifier_is_inline(specifier),
            attributes != NULL ? attributes->asm_label : NULL, location);
        REQUIRE(ordinary_id != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocted AST scoped identifier"));
        const char *id = kefir_symbol_table_insert(mem, &context->symbols, identifier, NULL);
        REQUIRE(id != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert identifier into symbol table"));
        res = kefir_ast_identifier_flat_scope_insert(mem, &context->function_identifiers, id, ordinary_id);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_context_free_scoped_identifier(mem, ordinary_id, NULL);
            return res;
        });
        ordinary_id->function.flags.gnu_inline = attributes != NULL && attributes->gnu_inline;
    }

    REQUIRE_OK(insert_ordinary_identifier(mem, context, identifier, ordinary_id));
    ASSIGN_PTR(scoped_id, ordinary_id);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_global_context_define_function(struct kefir_mem *mem, struct kefir_ast_global_context *context,
                                                        kefir_ast_function_specifier_t specifier,
                                                        kefir_bool_t external_linkage, const char *identifier,
                                                        const struct kefir_ast_type *function,
                                                        const struct kefir_ast_declarator_attributes *attributes,
                                                        const struct kefir_source_location *location,
                                                        const struct kefir_ast_scoped_identifier **scoped_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translatation context"));
    REQUIRE(function != NULL && function->tag == KEFIR_AST_TYPE_FUNCTION,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST function type"));
    REQUIRE(identifier != NULL && strlen(identifier) > 0,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected function with non-empty identifier"));
    REQUIRE(!kefir_ast_type_is_variably_modified(function),
            KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Function type cannot be variably-modified"));

    REQUIRE_OK(require_ordinary_identifier_type(context, identifier, KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION, location));

    struct kefir_ast_scoped_identifier *ordinary_id = NULL;
    kefir_result_t res = kefir_ast_identifier_flat_scope_at(&context->function_identifiers, identifier, &ordinary_id);
    if (res == KEFIR_OK) {
        REQUIRE(ordinary_id->function.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN ||
                    ordinary_id->function.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Cannot declare the same identifier with different storage class"));
        REQUIRE(KEFIR_AST_TYPE_COMPATIBLE(context->type_traits, ordinary_id->function.type, function),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "All declarations of the same identifier shall have compatible types"));
        REQUIRE(!ordinary_id->function.defined,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Cannot redefine function with the same identifier"));
        ordinary_id->function.type = KEFIR_AST_TYPE_COMPOSITE(mem, &context->type_bundle, context->type_traits,
                                                              ordinary_id->function.type, function);
        ordinary_id->function.specifier =
            kefir_ast_context_merge_function_specifiers(ordinary_id->function.specifier, specifier);
        ordinary_id->function.external = false;
        ordinary_id->function.defined = true;
        ordinary_id->function.inline_definition = ordinary_id->function.inline_definition && !external_linkage &&
                                                  kefir_ast_function_specifier_is_inline(specifier);
        if (attributes != NULL) {
            ordinary_id->function.flags.gnu_inline = ordinary_id->function.flags.gnu_inline || attributes->gnu_inline;
            if (ordinary_id->function.asm_label == NULL) {
                ordinary_id->function.asm_label = attributes->asm_label;
            } else {
                REQUIRE(attributes->asm_label == NULL ||
                            strcmp(attributes->asm_label, ordinary_id->function.asm_label) == 0,
                        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                               "Assembly label does not match with previous declaration"));
            }
        }
    } else {
        REQUIRE(res == KEFIR_NOT_FOUND, res);
        ordinary_id = kefir_ast_context_allocate_scoped_function_identifier(
            mem, function, specifier, KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN, false, true,
            !external_linkage && kefir_ast_function_specifier_is_inline(specifier),
            attributes != NULL ? attributes->asm_label : NULL, location);
        REQUIRE(ordinary_id != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocted AST scoped identifier"));
        const char *id = kefir_symbol_table_insert(mem, &context->symbols, identifier, NULL);
        REQUIRE(id != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert identifier into symbol table"));
        res = kefir_ast_identifier_flat_scope_insert(mem, &context->function_identifiers, id, ordinary_id);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_context_free_scoped_identifier(mem, ordinary_id, NULL);
            return res;
        });
        ordinary_id->function.flags.gnu_inline = attributes != NULL && attributes->gnu_inline;
    }

    REQUIRE_OK(insert_ordinary_identifier(mem, context, identifier, ordinary_id));
    ASSIGN_PTR(scoped_id, ordinary_id);
    return KEFIR_OK;
}

kefir_result_t kefir_ast_global_context_define_static_function(
    struct kefir_mem *mem, struct kefir_ast_global_context *context, kefir_ast_function_specifier_t specifier,
    const char *identifier, kefir_bool_t declaration, const struct kefir_ast_type *function,
    const struct kefir_ast_declarator_attributes *attributes, const struct kefir_source_location *location,
    const struct kefir_ast_scoped_identifier **scoped_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translatation context"));
    REQUIRE(function != NULL && function->tag == KEFIR_AST_TYPE_FUNCTION,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST function type"));
    REQUIRE(identifier != NULL && strlen(identifier) > 0,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected function with non-empty identifier"));
    REQUIRE(!kefir_ast_type_is_variably_modified(function),
            KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Function type cannot be variably-modified"));

    REQUIRE_OK(require_ordinary_identifier_type(context, identifier, KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION, location));

    struct kefir_ast_scoped_identifier *ordinary_id = NULL;
    kefir_result_t res = kefir_ast_identifier_flat_scope_at(&context->function_identifiers, identifier, &ordinary_id);
    if (res == KEFIR_OK) {
        REQUIRE(ordinary_id->function.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Cannot declare the same identifier with different storage class"));
        REQUIRE(KEFIR_AST_TYPE_COMPATIBLE(context->type_traits, ordinary_id->function.type, function),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "All declarations of the same identifier shall have compatible types"));
        REQUIRE(!ordinary_id->function.external,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Function identifier with static storage cannot be external"));
        REQUIRE(!ordinary_id->function.defined || declaration,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                       "Cannot redefine function with the same identifier"));
        ordinary_id->function.type = KEFIR_AST_TYPE_COMPOSITE(mem, &context->type_bundle, context->type_traits,
                                                              ordinary_id->function.type, function);
        ordinary_id->function.specifier =
            kefir_ast_context_merge_function_specifiers(ordinary_id->function.specifier, specifier);
        ordinary_id->function.defined = ordinary_id->function.defined || !declaration;
        ordinary_id->function.inline_definition =
            ordinary_id->function.inline_definition && kefir_ast_function_specifier_is_inline(specifier);
        if (attributes != NULL) {
            if (ordinary_id->function.asm_label == NULL) {
                ordinary_id->function.asm_label = attributes->asm_label;
            } else {
                REQUIRE(attributes->asm_label == NULL ||
                            strcmp(attributes->asm_label, ordinary_id->function.asm_label) == 0,
                        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, location,
                                               "Assembly label does not match with previous declaration"));
            }
        }
    } else {
        REQUIRE(res == KEFIR_NOT_FOUND, res);
        ordinary_id = kefir_ast_context_allocate_scoped_function_identifier(
            mem, function, specifier, KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC, false, !declaration,
            kefir_ast_function_specifier_is_inline(specifier), attributes != NULL ? attributes->asm_label : NULL,
            location);
        REQUIRE(ordinary_id != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocted AST scoped identifier"));
        const char *id = kefir_symbol_table_insert(mem, &context->symbols, identifier, NULL);
        REQUIRE(id != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert identifier into symbol table"));
        res = kefir_ast_identifier_flat_scope_insert(mem, &context->function_identifiers, id, ordinary_id);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_context_free_scoped_identifier(mem, ordinary_id, NULL);
            return res;
        });
    }

    REQUIRE_OK(insert_ordinary_identifier(mem, context, identifier, ordinary_id));
    ASSIGN_PTR(scoped_id, ordinary_id);
    return KEFIR_OK;
}
