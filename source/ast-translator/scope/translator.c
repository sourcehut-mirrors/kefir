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

#include "kefir/ast-translator/scope/translator.h"
#include "kefir/ast-translator/context.h"
#include "kefir/ast/runtime.h"
#include "kefir/ast/target_environment.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/ir/type_tree.h"
#include <stdio.h>

static kefir_size_t resolve_base_slot(const struct kefir_ir_type_tree_node *node) {
    REQUIRE(node != NULL, 0);
    return resolve_base_slot(node->parent) + node->relative_slot;
}

static kefir_result_t initialize_data(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                      struct kefir_ir_module *module, const struct kefir_ir_type *type,
                                      struct kefir_ast_type_layout *type_layout,
                                      struct kefir_ast_initializer *initializer, struct kefir_ir_data *data) {

    struct kefir_ir_type_tree type_tree;
    REQUIRE_OK(kefir_ir_type_tree_init(mem, type, &type_tree));

    const struct kefir_ir_type_tree_node *tree_node;
    kefir_result_t res = kefir_ir_type_tree_at(&type_tree, type_layout->value, &tree_node);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ir_type_tree_free(mem, &type_tree);
        return res;
    });

    res = kefir_ast_translate_data_initializer(mem, context, module, type_layout, type, initializer, data,
                                               resolve_base_slot(tree_node));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ir_type_tree_free(mem, &type_tree);
        return res;
    });

    REQUIRE_OK(kefir_ir_type_tree_free(mem, &type_tree));
    return KEFIR_OK;
}

static kefir_ir_identifier_visibility_t get_ir_visibility(kefir_ast_declarator_visibility_attr_t visibility) {
    switch (visibility) {
        case KEFIR_AST_DECLARATOR_VISIBILITY_HIDDEN:
            return KEFIR_IR_IDENTIFIER_VISIBILITY_HIDDEN;

        case KEFIR_AST_DECLARATOR_VISIBILITY_INTERNAL:
            return KEFIR_IR_IDENTIFIER_VISIBILITY_INTERNAL;

        case KEFIR_AST_DECLARATOR_VISIBILITY_PROTECTED:
            return KEFIR_IR_IDENTIFIER_VISIBILITY_PROTECTED;

        default:
            return KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT;
    }
}

static kefir_result_t assign_common(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                    const struct kefir_ast_type *type, struct kefir_ir_identifier *ir_identifier) {
    ir_identifier->common = true;
    kefir_ast_target_environment_opaque_type_t opaque_type;
    REQUIRE_OK(kefir_ast_context_type_cache_get_type(mem, context->cache, type, &opaque_type, NULL));
    struct kefir_ast_target_environment_object_info object_info;
    REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_OBJECT_INFO(mem, context->target_env, opaque_type, NULL, &object_info));
    ir_identifier->common_props.size = object_info.size;
    ir_identifier->common_props.alignment = object_info.alignment;
    return KEFIR_OK;
}

#define SCOPED_IDENTIFIER_DEBUG_INFO_ENTRY(_data) \
    ((_data)->debug_info.present ? (_data)->debug_info.variable : KEFIR_IR_DEBUG_ENTRY_ID_NONE)
#define SCOPED_IDENTIFIER_DEBUG_INFO_FUNCTION_ENTRY(_data) \
    ((_data)->debug_info.present ? (_data)->debug_info.subprogram : KEFIR_IR_DEBUG_ENTRY_ID_NONE)
