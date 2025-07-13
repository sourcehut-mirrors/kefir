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

#include "kefir/ast/analyzer/nodes.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/ast/analyzer/member_designator.h"
#include "kefir/ast/type_conv.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"
#include "kefir/ast/downcast.h"

kefir_result_t kefir_ast_analyze_builtin_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                              const struct kefir_ast_builtin *node, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST builtin"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST base node"));

    REQUIRE_OK(kefir_ast_node_properties_init(&base->properties));
    base->properties.category = KEFIR_AST_NODE_CATEGORY_EXPRESSION;
    switch (node->builtin) {
        case KEFIR_AST_BUILTIN_VA_START: {
            REQUIRE(
                context->surrounding_function != NULL && context->surrounding_function->type->function_type.ellipsis,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                       "va_start builtin cannot be used outside of vararg function"));
            REQUIRE(kefir_list_length(&node->arguments) >= 1,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                           "va_start builtin invocation should have at least one parameter"));
            const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, vararg_list, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, vararg_list));
            REQUIRE(vararg_list->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &vararg_list->source_location,
                                           "Expected an expression referencing va_list"));
            base->properties.type = kefir_ast_type_void();
        } break;

        case KEFIR_AST_BUILTIN_VA_END: {
            REQUIRE(kefir_list_length(&node->arguments) == 1,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                           "va_end builtin invocation should have exactly one parameter"));
            const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, vararg_list, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, vararg_list));
            REQUIRE(vararg_list->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &vararg_list->source_location,
                                           "Expected an expression referencing va_list"));
            base->properties.type = kefir_ast_type_void();
        } break;

        case KEFIR_AST_BUILTIN_VA_ARG: {
            REQUIRE(kefir_list_length(&node->arguments) == 2,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                           "va_arg builtin invocation should have exactly two parameters"));
            const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, vararg_list, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, vararg_list));
            REQUIRE(vararg_list->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &vararg_list->source_location,
                                           "Expected an expression referencing va_list"));
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, type, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, type));
            REQUIRE(type->properties.category == KEFIR_AST_NODE_CATEGORY_TYPE,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &type->source_location, "Expected type name"));
            REQUIRE(!KEFIR_AST_TYPE_IS_INCOMPLETE(type->properties.type),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &type->source_location, "Expected complete type"));
            base->properties.type = type->properties.type;

            const struct kefir_ast_type *unqualified_type = kefir_ast_unqualified_type(type->properties.type);
            if (KEFIR_AST_TYPE_IS_AGGREGATE_TYPE(unqualified_type) ||
                KEFIR_AST_TYPE_IS_COMPLEX_TYPE(unqualified_type)) {
                REQUIRE_OK(context->allocate_temporary_value(
                    mem, context, unqualified_type, KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN, NULL,
                    &base->source_location, &base->properties.expression_props.temporary_identifier));
            }
        } break;

        case KEFIR_AST_BUILTIN_VA_COPY: {
            REQUIRE(kefir_list_length(&node->arguments) == 2,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                           "va_copy builtin invocation should have exactly two parameters"));
            const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, vararg_list, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, vararg_list));
            REQUIRE(vararg_list->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &vararg_list->source_location,
                                           "Expected an expression referencing va_list"));
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, vararg2_list, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, vararg2_list));
            REQUIRE(vararg2_list->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &vararg2_list->source_location,
                                           "Expected an expression referencing va_list"));
            base->properties.type = kefir_ast_type_void();
        } break;

        case KEFIR_AST_BUILTIN_ALLOCA: {
            REQUIRE(kefir_list_length(&node->arguments) == 1,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                           "alloca builtin invocation should have exactly one parameter"));
            const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, size, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, size));
            REQUIRE(size->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION &&
                        KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(size->properties.type),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &size->source_location,
                                           "Expected an integral expression"));
            base->properties.type = kefir_ast_type_pointer(mem, context->type_bundle, kefir_ast_type_void());
        } break;

        case KEFIR_AST_BUILTIN_ALLOCA_WITH_ALIGN: {
            REQUIRE(kefir_list_length(&node->arguments) == 2,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                           "alloca_with_align builtin invocation should have exactly two parameters"));
            const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, size, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, size));
            REQUIRE(size->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION &&
                        KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(size->properties.type),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &size->source_location,
                                           "Expected an integral expression"));
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, alignment, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, alignment));
            REQUIRE(alignment->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION &&
                        KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(alignment->properties.type),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &alignment->source_location,
                                           "Expected an integral expression"));
            base->properties.type = kefir_ast_type_pointer(mem, context->type_bundle, kefir_ast_type_void());
        } break;

        case KEFIR_AST_BUILTIN_ALLOCA_WITH_ALIGN_AND_MAX: {
            REQUIRE(kefir_list_length(&node->arguments) == 3,
                    KEFIR_SET_SOURCE_ERROR(
                        KEFIR_ANALYSIS_ERROR, &base->source_location,
                        "alloca_with_align_and_max builtin invocation should have exactly three parameters"));
            const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, size, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, size));
            REQUIRE(size->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION &&
                        KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(size->properties.type),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &size->source_location,
                                           "Expected an integral expression"));
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, alignment, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, alignment));
            REQUIRE(alignment->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION &&
                        KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(alignment->properties.type),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &alignment->source_location,
                                           "Expected an integral expression"));
            base->properties.type = kefir_ast_type_pointer(mem, context->type_bundle, kefir_ast_type_void());
        } break;

        case KEFIR_AST_BUILTIN_OFFSETOF: {
            REQUIRE(kefir_list_length(&node->arguments) == 2,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                           "offset builtin invocation should have exactly two parameters"));
            const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, offset_base, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, offset_base));
            REQUIRE(
                offset_base->properties.category == KEFIR_AST_NODE_CATEGORY_TYPE,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &offset_base->source_location, "Expected a type name"));
            kefir_list_next(&iter);

            const struct kefir_ast_type *base_type = kefir_ast_unqualified_type(offset_base->properties.type);
            REQUIRE(base_type->tag == KEFIR_AST_TYPE_STRUCTURE || base_type->tag == KEFIR_AST_TYPE_UNION,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &offset_base->source_location,
                                           "Expected structure or union type"));

            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, field, iter->value);
            REQUIRE_OK(kefir_ast_analyze_member_designator(mem, context, base_type, field));

            base->properties.type = context->type_traits->size_type;
        } break;

        case KEFIR_AST_BUILTIN_TYPES_COMPATIBLE: {
            REQUIRE(kefir_list_length(&node->arguments) == 2,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                           "type compatible builtin invocation should have exactly two parameters"));

            const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, type1_node, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, type1_node));
            REQUIRE(type1_node->properties.category == KEFIR_AST_NODE_CATEGORY_TYPE,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &type1_node->source_location, "Expected a type name"));
            kefir_list_next(&iter);

            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, type2_node, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, type2_node));
            REQUIRE(type2_node->properties.category == KEFIR_AST_NODE_CATEGORY_TYPE,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &type2_node->source_location, "Expected a type name"));

            base->properties.type = kefir_ast_type_signed_int();
        } break;

        case KEFIR_AST_BUILTIN_CHOOSE_EXPRESSION: {
            REQUIRE(
                kefir_list_length(&node->arguments) == 3,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                       "choose expression builtin invocation should have exactly three parameters"));

            const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, cond_node, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, cond_node));
            REQUIRE(cond_node->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION &&
                        cond_node->properties.expression_props.constant_expression,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &cond_node->source_location,
                                           "Expected a constant expression"));
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
                REQUIRE_OK(kefir_ast_analyze_node(mem, context, expr1_node));
                REQUIRE_OK(kefir_ast_node_properties_clone(&base->properties, &expr1_node->properties));
            } else {
                REQUIRE_OK(kefir_ast_analyze_node(mem, context, expr2_node));
                REQUIRE_OK(kefir_ast_node_properties_clone(&base->properties, &expr2_node->properties));
            }
        } break;

        case KEFIR_AST_BUILTIN_CONSTANT: {
            REQUIRE(kefir_list_length(&node->arguments) == 1,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                           "constant builtin invocation should have exactly one parameter"));

            const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, node));
            REQUIRE(node->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->source_location, "Expected an expression"));
            kefir_list_next(&iter);

            base->properties.type = kefir_ast_type_signed_int();
        } break;

        case KEFIR_AST_BUILTIN_CLASSIFY_TYPE: {
            REQUIRE(kefir_list_length(&node->arguments) == 1,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                           "classify type builtin invocation should have exactly one parameter"));

            const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, node));
            REQUIRE(node->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->source_location, "Expected an expression"));
            kefir_list_next(&iter);

            base->properties.type = kefir_ast_type_signed_int();
        } break;

        case KEFIR_AST_BUILTIN_INFINITY_FLOAT32: {
            REQUIRE(kefir_list_length(&node->arguments) == 0,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                           "inff type builtin invocation should have no parameters"));

            base->properties.type = kefir_ast_type_float();
        } break;

        case KEFIR_AST_BUILTIN_INFINITY_FLOAT64: {
            REQUIRE(kefir_list_length(&node->arguments) == 0,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                           "inf type builtin invocation should have no parameters"));

            base->properties.type = kefir_ast_type_double();
        } break;

        case KEFIR_AST_BUILTIN_INFINITY_LONG_DOUBLE: {
            REQUIRE(kefir_list_length(&node->arguments) == 0,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                           "infl type builtin invocation should have no parameters"));

            base->properties.type = kefir_ast_type_long_double();
        } break;

        case KEFIR_AST_BUILTIN_NAN_FLOAT32: {
            REQUIRE(kefir_list_length(&node->arguments) == 1,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                           "nanf builtin invocation should have single string literal parameter"));
            const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, arg1_node, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, arg1_node));
            REQUIRE(
                arg1_node->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION &&
                    (arg1_node->properties.expression_props.string_literal.type == KEFIR_AST_STRING_LITERAL_MULTIBYTE ||
                     arg1_node->properties.expression_props.string_literal.type == KEFIR_AST_STRING_LITERAL_UNICODE8),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &arg1_node->source_location,
                                       "Expected multibyte string literal"));

            base->properties.type = kefir_ast_type_float();
        } break;

        case KEFIR_AST_BUILTIN_NAN_FLOAT64: {
            REQUIRE(kefir_list_length(&node->arguments) == 1,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                           "nanf builtin invocation should have single string literal parameter"));
            const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, arg1_node, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, arg1_node));
            REQUIRE(
                arg1_node->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION &&
                    (arg1_node->properties.expression_props.string_literal.type == KEFIR_AST_STRING_LITERAL_MULTIBYTE ||
                     arg1_node->properties.expression_props.string_literal.type == KEFIR_AST_STRING_LITERAL_UNICODE8),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &arg1_node->source_location,
                                       "Expected multibyte string literal"));

            base->properties.type = kefir_ast_type_double();
        } break;

        case KEFIR_AST_BUILTIN_NAN_LONG_DOUBLE: {
            REQUIRE(kefir_list_length(&node->arguments) == 1,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                           "nanf builtin invocation should have single string literal parameter"));
            const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, arg1_node, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, arg1_node));
            REQUIRE(
                arg1_node->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION &&
                    (arg1_node->properties.expression_props.string_literal.type == KEFIR_AST_STRING_LITERAL_MULTIBYTE ||
                     arg1_node->properties.expression_props.string_literal.type == KEFIR_AST_STRING_LITERAL_UNICODE8),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &arg1_node->source_location,
                                       "Expected multibyte string literal"));

            base->properties.type = kefir_ast_type_long_double();
        } break;

        case KEFIR_AST_BUILTIN_ADD_OVERFLOW:
        case KEFIR_AST_BUILTIN_SUB_OVERFLOW:
        case KEFIR_AST_BUILTIN_MUL_OVERFLOW: {
            REQUIRE(
                kefir_list_length(&node->arguments) == 3,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                       "Overflow arithmetics builtin invocation should have exactly three parameters"));

            const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, arg1_node, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, arg1_node));
            REQUIRE(arg1_node->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &arg1_node->source_location,
                                           "Expected an expression of integer type"));
            const struct kefir_ast_type *arg1_type = kefir_ast_unqualified_type(arg1_node->properties.type);
            REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(arg1_type),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &arg1_node->source_location,
                                           "Expected an expression of integer type"));
            kefir_list_next(&iter);

            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, arg2_node, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, arg2_node));
            REQUIRE(arg2_node->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &arg2_node->source_location,
                                           "Expected an expression of integer type"));
            const struct kefir_ast_type *arg2_type = kefir_ast_unqualified_type(arg2_node->properties.type);
            REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(arg2_type),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &arg2_node->source_location,
                                           "Expected an expression of integer type"));
            kefir_list_next(&iter);

            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, ptr_node, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, ptr_node));
            REQUIRE(ptr_node->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
                    KEFIR_SET_SOURCE_ERROR(
                        KEFIR_ANALYSIS_ERROR, &ptr_node->source_location,
                        "Expected a non-constant pointer to non-boolean & non-enumeration integral type"));
            const struct kefir_ast_type *ptr_type =
                KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->type_bundle, ptr_node->properties.type);
            REQUIRE(ptr_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER,
                    KEFIR_SET_SOURCE_ERROR(
                        KEFIR_ANALYSIS_ERROR, &ptr_node->source_location,
                        "Expected a non-constant pointer to non-boolean & non-enumeration integral type"));
            struct kefir_ast_type_qualification qualification;
            REQUIRE_OK(kefir_ast_type_retrieve_qualifications(&qualification, ptr_type->referenced_type));
            const struct kefir_ast_type *unqualified_type = kefir_ast_unqualified_type(ptr_type->referenced_type);
            REQUIRE(KEFIR_AST_TYPE_IS_NONENUM_INTEGRAL_TYPE(unqualified_type) &&
                        unqualified_type->tag != KEFIR_AST_TYPE_SCALAR_BOOL && !qualification.constant,
                    KEFIR_SET_SOURCE_ERROR(
                        KEFIR_ANALYSIS_ERROR, &ptr_node->source_location,
                        "Expected a non-constant pointer to non-boolean & non-enumeration integral type"));

            base->properties.type = kefir_ast_type_boolean();
        } break;
    }
    return KEFIR_OK;
}
