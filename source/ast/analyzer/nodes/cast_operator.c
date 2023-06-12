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

#include <string.h>
#include "kefir/ast/analyzer/nodes.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/ast/type_conv.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

kefir_result_t kefir_ast_analyze_cast_operator_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                    const struct kefir_ast_cast_operator *cast,
                                                    struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(cast != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST cast operator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));

    REQUIRE_OK(kefir_ast_analyze_node(mem, context, KEFIR_AST_NODE_BASE(cast->type_name)));
    REQUIRE_OK(kefir_ast_analyze_node(mem, context, cast->expr));
    REQUIRE(cast->expr->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &cast->base.source_location,
                                   "Cast operator operand shall be an expression"));

    REQUIRE_OK(kefir_ast_node_properties_init(&base->properties));
    const struct kefir_ast_type *expr_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->type_bundle, cast->expr->properties.type);
    const struct kefir_ast_type *cast_type = kefir_ast_unqualified_type(cast->type_name->base.properties.type);
    REQUIRE((KEFIR_AST_TYPE_IS_SCALAR_TYPE(expr_type) && KEFIR_AST_TYPE_IS_SCALAR_TYPE(cast_type)) ||
                cast->type_name->base.properties.type->tag == KEFIR_AST_TYPE_VOID,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &cast->base.source_location,
                                   "Cast should involve scalar types unless type name is void"));
    if (KEFIR_AST_TYPE_IS_FLOATING_POINT(cast_type)) {
        REQUIRE(expr_type->tag != KEFIR_AST_TYPE_SCALAR_POINTER,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &cast->expr->source_location,
                                       "Pointer cannot be cast to floating-point value"));
        if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(cast_type)) {
            REQUIRE_OK(context->allocate_temporary_value(mem, context, kefir_ast_type_long_double(), NULL,
                                                         &base->source_location,
                                                         &base->properties.expression_props.temp_identifier));
        }
    }
    if (KEFIR_AST_TYPE_IS_FLOATING_POINT(expr_type)) {
        REQUIRE(cast_type->tag != KEFIR_AST_TYPE_SCALAR_POINTER,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &cast->expr->source_location,
                                       "Floating-point value cannot be cast to pointer"));
    }

    base->properties.category = KEFIR_AST_NODE_CATEGORY_EXPRESSION;
    base->properties.type = cast_type;
    base->properties.expression_props.constant_expression = cast->expr->properties.expression_props.constant_expression;
    return KEFIR_OK;
}
