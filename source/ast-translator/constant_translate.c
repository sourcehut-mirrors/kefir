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
    switch (value->klass) {
        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER:
            if (value->bitprecise != NULL) {
                // Intentionally left blank
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, value->integer));
                REQUIRE_OK(kefir_ast_translate_typeconv(
                    mem, context->module, builder, context->ast_context->type_traits, kefir_ast_type_signed_long_long(),
                    kefir_ast_unqualified_type(base->properties.type)));
                *success_ptr = true;
            }
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT: {
            const struct kefir_ast_type *unqualified_type = kefir_ast_unqualified_type(base->properties.type);
            switch (unqualified_type->tag) {
                case KEFIR_AST_TYPE_SCALAR_FLOAT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IR_OPCODE_FLOAT32_CONST,
                                                               (kefir_float32_t) value->floating_point, 0.0f));
                    break;

                case KEFIR_AST_TYPE_SCALAR_DOUBLE:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST,
                                                               (kefir_float64_t) value->floating_point));
                    break;

                case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST,
                                                                        value->floating_point));
                    break;

                default:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST,
                                                                        value->floating_point));
                    REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder,
                                                            context->ast_context->type_traits,
                                                            kefir_ast_type_long_double(), unqualified_type));
                    break;
            }
            *success_ptr = true;
        } break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT: {
            const struct kefir_ast_type *unqualified_type = kefir_ast_unqualified_type(base->properties.type);
            switch (unqualified_type->tag) {
                case KEFIR_AST_TYPE_COMPLEX_FLOAT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IR_OPCODE_FLOAT32_CONST,
                                                               (kefir_float32_t) value->complex_floating_point.real,
                                                               0.0f));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(
                        builder, KEFIR_IR_OPCODE_FLOAT32_CONST,
                        (kefir_float32_t) value->complex_floating_point.imaginary, 0.0f));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_FROM, 0));
                    break;

                case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST,
                                                               (kefir_float64_t) value->complex_floating_point.real));
                    REQUIRE_OK(
                        KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST,
                                                        (kefir_float64_t) value->complex_floating_point.imaginary));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_FROM, 0));
                    break;

                case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST,
                                                                        value->complex_floating_point.real));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST,
                                                                        value->complex_floating_point.imaginary));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_FROM, 0));
                    break;

                default:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST,
                                                                        value->complex_floating_point.real));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST,
                                                                        value->complex_floating_point.imaginary));
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_FROM, 0));
                    REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder,
                                                            context->ast_context->type_traits,
                                                            kefir_ast_type_complex_long_double(), unqualified_type));
                    break;
            }
            *success_ptr = true;
        } break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS:
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
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_NONE:
        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPOUND:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}
