/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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
            REQUIRE(kefir_list_length(&node->arguments) == 2,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                           "va_start builtin invocation should have exactly two parameters"));
            const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, vararg_list, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, vararg_list));
            REQUIRE(vararg_list->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &vararg_list->source_location,
                                           "Expected an expression referencing va_list"));
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, paramN, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, paramN));
            REQUIRE(paramN->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION &&
                        paramN->properties.expression_props.identifier != NULL,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &paramN->source_location,
                                           "Expected parameter identifier"));
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
            base->properties.expression_props.constant_expression = field->properties.member_designator.constant;
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
            base->properties.expression_props.constant_expression = true;
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

            struct kefir_ast_constant_expression_value cond_value;
            REQUIRE_OK(kefir_ast_constant_expression_value_evaluate(mem, context, cond_node, &cond_value));

            REQUIRE(cond_value.klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &cond_node->source_location,
                                           "Expected a constant expression evaluating to an integer"));

            if (cond_value.integer != 0) {
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
            base->properties.expression_props.constant_expression = true;
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
            base->properties.expression_props.constant_expression = true;
        } break;
    }
    return KEFIR_OK;
}
