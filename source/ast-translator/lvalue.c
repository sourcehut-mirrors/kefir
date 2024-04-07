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

#include <string.h>
#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/lvalue.h"
#include "kefir/ast-translator/temporaries.h"
#include "kefir/ast-translator/initializer.h"
#include "kefir/ast-translator/misc.h"
#include "kefir/ast-translator/type.h"
#include "kefir/ast-translator/typeconv.h"
#include "kefir/ast/type_completion.h"
#include "kefir/ast/runtime.h"
#include "kefir/ast/type_conv.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/extensions.h"

kefir_result_t kefir_ast_translator_object_lvalue(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                                  struct kefir_irbuilder_block *builder, const char *identifier,
                                                  const struct kefir_ast_scoped_identifier *scoped_identifier) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(identifier != NULL && strlen(identifier) > 0,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid object identifier"));
    REQUIRE(scoped_identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid scope identifier"));
    REQUIRE(scoped_identifier->klass == KEFIR_AST_SCOPE_IDENTIFIER_OBJECT,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected object scoped identifier"));

    if (scoped_identifier->object.asm_label != NULL) {
        identifier = scoped_identifier->object.asm_label;
    }

    switch (scoped_identifier->object.storage) {
        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN: {
            kefir_id_t id;
            REQUIRE(kefir_ir_module_symbol(mem, context->module, identifier, &id) != NULL,
                    KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR module symbol"));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_GETGLOBAL, id));
        } break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC: {
            ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, identifier_data,
                             scoped_identifier->payload.ptr);
            kefir_id_t id;
            if (scoped_identifier->object.initializer != NULL) {
                REQUIRE(kefir_ir_module_symbol(mem, context->module, KEFIR_AST_TRANSLATOR_STATIC_VARIABLES_IDENTIFIER,
                                               &id) != NULL,
                        KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR module symbol"));
            } else {
                REQUIRE(kefir_ir_module_symbol(mem, context->module,
                                               KEFIR_AST_TRANSLATOR_STATIC_UNINIT_VARIABLES_IDENTIFIER, &id) != NULL,
                        KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR module symbol"));
            }
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_GETGLOBAL, id));
            REQUIRE_OK(kefir_ast_translator_resolve_type_layout(builder, identifier_data->type_id,
                                                                identifier_data->layout, NULL));

        } break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_THREAD_LOCAL:
        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN_THREAD_LOCAL: {
            kefir_id_t id;
            REQUIRE(kefir_ir_module_symbol(mem, context->module, identifier, &id) != NULL,
                    KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR module symbol"));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_GETTHRLOCAL, id));
        } break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC_THREAD_LOCAL: {
            ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, identifier_data,
                             scoped_identifier->payload.ptr);
            kefir_id_t id;
            if (scoped_identifier->object.initializer != NULL) {
                REQUIRE(
                    kefir_ir_module_symbol(mem, context->module,
                                           KEFIR_AST_TRANSLATOR_STATIC_THREAD_LOCAL_VARIABLES_IDENTIFIER, &id) != NULL,
                    KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR module symbol"));
            } else {
                REQUIRE(kefir_ir_module_symbol(mem, context->module,
                                               KEFIR_AST_TRANSLATOR_STATIC_THREAD_LOCAL_UNINIT_VARIABLES_IDENTIFIER,
                                               &id) != NULL,
                        KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR module symbol"));
            }
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_GETTHRLOCAL, id));
            REQUIRE_OK(kefir_ast_translator_resolve_type_layout(builder, identifier_data->type_id,
                                                                identifier_data->layout, NULL));

        } break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_AUTO:
        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_REGISTER: {
            ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, identifier_data,
                             scoped_identifier->payload.ptr);
            if (KEFIR_AST_TYPE_IS_VL_ARRAY(scoped_identifier->object.type)) {
                REQUIRE_OK(kefir_ast_translator_resolve_local_type_layout(builder, identifier_data->type_id,
                                                                          identifier_data->layout));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(
                    builder, KEFIR_IROPCODE_PUSHU64, identifier_data->layout->vl_array.array_ptr_relative_offset));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_IADD64, 0));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_LOAD64, KEFIR_IR_MEMORY_FLAG_NONE));
            } else {
                REQUIRE_OK(kefir_ast_translator_resolve_local_type_layout(builder, identifier_data->type_id,
                                                                          identifier_data->layout));
            }
        } break;

        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_TYPEDEF:
        case KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected storage class for lvalue");
    }

    return KEFIR_OK;
}

