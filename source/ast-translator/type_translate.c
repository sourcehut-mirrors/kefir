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

#include "kefir/ast-translator/translator.h"
#include "kefir/ast/alignment.h"
#include "kefir/ast/type_completion.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"
#include "kefir/ir/builtins.h"

static kefir_result_t scalar_typeentry(const struct kefir_ast_type *type, kefir_size_t alignment,
                                       struct kefir_ir_typeentry *typeentry) {
    typeentry->alignment = alignment;
    typeentry->param = 0;
    switch (type->tag) {
        case KEFIR_AST_TYPE_VOID:
            break;

        case KEFIR_AST_TYPE_SCALAR_BOOL:
            typeentry->typecode = KEFIR_IR_TYPE_BOOL;
            break;

        case KEFIR_AST_TYPE_SCALAR_CHAR:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            typeentry->typecode = KEFIR_IR_TYPE_CHAR;
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            typeentry->typecode = KEFIR_IR_TYPE_SHORT;
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
        case KEFIR_AST_TYPE_ENUMERATION:
            typeentry->typecode = KEFIR_IR_TYPE_INT;
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
            typeentry->typecode = KEFIR_IR_TYPE_LONG;
            break;

        case KEFIR_AST_TYPE_SCALAR_FLOAT:
            typeentry->typecode = KEFIR_IR_TYPE_FLOAT32;
            break;

        case KEFIR_AST_TYPE_SCALAR_DOUBLE:
            typeentry->typecode = KEFIR_IR_TYPE_FLOAT64;
            break;

        case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
            typeentry->typecode = KEFIR_IR_TYPE_LONG_DOUBLE;
            break;

        case KEFIR_AST_TYPE_SCALAR_POINTER:
            typeentry->typecode = KEFIR_IR_TYPE_WORD;
            break;

        case KEFIR_AST_TYPE_VA_LIST:
            typeentry->typecode = KEFIR_IR_TYPE_BUILTIN;
            typeentry->param = KEFIR_IR_TYPE_BUILTIN_VARARG;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Not a scalar type");
    }
    return KEFIR_OK;
}