static kefir_result_t translate_externals(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                          struct kefir_ir_module *module,
                                          const struct kefir_ast_translator_global_scope_layout *global_scope) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&global_scope->external_objects); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ast_translator_scoped_identifier_entry *, scoped_identifier, iter->value);
        switch (scoped_identifier->value->klass) {
            case KEFIR_AST_SCOPE_IDENTIFIER_OBJECT: {
                ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, identifier_data,
                                 scoped_identifier->value->payload.ptr);

                struct kefir_ir_identifier ir_identifier = {
                    .symbol = scoped_identifier->value->object.asm_label == NULL
                                  ? scoped_identifier->identifier
                                  : scoped_identifier->value->object.asm_label,
                    .type = KEFIR_IR_IDENTIFIER_GLOBAL_DATA,
                    .visibility = !scoped_identifier->value->object.external
                                      ? get_ir_visibility(scoped_identifier->value->object.visibility)
                                      : KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT,
                    .alias = NULL,
                    .debug_info = {.entry = SCOPED_IDENTIFIER_DEBUG_INFO_ENTRY(identifier_data)}};
#define DECL_GLOBAL_WEAK                                                 \
    do {                                                                 \
        if (scoped_identifier->value->object.flags.weak) {               \
            ir_identifier.scope = KEFIR_IR_IDENTIFIER_SCOPE_EXPORT_WEAK; \
        } else {                                                         \
            ir_identifier.scope = KEFIR_IR_IDENTIFIER_SCOPE_EXPORT;      \
        }                                                                \
    } while (0)
                if (scoped_identifier->value->object.flags.common && !scoped_identifier->value->object.external &&
                    !scoped_identifier->value->object.flags.weak &&
                    scoped_identifier->value->object.initializer == NULL) {
                    REQUIRE_OK(assign_common(mem, context, scoped_identifier->value->object.type, &ir_identifier));
                }

                if (scoped_identifier->value->object.alias != NULL) {
                    ir_identifier.alias = scoped_identifier->value->object.alias;
                    DECL_GLOBAL_WEAK;
                } else if (scoped_identifier->value->object.external) {
                    if (scoped_identifier->value->object.flags.weak) {
                        ir_identifier.scope = KEFIR_IR_IDENTIFIER_SCOPE_EXPORT_WEAK;
                    } else {
                        ir_identifier.scope = KEFIR_IR_IDENTIFIER_SCOPE_IMPORT;
                    }
                } else if (!ir_identifier.common) {
                    const kefir_ir_data_storage_t storage =
                        scoped_identifier->value->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_CONSTEXPR_STATIC
                            ? KEFIR_IR_DATA_GLOBAL_READONLY_STORAGE
                            : KEFIR_IR_DATA_GLOBAL_STORAGE;

                    struct kefir_ir_data *data = kefir_ir_module_new_named_data(
                        mem, module, scoped_identifier->identifier, storage, identifier_data->type_id);
                    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR named data"));
                    if (scoped_identifier->value->object.initializer != NULL) {
                        REQUIRE_OK(initialize_data(mem, context, module, identifier_data->type, identifier_data->layout,
                                                   scoped_identifier->value->object.initializer, data));
                    }
                    REQUIRE_OK(kefir_ir_data_finalize(mem, data));

                    DECL_GLOBAL_WEAK;
                }

                REQUIRE_OK(
                    kefir_ir_module_declare_identifier(mem, module, scoped_identifier->identifier, &ir_identifier));
#undef DECL_GLOBAL_WEAK
            } break;

            case KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION: {
                ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_function *, identifier_data,
                                 scoped_identifier->value->payload.ptr);
                struct kefir_ir_identifier ir_identifier = {
                    .symbol = scoped_identifier->identifier,
                    .type = KEFIR_IR_IDENTIFIER_FUNCTION,
                    .scope = KEFIR_IR_IDENTIFIER_SCOPE_LOCAL,
                    .visibility = KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT,
                    .debug_info = {.entry = SCOPED_IDENTIFIER_DEBUG_INFO_FUNCTION_ENTRY(identifier_data)}};

