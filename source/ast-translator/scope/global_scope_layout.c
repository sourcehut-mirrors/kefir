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

#include "kefir/ast-translator/scope/global_scope_layout.h"
#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/layout.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/ast-translator/scope/scope_layout_impl.h"
#include "kefir/ast/type_completion.h"
#include "kefir/core/source_error.h"

kefir_result_t kefir_ast_translator_global_scope_layout_init(struct kefir_mem *mem, struct kefir_ir_module *module,
                                                             struct kefir_ast_translator_global_scope_layout *layout) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST global scope layout"));
    layout->global_context = NULL;
    REQUIRE_OK(kefir_list_init(&layout->external_objects));
    REQUIRE_OK(kefir_list_init(&layout->external_thread_local_objects));
    REQUIRE_OK(kefir_list_init(&layout->static_objects));
    REQUIRE_OK(kefir_list_init(&layout->static_thread_local_objects));

    REQUIRE_OK(kefir_list_on_remove(&layout->external_objects, kefir_ast_translator_scoped_identifier_remove, NULL));
    REQUIRE_OK(kefir_list_on_remove(&layout->external_thread_local_objects,
                                    kefir_ast_translator_scoped_identifier_remove, NULL));
    REQUIRE_OK(kefir_list_on_remove(&layout->static_objects, kefir_ast_translator_scoped_identifier_remove, NULL));
    REQUIRE_OK(kefir_list_on_remove(&layout->static_thread_local_objects, kefir_ast_translator_scoped_identifier_remove,
                                    NULL));

    layout->static_layout = kefir_ir_module_new_type(mem, module, 0, &layout->static_layout_id);
    REQUIRE(layout->static_layout != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate new IR type"));
    layout->static_uninit_layout = kefir_ir_module_new_type(mem, module, 0, &layout->static_uninit_layout_id);
    REQUIRE(layout->static_uninit_layout != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate new IR type"));
    layout->static_thread_local_layout =
        kefir_ir_module_new_type(mem, module, 0, &layout->static_thread_local_layout_id);
    REQUIRE(layout->static_thread_local_layout != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate new IR type"));
    layout->static_thread_local_uninit_layout =
        kefir_ir_module_new_type(mem, module, 0, &layout->static_thread_local_uninit_layout_id);
    REQUIRE(layout->static_thread_local_uninit_layout != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate new IR type"));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_global_scope_layout_free(struct kefir_mem *mem,
                                                             struct kefir_ast_translator_global_scope_layout *layout) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST global scope layout"));
    if (layout->global_context != NULL) {
        REQUIRE_OK(kefir_ast_identifier_flat_scope_free(mem, &layout->global_context->object_identifiers));
        REQUIRE_OK(kefir_ast_identifier_flat_scope_free(mem, &layout->global_context->function_identifiers));
        layout->global_context = NULL;
    }
    REQUIRE_OK(kefir_list_free(mem, &layout->external_objects));
    REQUIRE_OK(kefir_list_free(mem, &layout->external_thread_local_objects));
    REQUIRE_OK(kefir_list_free(mem, &layout->static_objects));
    REQUIRE_OK(kefir_list_free(mem, &layout->static_thread_local_objects));
    layout->static_layout = NULL;
    layout->static_uninit_layout = NULL;
    layout->static_thread_local_layout = NULL;
    layout->static_thread_local_uninit_layout = NULL;
    return KEFIR_OK;
}

static kefir_result_t translate_scoped_identifier_type(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                       struct kefir_ir_module *module,
                                                       const struct kefir_ast_translator_environment *env,
                                                       const char *identifier,
                                                       const struct kefir_ast_scoped_identifier *scoped_identifier,
                                                       struct kefir_ir_type **type_ptr) {
    ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, scoped_identifier_layout,
                     scoped_identifier->payload.ptr);
    KEFIR_AST_SCOPE_SET_CLEANUP(scoped_identifier, kefir_ast_translator_scoped_identifer_payload_free, NULL);
    scoped_identifier_layout->type = kefir_ir_module_new_type(mem, module, 0, &scoped_identifier_layout->type_id);
    REQUIRE(scoped_identifier_layout->type != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Unable to allocate new IR type"));
    struct kefir_irbuilder_type builder;
    REQUIRE_OK(kefir_irbuilder_type_init(mem, &builder, scoped_identifier_layout->type));

    const struct kefir_ast_type *object_type = NULL;
    REQUIRE_OK(kefir_ast_type_completion(mem, context, &object_type, scoped_identifier->object.type));

    const struct kefir_ast_type *unqualified_object_type = kefir_ast_unqualified_type(object_type);
    if (KEFIR_AST_TYPE_IS_INCOMPLETE(unqualified_object_type) &&
        (unqualified_object_type->tag == KEFIR_AST_TYPE_STRUCTURE ||
         unqualified_object_type->tag == KEFIR_AST_TYPE_UNION)) {
        REQUIRE((scoped_identifier->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN ||
                 scoped_identifier->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN_THREAD_LOCAL) &&
                    scoped_identifier->object.external,
                KEFIR_SET_ERRORF(KEFIR_ANALYSIS_ERROR, "Global identifier '%s' with incomplete type shall be external",
                                 identifier));
        object_type = context->type_traits->incomplete_type_substitute;
    }

    REQUIRE_OK(kefir_ast_translate_object_type(mem, object_type, scoped_identifier->object.alignment->value, env,
                                               &builder, &scoped_identifier_layout->layout));
    REQUIRE_OK(KEFIR_IRBUILDER_TYPE_FREE(&builder));

    REQUIRE_OK(kefir_ast_translator_evaluate_type_layout(mem, env, scoped_identifier_layout->layout,
                                                         scoped_identifier_layout->type));
    *type_ptr = scoped_identifier_layout->type;
    return KEFIR_OK;
}

static kefir_result_t translate_extern_identifier(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                  struct kefir_ir_module *module,
                                                  const struct kefir_ast_translator_environment *env,
                                                  struct kefir_ast_translator_global_scope_layout *layout,
                                                  const char *identifier,
                                                  const struct kefir_ast_scoped_identifier *scoped_identifier) {
    struct kefir_ir_type *type = NULL;
    REQUIRE_OK(translate_scoped_identifier_type(mem, context, module, env, identifier, scoped_identifier, &type));
    REQUIRE_OK(
        kefir_ast_translator_scoped_identifier_insert(mem, identifier, scoped_identifier, &layout->external_objects));
    return KEFIR_OK;
}

static kefir_result_t translate_extern_thread_local_identifier(
    struct kefir_mem *mem, const struct kefir_ast_context *context, struct kefir_ir_module *module,
    const struct kefir_ast_translator_environment *env, struct kefir_ast_translator_global_scope_layout *layout,
    const char *identifier, const struct kefir_ast_scoped_identifier *scoped_identifier) {
    struct kefir_ir_type *type = NULL;
    REQUIRE_OK(translate_scoped_identifier_type(mem, context, module, env, identifier, scoped_identifier, &type));
    REQUIRE_OK(kefir_ast_translator_scoped_identifier_insert(mem, identifier, scoped_identifier,
                                                             &layout->external_thread_local_objects));
    return KEFIR_OK;
}

static kefir_result_t translate_thread_local_identifier(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                        struct kefir_ir_module *module,
                                                        const struct kefir_ast_translator_environment *env,
                                                        struct kefir_ast_translator_global_scope_layout *layout,
                                                        const char *identifier,
                                                        const struct kefir_ast_scoped_identifier *scoped_identifier) {
    struct kefir_ir_type *type = NULL;
    REQUIRE_OK(translate_scoped_identifier_type(mem, context, module, env, identifier, scoped_identifier, &type));
    REQUIRE_OK(kefir_ast_translator_scoped_identifier_insert(mem, identifier, scoped_identifier,
                                                             &layout->external_thread_local_objects));
    return KEFIR_OK;
}

static kefir_result_t translate_static_identifier(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                  const struct kefir_ast_translator_environment *env,
                                                  struct kefir_ast_translator_global_scope_layout *layout,
                                                  const char *identifier,
                                                  const struct kefir_ast_scoped_identifier *scoped_identifier) {
    ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, scoped_identifier_layout,
                     scoped_identifier->payload.ptr);
    KEFIR_AST_SCOPE_SET_CLEANUP(scoped_identifier, kefir_ast_translator_scoped_identifer_payload_free, NULL);

    if (scoped_identifier->object.initializer != NULL) {
        scoped_identifier_layout->type_id = layout->static_layout_id;
        scoped_identifier_layout->type = layout->static_layout;
    } else {
        scoped_identifier_layout->type_id = layout->static_uninit_layout_id;
        scoped_identifier_layout->type = layout->static_uninit_layout;
    }

    struct kefir_irbuilder_type builder;
    REQUIRE_OK(kefir_irbuilder_type_init(mem, &builder, scoped_identifier_layout->type));

    const struct kefir_ast_type *object_type = NULL;
    REQUIRE_OK(kefir_ast_type_completion(mem, context, &object_type, scoped_identifier->object.type));

    REQUIRE_OK(kefir_ast_translate_object_type(mem, object_type, scoped_identifier->object.alignment->value, env,
                                               &builder, &scoped_identifier_layout->layout));
    REQUIRE_OK(KEFIR_IRBUILDER_TYPE_FREE(&builder));

    REQUIRE_OK(kefir_ast_translator_evaluate_type_layout(mem, env, scoped_identifier_layout->layout,
                                                         scoped_identifier_layout->type));
    REQUIRE_OK(
        kefir_ast_translator_scoped_identifier_insert(mem, identifier, scoped_identifier, &layout->static_objects));
    return KEFIR_OK;
}

static kefir_result_t translate_static_thread_local_identifier(
    struct kefir_mem *mem, const struct kefir_ast_context *context, const struct kefir_ast_translator_environment *env,
    struct kefir_ast_translator_global_scope_layout *layout, const char *identifier,
    const struct kefir_ast_scoped_identifier *scoped_identifier) {
    ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, scoped_identifier_layout,
                     scoped_identifier->payload.ptr);
    KEFIR_AST_SCOPE_SET_CLEANUP(scoped_identifier, kefir_ast_translator_scoped_identifer_payload_free, NULL);

    if (scoped_identifier->object.initializer != NULL) {
        scoped_identifier_layout->type_id = layout->static_thread_local_layout_id;
        scoped_identifier_layout->type = layout->static_thread_local_layout;
    } else {
        scoped_identifier_layout->type_id = layout->static_thread_local_uninit_layout_id;
        scoped_identifier_layout->type = layout->static_thread_local_uninit_layout;
    }

    struct kefir_irbuilder_type builder;
    REQUIRE_OK(kefir_irbuilder_type_init(mem, &builder, scoped_identifier_layout->type));

    const struct kefir_ast_type *object_type = NULL;
    REQUIRE_OK(kefir_ast_type_completion(mem, context, &object_type, scoped_identifier->object.type));

    REQUIRE_OK(kefir_ast_translate_object_type(mem, object_type, scoped_identifier->object.alignment->value, env,
                                               &builder, &scoped_identifier_layout->layout));
    REQUIRE_OK(KEFIR_IRBUILDER_TYPE_FREE(&builder));

    REQUIRE_OK(kefir_ast_translator_evaluate_type_layout(mem, env, scoped_identifier_layout->layout,
                                                         scoped_identifier_layout->type));
    REQUIRE_OK(kefir_ast_translator_scoped_identifier_insert(mem, identifier, scoped_identifier,
                                                             &layout->static_thread_local_objects));
    return KEFIR_OK;
}

