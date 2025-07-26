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
#include "kefir/ast/downcast.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

static kefir_result_t is_nodicard_call(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                       const struct kefir_ast_function_call *call, kefir_bool_t *nodiscard,
                                       const char **nodiscard_message) {
    *nodiscard = false;
    const struct kefir_ast_type *return_type = kefir_ast_unqualified_type(call->base.properties.type);

    const struct kefir_ast_type *func_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->type_bundle, call->function->properties.type);
    if (func_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER && func_type->referenced_type->tag == KEFIR_AST_TYPE_FUNCTION) {
        const struct kefir_ast_type *function_type = func_type->referenced_type;
        *nodiscard = function_type->function_type.attributes.no_discard;
        *nodiscard_message = function_type->function_type.attributes.no_discard_message;
    }

    if (!*nodiscard) {
        if (return_type->tag == KEFIR_AST_TYPE_STRUCTURE || return_type->tag == KEFIR_AST_TYPE_UNION) {
            *nodiscard = return_type->structure_type.flags.no_discard;
            *nodiscard_message = return_type->structure_type.flags.no_discard_message;
        } else if (return_type->tag == KEFIR_AST_TYPE_ENUMERATION) {
            *nodiscard = return_type->enumeration_type.flags.no_discard;
            *nodiscard_message = return_type->enumeration_type.flags.no_discard_message;
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_analyze_expression_statement_node(struct kefir_mem *mem,
                                                           const struct kefir_ast_context *context,
                                                           const struct kefir_ast_expression_statement *node,
                                                           struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST expression statement"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST base node"));

    REQUIRE_OK(kefir_ast_node_properties_init(&base->properties));
    base->properties.category = KEFIR_AST_NODE_CATEGORY_STATEMENT;

    if (node->expression != NULL) {
        REQUIRE_OK(kefir_ast_analyze_node(mem, context, node->expression));
        REQUIRE(node->expression->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->expression->source_location,
                                       "Expected AST expression node as part of expression statement"));

        struct kefir_ast_function_call *function_call;
        kefir_result_t res = kefir_ast_downcast_function_call(node->expression, &function_call, false);
        if (res != KEFIR_NO_MATCH) {
            REQUIRE_OK(res);

            kefir_bool_t is_nodiscard = false;
            const char *nodiscard_message = NULL;
            REQUIRE_OK(is_nodicard_call(mem, context, function_call, &is_nodiscard, &nodiscard_message));
            if (is_nodiscard) {
                if (nodiscard_message == NULL) {
                    nodiscard_message = "nodiscard call return value ignored";
                }
                if (base->source_location.source != NULL) {
                    fprintf(context->configuration->warning_output,
                            "%s@%" KEFIR_UINT_FMT ":%" KEFIR_UINT_FMT " warning: %s\n", base->source_location.source,
                            base->source_location.line, base->source_location.column, nodiscard_message);
                } else {
                    fprintf(context->configuration->warning_output, "warning: %s\n", nodiscard_message);
                }
            }
        }
    }
    return KEFIR_OK;
}
