/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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
#include "kefir/ast/global_context.h"
#include "kefir/ast/type_conv.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

kefir_result_t kefir_ast_analyze_translation_unit_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                       const struct kefir_ast_translation_unit *node,
                                                       struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST labeled statement"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST base node"));

    REQUIRE_OK(kefir_ast_node_properties_init(&base->properties));
    base->properties.category = KEFIR_AST_NODE_CATEGORY_TRANSLATION_UNIT;

    kefir_bool_t has_analysis_errors = false;
    for (kefir_size_t i = 0; i < node->external_definitions_length; i++) {

        struct kefir_ast_node_base *entry = node->external_definitions[i];
        kefir_result_t res = kefir_ast_analyze_node(mem, context, entry);
        REQUIRE_CHAIN_SET(&res,
                          entry->properties.category == KEFIR_AST_NODE_CATEGORY_DECLARATION ||
                              entry->properties.category == KEFIR_AST_NODE_CATEGORY_INIT_DECLARATOR ||
                              entry->properties.category == KEFIR_AST_NODE_CATEGORY_FUNCTION_DEFINITION ||
                              entry->properties.category == KEFIR_AST_NODE_CATEGORY_INLINE_ASSEMBLY,
                          KEFIR_SET_SOURCE_ERROR(
                              KEFIR_ANALYSIS_ERROR, &entry->source_location,
                              "AST Translation unit must contain exclusively declarations and function definitions"));
        if (res != KEFIR_ANALYSIS_ERROR ||
            !KEFIR_AST_CONTEXT_DO_ERROR_RECOVERY(context, context->global_context->encountered_errors)) {
            REQUIRE_OK(res);
        } else {
            has_analysis_errors = true;
            context->global_context->encountered_errors++;
        }
    }

    return has_analysis_errors ? KEFIR_ANALYSIS_ERROR : KEFIR_OK;
}