kefir_result_t kefir_ast_translator_function_lvalue(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                                    struct kefir_irbuilder_block *builder, const char *identifier) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(identifier != NULL && strlen(identifier) > 0,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid function identifier"));

    kefir_id_t id;
    REQUIRE(kefir_ir_module_symbol(mem, context->module, identifier, &id) != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate IR module symbol"));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_GETGLOBAL, id));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_array_subscript_lvalue(struct kefir_mem *mem,
                                                          struct kefir_ast_translator_context *context,
                                                          struct kefir_irbuilder_block *builder,
                                                          const struct kefir_ast_array_subscript *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST array subscript node"));

    struct kefir_ast_translator_type *translator_type = NULL;
    REQUIRE_OK(kefir_ast_translator_type_new(mem, context->ast_context, context->environment, context->module,
                                             node->base.properties.type, 0, &translator_type,
                                             &node->base.source_location));

    kefir_result_t res = KEFIR_OK;
    const struct kefir_ast_type *array_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, node->array->properties.type);
    if (array_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER) {
        REQUIRE_CHAIN(&res, kefir_ast_translate_expression(mem, node->array, builder, context));
        REQUIRE_CHAIN(&res, kefir_ast_translate_expression(mem, node->subscript, builder, context));
        REQUIRE_CHAIN(&res, kefir_ast_translate_typeconv(
                                mem, context->module, builder, context->ast_context->type_traits,
                                node->subscript->properties.type, context->ast_context->type_traits->size_type));
    } else {
        REQUIRE_CHAIN(&res, kefir_ast_translate_expression(mem, node->subscript, builder, context));
        REQUIRE_CHAIN(&res, kefir_ast_translate_expression(mem, node->array, builder, context));
        REQUIRE_CHAIN(&res, kefir_ast_translate_typeconv(
                                mem, context->module, builder, context->ast_context->type_traits,
                                node->array->properties.type, context->ast_context->type_traits->size_type));
    }
    REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PUSHU64,
                                                        translator_type->object.layout->properties.size));
    REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_IMUL64, 0));
    REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_IADD64, 0));

    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_translator_type_free(mem, translator_type);
        return res;
    });
    REQUIRE_OK(kefir_ast_translator_type_free(mem, translator_type));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_struct_member_lvalue(struct kefir_mem *mem,
                                                        struct kefir_ast_translator_context *context,
                                                        struct kefir_irbuilder_block *builder,
                                                        const struct kefir_ast_struct_member *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST struct member node"));

    const struct kefir_ast_type *structure_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, node->structure->properties.type);
    if (structure_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER) {
        structure_type = kefir_ast_unqualified_type(structure_type->referenced_type);
    }

    REQUIRE_OK(kefir_ast_type_completion(mem, context->ast_context, &structure_type, structure_type));
    REQUIRE((structure_type->tag == KEFIR_AST_TYPE_STRUCTURE || structure_type->tag == KEFIR_AST_TYPE_UNION) &&
                structure_type->structure_type.complete,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected expression of complete structure/union type"));

    struct kefir_ast_translator_type *translator_type = NULL;
    REQUIRE_OK(kefir_ast_translator_type_new(mem, context->ast_context, context->environment, context->module,
                                             structure_type, 0, &translator_type, &node->base.source_location));

    struct kefir_ast_type_layout *member_layout = NULL;
    struct kefir_ast_designator designator = {
        .type = KEFIR_AST_DESIGNATOR_MEMBER, .member = node->member, .next = NULL};
    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(
        &res, kefir_ast_type_layout_resolve(translator_type->object.layout, &designator, &member_layout, NULL, NULL));

    REQUIRE_CHAIN(&res, kefir_ast_translate_expression(mem, node->structure, builder, context));
    REQUIRE_CHAIN(&res, kefir_ast_translator_resolve_type_layout(builder, translator_type->object.ir_type_id,
                                                                 member_layout, translator_type->object.layout));

    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_translator_type_free(mem, translator_type);
        return res;
    });
    REQUIRE_OK(kefir_ast_translator_type_free(mem, translator_type));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_compound_literal_lvalue(struct kefir_mem *mem,
                                                           struct kefir_ast_translator_context *context,
                                                           struct kefir_irbuilder_block *builder,
                                                           const struct kefir_ast_compound_literal *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST compound literal node"));

    REQUIRE_OK(kefir_ast_translator_fetch_temporary(mem, context, builder,
                                                    &node->base.properties.expression_props.temporary_identifier));
    REQUIRE_OK(kefir_ast_translate_initializer(mem, context, builder, node->base.properties.type, node->initializer));
    return KEFIR_OK;
}

static kefir_result_t translate_not_impl(const struct kefir_ast_visitor *visitor,
                                         const struct kefir_ast_node_base *base, void *payload) {
    UNUSED(visitor);
    UNUSED(base);
    UNUSED(payload);
    return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Cannot translate non-lvalue AST node");
}

