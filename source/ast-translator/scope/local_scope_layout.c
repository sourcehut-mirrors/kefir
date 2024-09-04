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

#include "kefir/ast-translator/scope/local_scope_layout.h"
#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/layout.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/ast-translator/scope/scope_layout_impl.h"
#include "kefir/ast-translator/flow_control.h"
#include "kefir/ast/type_completion.h"
#include "kefir/core/source_error.h"

kefir_result_t kefir_ast_translator_local_scope_layout_init(struct kefir_mem *mem, struct kefir_ir_module *module,
                                                            struct kefir_ast_translator_global_scope_layout *global,
                                                            struct kefir_ast_translator_local_scope_layout *layout) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(global != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST global scope layout"));
    REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST local scope layout"));
    layout->local_context = NULL;
    REQUIRE_OK(kefir_list_init(&layout->local_objects));
    REQUIRE_OK(kefir_list_init(&layout->static_objects));
    REQUIRE_OK(kefir_list_init(&layout->static_thread_local_objects));
    REQUIRE_OK(kefir_list_on_remove(&layout->local_objects, kefir_ast_translator_scoped_identifier_remove, NULL));
    REQUIRE_OK(kefir_list_on_remove(&layout->static_objects, kefir_ast_translator_scoped_identifier_remove, NULL));
    REQUIRE_OK(kefir_list_on_remove(&layout->static_thread_local_objects, kefir_ast_translator_scoped_identifier_remove,
                                    NULL));
    layout->global = global;
    layout->local_layout = kefir_ir_module_new_type(mem, module, 0, &layout->local_layout_id);
    layout->local_type_layout = NULL;
    REQUIRE(layout->local_layout != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate new IR type"));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_local_scope_layout_free(struct kefir_mem *mem,
                                                            struct kefir_ast_translator_local_scope_layout *layout) {
    REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST local scope layout"));
    if (layout->local_context != NULL) {

        for (const struct kefir_list_entry *iter = kefir_list_head(&layout->local_context->identifiers); iter != NULL;
             kefir_list_next(&iter)) {

            ASSIGN_DECL_CAST(struct kefir_ast_scoped_identifier *, scoped_id, iter->value);
            REQUIRE_OK(kefir_ast_scoped_identifier_run_cleanup(mem, scoped_id));
        }
        layout->local_context = NULL;
    }
    if (layout->local_type_layout != NULL) {
        REQUIRE_OK(kefir_ast_type_layout_free(mem, layout->local_type_layout));
        layout->local_type_layout = NULL;
    }
    REQUIRE_OK(kefir_list_free(mem, &layout->local_objects));
    REQUIRE_OK(kefir_list_free(mem, &layout->static_objects));
    REQUIRE_OK(kefir_list_free(mem, &layout->static_thread_local_objects));
    layout->global = NULL;
    layout->local_layout = NULL;
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
    const struct kefir_ast_translator_environment *env, struct kefir_ast_translator_local_scope_layout *local_layout,
    const char *identifier, const struct kefir_ast_scoped_identifier *scoped_identifier,
    struct kefir_ir_type **type_ptr, const struct kefir_source_location *source_location) {
    ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, scoped_identifier_layout,
                     scoped_identifier->payload.ptr);
    KEFIR_AST_SCOPE_SET_CLEANUP(scoped_identifier, kefir_ast_translator_scoped_identifer_payload_free, NULL);
    scoped_identifier_layout->identifier = local_layout->global->next_object_identifier++;
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

static kefir_result_t translate_static_identifier(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                  struct kefir_ir_module *module,
                                                  const struct kefir_ast_translator_environment *env,
                                                  struct kefir_ast_translator_local_scope_layout *local_layout,
                                                  const char *identifier,
                                                  const struct kefir_ast_scoped_identifier *scoped_identifier,
                                                  const struct kefir_source_location *source_location) {
    struct kefir_ir_type *type;
    REQUIRE_OK(translate_scoped_identifier_type(mem, context, module, env, local_layout, identifier, scoped_identifier,
                                                &type, source_location));
    REQUIRE_OK(kefir_ast_translator_scoped_identifier_insert(mem, identifier, scoped_identifier,
                                                             &local_layout->static_objects));
    return KEFIR_OK;
}

static kefir_result_t translate_static_thread_local_identifier(
    struct kefir_mem *mem, const struct kefir_ast_context *context, struct kefir_ir_module *module,
    const struct kefir_ast_translator_environment *env, struct kefir_ast_translator_local_scope_layout *local_layout,
    const char *identifier, const struct kefir_ast_scoped_identifier *scoped_identifier,
    const struct kefir_source_location *source_location) {
    struct kefir_ir_type *type;
    REQUIRE_OK(translate_scoped_identifier_type(mem, context, module, env, local_layout, identifier, scoped_identifier,
                                                &type, source_location));
    REQUIRE_OK(kefir_ast_translator_scoped_identifier_insert(mem, identifier, scoped_identifier,
                                                             &local_layout->static_thread_local_objects));
    return KEFIR_OK;
}

static kefir_result_t translate_auto_register_identifier(
    struct kefir_mem *mem, const struct kefir_ast_context *context, struct kefir_irbuilder_type *builder,
    const struct kefir_ast_translator_environment *env, struct kefir_ast_translator_local_scope_layout *local_layout,
    const char *identifier, const struct kefir_ast_scoped_identifier *scoped_identifier,
    struct kefir_ast_type_layout *scope_type_layout, const struct kefir_source_location *source_location) {
    ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, scoped_identifier_layout,
                     scoped_identifier->payload.ptr);
    REQUIRE(scoped_identifier_layout->layout == NULL, KEFIR_OK);

    KEFIR_AST_SCOPE_SET_CLEANUP(scoped_identifier, kefir_ast_translator_scoped_identifer_payload_free, NULL);
    scoped_identifier_layout->identifier = local_layout->global->next_object_identifier++;

    const struct kefir_ast_type *object_type = NULL;
    REQUIRE_OK(kefir_ast_type_completion(mem, context, &object_type, scoped_identifier->object.type));

    REQUIRE_OK(kefir_ast_translate_object_type(mem, context, object_type, scoped_identifier->object.alignment->value,
                                               env, builder, &scoped_identifier_layout->layout, source_location));
    scoped_identifier_layout->type_id = local_layout->local_layout_id;
    scoped_identifier_layout->type = builder->type;
    REQUIRE_OK(kefir_list_insert_after(mem, &scope_type_layout->custom_layout.sublayouts,
                                       kefir_list_tail(&scope_type_layout->custom_layout.sublayouts),
                                       scoped_identifier_layout->layout));
    scoped_identifier_layout->layout->parent = scope_type_layout;

    REQUIRE_OK(kefir_ast_translator_evaluate_type_layout(mem, env, scoped_identifier_layout->layout,
                                                         scoped_identifier_layout->type));
    REQUIRE_OK(kefir_ast_translator_scoped_identifier_insert(mem, identifier, scoped_identifier,
                                                             &local_layout->local_objects));
    return KEFIR_OK;
}

