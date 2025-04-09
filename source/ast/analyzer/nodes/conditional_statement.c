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
#include "kefir/ast/analyzer/declarator.h"
#include "kefir/ast/type_conv.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

kefir_result_t kefir_ast_analyze_conditional_statement_node(struct kefir_mem *mem,
                                                            const struct kefir_ast_context *context,
                                                            const struct kefir_ast_conditional_statement *node,
                                                            struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST conditional statement"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST base node"));

    REQUIRE_OK(kefir_ast_node_properties_init(&base->properties));
    base->properties.category = KEFIR_AST_NODE_CATEGORY_STATEMENT;

    REQUIRE(context->flow_control_tree != NULL,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->base.source_location,
                                   "Unable to use conditional statement in current context"));
    struct kefir_ast_flow_control_structure *direct_parent = NULL;
    REQUIRE_OK(kefir_ast_flow_control_tree_top(context->flow_control_tree, &direct_parent));
    struct kefir_ast_flow_control_structure_associated_scopes associated_scopes;
    REQUIRE_OK(context->push_block(mem, context, &associated_scopes.ordinary_scope, &associated_scopes.tag_scope));
    REQUIRE_OK(kefir_ast_flow_control_tree_push(mem, context->flow_control_tree, KEFIR_AST_FLOW_CONTROL_STRUCTURE_IF,
                                                &associated_scopes,
                                                &base->properties.statement_props.flow_control_statement));

    base->properties.statement_props.flow_control_statement->value.conditional.thenBranchEnd =
        kefir_ast_flow_control_point_alloc(mem, context->flow_control_tree, direct_parent);
    REQUIRE(base->properties.statement_props.flow_control_statement->value.conditional.thenBranchEnd != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST flow control point"));

    if (node->elseBranch != NULL) {
        base->properties.statement_props.flow_control_statement->value.conditional.elseBranchEnd =
            kefir_ast_flow_control_point_alloc(mem, context->flow_control_tree, direct_parent);
        REQUIRE(base->properties.statement_props.flow_control_statement->value.conditional.elseBranchEnd != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST flow control point"));
    }

    REQUIRE_OK(kefir_ast_analyze_node(mem, context, node->condition));
    REQUIRE(node->condition->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->condition->source_location,
                                   "Expected if statement condition to be scalar expression"));

    const struct kefir_ast_type *condition_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->type_bundle, node->condition->properties.type);
    REQUIRE(KEFIR_AST_TYPE_IS_SCALAR_TYPE(condition_type),
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->condition->source_location,
                                   "Expected if statement condition to be scalar expression"));

    REQUIRE_OK(kefir_ast_analyze_node(mem, context, node->thenBranch));
    REQUIRE(node->thenBranch->properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT ||
                node->thenBranch->properties.category == KEFIR_AST_NODE_CATEGORY_INLINE_ASSEMBLY,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->thenBranch->source_location,
                                   "Expected the first if branch to be a statement"));

    if (node->elseBranch != NULL) {
        REQUIRE_OK(kefir_ast_analyze_node(mem, context, node->elseBranch));
        REQUIRE(node->elseBranch->properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT ||
                    node->elseBranch->properties.category == KEFIR_AST_NODE_CATEGORY_INLINE_ASSEMBLY,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->elseBranch->source_location,
                                       "Expected the second if branch to be a statement"));
    }

    REQUIRE_OK(kefir_ast_flow_control_tree_pop(context->flow_control_tree));
    REQUIRE_OK(context->pop_block(mem, context));
    return KEFIR_OK;
}