static kefir_result_t translate_identifier_node(const struct kefir_ast_visitor *visitor,
                                                const struct kefir_ast_identifier *node, void *payload) {
    REQUIRE(visitor != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST visitor"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct kefir_ast_translator_parameters *, param, payload);

    const struct kefir_ast_scoped_identifier *scoped_identifier = node->base.properties.expression_props.scoped_id;
    switch (scoped_identifier->klass) {
        case KEFIR_AST_SCOPE_IDENTIFIER_OBJECT:
            REQUIRE_OK(kefir_ast_translator_object_lvalue(param->mem, param->context, param->builder, node->identifier,
                                                          scoped_identifier));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_ENUM_CONSTANT:

        case KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION:
            REQUIRE_OK(
                kefir_ast_translator_function_lvalue(param->mem, param->context, param->builder, node->identifier));
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_TYPE_TAG:
        case KEFIR_AST_SCOPE_IDENTIFIER_TYPE_DEFINITION:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Type definition is not an lvalue");

        case KEFIR_AST_SCOPE_IDENTIFIER_LABEL:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Label is not an lvalue");
    }
    return KEFIR_OK;
    return KEFIR_OK;
}

static kefir_result_t translate_array_subscript_node(const struct kefir_ast_visitor *visitor,
                                                     const struct kefir_ast_array_subscript *node, void *payload) {
    REQUIRE(visitor != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST visitor"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct kefir_ast_translator_parameters *, param, payload);

    REQUIRE_OK(kefir_ast_translate_array_subscript_lvalue(param->mem, param->context, param->builder, node));
    return KEFIR_OK;
}

static kefir_result_t translate_struct_member_node(const struct kefir_ast_visitor *visitor,
                                                   const struct kefir_ast_struct_member *node, void *payload) {
    REQUIRE(visitor != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST visitor"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct kefir_ast_translator_parameters *, param, payload);

    REQUIRE_OK(kefir_ast_translate_struct_member_lvalue(param->mem, param->context, param->builder, node));
    return KEFIR_OK;
}

static kefir_result_t translate_unary_operation_node(const struct kefir_ast_visitor *visitor,
                                                     const struct kefir_ast_unary_operation *node, void *payload) {
    REQUIRE(visitor != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST visitor"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct kefir_ast_translator_parameters *, param, payload);

    switch (node->type) {
        case KEFIR_AST_OPERATION_INDIRECTION:
            REQUIRE_OK(kefir_ast_translate_expression(param->mem, node->arg, param->builder, param->context));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Specified unary operator does not produce an lvalue");
    }
    return KEFIR_OK;
}

static kefir_result_t translate_compound_literal_node(const struct kefir_ast_visitor *visitor,
                                                      const struct kefir_ast_compound_literal *node, void *payload) {
    REQUIRE(visitor != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST visitor"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct kefir_ast_translator_parameters *, param, payload);

    REQUIRE_OK(kefir_ast_translate_compound_literal_lvalue(param->mem, param->context, param->builder, node));
    return KEFIR_OK;
}

static kefir_result_t translate_extension_node(const struct kefir_ast_visitor *visitor,
                                               const struct kefir_ast_extension_node *node, void *payload) {
    REQUIRE(visitor != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST visitor"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct kefir_ast_translator_parameters *, param, payload);

    REQUIRE(param->context->extensions != NULL && param->context->extensions->translate_extension_node != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Extension node translation procedure is not defined"));
    REQUIRE_OK(param->context->extensions->translate_extension_node(param->mem, param->context, node, param->builder,
                                                                    KEFIR_AST_TRANSLATOR_CONTEXT_EXTENSION_TAG_LVALUE));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_lvalue(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                          struct kefir_irbuilder_block *builder,
                                          const struct kefir_ast_node_base *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));

    struct kefir_ast_visitor visitor;
    REQUIRE_OK(kefir_ast_visitor_init(&visitor, translate_not_impl));
    visitor.identifier = translate_identifier_node;
    visitor.array_subscript = translate_array_subscript_node;
    visitor.struct_member = translate_struct_member_node;
    visitor.struct_indirect_member = translate_struct_member_node;
    visitor.unary_operation = translate_unary_operation_node;
    visitor.compound_literal = translate_compound_literal_node;
    visitor.extension_node = translate_extension_node;

    kefir_result_t res;
    KEFIR_RUN_EXTENSION(&res, mem, context, before_translate, node, builder,
                        KEFIR_AST_TRANSLATOR_CONTEXT_EXTENSION_TAG_LVALUE, &visitor);
    REQUIRE_OK(res);
    struct kefir_ast_translator_parameters param = {.mem = mem, .builder = builder, .context = context};
    REQUIRE_OK(KEFIR_AST_NODE_VISIT(&visitor, node, &param));
    KEFIR_RUN_EXTENSION(&res, mem, context, after_translate, node, builder,
                        KEFIR_AST_TRANSLATOR_CONTEXT_EXTENSION_TAG_LVALUE);
    REQUIRE_OK(res);
    return KEFIR_OK;
}
