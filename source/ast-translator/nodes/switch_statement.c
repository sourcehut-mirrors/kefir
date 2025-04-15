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
#include "kefir/ast-translator/misc.h"
#include "kefir/ast-translator/typeconv.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

kefir_result_t kefir_ast_translate_switch_statement_node(struct kefir_mem *mem,
                                                         struct kefir_ast_translator_context *context,
                                                         struct kefir_irbuilder_block *builder,
                                                         const struct kefir_ast_switch_statement *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST switch statement node"));

    const kefir_size_t statement_begin_index = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
    kefir_ir_debug_entry_id_t lexical_block_entry_id;
    REQUIRE_OK(kefir_ast_translator_context_push_debug_hierarchy_entry(mem, context, KEFIR_IR_DEBUG_ENTRY_LEXICAL_BLOCK,
                                                                       &lexical_block_entry_id));
    REQUIRE_OK(kefir_ir_debug_entry_add_attribute(mem, &context->module->debug_info.entries, &context->module->symbols,
                                                  lexical_block_entry_id,
                                                  &KEFIR_IR_DEBUG_ENTRY_ATTR_CODE_BEGIN(statement_begin_index)));
    REQUIRE_OK(kefir_ast_translate_expression(mem, node->expression, builder, context));

    struct kefir_ast_flow_control_structure *flow_control_stmt =
        node->base.properties.statement_props.flow_control_statement;
    REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                            node->expression->properties.type,
                                            flow_control_stmt->value.switchStatement.controlling_expression_type));
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *switchCase =
             kefir_hashtree_iter(&flow_control_stmt->value.switchStatement.cases, &iter);
         switchCase != NULL; switchCase = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(kefir_ast_constant_expression_int_t, value, iter.node->key);
        ASSIGN_DECL_CAST(struct kefir_ast_flow_control_point *, point, iter.node->value);

        kefir_ast_constant_expression_int_t range = 1;
        struct kefir_hashtree_node *range_node;
        kefir_result_t res = kefir_hashtree_at(&flow_control_stmt->value.switchStatement.case_ranges,
                                               (kefir_hashtree_key_t) value, &range_node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            range = (kefir_ast_constant_expression_int_t) range_node->value;
        }

        if (range == 1) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, value));
            switch (flow_control_stmt->value.switchStatement.controlling_expression_type->tag) {
                case KEFIR_AST_TYPE_SCALAR_BOOL:
                case KEFIR_AST_TYPE_SCALAR_CHAR:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT8_EQUALS));
                    break;

                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT16_EQUALS));
                    break;

                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT32_EQUALS));
                    break;

                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT64_EQUALS));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
            }
        } else {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, value));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
            switch (flow_control_stmt->value.switchStatement.controlling_expression_type->tag) {
                case KEFIR_AST_TYPE_SCALAR_BOOL:
                case KEFIR_AST_TYPE_SCALAR_CHAR:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT8_GREATER));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 2));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT8_EQUALS));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_BOOL_OR, 0));

                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, range));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT8_LESSER));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 2));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT8_EQUALS));
                    break;

                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT16_GREATER));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 2));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT16_EQUALS));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_BOOL_OR, 0));

                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, range));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT16_LESSER));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 2));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT16_EQUALS));
                    break;

                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT32_GREATER));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 2));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT32_EQUALS));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_BOOL_OR, 0));

                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, range));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT32_LESSER));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 2));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT32_EQUALS));
                    break;

                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT64_GREATER));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 2));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT64_EQUALS));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_BOOL_OR, 0));

                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, range));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT64_LESSER));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 2));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                               KEFIR_IR_COMPARE_INT64_EQUALS));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
            }
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_BOOL_OR, 0));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_BOOL_AND, 0));
        }
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_BOOL_NOT, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64_2(builder, KEFIR_IR_OPCODE_BRANCH,
                                                     KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder) + 3,
                                                     KEFIR_IR_BRANCH_CONDITION_8BIT));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_POP, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_JUMP, 0));
        REQUIRE_OK(kefir_ast_translator_flow_control_point_reference(mem, point, builder->block,
                                                                     KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder) - 1));
    }

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_POP, 0));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_JUMP, 0));
    if (flow_control_stmt->value.switchStatement.defaultCase != NULL) {
        REQUIRE_OK(kefir_ast_translator_flow_control_point_reference(
            mem, flow_control_stmt->value.switchStatement.defaultCase, builder->block,
            KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder) - 1));

    } else {
        REQUIRE_OK(kefir_ast_translator_flow_control_point_reference(mem, flow_control_stmt->value.switchStatement.end,
                                                                     builder->block,
                                                                     KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder) - 1));
    }

    REQUIRE_OK(kefir_ast_translate_statement(mem, node->statement, builder, context));
    REQUIRE_OK(kefir_ast_translator_flow_control_point_resolve(mem, flow_control_stmt->value.switchStatement.end,
                                                               KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder)));

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
    REQUIRE_OK(kefir_ast_translator_mark_flat_scope_objects_lifetime(mem, context, builder, associated_ordinary_scope));

    return KEFIR_OK;
}
