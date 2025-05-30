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

#include "kefir/ast/constant_expression_impl.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

kefir_result_t kefir_ast_evaluate_scalar_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                              const struct kefir_ast_constant *node,
                                              struct kefir_ast_constant_expression_value *value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant node"));
    REQUIRE(value != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant expression value pointer"));
    REQUIRE(node->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
            KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                   "Expected constant expression AST node"));

    switch (node->type) {
        case KEFIR_AST_BOOL_CONSTANT:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            value->integer = node->value.boolean;
            break;

        case KEFIR_AST_CHAR_CONSTANT:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            value->integer = node->value.character;
            break;

        case KEFIR_AST_WIDE_CHAR_CONSTANT:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            value->integer = node->value.wide_character;
            break;

        case KEFIR_AST_UNICODE16_CHAR_CONSTANT:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            value->integer = node->value.unicode16_character;
            break;

        case KEFIR_AST_UNICODE32_CHAR_CONSTANT:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            value->integer = node->value.unicode32_character;
            break;

        case KEFIR_AST_INT_CONSTANT:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            value->integer = node->value.integer;
            break;

        case KEFIR_AST_UINT_CONSTANT:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            value->uinteger = node->value.uinteger;
            break;

        case KEFIR_AST_LONG_CONSTANT:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            value->integer = node->value.long_integer;
            break;

        case KEFIR_AST_ULONG_CONSTANT:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            value->uinteger = node->value.ulong_integer;
            break;

        case KEFIR_AST_LONG_LONG_CONSTANT:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            value->integer = node->value.long_long;
            break;

        case KEFIR_AST_ULONG_LONG_CONSTANT:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            value->uinteger = node->value.ulong_long;
            break;

        case KEFIR_AST_BITPRECISE_CONSTANT:
            // REQUIRE(node->value.bitprecise.width <= sizeof(kefir_ast_constant_expression_int_t) * CHAR_BIT,
            //     KEFIR_SET_ERROR(KEFIR_NOT_IMPLEMENTED, "Bit-precise integers wider than native types are not
            //     implemented yet"));
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            REQUIRE_OK(kefir_bigint_get_signed(&node->value.bitprecise, &value->integer));
            break;

        case KEFIR_AST_UNSIGNED_BITPRECISE_CONSTANT:
            // REQUIRE(node->value.bitprecise.width <= sizeof(kefir_ast_constant_expression_int_t) * CHAR_BIT,
            //     KEFIR_SET_ERROR(KEFIR_NOT_IMPLEMENTED, "Bit-precise integers wider than native types are not
            //     implemented yet"));
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            REQUIRE_OK(kefir_bigint_get_unsigned(&node->value.bitprecise, &value->uinteger));
            break;

        case KEFIR_AST_FLOAT_CONSTANT:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT;
            value->floating_point = node->value.float32;
            break;

        case KEFIR_AST_DOUBLE_CONSTANT:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT;
            value->floating_point = node->value.float64;
            break;

        case KEFIR_AST_LONG_DOUBLE_CONSTANT:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT;
            value->floating_point = node->value.long_double;
            break;

        case KEFIR_AST_COMPLEX_FLOAT_CONSTANT:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT;
            value->complex_floating_point.real = node->value.complex_float32.real;
            value->complex_floating_point.imaginary = node->value.complex_float32.imaginary;
            break;

        case KEFIR_AST_COMPLEX_DOUBLE_CONSTANT:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT;
            value->complex_floating_point.real = node->value.complex_float64.real;
            value->complex_floating_point.imaginary = node->value.complex_float64.imaginary;
            break;

        case KEFIR_AST_COMPLEX_LONG_DOUBLE_CONSTANT:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT;
            value->complex_floating_point.real = node->value.complex_long_double.real;
            value->complex_floating_point.imaginary = node->value.complex_long_double.imaginary;
            break;
    }
    return KEFIR_OK;
}
