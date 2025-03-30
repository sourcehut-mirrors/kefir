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

#include "kefir/ast-translator/translator_impl.h"
#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/util.h"
#include "kefir/ast-translator/misc.h"
#include "kefir/ast/downcast.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

kefir_result_t kefir_ast_translate_statement_expression_node(struct kefir_mem *mem,
                                                             struct kefir_ast_translator_context *context,
                                                             struct kefir_irbuilder_block *builder,
                                                             const struct kefir_ast_statement_expression *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST statement expression node"));

    const kefir_size_t statement_begin_index = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
    kefir_ir_debug_entry_id_t lexical_block_entry_id;
    REQUIRE_OK(kefir_ast_translator_context_push_debug_hierarchy_entry(mem, context, KEFIR_IR_DEBUG_ENTRY_LEXICAL_BLOCK,
                                                                       &lexical_block_entry_id));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &context->module->debug_info.entries, &context->module->symbols,
                                                  lexical_block_entry_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_CODE_BEGIN(statement_begin_index)));

    for (const struct kefir_list_entry *iter = kefir_list_head(&node->block_items); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_node_base *, item, iter->value);

        if (item->properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT ||
            item->properties.category == KEFIR_AST_NODE_CATEGORY_INLINE_ASSEMBLY) {
            REQUIRE_OK(kefir_ast_translate_statement(mem, item, builder, context));
        } else if (item->properties.category == KEFIR_AST_NODE_CATEGORY_DECLARATION ||
                   item->properties.category == KEFIR_AST_NODE_CATEGORY_INIT_DECLARATOR) {
            REQUIRE_OK(kefir_ast_translate_declaration(mem, item, builder, context));
        } else {
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected compound statement item");
        }
    }

    if (node->result != NULL) {
        kefir_result_t res;
        struct kefir_ast_expression_statement *expr_statement = NULL;

        REQUIRE_MATCH_OK(&res, kefir_ast_downcast_expression_statement(node->result, &expr_statement, false),
                         KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                                         "Last statement of statement expression shall be an expression statement"));
        REQUIRE_OK(kefir_ast_translate_expression(mem, expr_statement->expression, builder, context));
    }

    const kefir_size_t statement_end_index = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &context->module->debug_info.entries, &context->module->symbols,
                                                  lexical_block_entry_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_CODE_END(statement_end_index)));
    const struct kefir_ast_identifier_flat_scope *associated_ordinary_scope =
        node->base.properties.expression_props.flow_control_statement->associated_scopes.ordinary_scope;
    REQUIRE(associated_ordinary_scope != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                            "Expected AST flow control statement to have an associated ordinary scope"));
    REQUIRE_OK(kefir_ast_translator_generate_object_scope_debug_information(
        mem, context->ast_context, context->environment, context->module, context->debug_entries,
        associated_ordinary_scope, lexical_block_entry_id, statement_begin_index, statement_end_index));
    REQUIRE_OK(kefir_ast_translator_context_pop_debug_hierarchy_entry(mem, context));

    if (kefir_ast_flow_control_block_contains_vl_arrays(
            node->base.properties.expression_props.flow_control_statement)) {
        kefir_id_t vla_element;
        REQUIRE_OK(kefir_ast_flow_control_block_vl_array_head(
            node->base.properties.expression_props.flow_control_statement, &vla_element));

        REQUIRE_OK(kefir_ast_translator_resolve_vla_element(mem, context, builder, vla_element));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_LOAD, KEFIR_IR_MEMORY_FLAG_NONE));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCOPE_POP, 0));
    }
    return KEFIR_OK;
}
