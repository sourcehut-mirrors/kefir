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

#include "kefir/ast-translator/misc.h"
#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/typeconv.h"
#include "kefir/ast-translator/lvalue.h"
#include "kefir/ast/runtime.h"
#include "kefir/ast/type_completion.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

static kefir_result_t sizeof_nonvla_type(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                         struct kefir_irbuilder_block *builder, const struct kefir_ast_type *type,
                                         const struct kefir_source_location *source_location) {
    kefir_ast_target_environment_opaque_type_t opaque_type;
    struct kefir_ast_target_environment_object_info type_info;
    REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_GET_TYPE(mem, context->ast_context, &context->environment->target_env, type,
                                                     &opaque_type, source_location));
    kefir_result_t res =
        KEFIR_AST_TARGET_ENVIRONMENT_OBJECT_INFO(mem, &context->environment->target_env, opaque_type, NULL, &type_info);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, &context->environment->target_env, opaque_type);
        return res;
    });

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, type_info.size));
    REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, &context->environment->target_env, opaque_type));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_sizeof(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                          struct kefir_irbuilder_block *builder, const struct kefir_ast_type *type,
                                          const struct kefir_source_location *source_location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));

    if (KEFIR_AST_TYPE_IS_VL_ARRAY(type)) {
        const struct kefir_ast_node_base *vlen = kefir_ast_type_get_top_variable_modificator(type);
        REQUIRE(vlen != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                              "Unable to determine size of VLA with unspecified variable modifier"));
        REQUIRE_OK(kefir_ast_translate_sizeof(mem, context, builder, type->array_type.element_type, source_location));
        REQUIRE_OK(kefir_ast_translate_expression(mem, vlen, builder, context));
        REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                vlen->properties.type,
                                                context->ast_context->type_traits->ptrdiff_type));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_MUL, 0));
    } else {
        REQUIRE_OK(kefir_ast_type_completion(mem, context->ast_context, &type, type));
        REQUIRE_OK(sizeof_nonvla_type(mem, context, builder, type, source_location));
    }
    return KEFIR_OK;
}

static kefir_result_t unwrap_vla_type(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                      const struct kefir_ast_type *type, const struct kefir_ast_type **result) {
    REQUIRE_OK(kefir_ast_type_completion(mem, context, &type, type));

    if (KEFIR_AST_TYPE_IS_VL_ARRAY(type)) {
        const struct kefir_ast_type *element_type = NULL;
        REQUIRE_OK(unwrap_vla_type(mem, context, type->array_type.element_type, &element_type));
        const struct kefir_ast_type *array_type =
            kefir_ast_type_array(mem, context->type_bundle, element_type, 1, NULL);
        REQUIRE(array_type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed toa allocate AST array type"));
        *result = array_type;
    } else {
        *result = type;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_alignof(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                           struct kefir_irbuilder_block *builder,
                                           const struct kefir_ast_type *base_type,
                                           const struct kefir_source_location *source_location) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(base_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));

    kefir_ast_target_environment_opaque_type_t opaque_type;
    struct kefir_ast_target_environment_object_info type_info;
    const struct kefir_ast_type *type = NULL;
    REQUIRE_OK(unwrap_vla_type(mem, context->ast_context, base_type, &type));
    REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_GET_TYPE(mem, context->ast_context, &context->environment->target_env, type,
                                                     &opaque_type, source_location));
    kefir_result_t res =
        KEFIR_AST_TARGET_ENVIRONMENT_OBJECT_INFO(mem, &context->environment->target_env, opaque_type, NULL, &type_info);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, &context->environment->target_env, opaque_type);
        return res;
    });

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, type_info.alignment));
    REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, &context->environment->target_env, opaque_type));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_resolve_type_layout(struct kefir_irbuilder_block *builder, kefir_id_t type_id,
                                                        const struct kefir_ast_type_layout *layout,
                                                        const struct kefir_ast_type_layout *root) {
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type layout"));

    if (layout->parent != NULL && layout->parent != root) {
        REQUIRE_OK(kefir_ast_translator_resolve_type_layout(builder, type_id, layout->parent, root));
    }

    if (layout->properties.relative_offset != 0) {
        REQUIRE_OK(
            KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, layout->properties.relative_offset));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
    }
    return KEFIR_OK;
}

