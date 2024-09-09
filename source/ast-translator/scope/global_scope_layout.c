/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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
    layout->next_object_identifier = 0;
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
    return KEFIR_OK;
}

static kefir_result_t obtain_complete_object_type(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                  const char *identifier,
                                                  const struct kefir_ast_scoped_identifier *scoped_identifier,
                                                  const struct kefir_ast_type **object_type_ptr) {
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
    } else if (unqualified_object_type->tag == KEFIR_AST_TYPE_ARRAY &&
               unqualified_object_type->array_type.boundary == KEFIR_AST_ARRAY_UNBOUNDED) {
        struct kefir_ast_constant_expression *array_length = kefir_ast_constant_expression_integer(mem, 1);
        REQUIRE(array_length != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate constant expression"));
        object_type = kefir_ast_type_array(mem, context->type_bundle, unqualified_object_type->array_type.element_type,
                                           array_length, &unqualified_object_type->array_type.qualifications);
        REQUIRE(array_length != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate bounded array type"));
    }

    *object_type_ptr = object_type;
    return KEFIR_OK;
}

static kefir_result_t translate_scoped_identifier_type(
    struct kefir_mem *mem, const struct kefir_ast_context *context, struct kefir_ir_module *module,
    const struct kefir_ast_translator_environment *env, struct kefir_ast_translator_global_scope_layout *layout,
    const char *identifier, const struct kefir_ast_scoped_identifier *scoped_identifier,
    struct kefir_ir_type **type_ptr, const struct kefir_source_location *source_location) {
    ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, scoped_identifier_layout,
                     scoped_identifier->payload.ptr);
    KEFIR_AST_SCOPE_SET_CLEANUP(scoped_identifier, kefir_ast_translator_scoped_identifer_payload_free, NULL);
    scoped_identifier_layout->identifier = layout->next_object_identifier++;
    scoped_identifier_layout->type = kefir_ir_module_new_type(mem, module, 0, &scoped_identifier_layout->type_id);
    REQUIRE(scoped_identifier_layout->type != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Unable to allocate new IR type"));
    struct kefir_irbuilder_type builder;
    REQUIRE_OK(kefir_irbuilder_type_init(mem, &builder, scoped_identifier_layout->type));

    const struct kefir_ast_type *object_type = NULL;
    REQUIRE_OK(obtain_complete_object_type(mem, context, identifier, scoped_identifier, &object_type));

    REQUIRE_OK(kefir_ast_translate_object_type(mem, context, object_type, scoped_identifier->object.alignment->value,
                                               env, &builder, &scoped_identifier_layout->layout, source_location));
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
                                                  const struct kefir_ast_scoped_identifier *scoped_identifier,
                                                  const struct kefir_source_location *source_location) {
    struct kefir_ir_type *type = NULL;
    REQUIRE_OK(translate_scoped_identifier_type(mem, context, module, env, layout, identifier, scoped_identifier, &type,
                                                source_location));
    REQUIRE_OK(
        kefir_ast_translator_scoped_identifier_insert(mem, identifier, scoped_identifier, &layout->external_objects));
    return KEFIR_OK;
}

static kefir_result_t translate_extern_thread_local_identifier(
    struct kefir_mem *mem, const struct kefir_ast_context *context, struct kefir_ir_module *module,
    const struct kefir_ast_translator_environment *env, struct kefir_ast_translator_global_scope_layout *layout,
    const char *identifier, const struct kefir_ast_scoped_identifier *scoped_identifier,
    const struct kefir_source_location *source_location) {
    struct kefir_ir_type *type = NULL;
    REQUIRE_OK(translate_scoped_identifier_type(mem, context, module, env, layout, identifier, scoped_identifier, &type,
                                                source_location));
    REQUIRE_OK(kefir_ast_translator_scoped_identifier_insert(mem, identifier, scoped_identifier,
                                                             &layout->external_thread_local_objects));
    return KEFIR_OK;
}

static kefir_result_t translate_thread_local_identifier(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                        struct kefir_ir_module *module,
                                                        const struct kefir_ast_translator_environment *env,
                                                        struct kefir_ast_translator_global_scope_layout *layout,
                                                        const char *identifier,
                                                        const struct kefir_ast_scoped_identifier *scoped_identifier,
                                                        const struct kefir_source_location *source_location) {
    struct kefir_ir_type *type = NULL;
    REQUIRE_OK(translate_scoped_identifier_type(mem, context, module, env, layout, identifier, scoped_identifier, &type,
                                                source_location));
    REQUIRE_OK(kefir_ast_translator_scoped_identifier_insert(mem, identifier, scoped_identifier,
                                                             &layout->external_thread_local_objects));
    return KEFIR_OK;
}

static kefir_result_t translate_static_identifier(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                  struct kefir_ir_module *module,
                                                  const struct kefir_ast_translator_environment *env,
                                                  struct kefir_ast_translator_global_scope_layout *layout,
                                                  const char *identifier,
                                                  const struct kefir_ast_scoped_identifier *scoped_identifier,
                                                  const struct kefir_source_location *source_location) {
    struct kefir_ir_type *type = NULL;
    REQUIRE_OK(translate_scoped_identifier_type(mem, context, module, env, layout, identifier, scoped_identifier, &type,
                                                source_location));
    REQUIRE_OK(
        kefir_ast_translator_scoped_identifier_insert(mem, identifier, scoped_identifier, &layout->static_objects));
    return KEFIR_OK;
}

static kefir_result_t translate_static_thread_local_identifier(
    struct kefir_mem *mem, const struct kefir_ast_context *context, struct kefir_ir_module *module,
    const struct kefir_ast_translator_environment *env, struct kefir_ast_translator_global_scope_layout *layout,
    const char *identifier, const struct kefir_ast_scoped_identifier *scoped_identifier,
    const struct kefir_source_location *source_location) {
    struct kefir_ir_type *type = NULL;
    REQUIRE_OK(translate_scoped_identifier_type(mem, context, module, env, layout, identifier, scoped_identifier, &type,
                                                source_location));
    REQUIRE_OK(kefir_ast_translator_scoped_identifier_insert(mem, identifier, scoped_identifier,
                                                             &layout->static_thread_local_objects));
    return KEFIR_OK;
}

static kefir_result_t translate_global_scoped_identifier_object(
    struct kefir_mem *mem, const struct kefir_ast_context *context, struct kefir_ir_module *module,
    const char *identifier, const struct kefir_ast_scoped_identifier *scoped_identifier,
    struct kefir_ast_translator_global_scope_layout *layout, const struct kefir_ast_translator_environment *env,
    const struct kefir_source_location *source_location) {
    REQUIRE(scoped_identifier->klass == KEFIR_AST_SCOPE_IDENTIFIER_OBJECT, KEFIR_OK);

    switch (scoped_identifier->object.storage) {
        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN:
            REQUIRE_OK(translate_extern_identifier(mem, context, module, env, layout, identifier, scoped_identifier,
                                                   source_location));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN_THREAD_LOCAL:
            REQUIRE_OK(translate_extern_thread_local_identifier(mem, context, module, env, layout, identifier,
                                                                scoped_identifier, source_location));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_THREAD_LOCAL:
            REQUIRE_OK(translate_thread_local_identifier(mem, context, module, env, layout, identifier,
                                                         scoped_identifier, source_location));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC:
            REQUIRE_OK(translate_static_identifier(mem, context, module, env, layout, identifier, scoped_identifier,
                                                   source_location));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC_THREAD_LOCAL:
            REQUIRE_OK(translate_static_thread_local_identifier(mem, context, module, env, layout, identifier,
                                                                scoped_identifier, source_location));
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

static kefir_result_t generate_object_debug_entry(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                  const struct kefir_ast_translator_environment *env,
                                                  struct kefir_ir_module *module, const char *identifier,
                                                  const struct kefir_ast_type *type,
                                                  struct kefir_ast_translator_debug_entries *debug_entries,
                                                  const struct kefir_ast_scoped_identifier *scoped_identifier) {
    REQUIRE(debug_entries != NULL, KEFIR_OK);

    ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, scoped_identifier_layout,
                     scoped_identifier->payload.ptr);
    kefir_ir_debug_entry_id_t type_entry_id;
    REQUIRE_OK(kefir_ast_translate_debug_type(mem, context, env, module, debug_entries, type, &type_entry_id));

    REQUIRE_OK(kefir_ir_debug_entry_new(mem, &module->debug_info.entries, KEFIR_IR_DEBUG_ENTRY_VARIABLE,
                                        &scoped_identifier_layout->debug_info.variable));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols,
                                                  scoped_identifier_layout->debug_info.variable,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_NAME(identifier)));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols,
                                                  scoped_identifier_layout->debug_info.variable,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_TYPE(type_entry_id)));

    kefir_id_t identifier_id;
    REQUIRE(kefir_ir_module_symbol(mem, module, identifier, &identifier_id) != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert symbol into IR module"));
    switch (scoped_identifier->object.storage) {
        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN:
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols,
                                                          scoped_identifier_layout->debug_info.variable,
                                                          &KEFIR_IR_DEBUG_ENTRY_ATTR_GLOBAL_VARIABLE(identifier_id)));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols,
                                                          scoped_identifier_layout->debug_info.variable,
                                                          &KEFIR_IR_DEBUG_ENTRY_ATTR_EXTERNAL(true)));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN_THREAD_LOCAL:
        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_THREAD_LOCAL:
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(
                mem, &module->debug_info.entries, &module->symbols, scoped_identifier_layout->debug_info.variable,
                &KEFIR_IR_DEBUG_ENTRY_ATTR_THREAD_LOCAL_VARIABLE(identifier_id)));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols,
                                                          scoped_identifier_layout->debug_info.variable,
                                                          &KEFIR_IR_DEBUG_ENTRY_ATTR_EXTERNAL(true)));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC:
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols,
                                                          scoped_identifier_layout->debug_info.variable,
                                                          &KEFIR_IR_DEBUG_ENTRY_ATTR_GLOBAL_VARIABLE(identifier_id)));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols,
                                                          scoped_identifier_layout->debug_info.variable,
                                                          &KEFIR_IR_DEBUG_ENTRY_ATTR_EXTERNAL(false)));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC_THREAD_LOCAL:
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(
                mem, &module->debug_info.entries, &module->symbols, scoped_identifier_layout->debug_info.variable,
                &KEFIR_IR_DEBUG_ENTRY_ATTR_THREAD_LOCAL_VARIABLE(identifier_id)));
            REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &module->debug_info.entries, &module->symbols,
                                                          scoped_identifier_layout->debug_info.variable,
                                                          &KEFIR_IR_DEBUG_ENTRY_ATTR_EXTERNAL(false)));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_TYPEDEF:
        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_AUTO:
        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_REGISTER:
        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected AST scoped identifier storage class");
    }
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(
        mem, &module->debug_info.entries, &module->symbols, scoped_identifier_layout->debug_info.variable,
        &KEFIR_IR_DEBUG_ENTRY_ATTR_DECLARATION(scoped_identifier->object.external)));

    if (scoped_identifier->source_location.source != NULL) {
        REQUIRE_OK(kefir_ir_debug_entry_add_attribute(
            mem, &module->debug_info.entries, &module->symbols, scoped_identifier_layout->debug_info.variable,
            &KEFIR_IR_DEBUG_ENTRY_ATTR_SOURCE_LOCATION(scoped_identifier->source_location.source)));
        REQUIRE_OK(kefir_ir_debug_entry_add_attribute(
            mem, &module->debug_info.entries, &module->symbols, scoped_identifier_layout->debug_info.variable,
            &KEFIR_IR_DEBUG_ENTRY_ATTR_SOURCE_LOCATION_LINE(scoped_identifier->source_location.line)));
        REQUIRE_OK(kefir_ir_debug_entry_add_attribute(
            mem, &module->debug_info.entries, &module->symbols, scoped_identifier_layout->debug_info.variable,
            &KEFIR_IR_DEBUG_ENTRY_ATTR_SOURCE_LOCATION_COLUMN(scoped_identifier->source_location.column)));
    }

    scoped_identifier_layout->debug_info.present = true;
    return KEFIR_OK;
}

