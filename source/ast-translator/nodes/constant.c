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
#include "kefir/ast-translator/temporaries.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

kefir_result_t kefir_ast_translate_constant_node(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                                 struct kefir_irbuilder_block *builder,
                                                 const struct kefir_ast_constant *node) {
    UNUSED(mem);
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant node"));

    switch (node->type) {
        case KEFIR_AST_BOOL_CONSTANT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST,
                                                       (kefir_uint64_t) node->value.boolean));
            break;

        case KEFIR_AST_CHAR_CONSTANT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST,
                                                       (kefir_int64_t) node->value.character));
            break;

        case KEFIR_AST_WIDE_CHAR_CONSTANT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST,
                                                       (kefir_int64_t) node->value.wide_character));
            break;

        case KEFIR_AST_UNICODE16_CHAR_CONSTANT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST,
                                                       (kefir_uint64_t) node->value.unicode16_character));
            break;

        case KEFIR_AST_UNICODE32_CHAR_CONSTANT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST,
                                                       (kefir_uint64_t) node->value.unicode32_character));
            break;

        case KEFIR_AST_INT_CONSTANT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST,
                                                       (kefir_int64_t) node->value.integer));
            break;

        case KEFIR_AST_UINT_CONSTANT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST,
                                                       (kefir_uint64_t) node->value.uinteger));
            break;

        case KEFIR_AST_LONG_CONSTANT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST,
                                                       (kefir_int64_t) node->value.long_integer));
            break;

        case KEFIR_AST_ULONG_CONSTANT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST,
                                                       (kefir_uint64_t) node->value.ulong_integer));
            break;

        case KEFIR_AST_LONG_LONG_CONSTANT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST,
                                                       (kefir_int64_t) node->value.long_long));
            break;

        case KEFIR_AST_ULONG_LONG_CONSTANT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST,
                                                       (kefir_uint64_t) node->value.ulong_long));
            break;

        case KEFIR_AST_FLOAT_CONSTANT:
            REQUIRE_OK(
                KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IR_OPCODE_FLOAT32_CONST, node->value.float32, 0.0f));
            break;

        case KEFIR_AST_DOUBLE_CONSTANT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST, node->value.float64));
            break;

        case KEFIR_AST_LONG_DOUBLE_CONSTANT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST,
                                                                node->value.long_double));
            break;

        case KEFIR_AST_COMPLEX_FLOAT_CONSTANT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IR_OPCODE_FLOAT32_CONST,
                                                       node->value.complex_float32.real, 0.0f));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IR_OPCODE_FLOAT32_CONST,
                                                       node->value.complex_float32.imaginary, 0.0f));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_FROM, 0));
            break;

        case KEFIR_AST_COMPLEX_DOUBLE_CONSTANT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST,
                                                       node->value.complex_float64.real));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST,
                                                       node->value.complex_float64.imaginary));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_FROM, 0));
            break;

        case KEFIR_AST_COMPLEX_LONG_DOUBLE_CONSTANT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST,
                                                                node->value.complex_long_double.real));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST,
                                                                node->value.complex_long_double.imaginary));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_FROM, 0));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected AST constant type");
    }
    return KEFIR_OK;
}