#define DECL_GLOBAL_WEAK                                                 \
    do {                                                                 \
        if (scoped_identifier->value->function.flags.weak) {             \
            ir_identifier.scope = KEFIR_IR_IDENTIFIER_SCOPE_EXPORT_WEAK; \
        } else {                                                         \
            ir_identifier.scope = KEFIR_IR_IDENTIFIER_SCOPE_EXPORT;      \
        }                                                                \
    } while (0)
                if (scoped_identifier->value->function.alias != NULL) {
                    ir_identifier.alias = scoped_identifier->value->function.alias;
                    DECL_GLOBAL_WEAK;
                } else {
                    if (scoped_identifier->value->function.asm_label != NULL) {
                        ir_identifier.symbol = scoped_identifier->value->function.asm_label;
                    }
                    if (!scoped_identifier->value->function.flags.gnu_inline ||
                        !kefir_ast_function_specifier_is_inline(scoped_identifier->value->function.specifier)) {
                        if (scoped_identifier->value->function.external) {
                            ir_identifier.scope = KEFIR_IR_IDENTIFIER_SCOPE_IMPORT;
                        } else if (scoped_identifier->value->function.storage !=
                                       KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC &&
                                   !scoped_identifier->value->function.inline_definition) {
                            DECL_GLOBAL_WEAK;
                        }
                    } else if (scoped_identifier->value->function.storage ==
                               KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN) {
                        if (scoped_identifier->value->function.inline_definition) {
                            DECL_GLOBAL_WEAK;
                        } else {
                            ir_identifier.scope = KEFIR_IR_IDENTIFIER_SCOPE_IMPORT;
                        }
                    }
                }
                if (scoped_identifier->value->function.defined) {
                    ir_identifier.visibility = get_ir_visibility(scoped_identifier->value->function.visibility);
                }

                REQUIRE_OK(
                    kefir_ir_module_declare_identifier(mem, module, scoped_identifier->identifier, &ir_identifier));
                if (scoped_identifier->value->function.flags.gnu_inline &&
                    scoped_identifier->value->function.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN &&
                    kefir_ast_function_specifier_is_inline(scoped_identifier->value->function.specifier) &&
                    !scoped_identifier->value->function.inline_definition &&
                    scoped_identifier->value->function.asm_label == NULL) {
                    char identifier_buf[1024];
                    snprintf(identifier_buf, sizeof(identifier_buf) - 1,
                             KEFIR_AST_TRANSLATOR_GNU_INLINE_FUNCTION_IDENTIFIER, scoped_identifier->identifier);
                    const char *function_name = kefir_ir_module_symbol(mem, module, identifier_buf, NULL);
                    REQUIRE(function_name != NULL,
                            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                            "Failed to insert generated function identifier into symbol table"));

                    ir_identifier.scope = KEFIR_IR_IDENTIFIER_SCOPE_LOCAL;
                    ir_identifier.symbol = function_name;
                    REQUIRE_OK(kefir_ir_module_declare_identifier(mem, module, function_name, &ir_identifier));
                }
