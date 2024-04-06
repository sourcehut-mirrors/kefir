/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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
#include "kefir/ast-translator/util.h"
#include "kefir/ast/type_conv.h"
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

    REQUIRE_OK(kefir_ast_translate_expression(mem, node->expression, builder, context));

    struct kefir_ast_flow_control_structure *flow_control_stmt =
        node->base.properties.statement_props.flow_control_statement;
    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *switchCase =
             kefir_hashtree_iter(&flow_control_stmt->value.switchStatement.cases, &iter);
         switchCase != NULL; switchCase = kefir_hashtree_next(&iter)) {

        ASSIGN_DECL_CAST(kefir_ast_constant_expression_int_t, value, iter.node->key);
        ASSIGN_DECL_CAST(struct kefir_ast_flow_control_point *, point, iter.node->value);

        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_PICK, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_PUSHI64, value));
        switch (flow_control_stmt->value.switchStatement.controlling_expression_type->tag) {
            case KEFIR_AST_TYPE_SCALAR_BOOL:
            case KEFIR_AST_TYPE_SCALAR_CHAR:
            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
            case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IEQUALS8, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
            case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IEQUALS16, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
            case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IEQUALS32, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
            case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
            case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IEQUALS64, 0));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
        }
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BNOT, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BRANCH,
                                                   KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder) + 3));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_POP, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_JMP, 0));
        REQUIRE_OK(kefir_ast_translator_flow_control_point_reference(mem, point, builder->block,
                                                                     KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder) - 1));
    }

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_POP, 0));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_JMP, 0));
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

    return KEFIR_OK;
}
