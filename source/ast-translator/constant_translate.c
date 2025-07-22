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

#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/scope/translator.h"
#include "kefir/ast-translator/typeconv.h"
#include "kefir/ast-translator/lvalue.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_ast_try_translate_constant(struct kefir_mem *mem, const struct kefir_ast_node_base *base,
                                                const struct kefir_ast_constant_expression_value *value,
                                                struct kefir_irbuilder_block *builder,
                                                struct kefir_ast_translator_context *context,
                                                kefir_bool_t *success_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    REQUIRE(base->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid expression AST node base"));
    REQUIRE(value != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant expression value"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translator context"));
    REQUIRE(success_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    *success_ptr = false;
    const struct kefir_ast_type *unqualified_type = kefir_ast_unqualified_type(base->properties.type);
    switch (value->klass) {
        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER:
            if (value->bitprecise != NULL) {
                if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(unqualified_type) &&
                    unqualified_type->bitprecise.width == value->bitprecise->bitwidth) {
                    kefir_id_t bigint_id;
                    REQUIRE_OK(kefir_ir_module_new_bigint(mem, context->module, value->bitprecise, &bigint_id));
                    REQUIRE_OK(
                        KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_BITINT_SIGNED_CONST, bigint_id));
                    *success_ptr = true;
                }
            } else if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(unqualified_type) &&
                       !KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(unqualified_type)) {
                kefir_bool_t signed_type;
                REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, unqualified_type, &signed_type));
                if (signed_type) {
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, value->integer));
                } else {
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT_CONST,
                                                               (kefir_uint64_t) value->integer));
                }
                *success_ptr = true;
            }
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT:
            switch (unqualified_type->tag) {
                case KEFIR_AST_TYPE_SCALAR_FLOAT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IR_OPCODE_FLOAT32_CONST,
                                                               (kefir_float32_t) value->floating_point, 0.0f));
                    *success_ptr = true;
                    break;

                case KEFIR_AST_TYPE_SCALAR_DOUBLE:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST,
                                                               (kefir_float64_t) value->floating_point));
                    *success_ptr = true;
                    break;

                case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST,
                                                                        value->floating_point));
                    *success_ptr = true;
                    break;

                default:
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT:
            switch (unqualified_type->tag) {
                case KEFIR_AST_TYPE_COMPLEX_FLOAT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IR_OPCODE_FLOAT32_CONST,
                                                               (kefir_float32_t) value->complex_floating_point.real,
                                                               0.0f));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(
                        builder, KEFIR_IR_OPCODE_FLOAT32_CONST,
                        (kefir_float32_t) value->complex_floating_point.imaginary, 0.0f));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_FROM, 0));
                    *success_ptr = true;
                    break;

                case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST,
                                                               (kefir_float64_t) value->complex_floating_point.real));
                    REQUIRE_OK(
                        KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST,
                                                        (kefir_float64_t) value->complex_floating_point.imaginary));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_FROM, 0));
                    *success_ptr = true;
                    break;

                case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST,
                                                                        value->complex_floating_point.real));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST,
                                                                        value->complex_floating_point.imaginary));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_FROM, 0));
                    *success_ptr = true;
                    break;

                default:
                    // Intentionally left blank
                    break;
            }
            *success_ptr = true;
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS:
            if (unqualified_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER ||
                unqualified_type->tag == KEFIR_AST_TYPE_SCALAR_NULL_POINTER ||
                unqualified_type->tag == KEFIR_AST_TYPE_FUNCTION || unqualified_type->tag == KEFIR_AST_TYPE_ARRAY) {
                switch (value->pointer.type) {
                    case KEFIR_AST_CONSTANT_EXPRESSION_POINTER_INTEGER:
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(
                            builder, KEFIR_IR_OPCODE_UINT_CONST,
                            value->pointer.offset + (kefir_uint64_t) value->pointer.base.integral));
                        *success_ptr = true;
                        break;

                    case KEFIR_AST_CONSTANT_EXPRESSION_POINTER_LITERAL: {
                        kefir_id_t id;
                        REQUIRE_OK(kefir_ir_module_string_literal(
                            mem, context->module, KefirAstIrStringLiteralTypes[value->pointer.base.string.type], true,
                            value->pointer.base.string.content, value->pointer.base.string.length, &id));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_STRING_REF, id));
                        REQUIRE_OK(
                            KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, value->pointer.offset));
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
                        *success_ptr = true;
                    } break;

                    case KEFIR_AST_CONSTANT_EXPRESSION_POINTER_IDENTIFER:
                        switch (value->pointer.scoped_id->klass) {
                            case KEFIR_AST_SCOPE_IDENTIFIER_OBJECT:
                                REQUIRE_OK(kefir_ast_translator_object_lvalue(
                                    mem, context, builder, value->pointer.base.literal, value->pointer.scoped_id));
                                *success_ptr = true;
                                break;

                            case KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION:
                                REQUIRE_OK(kefir_ast_translator_function_lvalue(mem, context, builder,
                                                                                value->pointer.base.literal));
                                *success_ptr = true;
                                break;

                            case KEFIR_AST_SCOPE_IDENTIFIER_LABEL: {
                                kefir_id_t id;
                                const char *literal =
                                    kefir_ir_module_symbol(mem, context->module, value->pointer.base.literal, &id);
                                REQUIRE(literal != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE,
                                                                         "Failed to insert symbol into IR module"));
                                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_GET_GLOBAL, id));
                                *success_ptr = true;
                            } break;

                            default:
                                // Intentionally left blank
                                break;
                        }
                        if (*success_ptr && value->pointer.offset != 0) {
                            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST,
                                                                       value->pointer.offset));
                            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
                        }
                        break;
                }
            }
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_NONE:
        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPOUND:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}