static kefir_result_t translate_global_scoped_identifier_object(
    struct kefir_mem *mem, const struct kefir_ast_context *context, struct kefir_ir_module *module,
    const char *identifier, const struct kefir_ast_scoped_identifier *scoped_identifier,
    struct kefir_ast_translator_global_scope_layout *layout, const struct kefir_ast_translator_environment *env) {
    REQUIRE(scoped_identifier->klass == KEFIR_AST_SCOPE_IDENTIFIER_OBJECT, KEFIR_OK);

    if (scoped_identifier->object.asm_label != NULL) {
        identifier = scoped_identifier->object.asm_label;
    }
    switch (scoped_identifier->object.storage) {
        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN:
            REQUIRE_OK(translate_extern_identifier(mem, context, module, env, layout, identifier, scoped_identifier));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN_THREAD_LOCAL:
            REQUIRE_OK(translate_extern_thread_local_identifier(mem, context, module, env, layout, identifier,
                                                                scoped_identifier));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_THREAD_LOCAL:
            REQUIRE_OK(
                translate_thread_local_identifier(mem, context, module, env, layout, identifier, scoped_identifier));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC:
            REQUIRE_OK(translate_static_identifier(mem, context, env, layout, identifier, scoped_identifier));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC_THREAD_LOCAL:
            REQUIRE_OK(
                translate_static_thread_local_identifier(mem, context, env, layout, identifier, scoped_identifier));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_AUTO:
        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_REGISTER:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "File-scope variable cannot have auto/register storage");

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_TYPEDEF:
        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected storage class of file-scope variable");
    }
    return KEFIR_OK;
}

