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
#include "kefir/ast/type_conv.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

kefir_result_t kefir_ast_translate_conditional_statement_node(struct kefir_mem *mem,
                                                              struct kefir_ast_translator_context *context,
                                                              struct kefir_irbuilder_block *builder,
                                                              const struct kefir_ast_conditional_statement *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST conditional statement node"));

    const kefir_size_t statement_begin_index = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
    kefir_ir_debug_entry_id_t lexical_block_entry_id;
    REQUIRE_OK(kefir_ast_translator_context_push_debug_hierarchy_entry(mem, context, KEFIR_IR_DEBUG_ENTRY_LEXICAL_BLOCK,
                                                                       &lexical_block_entry_id));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &context->module->debug_info.entries, &context->module->symbols,
                                                  lexical_block_entry_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_CODE_BEGIN(statement_begin_index)));

    REQUIRE_OK(kefir_ast_translate_expression(mem, node->condition, builder, context));
    const struct kefir_ast_type *condition_type =
        kefir_ast_translator_normalize_type(kefir_ast_translator_normalize_type(KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(
            mem, context->ast_context->type_bundle, node->condition->properties.type)));
    if (KEFIR_AST_TYPE_IS_FLOATING_POINT(condition_type)) {
        REQUIRE_OK(kefir_ast_translate_typeconv_to_bool(builder, condition_type));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BNOT64, 0));
    } else {
        switch (condition_type->tag) {
            case KEFIR_AST_TYPE_SCALAR_BOOL:
            case KEFIR_AST_TYPE_SCALAR_CHAR:
            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
            case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BNOT8, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
            case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BNOT16, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
            case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BNOT32, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
            case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
            case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
            case KEFIR_AST_TYPE_SCALAR_POINTER:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BNOT64, 0));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected condition type");
        }
    }
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BRANCH8, 0));
    REQUIRE_OK(kefir_ast_translator_flow_control_point_reference(
        mem, node->base.properties.statement_props.flow_control_statement->value.conditional.thenBranchEnd,
        builder->block, KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder) - 1));

    REQUIRE_OK(kefir_ast_translate_statement(mem, node->thenBranch, builder, context));
    if (node->elseBranch != NULL) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_JMP, 0));
        REQUIRE_OK(kefir_ast_translator_flow_control_point_reference(
            mem, node->base.properties.statement_props.flow_control_statement->value.conditional.elseBranchEnd,
            builder->block, KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder) - 1));
    }
    REQUIRE_OK(kefir_ast_translator_flow_control_point_resolve(
        mem, node->base.properties.statement_props.flow_control_statement->value.conditional.thenBranchEnd,
        KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder)));

    if (node->elseBranch != NULL) {
        REQUIRE_OK(kefir_ast_translate_statement(mem, node->elseBranch, builder, context));
        REQUIRE_OK(kefir_ast_translator_flow_control_point_resolve(
            mem, node->base.properties.statement_props.flow_control_statement->value.conditional.elseBranchEnd,
            KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder)));
    }

    const kefir_size_t statement_end_index = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &context->module->debug_info.entries, &context->module->symbols,
                                                  lexical_block_entry_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_CODE_END(statement_end_index)));
    const struct kefir_ast_identifier_flat_scope *associated_ordinary_scope =
        node->base.properties.statement_props.flow_control_statement->associated_scopes.ordinary_scope;
    REQUIRE(associated_ordinary_scope != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE,
                            "Expected AST flow control statement to have an associated ordinary scope"));
    REQUIRE_OK(kefir_ast_translator_generate_object_scope_debug_information(
        mem, context->ast_context, context->environment, context->module, context->debug_entries,
        associated_ordinary_scope, lexical_block_entry_id, statement_begin_index, statement_end_index));
    REQUIRE_OK(kefir_ast_translator_context_pop_debug_hierarchy_entry(mem, context));

    return KEFIR_OK;
}