static kefir_result_t translate_local_scoped_identifier_object(
    struct kefir_mem *mem, const struct kefir_ast_context *context, struct kefir_ir_module *module,
    struct kefir_irbuilder_type *builder, const char *identifier,
    const struct kefir_ast_scoped_identifier *scoped_identifier, const struct kefir_ast_translator_environment *env,
    struct kefir_ast_translator_local_scope_layout *local_layout, struct kefir_ast_type_layout *scope_type_layout,
    struct kefir_ir_typeentry *wrapper_structure, const struct kefir_source_location *source_location) {
    REQUIRE(scoped_identifier->klass == KEFIR_AST_SCOPE_IDENTIFIER_OBJECT, KEFIR_OK);
    switch (scoped_identifier->object.storage) {
        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN:
        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN_THREAD_LOCAL:
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC:
            REQUIRE_OK(translate_static_identifier(mem, context, module, env, local_layout, identifier,
                                                   scoped_identifier, source_location));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_THREAD_LOCAL:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST,
                                   "Cannot have thread local block-scope variable with no linkage");

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC_THREAD_LOCAL:
            REQUIRE_OK(translate_static_thread_local_identifier(mem, context, module, env, local_layout, identifier,
                                                                scoped_identifier, source_location));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_AUTO:
        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_REGISTER:
            if (wrapper_structure != NULL) {
                wrapper_structure->param++;
            }
            REQUIRE_OK(translate_auto_register_identifier(mem, context, builder, env, local_layout, identifier,
                                                          scoped_identifier, scope_type_layout, source_location));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_TYPEDEF:
        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected storage class of local-scope variable");
    }
    return KEFIR_OK;
}