static kefir_result_t translate_global_scoped_identifier_function(
    struct kefir_mem *mem, const struct kefir_ast_context *context, struct kefir_ir_module *module,
    struct kefir_ast_type_bundle *type_bundle, const struct kefir_ast_type_traits *type_traits, const char *identifier,
    const struct kefir_ast_scoped_identifier *scoped_identifier,
    struct kefir_ast_translator_global_scope_layout *layout, const struct kefir_ast_translator_environment *env) {
    ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_function *, scoped_identifier_func,
                     scoped_identifier->payload.ptr);
    KEFIR_AST_SCOPE_SET_CLEANUP(scoped_identifier, kefir_ast_translator_scoped_identifer_payload_free, NULL);

    const struct kefir_ast_type *function_type = NULL;
    REQUIRE_OK(kefir_ast_type_completion(mem, context, &function_type, scoped_identifier->function.type));
    REQUIRE_OK(kefir_ast_translator_function_declaration_init(
        mem, context, env, type_bundle, type_traits, module, identifier, function_type, NULL,
        &scoped_identifier_func->declaration, &scoped_identifier->source_location));

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
    struct kefir_ast_translator_global_scope_layout *layout, const struct kefir_ast_translator_environment *env,
    struct kefir_ast_translator_debug_entries *debug_entries) {
    switch (scoped_identifier->klass) {
        case KEFIR_AST_SCOPE_IDENTIFIER_OBJECT:
            REQUIRE_OK(translate_global_scoped_identifier_object(mem, context, module, identifier, scoped_identifier,
                                                                 layout, env, &scoped_identifier->source_location));
            REQUIRE_OK(generate_object_debug_entry(mem, context, env, module, identifier,
                                                   scoped_identifier->object.type, debug_entries, scoped_identifier));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION:
            REQUIRE_OK(translate_global_scoped_identifier_function(mem, context, module, type_bundle, type_traits,
                                                                   identifier, scoped_identifier, layout, env));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_TYPE_TAG:
            // REQUIRE_OK(generate_debug_entry(mem, context, env, module, scoped_identifier->type, debug_entries,
            //                                 scoped_identifier));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_ENUM_CONSTANT:
            // REQUIRE_OK(generate_debug_entry(mem, context, env, module, scoped_identifier->enum_constant.type,
            //                                 debug_entries, scoped_identifier));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_TYPE_DEFINITION:
            // REQUIRE_OK(generate_debug_entry(mem, context, env, module, scoped_identifier->type_definition.type,
            //                                 debug_entries, scoped_identifier));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_LABEL:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "No labels are allowed in the global scope");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_build_global_scope_layout(struct kefir_mem *mem, struct kefir_ir_module *module,
                                                              struct kefir_ast_global_context *context,
                                                              const struct kefir_ast_translator_environment *env,
                                                              struct kefir_ast_translator_debug_entries *debug_entries,
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
                                                      context->type_traits, iter.identifier, iter.value, layout, env,
                                                      debug_entries));
    }
    REQUIRE(res == KEFIR_ITERATOR_END, res);

    for (res = kefir_ast_identifier_flat_scope_iter(&context->function_identifiers, &iter); res == KEFIR_OK;
         res = kefir_ast_identifier_flat_scope_next(&context->function_identifiers, &iter)) {
        REQUIRE_OK(translate_global_scoped_identifier_function(mem, &context->context, module, &context->type_bundle,
                                                               context->type_traits, iter.identifier, iter.value,
                                                               layout, env));
    }
    REQUIRE(res == KEFIR_ITERATOR_END, res);
    return KEFIR_OK;
}
