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
#include "kefir/ast-translator/typeconv.h"
#include "kefir/ast-translator/util.h"
#include "kefir/ast/type_conv.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

static kefir_result_t generate_branch(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                      struct kefir_irbuilder_block *builder, const struct kefir_ast_type *type,
                                      kefir_size_t *index) {
    type = kefir_ast_translator_normalize_type(
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, type));
    switch (type->tag) {
        case KEFIR_AST_TYPE_SCALAR_BOOL:
        case KEFIR_AST_TYPE_SCALAR_CHAR:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            *index = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
            REQUIRE_OK(
                KEFIR_IRBUILDER_BLOCK_APPENDU64_2(builder, KEFIR_IR_OPCODE_BRANCH, 0, KEFIR_IR_BRANCH_CONDITION_8BIT));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            *index = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
            REQUIRE_OK(
                KEFIR_IRBUILDER_BLOCK_APPENDU64_2(builder, KEFIR_IR_OPCODE_BRANCH, 0, KEFIR_IR_BRANCH_CONDITION_16BIT));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
            *index = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
            REQUIRE_OK(
                KEFIR_IRBUILDER_BLOCK_APPENDU64_2(builder, KEFIR_IR_OPCODE_BRANCH, 0, KEFIR_IR_BRANCH_CONDITION_32BIT));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_POINTER:
            *index = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
            REQUIRE_OK(
                KEFIR_IRBUILDER_BLOCK_APPENDU64_2(builder, KEFIR_IR_OPCODE_BRANCH, 0, KEFIR_IR_BRANCH_CONDITION_64BIT));
            break;

        default:
            REQUIRE_OK(kefir_ast_translate_typeconv_to_bool(builder, type));
            *index = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
            REQUIRE_OK(
                KEFIR_IRBUILDER_BLOCK_APPENDU64_2(builder, KEFIR_IR_OPCODE_BRANCH, 0, KEFIR_IR_BRANCH_CONDITION_8BIT));
            break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_conditional_operator_node(struct kefir_mem *mem,
                                                             struct kefir_ast_translator_context *context,
                                                             struct kefir_irbuilder_block *builder,
                                                             const struct kefir_ast_conditional_operator *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST conditional operator node"));

    const struct kefir_ast_type *unqualified_condition_type =
        kefir_ast_unqualified_type(node->condition->properties.type);
    const struct kefir_ast_type *unqualified_expr1_type =
        node->expr1 != NULL ? kefir_ast_unqualified_type(node->expr1->properties.type) : NULL;
    const struct kefir_ast_type *unqualified_expr2_type = kefir_ast_unqualified_type(node->expr2->properties.type);

    if (node->expr1 != NULL) {
        REQUIRE_OK(kefir_ast_translate_expression(mem, node->condition, builder, context));
        kefir_size_t jmp1Index;
        REQUIRE_OK(generate_branch(mem, context, builder, unqualified_condition_type, &jmp1Index));
        REQUIRE_OK(kefir_ast_translate_expression(mem, node->expr2, builder, context));
        if (KEFIR_AST_TYPE_IS_SCALAR_TYPE(unqualified_expr2_type)) {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                    unqualified_expr2_type, node->base.properties.type));
        }
        kefir_size_t jmp2Index = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_JUMP, 0));
        KEFIR_IRBUILDER_BLOCK_INSTR_AT(builder, jmp1Index)->arg.i64 = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
        REQUIRE_OK(kefir_ast_translate_expression(mem, node->expr1, builder, context));
        if (KEFIR_AST_TYPE_IS_SCALAR_TYPE(unqualified_expr1_type)) {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                    unqualified_expr1_type, node->base.properties.type));
        }
        KEFIR_IRBUILDER_BLOCK_INSTR_AT(builder, jmp2Index)->arg.i64 = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
    } else {
        REQUIRE_OK(kefir_ast_translate_expression(mem, node->condition, builder, context));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
        kefir_size_t jmp1Index;
        REQUIRE_OK(generate_branch(mem, context, builder, unqualified_condition_type, &jmp1Index));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_POP, 0));
        REQUIRE_OK(kefir_ast_translate_expression(mem, node->expr2, builder, context));
        if (KEFIR_AST_TYPE_IS_SCALAR_TYPE(unqualified_expr2_type)) {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                    unqualified_expr2_type, node->base.properties.type));
        }
        kefir_size_t jmp2Index = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_JUMP, 0));
        KEFIR_IRBUILDER_BLOCK_INSTR_AT(builder, jmp1Index)->arg.i64 = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
        if (KEFIR_AST_TYPE_IS_SCALAR_TYPE(unqualified_condition_type)) {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                    unqualified_condition_type, node->base.properties.type));
        }
        KEFIR_IRBUILDER_BLOCK_INSTR_AT(builder, jmp2Index)->arg.i64 = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
    }

    return KEFIR_OK;
}