#undef DECL_GLOBAL_WEAK
            } break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to translate global scoped identifier");
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_static(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                       struct kefir_ir_module *module,
                                       const struct kefir_ast_translator_global_scope_layout *global_scope) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&global_scope->static_objects); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ast_translator_scoped_identifier_entry *, scoped_identifier, iter->value);

        switch (scoped_identifier->value->klass) {
            case KEFIR_AST_SCOPE_IDENTIFIER_OBJECT: {
                ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, identifier_data,
                                 scoped_identifier->value->payload.ptr);

                struct kefir_ir_identifier ir_identifier = {
                    .symbol = scoped_identifier->value->object.asm_label == NULL
                                  ? scoped_identifier->identifier
                                  : scoped_identifier->value->object.asm_label,
                    .type = KEFIR_IR_IDENTIFIER_GLOBAL_DATA,
                    .scope = KEFIR_IR_IDENTIFIER_SCOPE_LOCAL,
                    .visibility = KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT,
                    .alias = NULL,
                    .debug_info = {.entry = SCOPED_IDENTIFIER_DEBUG_INFO_ENTRY(identifier_data)}};

                if (scoped_identifier->value->object.flags.common && !scoped_identifier->value->object.flags.weak &&
                    scoped_identifier->value->object.initializer == NULL) {
                    REQUIRE_OK(assign_common(mem, context, scoped_identifier->value->object.type, &ir_identifier));
                }

                if (!ir_identifier.common) {
                    const kefir_ir_data_storage_t storage =
                        scoped_identifier->value->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_CONSTEXPR_STATIC
                            ? KEFIR_IR_DATA_GLOBAL_READONLY_STORAGE
                            : KEFIR_IR_DATA_GLOBAL_STORAGE;
                    struct kefir_ir_data *data = kefir_ir_module_new_named_data(
                        mem, module, scoped_identifier->identifier, storage, identifier_data->type_id);
                    if (scoped_identifier->value->object.initializer != NULL) {
                        REQUIRE_OK(initialize_data(mem, context, module, identifier_data->type, identifier_data->layout,
                                                   scoped_identifier->value->object.initializer, data));
                    }
                    REQUIRE_OK(kefir_ir_data_finalize(mem, data));
                }

                REQUIRE_OK(
                    kefir_ir_module_declare_identifier(mem, module, scoped_identifier->identifier, &ir_identifier));
            } break;

            case KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION: {
                ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_function *, identifier_data,
                                 scoped_identifier->value->payload.ptr);
                struct kefir_ir_identifier ir_identifier = {
                    .symbol = scoped_identifier->identifier,
                    .type = KEFIR_IR_IDENTIFIER_FUNCTION,
                    .scope = KEFIR_IR_IDENTIFIER_SCOPE_LOCAL,
                    .visibility = KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT,
                    .alias = scoped_identifier->value->function.alias,
                    .debug_info = {.entry = SCOPED_IDENTIFIER_DEBUG_INFO_FUNCTION_ENTRY(identifier_data)}};
                REQUIRE_OK(
                    kefir_ir_module_declare_identifier(mem, module, scoped_identifier->identifier, &ir_identifier));
            } break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to translate global scoped identifier");
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_external_thread_locals(
    struct kefir_mem *mem, const struct kefir_ast_context *context, struct kefir_ir_module *module,
    const struct kefir_ast_translator_global_scope_layout *global_scope) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&global_scope->external_thread_local_objects);
         iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ast_translator_scoped_identifier_entry *, scoped_identifier, iter->value);
        switch (scoped_identifier->value->klass) {
            case KEFIR_AST_SCOPE_IDENTIFIER_OBJECT: {
                ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, identifier_data,
                                 scoped_identifier->value->payload.ptr);
                if (scoped_identifier->value->object.external) {
                    struct kefir_ir_identifier ir_identifier = {
                        .symbol = scoped_identifier->identifier,
                        .type = KEFIR_IR_IDENTIFIER_THREAD_LOCAL_DATA,
                        .scope = KEFIR_IR_IDENTIFIER_SCOPE_IMPORT,
                        .visibility = KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT,
                        .alias = NULL,
                        .debug_info = {.entry = SCOPED_IDENTIFIER_DEBUG_INFO_ENTRY(identifier_data)}};

                    if (scoped_identifier->value->object.flags.weak) {
                        ir_identifier.scope = KEFIR_IR_IDENTIFIER_SCOPE_EXPORT_WEAK;
                    }

                    REQUIRE_OK(
                        kefir_ir_module_declare_identifier(mem, module, scoped_identifier->identifier, &ir_identifier));
                } else {
                    struct kefir_ir_identifier ir_identifier = {
                        .symbol = scoped_identifier->identifier,
                        .type = KEFIR_IR_IDENTIFIER_THREAD_LOCAL_DATA,
                        .scope = KEFIR_IR_IDENTIFIER_SCOPE_EXPORT,
                        .visibility = KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT,
                        .alias = NULL,
                        .debug_info = {.entry = SCOPED_IDENTIFIER_DEBUG_INFO_ENTRY(identifier_data)}};
                    if (scoped_identifier->value->object.flags.common && !scoped_identifier->value->object.flags.weak &&
                        scoped_identifier->value->object.initializer == NULL &&
                        context->configuration->analysis.enable_thread_local_common) {
                        REQUIRE_OK(assign_common(mem, context, scoped_identifier->value->object.type, &ir_identifier));
                    }
                    REQUIRE_OK(
                        kefir_ir_module_declare_identifier(mem, module, scoped_identifier->identifier, &ir_identifier));

                    if (!ir_identifier.common) {
                        struct kefir_ir_data *data = kefir_ir_module_new_named_data(
                            mem, module, scoped_identifier->identifier, KEFIR_IR_DATA_THREAD_LOCAL_STORAGE,
                            identifier_data->type_id);
                        REQUIRE(data != NULL,
                                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR named data"));
                        if (scoped_identifier->value->object.initializer != NULL) {
                            REQUIRE_OK(initialize_data(mem, context, module, identifier_data->type,
                                                       identifier_data->layout,
                                                       scoped_identifier->value->object.initializer, data));
                        }
                        REQUIRE_OK(kefir_ir_data_finalize(mem, data));
                    }
                }
            } break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to translate global scoped identifier");
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_static_thread_locals(
    struct kefir_mem *mem, const struct kefir_ast_context *context, struct kefir_ir_module *module,
    const struct kefir_ast_translator_global_scope_layout *global_scope) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&global_scope->static_thread_local_objects);
         iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ast_translator_scoped_identifier_entry *, scoped_identifier, iter->value);

        switch (scoped_identifier->value->klass) {
            case KEFIR_AST_SCOPE_IDENTIFIER_OBJECT: {
                ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, identifier_data,
                                 scoped_identifier->value->payload.ptr);

                struct kefir_ir_identifier ir_identifier = {
                    .symbol = scoped_identifier->value->object.asm_label == NULL
                                  ? scoped_identifier->identifier
                                  : scoped_identifier->value->object.asm_label,
                    .type = KEFIR_IR_IDENTIFIER_THREAD_LOCAL_DATA,
                    .scope = KEFIR_IR_IDENTIFIER_SCOPE_LOCAL,
                    .visibility = KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT,
                    .alias = NULL,
                    .debug_info = {.entry = SCOPED_IDENTIFIER_DEBUG_INFO_ENTRY(identifier_data)}};

                if (scoped_identifier->value->object.flags.common && !scoped_identifier->value->object.flags.weak &&
                    scoped_identifier->value->object.initializer == NULL &&
                        context->configuration->analysis.enable_thread_local_common) {
                    REQUIRE_OK(assign_common(mem, context, scoped_identifier->value->object.type, &ir_identifier));
                }

                if (!ir_identifier.common) {
                    struct kefir_ir_data *data =
                        kefir_ir_module_new_named_data(mem, module, scoped_identifier->identifier,
                                                       KEFIR_IR_DATA_THREAD_LOCAL_STORAGE, identifier_data->type_id);
                    if (scoped_identifier->value->object.initializer != NULL) {
                        REQUIRE_OK(initialize_data(mem, context, module, identifier_data->type, identifier_data->layout,
                                                   scoped_identifier->value->object.initializer, data));
                    }
                    REQUIRE_OK(kefir_ir_data_finalize(mem, data));
                }

                REQUIRE_OK(
                    kefir_ir_module_declare_identifier(mem, module, scoped_identifier->identifier, &ir_identifier));
            } break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to translate global scoped identifier");
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_global_scope(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                struct kefir_ir_module *module,
                                                const struct kefir_ast_translator_global_scope_layout *global_scope) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(global_scope != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator global scope"));

    REQUIRE_OK(translate_externals(mem, context, module, global_scope));
    REQUIRE_OK(translate_static(mem, context, module, global_scope));
    REQUIRE_OK(translate_external_thread_locals(mem, context, module, global_scope));
    REQUIRE_OK(translate_static_thread_locals(mem, context, module, global_scope));
    return KEFIR_OK;
}