static kefir_result_t translate_global_scoped_identifier_function(
    struct kefir_mem *mem, struct kefir_ir_module *module, struct kefir_ast_type_bundle *type_bundle,
    const struct kefir_ast_type_traits *type_traits, const char *identifier,
    const struct kefir_ast_scoped_identifier *scoped_identifier,
    struct kefir_ast_translator_global_scope_layout *layout, const struct kefir_ast_translator_environment *env) {
    ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_function *, scoped_identifier_func,
                     scoped_identifier->payload.ptr);
    KEFIR_AST_SCOPE_SET_CLEANUP(scoped_identifier, kefir_ast_translator_scoped_identifer_payload_free, NULL);
    REQUIRE_OK(kefir_ast_translator_function_declaration_init(mem, env, type_bundle, type_traits, module, identifier,
                                                              scoped_identifier->function.type, NULL,
                                                              &scoped_identifier_func->declaration));

    switch (scoped_identifier->function.storage) {
        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN:
            REQUIRE_OK(kefir_ast_translator_scoped_identifier_insert(mem, identifier, scoped_identifier,
                                                                     &layout->external_objects));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC:
            REQUIRE_OK(kefir_ast_translator_scoped_identifier_insert(mem, identifier, scoped_identifier,
                                                                     &layout->static_objects));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected function storage specifier");
    }
    return KEFIR_OK;
}