static kefir_result_t translate_local_scoped_identifier_function(
    struct kefir_mem *mem, const struct kefir_ast_context *context, const char *identifier,
    const struct kefir_ast_scoped_identifier *scoped_identifier, const struct kefir_ast_translator_environment *env,
    struct kefir_ast_type_bundle *type_bundle, const struct kefir_ast_type_traits *type_traits,
    struct kefir_ir_module *module, const struct kefir_source_location *source_location) {
    ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_function *, scoped_identifier_func,
                     scoped_identifier->payload.ptr);
    if (scoped_identifier_func->declaration == NULL) {
        KEFIR_AST_SCOPE_SET_CLEANUP(scoped_identifier, kefir_ast_translator_scoped_identifer_payload_free, NULL);
        REQUIRE_OK(kefir_ast_translator_function_declaration_init(
            mem, context, env, type_bundle, type_traits, module, identifier, scoped_identifier->function.type, NULL,
            &scoped_identifier_func->declaration, source_location));
    }
    return KEFIR_OK;
}

static kefir_result_t generate_debug_entry(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                           const struct kefir_ast_translator_environment *env,
                                           struct kefir_ir_module *module, const struct kefir_ast_type *type,
                                           struct kefir_ast_translator_debug_entries *debug_entries,
                                           const struct kefir_ast_scoped_identifier *scoped_identifier) {
    REQUIRE(debug_entries != NULL, KEFIR_OK);

    ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, scoped_identifier_layout,
                     scoped_identifier->payload.ptr);
    REQUIRE_OK(kefir_ast_translate_debug_type(mem, context, env, module, debug_entries, type,
                                              &scoped_identifier_layout->debug_info.type));
    scoped_identifier_layout->debug_info.present = true;
    return KEFIR_OK;
}

