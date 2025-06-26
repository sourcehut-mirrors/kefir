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
#include "kefir/ast-translator/flow_control.h"
#include "kefir/ast-translator/typeconv.h"
#include "kefir/ast-translator/util.h"
#include "kefir/ast-translator/misc.h"
#include "kefir/ast/type_conv.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

kefir_result_t kefir_ast_translate_do_while_statement_node(struct kefir_mem *mem,
                                                           struct kefir_ast_translator_context *context,
                                                           struct kefir_irbuilder_block *builder,
                                                           const struct kefir_ast_do_while_statement *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST do while statement node"));

    kefir_size_t beginning = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
    kefir_ir_debug_entry_id_t lexical_block_entry_id;
    REQUIRE_OK(kefir_ast_translator_context_push_debug_hierarchy_entry(mem, context, KEFIR_IR_DEBUG_ENTRY_LEXICAL_BLOCK,
                                                                       &lexical_block_entry_id));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &context->module->debug_info.entries, &context->module->symbols,
                                                  lexical_block_entry_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_CODE_BEGIN(beginning)));

    const struct kefir_ast_identifier_flat_scope *associated_ordinary_scope =
        node->base.properties.statement_props.flow_control_statement->associated_scopes.ordinary_scope;
    REQUIRE(associated_ordinary_scope != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                            "Expected AST flow control statement to have an associated ordinary scope"));
    REQUIRE_OK(kefir_ast_translator_mark_flat_scope_objects_lifetime(mem, context, builder, associated_ordinary_scope));

    REQUIRE_OK(kefir_ast_translate_statement(mem, node->body, builder, context));

    struct kefir_ast_flow_control_structure *flow_control_stmt =
        node->base.properties.statement_props.flow_control_statement;
    REQUIRE_OK(kefir_ast_translator_flow_control_point_resolve(mem, flow_control_stmt->value.loop.continuation,
                                                               KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder)));

    REQUIRE_OK(kefir_ast_translate_expression(mem, node->controlling_expr, builder, context));

    const struct kefir_ast_type *controlling_expr_type =
        kefir_ast_translator_normalize_type(KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(
            mem, context->ast_context->type_bundle, node->controlling_expr->properties.type));
    kefir_ast_type_data_model_classification_t controlling_expr_type_classification;
    REQUIRE_OK(kefir_ast_type_data_model_classify(context->ast_context->type_traits, controlling_expr_type,
                                                  &controlling_expr_type_classification));
    switch (controlling_expr_type_classification) {
        case KEFIR_AST_TYPE_DATA_MODEL_INT8:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64_2(builder, KEFIR_IR_OPCODE_BRANCH, beginning,
                                                         KEFIR_IR_BRANCH_CONDITION_8BIT));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT16:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64_2(builder, KEFIR_IR_OPCODE_BRANCH, beginning,
                                                         KEFIR_IR_BRANCH_CONDITION_16BIT));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT32:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64_2(builder, KEFIR_IR_OPCODE_BRANCH, beginning,
                                                         KEFIR_IR_BRANCH_CONDITION_32BIT));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT64:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64_2(builder, KEFIR_IR_OPCODE_BRANCH, beginning,
                                                         KEFIR_IR_BRANCH_CONDITION_64BIT));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
        default:
            REQUIRE_OK(kefir_ast_translate_typeconv_to_bool(context->ast_context->type_traits, builder,
                                                            controlling_expr_type));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64_2(builder, KEFIR_IR_OPCODE_BRANCH, beginning,
                                                         KEFIR_IR_BRANCH_CONDITION_8BIT));
            break;
    }

    REQUIRE_OK(kefir_ast_translator_flow_control_point_resolve(mem, flow_control_stmt->value.loop.end,
                                                               KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder)));

    const kefir_size_t statement_end_index = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &context->module->debug_info.entries, &context->module->symbols,
                                                  lexical_block_entry_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_CODE_END(statement_end_index)));
    REQUIRE_OK(kefir_ast_translator_generate_object_scope_debug_information(
        mem, context->ast_context, context->environment, context->module, context->debug_entries,
        associated_ordinary_scope, lexical_block_entry_id, beginning, statement_end_index));
    REQUIRE_OK(kefir_ast_translator_context_pop_debug_hierarchy_entry(mem, context));
    REQUIRE_OK(kefir_ast_translator_mark_flat_scope_objects_lifetime(mem, context, builder, associated_ordinary_scope));

    return KEFIR_OK;
}
