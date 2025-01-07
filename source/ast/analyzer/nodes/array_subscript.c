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
#include "kefir/ast/type_conv.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

kefir_result_t kefir_ast_analyze_array_subscript_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                      const struct kefir_ast_array_subscript *node,
                                                      struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST array subscript"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST base node"));

    REQUIRE_OK(kefir_ast_analyze_node(mem, context, node->array));
    REQUIRE_OK(kefir_ast_analyze_node(mem, context, node->subscript));

    REQUIRE(node->array->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->array->source_location,
                                   "Both array subscript operands shall be expressions"));
    REQUIRE(node->subscript->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->subscript->source_location,
                                   "Both array subscript operands shall be expressions"));

    const struct kefir_ast_type *array_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->type_bundle, node->array->properties.type);
    const struct kefir_ast_type *subcript_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->type_bundle, node->subscript->properties.type);
    const struct kefir_ast_type *type = NULL;

    struct kefir_ast_node_base *array_node;
    struct kefir_ast_node_base *subscript_node;
    if (array_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER) {
        REQUIRE(!node->array->properties.expression_props.atomic,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->array->source_location,
                                       "Array subscript cannot operate on atomic type"));
        REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(subcript_type),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->subscript->source_location,
                                       "Expected one of subscript operands to have integral type"));
        type = array_type->referenced_type;
        array_node = node->array;
        subscript_node = node->subscript;
    } else {
        REQUIRE(!node->subscript->properties.expression_props.atomic,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->subscript->source_location,
                                       "Array subscript cannot operate on atomic type"));
        REQUIRE(subcript_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->subscript->source_location,
                                       "Expected one of subscript operands to have pointer type"));
        REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(array_type),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->array->source_location,
                                       "Expected one of subscript operands to have integral type"));
        type = subcript_type->referenced_type;
        array_node = node->subscript;
        subscript_node = node->array;
    }
    REQUIRE_OK(kefir_ast_node_properties_init(&base->properties));
    base->properties.category = KEFIR_AST_NODE_CATEGORY_EXPRESSION;
    base->properties.type = type;
    base->properties.expression_props.lvalue = true;
    base->properties.expression_props.addressable = true;
    base->properties.expression_props.atomic = KEFIR_AST_TYPE_IS_ATOMIC(type);
    const struct kefir_ast_type *unqualified_type = kefir_ast_unqualified_type(type);
    if (base->properties.expression_props.atomic &&
        (KEFIR_AST_TYPE_IS_AGGREGATE_TYPE(unqualified_type) || KEFIR_AST_TYPE_IS_COMPLEX_TYPE(unqualified_type))) {
        REQUIRE_OK(context->allocate_temporary_value(mem, context, type, NULL, &base->source_location,
                                                     &base->properties.expression_props.temporary_identifier));
    }

    if (unqualified_type->tag == KEFIR_AST_TYPE_ARRAY) {
        kefir_bool_t array_const_ref = false;
        REQUIRE_OK(kefir_ast_node_is_lvalue_reference_constant(context, array_node, &array_const_ref));
        base->properties.expression_props.constant_expression =
            array_const_ref && subscript_node->properties.expression_props.constant_expression;
    }
    return KEFIR_OK;
}