static kefir_result_t local_static_identifier(struct kefir_mem *mem, struct kefir_ir_module *module,
                                              const char *function_name, const char *variable_identifier,
                                              kefir_id_t unique_id, const char **identifier) {
    int buflen = snprintf(NULL, 0, KEFIR_AST_TRANSLATOR_FUNCTION_STATIC_VARIABLE_IDENTIFIER, function_name,
                          variable_identifier, unique_id);
    char *buf = KEFIR_MALLOC(mem, sizeof(char) * (buflen + 2));
    REQUIRE(buf != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate local static variable identifier"));
    snprintf(buf, buflen + 1, KEFIR_AST_TRANSLATOR_FUNCTION_STATIC_VARIABLE_IDENTIFIER, function_name,
             variable_identifier, unique_id);
    *identifier = kefir_ir_module_symbol(mem, module, buf, NULL);
    KEFIR_FREE(mem, buf);
    REQUIRE(*identifier != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate local static variable identifier"));
    return KEFIR_OK;
}

static kefir_result_t translate_local_static(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                             struct kefir_ir_module *module,
                                             const struct kefir_ast_translator_local_scope_layout *local_scope) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&local_scope->static_objects); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ast_translator_scoped_identifier_entry *, scoped_identifier, iter->value);

        switch (scoped_identifier->value->klass) {
            case KEFIR_AST_SCOPE_IDENTIFIER_OBJECT: {
                ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, identifier_data,
                                 scoped_identifier->value->payload.ptr);

                const char *identifier;
                REQUIRE_OK(local_static_identifier(mem, module, scoped_identifier->value->object.defining_function,
                                                   scoped_identifier->identifier, identifier_data->identifier,
                                                   &identifier));

                struct kefir_ir_identifier ir_identifier = {
                    .symbol = scoped_identifier->value->object.asm_label == NULL
                                  ? identifier
                                  : scoped_identifier->value->object.asm_label,
                    .type = KEFIR_IR_IDENTIFIER_GLOBAL_DATA,
                    .scope = KEFIR_IR_IDENTIFIER_SCOPE_LOCAL,
                    .visibility = KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT,
                    .alias = NULL,
                    .debug_info = {.entry = SCOPED_IDENTIFIER_DEBUG_INFO_ENTRY(identifier_data)}};

                const kefir_ir_data_storage_t storage =
                    scoped_identifier->value->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_CONSTEXPR_STATIC
                        ? KEFIR_IR_DATA_GLOBAL_READONLY_STORAGE
                        : KEFIR_IR_DATA_GLOBAL_STORAGE;

                struct kefir_ir_data *data =
                    kefir_ir_module_new_named_data(mem, module, identifier, storage, identifier_data->type_id);
                if (scoped_identifier->value->object.initializer != NULL) {
                    if (scoped_identifier->value->definition_scope != NULL) {
                        REQUIRE_OK(context->push_external_ordinary_scope(
                            mem, scoped_identifier->value->definition_scope, context));
                    }
                    REQUIRE_OK(initialize_data(mem, context, module, identifier_data->type, identifier_data->layout,
                                               scoped_identifier->value->object.initializer, data));
                    if (scoped_identifier->value->definition_scope != NULL) {
                        REQUIRE_OK(context->pop_external_oridnary_scope(mem, context));
                    }
                }
                REQUIRE_OK(kefir_ir_data_finalize(mem, data));

                REQUIRE_OK(kefir_ir_module_declare_identifier(mem, module, identifier, &ir_identifier));
            } break;

            case KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION:
                // Do nothing
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to translate local scope identifier");
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_local_static_thread_locals(
    struct kefir_mem *mem, const struct kefir_ast_context *context, struct kefir_ir_module *module,
    const struct kefir_ast_translator_local_scope_layout *local_scope) {

    for (const struct kefir_list_entry *iter = kefir_list_head(&local_scope->static_thread_local_objects); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(const struct kefir_ast_translator_scoped_identifier_entry *, scoped_identifier, iter->value);

        switch (scoped_identifier->value->klass) {
            case KEFIR_AST_SCOPE_IDENTIFIER_OBJECT: {
                ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, identifier_data,
                                 scoped_identifier->value->payload.ptr);

                const char *identifier;
                REQUIRE_OK(local_static_identifier(mem, module, scoped_identifier->value->object.defining_function,
                                                   scoped_identifier->identifier, identifier_data->identifier,
                                                   &identifier));

                struct kefir_ir_identifier ir_identifier = {
                    .symbol = scoped_identifier->value->object.asm_label == NULL
                                  ? identifier
                                  : scoped_identifier->value->object.asm_label,
                    .type = KEFIR_IR_IDENTIFIER_THREAD_LOCAL_DATA,
                    .scope = KEFIR_IR_IDENTIFIER_SCOPE_LOCAL,
                    .visibility = KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT,
                    .alias = NULL,
                    .debug_info = {.entry = SCOPED_IDENTIFIER_DEBUG_INFO_ENTRY(identifier_data)}};

                struct kefir_ir_data *data = kefir_ir_module_new_named_data(
                    mem, module, identifier, KEFIR_IR_DATA_THREAD_LOCAL_STORAGE, identifier_data->type_id);
                if (scoped_identifier->value->object.initializer != NULL) {
                    if (scoped_identifier->value->definition_scope != NULL) {
                        REQUIRE_OK(context->push_external_ordinary_scope(
                            mem, scoped_identifier->value->definition_scope, context));
                    }
                    REQUIRE_OK(initialize_data(mem, context, module, identifier_data->type, identifier_data->layout,
                                               scoped_identifier->value->object.initializer, data));
                    if (scoped_identifier->value->definition_scope != NULL) {
                        REQUIRE_OK(context->pop_external_oridnary_scope(mem, context));
                    }
                }
                REQUIRE_OK(kefir_ir_data_finalize(mem, data));

                REQUIRE_OK(kefir_ir_module_declare_identifier(mem, module, identifier, &ir_identifier));
            } break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to translate local scope identifier");
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_label_scope(struct kefir_mem *mem, struct kefir_ir_module *module,
                                            const struct kefir_ast_identifier_flat_scope *label_scope) {

    struct kefir_ast_identifier_flat_scope_iterator iter;
    kefir_result_t res;
    for (res = kefir_ast_identifier_flat_scope_iter(label_scope, &iter); res == KEFIR_OK;
         res = kefir_ast_identifier_flat_scope_next(label_scope, &iter)) {
        switch (iter.value->klass) {
            case KEFIR_AST_SCOPE_IDENTIFIER_LABEL:
                if (iter.value->label.public_label != NULL) {
                    struct kefir_ir_identifier ir_identifier = {.symbol = iter.value->label.public_label,
                                                                .type = KEFIR_IR_IDENTIFIER_FUNCTION,
                                                                .scope = KEFIR_IR_IDENTIFIER_SCOPE_LOCAL,
                                                                .visibility = KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT,
                                                                .alias = NULL,
                                                                .debug_info = {.entry = KEFIR_IR_DEBUG_ENTRY_ID_NONE}};
                    REQUIRE_OK(kefir_ir_module_declare_identifier(mem, module, iter.value->label.public_label,
                                                                  &ir_identifier));
                }
                break;

            default:
                // Intentionally left blank
                break;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_local_scope(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                               struct kefir_ir_module *module,
                                               const struct kefir_ast_translator_local_scope_layout *local_scope) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(local_scope != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator local scope"));

    REQUIRE_OK(translate_label_scope(mem, module, &local_scope->local_context->label_scope));
    REQUIRE_OK(translate_local_static(mem, context, module, local_scope));
    REQUIRE_OK(translate_local_static_thread_locals(mem, context, module, local_scope));
    return KEFIR_OK;
}
