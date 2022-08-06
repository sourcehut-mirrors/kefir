/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/ast/type_conv.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"
#include "kefir/ast/downcast.h"

static kefir_result_t find_block_structure(const struct kefir_ast_flow_control_structure *stmt, void *payload,
                                           kefir_bool_t *result) {
    UNUSED(payload);
    REQUIRE(stmt != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST flow control structure"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to result"));

    *result = stmt->type == KEFIR_AST_FLOW_CONTROL_STRUCTURE_BLOCK;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_analyze_inline_assembly_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                      const struct kefir_ast_inline_assembly *node,
                                                      struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST compound statement"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST base node"));

    REQUIRE_OK(kefir_ast_node_properties_init(&base->properties));
    base->properties.category = KEFIR_AST_NODE_CATEGORY_INLINE_ASSEMBLY;

    if (context->flow_control_tree != NULL) {
        for (const struct kefir_list_entry *iter = kefir_list_head(&node->outputs); iter != NULL;
             kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(struct kefir_ast_inline_assembly_parameter *, param, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, param->parameter));
            REQUIRE(param->parameter->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION &&
                        param->parameter->properties.expression_props.lvalue,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->base.source_location,
                                           "Expected lvalue expression as inline assembly output"));
        }

        for (const struct kefir_list_entry *iter = kefir_list_head(&node->inputs); iter != NULL;
             kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(struct kefir_ast_inline_assembly_parameter *, param, iter->value);
            REQUIRE_OK(kefir_ast_analyze_node(mem, context, param->parameter));
            REQUIRE(param->parameter->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->base.source_location,
                                           "Expected an expression as inline assembly input"));
        }

        struct kefir_ast_flow_control_structure *block = NULL;
        kefir_result_t res =
            kefir_ast_flow_control_tree_traverse(context->flow_control_tree, find_block_structure, NULL, &block);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
        } else {
            return KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                          "Expected inline assembly statement to be enclosed into a code block");
        }
        REQUIRE_OK(context->current_flow_control_point(mem, context,
                                                       &base->properties.inline_assembly.origin_flow_control_point));

        REQUIRE_OK(kefir_ast_flow_control_block_add_branching_point(mem, block,
                                                                    &base->properties.inline_assembly.branching_point));
        for (const struct kefir_list_entry *iter = kefir_list_head(&node->jump_labels); iter != NULL;
             kefir_list_next(&iter)) {

            ASSIGN_DECL_CAST(const char *, jump_label, iter->value);

            const struct kefir_ast_scoped_identifier *scoped_id = NULL;
            REQUIRE_OK(
                context->reference_label(mem, context, jump_label, NULL, &node->base.source_location, &scoped_id));
            kefir_result_t res = kefir_ast_flow_control_branching_point_append(
                mem, base->properties.inline_assembly.branching_point, jump_label, scoped_id->label.point);
            if (res != KEFIR_ALREADY_EXISTS) {
                REQUIRE_OK(res);
            }
        }
    } else {
        REQUIRE(kefir_list_length(&node->outputs) == 0,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->base.source_location,
                                       "Inline assembly directive in global scope cannot have outputs"));
        REQUIRE(kefir_list_length(&node->inputs) == 0,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->base.source_location,
                                       "Inline assembly directive in global scope cannot have inputs"));
        REQUIRE(kefir_list_length(&node->clobbers) == 0,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->base.source_location,
                                       "Inline assembly directive in global scope cannot have clobbers"));
        REQUIRE(kefir_list_length(&node->jump_labels) == 0,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->base.source_location,
                                       "Inline assembly directive in global scope cannot have jump labels"));

        base->properties.inline_assembly.origin_flow_control_point = NULL;
        base->properties.inline_assembly.branching_point = NULL;
    }

    return KEFIR_OK;
}