static kefir_result_t translate_local_scoped_identifier(
    struct kefir_mem *mem, const struct kefir_ast_context *context, struct kefir_irbuilder_type *builder,
    const char *identifier, const struct kefir_ast_scoped_identifier *scoped_identifier,
    const struct kefir_ast_translator_environment *env, struct kefir_ast_type_bundle *type_bundle,
    const struct kefir_ast_type_traits *type_traits, struct kefir_ir_module *module,
    struct kefir_ast_translator_local_scope_layout *local_layout, struct kefir_ast_type_layout *scope_type_layout,
    struct kefir_ir_typeentry *wrapper_structure, struct kefir_ast_translator_debug_entries *debug_entries) {
    switch (scoped_identifier->klass) {
        case KEFIR_AST_SCOPE_IDENTIFIER_OBJECT:
            REQUIRE_OK(translate_local_scoped_identifier_object(
                mem, context, module, builder, identifier, scoped_identifier, env, local_layout, scope_type_layout,
                wrapper_structure, &scoped_identifier->source_location));
            REQUIRE_OK(generate_debug_entry(mem, context, env, module, scoped_identifier->object.type, debug_entries,
                                            scoped_identifier));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION:
            REQUIRE_OK(translate_local_scoped_identifier_function(mem, context, identifier, scoped_identifier, env,
                                                                  type_bundle, type_traits, module,
                                                                  &scoped_identifier->source_location));
            REQUIRE_OK(generate_debug_entry(mem, context, env, module, scoped_identifier->object.type, debug_entries,
                                            scoped_identifier));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_ENUM_CONSTANT:
        case KEFIR_AST_SCOPE_IDENTIFIER_TYPE_TAG:
        case KEFIR_AST_SCOPE_IDENTIFIER_TYPE_DEFINITION:
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_LABEL:
            REQUIRE(scoped_identifier->label.point != NULL && scoped_identifier->label.point->parent != NULL,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Cannot translate undefined label"));
            REQUIRE_OK(kefir_ast_translator_flow_control_point_init(mem, scoped_identifier->label.point, NULL));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t local_scope_empty(struct kefir_mem *mem, const struct kefir_tree_node *root,
                                        kefir_bool_t *empty) {
    *empty = true;
    ASSIGN_DECL_CAST(struct kefir_ast_identifier_flat_scope *, scope, root->value);
    struct kefir_ast_identifier_flat_scope_iterator iter;
    kefir_result_t res;
    for (res = kefir_ast_identifier_flat_scope_iter(scope, &iter); res == KEFIR_OK;
         res = kefir_ast_identifier_flat_scope_next(scope, &iter)) {
        if (iter.value->klass == KEFIR_AST_SCOPE_IDENTIFIER_OBJECT) {
            if (iter.value->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_AUTO ||
                iter.value->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_REGISTER) {
                *empty = false;
                return KEFIR_OK;
            }
        } else if (iter.value->klass == KEFIR_AST_SCOPE_IDENTIFIER_LABEL) {
            *empty = false;
            return KEFIR_OK;
        }
    }
    REQUIRE(res == KEFIR_ITERATOR_END, res);
    if (kefir_tree_first_child(root) != NULL) {
        for (struct kefir_tree_node *child = kefir_tree_first_child(root); child != NULL;
             child = kefir_tree_next_sibling(child)) {
            REQUIRE_OK(local_scope_empty(mem, child, empty));
            REQUIRE(*empty, KEFIR_OK);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t traverse_local_scope_flat(
    struct kefir_mem *mem, const struct kefir_ast_context *context, const struct kefir_tree_node *root,
    struct kefir_irbuilder_type *builder, const struct kefir_ast_translator_environment *env,
    struct kefir_ast_type_bundle *type_bundle, const struct kefir_ast_type_traits *type_traits,
    struct kefir_ir_module *module, struct kefir_ast_translator_local_scope_layout *local_layout,
    struct kefir_ast_translator_debug_entries *debug_entries, struct kefir_ast_type_layout **scope_type_layout) {
    ASSIGN_DECL_CAST(struct kefir_ast_identifier_flat_scope *, scope, root->value);
    kefir_bool_t empty_scope = true;
    REQUIRE_OK(local_scope_empty(mem, root, &empty_scope));
    if (!empty_scope && *scope_type_layout == NULL) {
        *scope_type_layout = kefir_ast_new_type_layout(mem, NULL, 0, ~(kefir_uptr_t) 0ull);
    }
    struct kefir_ast_identifier_flat_scope_iterator iter;
    kefir_result_t res;
    for (res = kefir_ast_identifier_flat_scope_iter(scope, &iter); res == KEFIR_OK;
         res = kefir_ast_identifier_flat_scope_next(scope, &iter)) {
        REQUIRE_OK(translate_local_scoped_identifier(mem, context, builder, iter.identifier, iter.value, env,
                                                     type_bundle, type_traits, module, local_layout, *scope_type_layout,
                                                     NULL, debug_entries));
    }
    REQUIRE(res == KEFIR_ITERATOR_END, res);

    kefir_bool_t empty_children = true;
    for (struct kefir_tree_node *child = kefir_tree_first_child(root); child != NULL && empty_children;
         child = kefir_tree_next_sibling(child)) {
        REQUIRE_OK(local_scope_empty(mem, child, &empty_children));
    }
    if (!empty_children) {
        for (struct kefir_tree_node *child = kefir_tree_first_child(root); child != NULL;
             child = kefir_tree_next_sibling(child)) {
            REQUIRE_OK(traverse_local_scope_flat(mem, context, child, builder, env, type_bundle, type_traits, module,
                                                 local_layout, debug_entries, scope_type_layout));
        }
    } else {
        for (struct kefir_tree_node *child = kefir_tree_first_child(root); child != NULL;
             child = kefir_tree_next_sibling(child)) {
            REQUIRE_OK(traverse_local_scope_flat(mem, context, child, builder, env, type_bundle, type_traits, module,
                                                 local_layout, debug_entries, scope_type_layout));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_build_local_scope_layout(struct kefir_mem *mem,
                                                             const struct kefir_ast_local_context *context,
                                                             const struct kefir_ast_translator_environment *env,
                                                             struct kefir_ir_module *module,
                                                             struct kefir_ast_translator_local_scope_layout *layout,
                                                             struct kefir_ast_translator_debug_entries *debug_entries) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST local scope layout"));
    REQUIRE(layout->local_context == NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected empty AST translator local scope layout"));

    layout->local_context = context;

    if (!kefir_ast_identifier_block_scope_empty(&context->ordinary_scope)) {
        struct kefir_irbuilder_type builder;
        REQUIRE_OK(kefir_irbuilder_type_init(mem, &builder, layout->local_layout));
        REQUIRE_OK(traverse_local_scope_flat(mem, &context->context, &context->ordinary_scope.root, &builder, env,
                                             context->context.type_bundle, context->context.type_traits, module, layout,
                                             debug_entries, &layout->local_type_layout));
        REQUIRE_OK(KEFIR_IRBUILDER_TYPE_FREE(&builder));

        if (layout->local_type_layout != NULL) {
            REQUIRE_OK(
                kefir_ast_translator_evaluate_type_layout(mem, env, layout->local_type_layout, layout->local_layout));
        }
    }

    struct kefir_ast_identifier_flat_scope_iterator iter;
    kefir_result_t res;
    for (res = kefir_ast_identifier_flat_scope_iter(&context->label_scope, &iter); res == KEFIR_OK;
         res = kefir_ast_identifier_flat_scope_next(&context->label_scope, &iter)) {

        REQUIRE(iter.value->klass == KEFIR_AST_SCOPE_IDENTIFIER_LABEL,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected label scope to contain only labels"));
        REQUIRE(iter.value->label.point->parent != NULL,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Cannot translate undefined label"));
        REQUIRE_OK(kefir_ast_translator_flow_control_point_init(mem, iter.value->label.point, NULL));
    }
    return KEFIR_OK;
}

static kefir_result_t build_local_identifier_map_entry(struct kefir_mem *mem, struct kefir_ir_module *ir_module,
                                                       struct kefir_ir_function *ir_function, const char *identifier,
                                                       struct kefir_ast_scoped_identifier *scoped_id) {
    ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, scoped_identifier_layout,
                     scoped_id->payload.ptr);

    if (scoped_identifier_layout->lifetime.bounded) {
        REQUIRE(scoped_identifier_layout->debug_info.present,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected scoped identifier to have debug information"));
        REQUIRE_OK(kefir_ir_debug_function_local_map_insert(
            mem, &ir_function->debug_info.local_map, &ir_module->symbols, identifier,
            scoped_identifier_layout->debug_info.type, (kefir_size_t) scoped_identifier_layout->layout->value,
            scoped_identifier_layout->lifetime.begin, scoped_identifier_layout->lifetime.end));
    }
    return KEFIR_OK;
}

static kefir_result_t build_local_scope_map(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                            const struct kefir_tree_node *root, struct kefir_ir_module *ir_module,
                                            struct kefir_ir_function *ir_function) {
    ASSIGN_DECL_CAST(struct kefir_ast_identifier_flat_scope *, scope, root->value);
    kefir_bool_t empty_scope = true;
    REQUIRE_OK(local_scope_empty(mem, root, &empty_scope));

    struct kefir_ast_identifier_flat_scope_iterator iter;
    kefir_result_t res;
    for (res = kefir_ast_identifier_flat_scope_iter(scope, &iter); res == KEFIR_OK;
         res = kefir_ast_identifier_flat_scope_next(scope, &iter)) {
        if (iter.value->klass == KEFIR_AST_SCOPE_IDENTIFIER_OBJECT &&
            (iter.value->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_AUTO ||
             iter.value->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_REGISTER)) {
            REQUIRE_OK(build_local_identifier_map_entry(mem, ir_module, ir_function, iter.identifier, iter.value));
        }
    }
    REQUIRE(res == KEFIR_ITERATOR_END, res);

    kefir_bool_t empty_children = true;
    for (struct kefir_tree_node *child = kefir_tree_first_child(root); child != NULL && empty_children;
         child = kefir_tree_next_sibling(child)) {
        REQUIRE_OK(local_scope_empty(mem, child, &empty_children));
    }
    if (!empty_children) {
        for (struct kefir_tree_node *child = kefir_tree_first_child(root); child != NULL;
             child = kefir_tree_next_sibling(child)) {
            REQUIRE_OK(build_local_scope_map(mem, context, child, ir_module, ir_function));
        }
    } else {
        for (struct kefir_tree_node *child = kefir_tree_first_child(root); child != NULL;
             child = kefir_tree_next_sibling(child)) {
            REQUIRE_OK(build_local_scope_map(mem, context, child, ir_module, ir_function));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_build_local_scope_map(struct kefir_mem *mem,
                                                          const struct kefir_ast_local_context *context,
                                                          struct kefir_ir_module *ir_module,
                                                          struct kefir_ir_function *ir_function) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(ir_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR module"));
    REQUIRE(ir_function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR function"));

    if (!kefir_ast_identifier_block_scope_empty(&context->ordinary_scope)) {
        REQUIRE_OK(
            build_local_scope_map(mem, &context->context, &context->ordinary_scope.root, ir_module, ir_function));
    }
    return KEFIR_OK;
}