static kefir_result_t translate_scalar_type(struct kefir_mem *mem, const struct kefir_ast_type *type,
                                            kefir_size_t alignment, struct kefir_irbuilder_type *builder,
                                            struct kefir_ast_type_layout **layout_ptr) {
    kefir_size_t type_index = kefir_ir_type_length(builder->type);

    if (type->tag != KEFIR_AST_TYPE_VOID) {
        struct kefir_ir_typeentry typeentry = {0};
        REQUIRE_OK(scalar_typeentry(type, alignment, &typeentry));
        REQUIRE_OK(KEFIR_IRBUILDER_TYPE_APPEND_ENTRY(builder, &typeentry));
    }

    if (layout_ptr != NULL) {
        *layout_ptr = kefir_ast_new_type_layout(mem, type, alignment, type_index);
        REQUIRE(*layout_ptr != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST type layout"));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_complex_type(struct kefir_mem *mem, const struct kefir_ast_type *type,
                                             kefir_size_t alignment, struct kefir_irbuilder_type *builder,
                                             struct kefir_ast_type_layout **layout_ptr) {
    kefir_size_t type_index = kefir_ir_type_length(builder->type);

    switch (type->tag) {
        case KEFIR_AST_TYPE_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_TYPE_APPEND(builder, KEFIR_IR_TYPE_COMPLEX_FLOAT32, 0, 0));
            break;

        case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_TYPE_APPEND(builder, KEFIR_IR_TYPE_COMPLEX_FLOAT64, 0, 0));
            break;

        case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_TYPE_APPEND(builder, KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE, 0, 0));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Not a complex type");
    }

    if (layout_ptr != NULL) {
        *layout_ptr = kefir_ast_new_type_layout(mem, type, alignment, type_index);
        REQUIRE(*layout_ptr != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST type layout"));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_array_type(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                           const struct kefir_ast_type *type, kefir_size_t alignment,
                                           const struct kefir_ast_translator_environment *env,
                                           struct kefir_irbuilder_type *builder,
                                           struct kefir_ast_type_layout **layout_ptr,
                                           const struct kefir_source_location *source_location) {
    kefir_size_t type_index = kefir_ir_type_length(builder->type);
    struct kefir_ast_type_layout *element_layout = NULL;

    switch (type->array_type.boundary) {
        case KEFIR_AST_ARRAY_UNBOUNDED: {
            REQUIRE_OK(KEFIR_IRBUILDER_TYPE_APPEND(builder, KEFIR_IR_TYPE_ARRAY, alignment, 0));
            REQUIRE_OK(kefir_ast_translate_object_type(mem, context, type->array_type.element_type,
                                                       KEFIR_AST_DEFAULT_ALIGNMENT, env, builder,
                                                       layout_ptr != NULL ? &element_layout : NULL, source_location));
            if (layout_ptr != NULL) {
                *layout_ptr = kefir_ast_new_type_layout(mem, type, alignment, type_index);
                REQUIRE(*layout_ptr != NULL,
                        KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST type layout"));
                (*layout_ptr)->array_layout.element_type = element_layout;
            }
        } break;

        case KEFIR_AST_ARRAY_BOUNDED:
        case KEFIR_AST_ARRAY_BOUNDED_STATIC:
            REQUIRE_OK(KEFIR_IRBUILDER_TYPE_APPEND(builder, KEFIR_IR_TYPE_ARRAY, alignment,
                                                   kefir_ast_type_array_const_length(&type->array_type)));
            REQUIRE_OK(kefir_ast_translate_object_type(mem, context, type->array_type.element_type,
                                                       KEFIR_AST_DEFAULT_ALIGNMENT, env, builder,
                                                       layout_ptr != NULL ? &element_layout : NULL, source_location));
            if (layout_ptr != NULL) {
                *layout_ptr = kefir_ast_new_type_layout(mem, type, alignment, type_index);
                REQUIRE(*layout_ptr != NULL,
                        KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST type layout"));
                (*layout_ptr)->array_layout.element_type = element_layout;
            }
            break;

        case KEFIR_AST_ARRAY_VLA:
        case KEFIR_AST_ARRAY_VLA_STATIC:
            REQUIRE_OK(KEFIR_IRBUILDER_TYPE_APPEND(builder, KEFIR_IR_TYPE_STRUCT, 0, 2));
            REQUIRE_OK(KEFIR_IRBUILDER_TYPE_APPEND(builder, KEFIR_IR_TYPE_WORD, 0, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_TYPE_APPEND(builder, KEFIR_IR_TYPE_WORD, 0, 0));
            if (layout_ptr != NULL) {
                *layout_ptr = kefir_ast_new_type_layout(mem, type, 0, type_index);
                REQUIRE(*layout_ptr != NULL,
                        KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST type layout"));
                (*layout_ptr)->vl_array.array_ptr_field = type_index + 1;
                (*layout_ptr)->vl_array.array_size_field = type_index + 2;
            }
            break;
    }

    if (layout_ptr != NULL && element_layout != NULL) {
        element_layout->parent = *layout_ptr;
    }
    return KEFIR_OK;
}

static kefir_result_t insert_struct_field(struct kefir_mem *mem, struct kefir_ast_struct_field *field,
                                          struct kefir_ast_type_layout *layout,
                                          struct kefir_ast_type_layout *element_layout) {

    element_layout->parent = layout;
    if (field->identifier == NULL || strlen(field->identifier) == 0) {
        REQUIRE_OK(kefir_ast_type_layout_add_structure_anonymous_member(mem, layout, element_layout));
    } else {
        REQUIRE_OK(kefir_ast_type_layout_insert_structure_member(mem, layout, field->identifier, element_layout));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_normal_struct_field(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                    struct kefir_ast_struct_field *field,
                                                    const struct kefir_ast_translator_environment *env,
                                                    struct kefir_ast_type_layout *layout, kefir_size_t type_index,
                                                    struct kefir_irbuilder_type *builder,
                                                    const struct kefir_source_location *source_location) {
    struct kefir_ast_type_layout *element_layout = NULL;
    REQUIRE_OK(kefir_ast_translate_object_type(mem, context, field->type, field->alignment->value, env, builder,
                                               layout != NULL ? &element_layout : NULL, source_location));
    if (element_layout != NULL) {
        element_layout->bitfield = field->bitfield;
        element_layout->bitfield_props.offset = 0;
        if (field->bitfield) {
            element_layout->bitfield_props.width = field->bitwidth->value.integer;
        } else {
            element_layout->bitfield_props.width = 0;
        }

        kefir_result_t res = insert_struct_field(mem, field, layout, element_layout);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_type_layout_free(mem, element_layout);
            return res;
        });
    }

    kefir_ir_type_at(builder->type, type_index)->param++;
    return KEFIR_OK;
}

struct bitfield_manager {
    struct kefir_ir_bitfield_allocator allocator;
    struct kefir_ast_type_layout *last_bitfield_layout;
    kefir_size_t last_bitfield_storage;
};

static kefir_result_t translate_bitfield(struct kefir_mem *mem, struct kefir_ast_type_bundle *type_bundle,
                                         struct kefir_ast_struct_field *field, struct kefir_ast_type_layout *layout,
                                         struct kefir_irbuilder_type *builder, kefir_size_t type_index,
                                         struct bitfield_manager *bitfield_mgr) {
    if (field->bitwidth->value.integer == 0) {
        REQUIRE_OK(KEFIR_IR_BITFIELD_ALLOCATOR_RESET(&bitfield_mgr->allocator));
        bitfield_mgr->last_bitfield_storage = 0;
        bitfield_mgr->last_bitfield_layout = NULL;
        return KEFIR_OK;
    }

    struct kefir_ast_type_qualification qualification;
    REQUIRE_OK(kefir_ast_type_retrieve_qualifications(&qualification, field->type));
    const struct kefir_ast_type *unqualified_field_type = kefir_ast_unqualified_type(field->type);

    kefir_bool_t allocated = false;
    struct kefir_ir_bitfield ir_bitfield;
    struct kefir_ast_type_layout *element_layout = NULL;
    if (KEFIR_IR_BITFIELD_ALLOCATOR_HAS_BITFIELD_RUN(&bitfield_mgr->allocator)) {
        struct kefir_ir_typeentry colocated_typeentry = {0};
        REQUIRE_OK(scalar_typeentry(unqualified_field_type, field->alignment->value, &colocated_typeentry));
        kefir_result_t res = KEFIR_IR_BITFIELD_ALLOCATOR_NEXT_COLOCATED(
            mem, &bitfield_mgr->allocator, field->identifier != NULL, colocated_typeentry.typecode,
            field->bitwidth->value.integer, kefir_ir_type_at(builder->type, bitfield_mgr->last_bitfield_storage),
            &ir_bitfield);
        if (res != KEFIR_OUT_OF_SPACE) {
            REQUIRE_OK(res);
            if (layout != NULL) {
                element_layout = kefir_ast_new_type_layout(mem, unqualified_field_type, field->alignment->value,
                                                           bitfield_mgr->last_bitfield_storage);
                REQUIRE(element_layout != NULL,
                        KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST type layout"));
            }
            allocated = true;
        }
    }

    if (!allocated) {
        struct kefir_ir_typeentry typeentry = {0};
        REQUIRE_OK(scalar_typeentry(unqualified_field_type, field->alignment->value, &typeentry));
        REQUIRE_OK(KEFIR_IR_BITFIELD_ALLOCATOR_NEXT(mem, &bitfield_mgr->allocator, type_index,
                                                    field->identifier != NULL, typeentry.typecode,
                                                    field->bitwidth->value.integer, &typeentry, &ir_bitfield));
        bitfield_mgr->last_bitfield_storage = kefir_ir_type_length(builder->type);
        REQUIRE_OK(KEFIR_IRBUILDER_TYPE_APPEND_ENTRY(builder, &typeentry));
        if (layout != NULL) {
            element_layout = kefir_ast_new_type_layout(mem, unqualified_field_type, field->alignment->value,
                                                       bitfield_mgr->last_bitfield_storage);
            REQUIRE(element_layout != NULL,
                    KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST type layout"));
        }
        bitfield_mgr->last_bitfield_layout = element_layout;
        kefir_ir_type_at(builder->type, type_index)->param++;
    }

    if (element_layout != NULL) {
        element_layout->bitfield = true;
        element_layout->bitfield_props.width = ir_bitfield.width;
        element_layout->bitfield_props.offset = ir_bitfield.offset;
        REQUIRE_OK(kefir_ast_type_layout_set_qualification(mem, type_bundle, element_layout, qualification));

        kefir_result_t res = insert_struct_field(mem, field, layout, element_layout);
        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_type_layout_free(mem, element_layout);
            return res;
        });
    }
    return KEFIR_OK;
}

static kefir_result_t set_packed(const struct kefir_ir_type *type, kefir_size_t index,
                                 const struct kefir_ir_typeentry *typeentry, void *payload) {
    UNUSED(typeentry);
    UNUSED(payload);
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type"));

    if (kefir_ir_type_at(type, index)->alignment == 0) {
        kefir_ir_type_at(type, index)->alignment = 1;
    }
    return KEFIR_OK;
}

static kefir_result_t translate_struct_type(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                            const struct kefir_ast_type *type, kefir_size_t alignment,
                                            const struct kefir_ast_translator_environment *env,
                                            struct kefir_irbuilder_type *builder,
                                            struct kefir_ast_type_layout **layout_ptr,
                                            const struct kefir_source_location *source_location) {
    REQUIRE_OK(kefir_ast_type_completion(mem, context, &type, type));
    REQUIRE(type->structure_type.complete,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, source_location, "Expected complete structure/union type"));

    kefir_size_t type_index = kefir_ir_type_length(builder->type);
    REQUIRE_OK(KEFIR_IRBUILDER_TYPE_APPEND(
        builder, type->tag == KEFIR_AST_TYPE_STRUCTURE ? KEFIR_IR_TYPE_STRUCT : KEFIR_IR_TYPE_UNION, alignment, 0));

    kefir_bool_t allocated = false;
    struct kefir_ast_type_layout *layout = NULL;
    if (layout_ptr != NULL) {
        layout = kefir_ast_new_type_layout(mem, type, alignment, type_index);
        REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST type layout"));
        allocated = true;
        *layout_ptr = layout;
    }

    struct bitfield_manager bitfield_mgr;
    REQUIRE_OK(
        KEFIR_IR_TARGET_PLATFORM_BITFIELD_ALLOCATOR(mem, env->target_platform, builder->type, &bitfield_mgr.allocator));
    bitfield_mgr.last_bitfield_layout = NULL;
    bitfield_mgr.last_bitfield_storage = 0;

    kefir_result_t res = KEFIR_OK;
    for (const struct kefir_list_entry *iter = kefir_list_head(&type->structure_type.fields);
         iter != NULL && res == KEFIR_OK; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_struct_field *, field, iter->value);

        if (field->bitfield) {
            if (type->tag == KEFIR_AST_TYPE_UNION) {
                REQUIRE_CHAIN(&res, KEFIR_IR_BITFIELD_ALLOCATOR_RESET(&bitfield_mgr.allocator));
                bitfield_mgr.last_bitfield_layout = NULL;
                bitfield_mgr.last_bitfield_storage = 0;
            }
            REQUIRE_CHAIN(
                &res, translate_bitfield(mem, context->type_bundle, field, layout, builder, type_index, &bitfield_mgr));
        } else {
            REQUIRE_CHAIN(&res, KEFIR_IR_BITFIELD_ALLOCATOR_RESET(&bitfield_mgr.allocator));
            bitfield_mgr.last_bitfield_layout = NULL;
            bitfield_mgr.last_bitfield_storage = 0;
            REQUIRE_CHAIN(&res, translate_normal_struct_field(mem, context, field, env, layout, type_index, builder,
                                                              source_location));
        }
    }

    if (res == KEFIR_OK && kefir_list_length(&type->structure_type.fields) == 0 && env->configuration->empty_structs) {
        res = KEFIR_IRBUILDER_TYPE_APPEND(builder, KEFIR_IR_TYPE_CHAR, 0, 0);
        kefir_ir_type_at(builder->type, type_index)->param++;
    }

    if (type->structure_type.packed) {
        struct kefir_ir_type_visitor visitor;
        REQUIRE_OK(kefir_ir_type_visitor_init(&visitor, set_packed));
        REQUIRE_OK(kefir_ir_type_visitor_list_nodes(builder->type, &visitor, NULL, type_index + 1,
                                                    kefir_ir_type_at(builder->type, type_index)->param));
    }

    REQUIRE_ELSE(res == KEFIR_OK, {
        if (allocated) {
            kefir_ast_type_layout_free(mem, layout);
            *layout_ptr = NULL;
        }
        KEFIR_IR_BITFIELD_ALLOCATOR_FREE(mem, &bitfield_mgr.allocator);
        return res;
    });

    REQUIRE_OK(KEFIR_IR_BITFIELD_ALLOCATOR_FREE(mem, &bitfield_mgr.allocator));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_object_type(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                               const struct kefir_ast_type *type, kefir_size_t alignment,
                                               const struct kefir_ast_translator_environment *env,
                                               struct kefir_irbuilder_type *builder,
                                               struct kefir_ast_type_layout **layout_ptr,
                                               const struct kefir_source_location *source_location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(env != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator environment"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR type builder"));

    switch (type->tag) {
        case KEFIR_AST_TYPE_VOID:
        case KEFIR_AST_TYPE_SCALAR_BOOL:
        case KEFIR_AST_TYPE_SCALAR_CHAR:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_FLOAT:
        case KEFIR_AST_TYPE_SCALAR_DOUBLE:
        case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
        case KEFIR_AST_TYPE_SCALAR_POINTER:
        case KEFIR_AST_TYPE_VA_LIST:
            REQUIRE_OK(translate_scalar_type(mem, type, alignment, builder, layout_ptr));
            break;

        case KEFIR_AST_TYPE_COMPLEX_FLOAT:
        case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
        case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(translate_complex_type(mem, type, alignment, builder, layout_ptr));
            break;

        case KEFIR_AST_TYPE_ENUMERATION:
            REQUIRE_OK(kefir_ast_translate_object_type(mem, context, type->enumeration_type.underlying_type, alignment,
                                                       env, builder, layout_ptr, source_location));
            break;

        case KEFIR_AST_TYPE_STRUCTURE:
        case KEFIR_AST_TYPE_UNION:
            REQUIRE_OK(translate_struct_type(mem, context, type, alignment, env, builder, layout_ptr, source_location));
            break;

        case KEFIR_AST_TYPE_ARRAY:
            REQUIRE_OK(translate_array_type(mem, context, type, alignment, env, builder, layout_ptr, source_location));
            break;

        case KEFIR_AST_TYPE_FUNCTION:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Cannot translate AST function type into IR type");

        case KEFIR_AST_TYPE_QUALIFIED:
            REQUIRE_OK(kefir_ast_translate_object_type(mem, context, type->qualified_type.type, alignment, env, builder,
                                                       layout_ptr, source_location));
            if (type->qualified_type.qualification.atomic_type) {
                struct kefir_ir_typeentry *typeentry =
                    kefir_ir_type_at(builder->type, kefir_ir_type_length(builder->type) - 1);
                REQUIRE(typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to retrive IR type entry"));
                typeentry->atomic = true;
            }
            if (layout_ptr != NULL && *layout_ptr != NULL) {
                REQUIRE_OK(kefir_ast_type_layout_set_qualification(mem, context->type_bundle, *layout_ptr,
                                                                   type->qualified_type.qualification));
            }
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected AST type tag");
    }

    return KEFIR_OK;
}
