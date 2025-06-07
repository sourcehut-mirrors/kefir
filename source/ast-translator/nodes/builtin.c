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

#include "kefir/ast-translator/translator_impl.h"
#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/typeconv.h"
#include "kefir/ast-translator/lvalue.h"
#include "kefir/ast-translator/layout.h"
#include "kefir/ast-translator/type.h"
#include "kefir/ast-translator/temporaries.h"
#include "kefir/ast/type_conv.h"
#include "kefir/ast-translator/util.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"
#include <math.h>

static kefir_result_t resolve_vararg(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                     struct kefir_irbuilder_block *builder, const struct kefir_ast_node_base *vararg) {
    if (vararg->properties.type->tag == KEFIR_AST_TYPE_SCALAR_POINTER) {
        REQUIRE_OK(kefir_ast_translate_expression(mem, vararg, builder, context));
    } else {
        REQUIRE_OK(kefir_ast_translate_lvalue(mem, context, builder, vararg));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_builtin_node(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                                struct kefir_irbuilder_block *builder,
                                                const struct kefir_ast_builtin *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST builtin node"));

    const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
    switch (node->builtin) {
        case KEFIR_AST_BUILTIN_VA_START: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, vararg, iter->value);
            REQUIRE_OK(resolve_vararg(mem, context, builder, vararg));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VARARG_START, 0));
        } break;

        case KEFIR_AST_BUILTIN_VA_END: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, vararg, iter->value);
            REQUIRE_OK(resolve_vararg(mem, context, builder, vararg));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VARARG_END, 0));
        } break;

        case KEFIR_AST_BUILTIN_VA_ARG: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, vararg, iter->value);
            REQUIRE_OK(resolve_vararg(mem, context, builder, vararg));
            if (node->base.properties.expression_props.temporary_identifier.scoped_id != NULL) {
                REQUIRE_OK(kefir_ast_translator_fetch_temporary(
                    mem, context, builder, &node->base.properties.expression_props.temporary_identifier));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_NULL_REF, 0));
            }

            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, type_arg, iter->value);

            kefir_id_t va_type_id;
            struct kefir_irbuilder_type type_builder;
            struct kefir_ast_type_layout *type_layout;

            struct kefir_ir_type *va_type = kefir_ir_module_new_type(mem, context->module, 0, &va_type_id);
            REQUIRE_OK(kefir_irbuilder_type_init(mem, &type_builder, va_type));
            kefir_result_t res = kefir_ast_translate_object_type(mem, context->ast_context, type_arg->properties.type,
                                                                 0, context->environment, &type_builder, &type_layout,
                                                                 &node->base.source_location);
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_IRBUILDER_TYPE_FREE(&type_builder);
                return res;
            });

            kefir_size_t type_layout_idx = type_layout->value;

            res = kefir_ast_type_layout_free(mem, type_layout);
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_IRBUILDER_TYPE_FREE(&type_builder);
                return res;
            });
            REQUIRE_OK(KEFIR_IRBUILDER_TYPE_FREE(&type_builder));

            REQUIRE_OK(
                KEFIR_IRBUILDER_BLOCK_APPENDU32(builder, KEFIR_IR_OPCODE_VARARG_GET, va_type_id, type_layout_idx));
        } break;

        case KEFIR_AST_BUILTIN_VA_COPY: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, dst_vararg, iter->value);
            REQUIRE_OK(resolve_vararg(mem, context, builder, dst_vararg));

            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, src_vararg, iter->value);
            REQUIRE_OK(resolve_vararg(mem, context, builder, src_vararg));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VARARG_COPY, 0));
        } break;

        case KEFIR_AST_BUILTIN_ALLOCA: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, size, iter->value);
            REQUIRE_OK(kefir_ast_translate_expression(mem, size, builder, context));
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                    size->properties.type,
                                                    context->ast_context->type_traits->size_type));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_STACK_ALLOC, 1));
        } break;

        case KEFIR_AST_BUILTIN_ALLOCA_WITH_ALIGN:
        case KEFIR_AST_BUILTIN_ALLOCA_WITH_ALIGN_AND_MAX: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, size, iter->value);
            REQUIRE_OK(kefir_ast_translate_expression(mem, size, builder, context));
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                    size->properties.type,
                                                    context->ast_context->type_traits->size_type));
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, alignment, iter->value);
            REQUIRE_OK(kefir_ast_translate_expression(mem, alignment, builder, context));
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                    size->properties.type,
                                                    context->ast_context->type_traits->size_type));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_STACK_ALLOC, 1));
        } break;

        case KEFIR_AST_BUILTIN_OFFSETOF: {
            REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(KEFIR_AST_NODE_BASE(node),
                                                             KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_INVALID_STATE, &node->base.source_location,
                                           "Unexpected constant expression value"));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(
                builder, KEFIR_IR_OPCODE_UINT_CONST,
                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(KEFIR_AST_NODE_BASE(node))->integer));
        } break;

        case KEFIR_AST_BUILTIN_TYPES_COMPATIBLE: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, type1_node, iter->value);
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, type2_node, iter->value);

            const struct kefir_ast_type *type1 = kefir_ast_unqualified_type(type1_node->properties.type);
            const struct kefir_ast_type *type2 = kefir_ast_unqualified_type(type2_node->properties.type);

            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(
                builder, KEFIR_IR_OPCODE_UINT_CONST,
                KEFIR_AST_TYPE_COMPATIBLE(context->ast_context->type_traits, type1, type2) ? 1 : 0));
        } break;

        case KEFIR_AST_BUILTIN_CHOOSE_EXPRESSION: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, cond_node, iter->value);
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, expr1_node, iter->value);
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, expr2_node, iter->value);

            REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(cond_node, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &cond_node->source_location,
                                           "Expected a constant expression evaluating to an integer"));

            kefir_bool_t condition = KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(cond_node)->integer != 0;
            const struct kefir_ast_type *cond_node_unqualified_type =
                kefir_ast_unqualified_type(cond_node->properties.type);
            if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(cond_node_unqualified_type)) {
                kefir_bool_t is_zero;
                REQUIRE_OK(
                    kefir_bigint_is_zero(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(cond_node)->bitprecise, &is_zero));
                condition = !is_zero;
            }

            if (condition) {
                REQUIRE_OK(kefir_ast_translate_expression(mem, expr1_node, builder, context));
            } else {
                REQUIRE_OK(kefir_ast_translate_expression(mem, expr2_node, builder, context));
            }
        } break;

        case KEFIR_AST_BUILTIN_CONSTANT: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, iter->value);

            if (!KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION(node)) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, 0));
            } else {
                const struct kefir_ast_constant_expression_value *node_value =
                    KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node);
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(
                    builder, KEFIR_IR_OPCODE_UINT_CONST,
                    (node_value->klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER ||
                     node_value->klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT ||
                     node_value->klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT ||
                     (node_value->klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS &&
                      node_value->pointer.type != KEFIR_AST_CONSTANT_EXPRESSION_POINTER_IDENTIFER))
                        ? 1
                        : 0));
            }
        } break;

        case KEFIR_AST_BUILTIN_CLASSIFY_TYPE: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, iter->value);

            kefir_int_t klass;
            REQUIRE_OK(kefir_ast_type_classify(node->properties.type, &klass));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT_CONST, klass));
        } break;

        case KEFIR_AST_BUILTIN_INFINITY_FLOAT32:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IR_OPCODE_FLOAT32_CONST, INFINITY, 0.0f));
            break;

        case KEFIR_AST_BUILTIN_INFINITY_FLOAT64:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST, INFINITY));
            break;

        case KEFIR_AST_BUILTIN_INFINITY_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST, INFINITY));
            break;

        case KEFIR_AST_BUILTIN_NAN_FLOAT32: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, arg1_node, iter->value);
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(
                builder, KEFIR_IR_OPCODE_FLOAT32_CONST,
                nan(arg1_node->properties.expression_props.string_literal.content), 0.0f));
        } break;

        case KEFIR_AST_BUILTIN_NAN_FLOAT64: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, arg1_node, iter->value);
            REQUIRE_OK(
                KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST,
                                                nan(arg1_node->properties.expression_props.string_literal.content)));
        } break;

        case KEFIR_AST_BUILTIN_NAN_LONG_DOUBLE: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, arg1_node, iter->value);
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(
                builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST,
                nan(arg1_node->properties.expression_props.string_literal.content)));
        } break;

        case KEFIR_AST_BUILTIN_ADD_OVERFLOW:
        case KEFIR_AST_BUILTIN_SUB_OVERFLOW:
        case KEFIR_AST_BUILTIN_MUL_OVERFLOW: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, arg1_node, iter->value);
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, arg2_node, iter->value);
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, ptr_node, iter->value);

            const struct kefir_ast_type *arg1_type = kefir_ast_unqualified_type(arg1_node->properties.type);
            const struct kefir_ast_type *arg2_type = kefir_ast_unqualified_type(arg2_node->properties.type);
            const struct kefir_ast_type *ptr_type =
                KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, ptr_node->properties.type);
            const struct kefir_ast_type *result_type = kefir_ast_unqualified_type(ptr_type->referenced_type);

            kefir_bool_t arg1_signed, arg2_signed, result_signed;
            REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, arg1_type, &arg1_signed));
            REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, arg2_type, &arg2_signed));
            REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_type, &result_signed));

            kefir_id_t overflow_type_arg_id;
            struct kefir_ir_type *overflow_type_arg =
                kefir_ir_module_new_type(mem, context->module, 0, &overflow_type_arg_id);
            REQUIRE(overflow_type_arg != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate IR type"));
            struct kefir_irbuilder_type overflow_type_builder;
            REQUIRE_OK(kefir_irbuilder_type_init(mem, &overflow_type_builder, overflow_type_arg));

            kefir_result_t res =
                kefir_ast_translate_object_type(mem, context->ast_context, arg1_type, 0, context->environment,
                                                &overflow_type_builder, NULL, &arg1_node->source_location);
            REQUIRE_CHAIN(&res,
                          kefir_ast_translate_object_type(mem, context->ast_context, arg2_type, 0, context->environment,
                                                          &overflow_type_builder, NULL, &arg1_node->source_location));
            REQUIRE_CHAIN(
                &res, kefir_ast_translate_object_type(mem, context->ast_context, result_type, 0, context->environment,
                                                      &overflow_type_builder, NULL, &arg1_node->source_location));
            REQUIRE_ELSE(res == KEFIR_OK, {
                overflow_type_builder.free(&overflow_type_builder);
                return res;
            });
            REQUIRE_OK(overflow_type_builder.free(&overflow_type_builder));

            REQUIRE_OK(kefir_ast_translate_expression(mem, arg1_node, builder, context));
            REQUIRE_OK(kefir_ast_translate_expression(mem, arg2_node, builder, context));
            REQUIRE_OK(kefir_ast_translate_expression(mem, ptr_node, builder, context));

            kefir_uint32_t signedness_flag = 0;
            if (arg1_signed) {
                signedness_flag |= 1;
            }
            if (arg2_signed) {
                signedness_flag |= 1 << 1;
            }
            if (result_signed) {
                signedness_flag |= 1 << 2;
            }
            if (node->builtin == KEFIR_AST_BUILTIN_ADD_OVERFLOW) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU32_4(builder, KEFIR_IR_OPCODE_ADD_OVERFLOW,
                                                             overflow_type_arg_id, 0, signedness_flag, 0));
            } else if (node->builtin == KEFIR_AST_BUILTIN_SUB_OVERFLOW) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU32_4(builder, KEFIR_IR_OPCODE_SUB_OVERFLOW,
                                                             overflow_type_arg_id, 0, signedness_flag, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU32_4(builder, KEFIR_IR_OPCODE_MUL_OVERFLOW,
                                                             overflow_type_arg_id, 0, signedness_flag, 0));
            }
        } break;
    }
    return KEFIR_OK;
}