static kefir_result_t translate_global_scoped_identifier(
    struct kefir_mem *mem, const struct kefir_ast_context *context, struct kefir_ir_module *module,
    struct kefir_ast_type_bundle *type_bundle, const struct kefir_ast_type_traits *type_traits, const char *identifier,
    const struct kefir_ast_scoped_identifier *scoped_identifier,
    struct kefir_ast_translator_global_scope_layout *layout, const struct kefir_ast_translator_environment *env) {
    switch (scoped_identifier->klass) {
        case KEFIR_AST_SCOPE_IDENTIFIER_OBJECT:
            REQUIRE_OK(translate_global_scoped_identifier_object(mem, context, module, identifier, scoped_identifier,
                                                                 layout, env));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION:
            REQUIRE_OK(translate_global_scoped_identifier_function(mem, module, type_bundle, type_traits, identifier,
                                                                   scoped_identifier, layout, env));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_ENUM_CONSTANT:
        case KEFIR_AST_SCOPE_IDENTIFIER_TYPE_TAG:
        case KEFIR_AST_SCOPE_IDENTIFIER_TYPE_DEFINITION:
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_LABEL:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "No labels are allowed in the global scope");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_build_global_scope_layout(struct kefir_mem *mem, struct kefir_ir_module *module,
                                                              struct kefir_ast_global_context *context,
                                                              const struct kefir_ast_translator_environment *env,
                                                              struct kefir_ast_translator_global_scope_layout *layout) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST global context"));
    REQUIRE(env != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator environment"));
    REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST global scope layout"));
    REQUIRE(layout->global_context == NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected empty AST translator global scope"));

    layout->global_context = context;

    struct kefir_ast_identifier_flat_scope_iterator iter;
    kefir_result_t res;
    for (res = kefir_ast_identifier_flat_scope_iter(&context->object_identifiers, &iter); res == KEFIR_OK;
         res = kefir_ast_identifier_flat_scope_next(&context->object_identifiers, &iter)) {
        REQUIRE_OK(translate_global_scoped_identifier(mem, &context->context, module, &context->type_bundle,
                                                      context->type_traits, iter.identifier, iter.value, layout, env));
    }
    REQUIRE(res == KEFIR_ITERATOR_END, res);

    for (res = kefir_ast_identifier_flat_scope_iter(&context->function_identifiers, &iter); res == KEFIR_OK;
         res = kefir_ast_identifier_flat_scope_next(&context->function_identifiers, &iter)) {
        REQUIRE_OK(translate_global_scoped_identifier_function(mem, module, &context->type_bundle, context->type_traits,
                                                               iter.identifier, iter.value, layout, env));
    }
    REQUIRE(res == KEFIR_ITERATOR_END, res);
    return KEFIR_OK;
}
