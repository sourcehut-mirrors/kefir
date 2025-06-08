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
#include "kefir/ast/node_base.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

static kefir_result_t match_switch_statement(const struct kefir_ast_flow_control_structure *statement, void *payload,
                                             kefir_bool_t *result) {
    UNUSED(payload);
    REQUIRE(statement != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST flow control statement"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid pointer to result"));

    *result = statement->type == KEFIR_AST_FLOW_CONTROL_STRUCTURE_SWITCH;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_analyze_case_statement_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                     const struct kefir_ast_case_statement *node,
                                                     struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST case statement"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST base node"));

    REQUIRE_OK(kefir_ast_node_properties_init(&base->properties));
    base->properties.category = KEFIR_AST_NODE_CATEGORY_STATEMENT;

    REQUIRE(context->flow_control_tree != NULL,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->base.source_location,
                                   "Unable to use case statement in current context"));

    struct kefir_ast_flow_control_structure *switch_statement = NULL, *direct_parent = NULL;
    kefir_result_t res = kefir_ast_flow_control_tree_traverse(context->flow_control_tree, match_switch_statement, NULL,
                                                              &switch_statement);
    if (res == KEFIR_NOT_FOUND) {
        return KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->base.source_location,
                                      "Cannot use case statement outside of switch");
    } else {
        REQUIRE_OK(res);
    }
    base->properties.statement_props.flow_control_statement = switch_statement;

    REQUIRE_OK(kefir_ast_flow_control_tree_top(context->flow_control_tree, &direct_parent));

    if (node->range_end_expression != NULL) {
        REQUIRE(node->expression != NULL,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                "End range for case statement may only be specified when expression is non-NULL"));
        REQUIRE_OK(kefir_ast_analyze_node(mem, context, node->expression));
        REQUIRE_OK(kefir_ast_analyze_node(mem, context, node->range_end_expression));
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->expression, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->expression->source_location,
                                       "Expected AST case label to be an integral constant expression"));
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->range_end_expression,
                                                         KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->range_end_expression->source_location,
                                       "Expected AST case label to be an integral constant expression"));
        REQUIRE(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->expression)->integer !=
                    KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->range_end_expression)->integer,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->range_end_expression->source_location,
                                       "Expected AST case label range to be non-empty"));
        struct kefir_ast_flow_control_point *point =
            kefir_ast_flow_control_point_alloc(mem, context->flow_control_tree, direct_parent);
        REQUIRE(point != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST flow control point"));

        struct kefir_ast_node_base *begin_node = node->expression;
        struct kefir_ast_node_base *end_node = node->range_end_expression;
        kefir_ast_constant_expression_int_t begin = KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->expression)->integer;
        kefir_ast_constant_expression_int_t end =
            KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->range_end_expression)->integer;
        if (end < begin) {
            kefir_ast_constant_expression_int_t tmp = begin;
            struct kefir_ast_node_base *tmp_node = begin_node;
            begin = end;
            begin_node = end_node;
            end = tmp;
            end_node = tmp_node;
        }

        const kefir_size_t case_identifier = switch_statement->value.switchStatement.num_of_cases;
        REQUIRE_OK(kefir_hashtree_insert(mem, &switch_statement->value.switchStatement.case_flow_control_points,
                                         (kefir_hashtree_key_t) case_identifier, (kefir_hashtree_value_t) point));
        REQUIRE_OK(kefir_hashtree_insert(mem, &switch_statement->value.switchStatement.case_label_nodes,
                                         (kefir_hashtree_key_t) case_identifier,
                                         (kefir_hashtree_value_t) KEFIR_AST_NODE_REF(begin_node)));
        REQUIRE_OK(kefir_hashtree_insert(mem, &switch_statement->value.switchStatement.case_range_end_nodes,
                                         (kefir_hashtree_key_t) case_identifier,
                                         (kefir_hashtree_key_t) KEFIR_AST_NODE_REF(end_node)));
        base->properties.statement_props.target_flow_control_point = point;

        switch_statement->value.switchStatement.num_of_cases++;
    } else if (node->expression != NULL) {
        REQUIRE_OK(kefir_ast_analyze_node(mem, context, node->expression));
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->expression, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->expression->source_location,
                                       "Expected AST case label to be an integral constant expression"));

        struct kefir_hashtree_node_iterator iter;
        for (const struct kefir_hashtree_node *switchCase =
                 kefir_hashtree_iter(&switch_statement->value.switchStatement.case_label_nodes, &iter);
             switchCase != NULL; switchCase = kefir_hashtree_next(&iter)) {

            ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, other_case_label, iter.node->value);
            REQUIRE(
                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->expression)->bitprecise == NULL &&
                    KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(other_case_label)->bitprecise == NULL,
                KEFIR_SET_ERROR(KEFIR_NOT_IMPLEMENTED, "Bit-precise integers in case labels are not implemented yet"));
            REQUIRE(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->expression)->integer !=
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(other_case_label)->integer,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->expression->source_location,
                                           "Cannot duplicate case statement constants"));
        }

        struct kefir_ast_flow_control_point *point =
            kefir_ast_flow_control_point_alloc(mem, context->flow_control_tree, direct_parent);
        REQUIRE(point != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST flow control point"));
        const kefir_size_t case_identifier = switch_statement->value.switchStatement.num_of_cases;
        REQUIRE_OK(kefir_hashtree_insert(mem, &switch_statement->value.switchStatement.case_flow_control_points,
                                         (kefir_hashtree_key_t) case_identifier, (kefir_hashtree_value_t) point));
        REQUIRE_OK(kefir_hashtree_insert(mem, &switch_statement->value.switchStatement.case_label_nodes,
                                         (kefir_hashtree_key_t) case_identifier,
                                         (kefir_hashtree_value_t) KEFIR_AST_NODE_REF(node->expression)));
        base->properties.statement_props.target_flow_control_point = point;
        switch_statement->value.switchStatement.num_of_cases++;
    } else {
        REQUIRE(switch_statement->value.switchStatement.defaultCase == NULL,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->base.source_location,
                                       "Switch statement cannot have multiple default labels"));
        switch_statement->value.switchStatement.defaultCase =
            kefir_ast_flow_control_point_alloc(mem, context->flow_control_tree, direct_parent);
        REQUIRE(switch_statement->value.switchStatement.defaultCase != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST flow control point"));
        base->properties.statement_props.target_flow_control_point =
            switch_statement->value.switchStatement.defaultCase;
    }

    REQUIRE_OK(kefir_ast_analyze_node(mem, context, node->statement));
    REQUIRE(node->statement->properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT ||
                node->statement->properties.category == KEFIR_AST_NODE_CATEGORY_INLINE_ASSEMBLY,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->statement->source_location,
                                   "Expected AST statement node to be associated with the case"));
    return KEFIR_OK;
}