#define VARIABLE_ID_UPPER32(_type_id, _type_index) ((kefir_uint32_t) (_type_id))
#define VARIABLE_ID_LOWER32(_type_id, _type_index) ((kefir_uint32_t) (_type_index))
kefir_result_t kefir_ast_translator_resolve_local_type_layout(struct kefir_irbuilder_block *builder, kefir_id_t type_id,
                                                              const struct kefir_ast_type_layout *layout) {
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type layout"));

    if (layout->parent == NULL || (layout->parent->type == NULL && layout->parent->value == ~(kefir_uptr_t) 0ull)) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU32_4(
            builder, KEFIR_IR_OPCODE_GET_LOCAL, VARIABLE_ID_UPPER32(type_id, layout->value),
            VARIABLE_ID_LOWER32(type_id, layout->value), type_id, layout->value));
    } else {
        REQUIRE_OK(kefir_ast_translator_resolve_local_type_layout(builder, type_id, layout->parent));
        if (layout->properties.relative_offset != 0) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST,
                                                       layout->properties.relative_offset));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t kefir_ast_translator_resolve_local_type_layout_no_offset(
    struct kefir_irbuilder_block *builder, kefir_id_t type_id, const struct kefir_ast_type_layout *layout) {
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(layout != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type layout"));

    if (layout->parent == NULL || (layout->parent->type == NULL && layout->parent->value == ~(kefir_uptr_t) 0ull)) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU32_4(
            builder, KEFIR_IR_OPCODE_GET_LOCAL, VARIABLE_ID_UPPER32(type_id, layout->value),
            VARIABLE_ID_LOWER32(type_id, layout->value), type_id, layout->value));
    } else {
        REQUIRE_OK(kefir_ast_translator_resolve_local_type_layout(builder, type_id, layout->parent));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_resolve_vla_element(struct kefir_mem *mem,
                                                        struct kefir_ast_translator_context *context,
                                                        struct kefir_irbuilder_block *builder, kefir_id_t vla_id) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));

    const struct kefir_ast_scoped_identifier *scoped_id = NULL;
    REQUIRE_OK(context->ast_context->resolve_ordinary_identifier(
        context->ast_context, KEFIR_AST_TRANSLATOR_VLA_ELEMENTS_IDENTIFIER, &scoped_id));
    ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, scoped_id_layout, scoped_id->payload.ptr);

    struct kefir_ast_designator vla_element_designator = {
        .type = KEFIR_AST_DESIGNATOR_SUBSCRIPT, .index = vla_id, .next = NULL};

    struct kefir_ast_type_layout *vla_element_layout = NULL;
    REQUIRE_OK(kefir_ast_type_layout_resolve(scoped_id_layout->layout, &vla_element_designator, &vla_element_layout,
                                             NULL, NULL));

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU32_4(
        builder, KEFIR_IR_OPCODE_GET_LOCAL,
        VARIABLE_ID_UPPER32(scoped_id_layout->type_id, scoped_id_layout->layout->value),
        VARIABLE_ID_LOWER32(scoped_id_layout->type_id, scoped_id_layout->layout->value), scoped_id_layout->type_id,
        scoped_id_layout->layout->value));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, vla_id));
    REQUIRE_OK(
        KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, vla_element_layout->properties.size));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_MUL, 0));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_mark_flat_scope_objects_lifetime(
    struct kefir_mem *mem, struct kefir_ast_translator_context *context, struct kefir_irbuilder_block *builder,
    const struct kefir_ast_identifier_flat_scope *scope) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(scope != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flat scope"));

    struct kefir_ast_identifier_flat_scope_iterator scope_iter;
    kefir_result_t res;
    for (res = kefir_ast_identifier_flat_scope_iter(scope, &scope_iter); res == KEFIR_OK;
         res = kefir_ast_identifier_flat_scope_next(scope, &scope_iter)) {
        if (scope_iter.value->klass == KEFIR_AST_SCOPE_IDENTIFIER_OBJECT &&
            (scope_iter.value->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_AUTO ||
             scope_iter.value->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_REGISTER ||
             scope_iter.value->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_CONSTEXPR)) {
            ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, identifier_data,
                             scope_iter.value->payload.ptr);
            if (KEFIR_AST_TYPE_IS_VL_ARRAY(scope_iter.value->object.type)) {
                REQUIRE_OK(kefir_ast_translator_resolve_local_type_layout_no_offset(builder, identifier_data->type_id,
                                                                                    identifier_data->layout));
            } else {
                REQUIRE_OK(kefir_ast_translator_resolve_local_type_layout_no_offset(builder, identifier_data->type_id,
                                                                                    identifier_data->layout));
            }
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LOCAL_LIFETIME_MARK, 0));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}
