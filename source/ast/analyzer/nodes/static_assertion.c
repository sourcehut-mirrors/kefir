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

#include <string.h>
#include "kefir/ast/analyzer/nodes.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

kefir_result_t kefir_ast_analyze_static_assertion_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                       const struct kefir_ast_static_assertion *node,
                                                       struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST static assertion"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST base node"));

    REQUIRE_OK(kefir_ast_node_properties_init(&base->properties));
    base->properties.category = KEFIR_AST_NODE_CATEGORY_INIT_DECLARATOR;
    base->properties.declaration_props.static_assertion = true;
    REQUIRE_OK(kefir_ast_analyze_node(mem, context, node->condition));
    REQUIRE_OK(kefir_ast_analyze_node(mem, context, KEFIR_AST_NODE_BASE(node->string)));

    REQUIRE(node->condition->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->condition->source_location,
                                   "Expected static assert condition expression"));
    REQUIRE(
        node->string->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION &&
            node->string->base.properties.expression_props.string_literal.content != NULL,
        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->string->base.source_location, "Expected string literal"));

    REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->condition, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->condition->source_location,
                                   "Static assertion condition shall be an integral constant expression"));
    REQUIRE(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->condition)->integer != 0,
            KEFIR_SET_SOURCE_ERRORF(KEFIR_STATIC_ASSERT, &node->base.source_location, "%s",
                                    node->string->base.properties.expression_props.string_literal.content));
    return KEFIR_OK;
}
