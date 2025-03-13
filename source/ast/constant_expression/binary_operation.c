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
#include "kefir/ast/type_conv.h"

#define ANY_OF(x, y, _klass) \
    (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF((x), (_klass)) || KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF((y), (_klass)))

static kefir_ast_constant_expression_float_t as_float(const struct kefir_ast_constant_expression_value *value) {
    switch (value->klass) {
        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER:
            return (kefir_ast_constant_expression_float_t) value->integer;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT:
            return value->floating_point;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT:
            return value->complex_floating_point.real;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS:
        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_NONE:
            return 0.0f;
    }
    return 0.0f;
}

struct complex_float {
    kefir_ast_constant_expression_float_t real;
    kefir_ast_constant_expression_float_t imaginary;
};

static struct complex_float as_complex_float(const struct kefir_ast_constant_expression_value *value) {
    switch (value->klass) {
        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER:
            return (struct complex_float) {.real = (kefir_ast_constant_expression_float_t) value->integer,
                                           .imaginary = 0.0};

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT:
            return (struct complex_float) {.real = (kefir_ast_constant_expression_float_t) value->floating_point,
                                           .imaginary = 0.0};

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT:
            return (struct complex_float) {
                .real = (kefir_ast_constant_expression_float_t) value->complex_floating_point.real,
                .imaginary = (kefir_ast_constant_expression_float_t) value->complex_floating_point.imaginary};

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS:
        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_NONE:
            return (struct complex_float) {0.0, 0.0};
    }
    return (struct complex_float) {0.0, 0.0};
}

static kefir_result_t evaluate_pointer_offset(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                              const struct kefir_ast_node_base *node,
                                              struct kefir_ast_constant_expression_pointer *pointer,
                                              kefir_ast_constant_expression_int_t index,
                                              struct kefir_ast_constant_expression_value *value) {
    kefir_int64_t offset = 0;

    const struct kefir_ast_type *unqualified_type = kefir_ast_unqualified_type(node->properties.type);
    if (unqualified_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER) {
        const struct kefir_ast_type *referenced_type = unqualified_type->referenced_type;
        if (context->configuration->analysis.ext_pointer_arithmetics &&
            (referenced_type->tag == KEFIR_AST_TYPE_FUNCTION ||
             kefir_ast_unqualified_type(referenced_type)->tag == KEFIR_AST_TYPE_VOID)) {
            referenced_type = context->type_traits->incomplete_type_substitute;
        }

        kefir_ast_target_environment_opaque_type_t opaque_type;
        REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_GET_TYPE(mem, context, context->target_env, referenced_type,
                                                         &opaque_type, &node->source_location));
        kefir_result_t res =
            KEFIR_AST_TARGET_ENVIRONMENT_OBJECT_OFFSET(mem, context->target_env, opaque_type, index, &offset);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->target_env, opaque_type);
            return res;
        });
        REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->target_env, opaque_type));
    } else {
        REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(unqualified_type),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->source_location,
                                       "Expected either pointer, or integral type"));
        offset = index;
    }

    value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS;
    value->pointer = *pointer;
    value->pointer.offset += offset;
    value->pointer.pointer_node = node;
    return KEFIR_OK;
}

static kefir_result_t evaluate_pointer_diff(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                            const struct kefir_ast_type *type,
                                            struct kefir_ast_constant_expression_pointer *pointer1,
                                            struct kefir_ast_constant_expression_pointer *pointer2,
                                            struct kefir_ast_constant_expression_value *value,
                                            const struct kefir_source_location *location1,
                                            const struct kefir_source_location *location2) {
    REQUIRE(pointer1->type == KEFIR_AST_CONSTANT_EXPRESSION_POINTER_INTEGER,
            KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, location1, "Expected pointer to be an integral constant"));
    REQUIRE(pointer2->type == KEFIR_AST_CONSTANT_EXPRESSION_POINTER_INTEGER,
            KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, location2, "Expected pointer to be an integral constant"));

    kefir_size_t diff_factor = 1;

    const struct kefir_ast_type *unqualified_type = kefir_ast_unqualified_type(type);
    if (unqualified_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER) {
        kefir_ast_target_environment_opaque_type_t opaque_type;
        REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_GET_TYPE(mem, context, context->target_env,
                                                         unqualified_type->referenced_type, &opaque_type, location1));

        struct kefir_ast_target_environment_object_info objinfo;
        kefir_result_t res =
            KEFIR_AST_TARGET_ENVIRONMENT_OBJECT_INFO(mem, context->target_env, opaque_type, NULL, &objinfo);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->target_env, opaque_type);
            return res;
        });
        REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->target_env, opaque_type));

        diff_factor = objinfo.size;
    } else {
        REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(unqualified_type),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, location1, "Expected either pointer, or integral type"));
    }

    kefir_int64_t diff = (pointer1->base.integral + pointer1->offset) - (pointer2->base.integral + pointer2->offset);

    value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
    value->integer = diff / diff_factor;
    return KEFIR_OK;
}

static kefir_result_t get_type_info(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                    const struct kefir_ast_type *type,
                                    const struct kefir_source_location *source_location,
                                    struct kefir_ast_target_environment_object_info *type_info) {
    kefir_ast_target_environment_opaque_type_t opaque_type;
    REQUIRE_OK(
        KEFIR_AST_TARGET_ENVIRONMENT_GET_TYPE(mem, context, context->target_env, type, &opaque_type, source_location));
    kefir_result_t res =
        KEFIR_AST_TARGET_ENVIRONMENT_OBJECT_INFO(mem, context->target_env, opaque_type, NULL, type_info);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->target_env, opaque_type);
        return res;
    });
    REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->target_env, opaque_type));

    return KEFIR_OK;
}

kefir_result_t kefir_ast_evaluate_binary_operation_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                        const struct kefir_ast_binary_operation *node,
                                                        struct kefir_ast_constant_expression_value *value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST binary operation node"));
    REQUIRE(value != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant expression value pointer"));
    REQUIRE(node->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
            KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                   "Expected constant expression AST node"));

    const struct kefir_ast_type *arg1_init_normalized_type =
        kefir_ast_type_conv_unwrap_enumeration(kefir_ast_unqualified_type(node->arg1->properties.type));
    const struct kefir_ast_type *arg1_normalized_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->type_bundle, arg1_init_normalized_type);
    REQUIRE(arg1_normalized_type != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to perform lvalue conversions"));
    const struct kefir_ast_type *arg2_init_normalized_type =
        kefir_ast_type_conv_unwrap_enumeration(kefir_ast_unqualified_type(node->arg2->properties.type));
    const struct kefir_ast_type *arg2_normalized_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->type_bundle, arg2_init_normalized_type);
    REQUIRE(arg2_normalized_type != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to perform lvalue conversions"));

    const struct kefir_ast_type *common_arith_type = kefir_ast_type_common_arithmetic(
        context->type_traits, arg1_normalized_type, node->arg1->properties.expression_props.bitfield_props,
        arg2_normalized_type, node->arg2->properties.expression_props.bitfield_props);
    kefir_bool_t common_type_signed_integer = false;
    if (common_arith_type != NULL && KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(common_arith_type)) {
        REQUIRE_OK(kefir_ast_type_is_signed(context->type_traits, common_arith_type, &common_type_signed_integer));
    }

    REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION(node->arg1),
            KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->arg1->source_location,
                                   "Unable to evaluate constant expression"));
    if (node->type != KEFIR_AST_OPERATION_LOGICAL_AND && node->type != KEFIR_AST_OPERATION_LOGICAL_OR) {
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION(node->arg2),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->arg2->source_location,
                                       "Unable to evaluate constant expression"));
    }

    switch (node->type) {
#define APPLY_SIGNED_OP(_size, _value, _arg1, _op, _arg2)                                                       \
    do {                                                                                                        \
        switch ((_size)) {                                                                                      \
            case 1:                                                                                             \
                (_value)->integer =                                                                             \
                    (kefir_int8_t) (((kefir_int8_t) (_arg1)->integer) _op((kefir_int8_t) (_arg2)->integer));    \
                break;                                                                                          \
                                                                                                                \
            case 2:                                                                                             \
                (_value)->integer =                                                                             \
                    (kefir_int16_t) (((kefir_int16_t) (_arg1)->integer) _op((kefir_int16_t) (_arg2)->integer)); \
                break;                                                                                          \
                                                                                                                \
            case 3:                                                                                             \
            case 4:                                                                                             \
                (_value)->integer =                                                                             \
                    (kefir_int32_t) (((kefir_int32_t) (_arg1)->integer) _op((kefir_int32_t) (_arg2)->integer)); \
                break;                                                                                          \
                                                                                                                \
            default:                                                                                            \
                (_value)->integer = (kefir_int64_t) ((_arg1)->integer _op(_arg2)->integer);                     \
                break;                                                                                          \
        }                                                                                                       \
    } while (0)
#define APPLY_UNSIGNED_OP(_size, _value, _arg1, _op, _arg2)                                                          \
    do {                                                                                                             \
        switch ((_size)) {                                                                                           \
            case 1:                                                                                                  \
                (_value)->uinteger =                                                                                 \
                    (kefir_uint8_t) (((kefir_uint8_t) (_arg1)->uinteger) _op((kefir_uint8_t) (_arg2)->uinteger));    \
                break;                                                                                               \
                                                                                                                     \
            case 2:                                                                                                  \
                (_value)->uinteger =                                                                                 \
                    (kefir_uint16_t) (((kefir_uint16_t) (_arg1)->uinteger) _op((kefir_uint16_t) (_arg2)->uinteger)); \
                break;                                                                                               \
                                                                                                                     \
            case 3:                                                                                                  \
            case 4:                                                                                                  \
                (_value)->uinteger =                                                                                 \
                    (kefir_uint32_t) (((kefir_uint32_t) (_arg1)->uinteger) _op((kefir_uint32_t) (_arg2)->uinteger)); \
                break;                                                                                               \
                                                                                                                     \
            default:                                                                                                 \
                (_value)->uinteger = (kefir_uint64_t) ((_arg1)->uinteger _op(_arg2)->uinteger);                      \
                break;                                                                                               \
        }                                                                                                            \
    } while (0)

        case KEFIR_AST_OPERATION_ADD:
            if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg1, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS)) {
                REQUIRE_OK(evaluate_pointer_offset(mem, context, KEFIR_AST_NODE_BASE(node),
                                                   &KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)->pointer,
                                                   KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)->integer,
                                                   value));
            } else if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg2,
                                                                KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS)) {
                REQUIRE_OK(evaluate_pointer_offset(mem, context, KEFIR_AST_NODE_BASE(node),
                                                   &KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)->pointer,
                                                   KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)->integer,
                                                   value));
            } else if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT)) {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT;
                struct complex_float f1 = as_complex_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1));
                struct complex_float f2 = as_complex_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
                value->complex_floating_point.real = f1.real + f2.real;
                value->complex_floating_point.imaginary = f1.imaginary + f2.imaginary;
            } else if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT)) {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT;
                value->floating_point = as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)) +
                                        as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else if (common_type_signed_integer) {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(
                    get_type_info(mem, context, node->base.properties.type, &node->base.source_location, &type_info));
                APPLY_SIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), +,
                                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(
                    get_type_info(mem, context, node->base.properties.type, &node->base.source_location, &type_info));
                APPLY_UNSIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), +,
                                  KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            }
            break;

        case KEFIR_AST_OPERATION_SUBTRACT:
            if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg1, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS)) {
                if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS)) {
                    REQUIRE_OK(evaluate_pointer_diff(mem, context, node->arg1->properties.type,
                                                     &KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)->pointer,
                                                     &KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)->pointer,
                                                     value, &node->arg1->source_location,
                                                     &node->arg2->source_location));
                } else {
                    REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg2,
                                                                     KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                            KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->arg2->source_location,
                                                   "Second subtraction operand shall have integral type"));
                    REQUIRE_OK(evaluate_pointer_offset(mem, context, KEFIR_AST_NODE_BASE(node),
                                                       &KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)->pointer,
                                                       -KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)->integer,
                                                       value));
                }
            } else if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT)) {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT;
                struct complex_float f1 = as_complex_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1));
                struct complex_float f2 = as_complex_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
                value->complex_floating_point.real = f1.real - f2.real;
                value->complex_floating_point.imaginary = f1.imaginary - f2.imaginary;
            } else if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT)) {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT;
                value->floating_point = as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)) -
                                        as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else if (common_type_signed_integer) {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(
                    get_type_info(mem, context, node->base.properties.type, &node->base.source_location, &type_info));
                APPLY_SIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), -,
                                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(
                    get_type_info(mem, context, node->base.properties.type, &node->base.source_location, &type_info));
                APPLY_UNSIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), -,
                                  KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            }
            break;

        case KEFIR_AST_OPERATION_MULTIPLY:
            if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT)) {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT;
                struct complex_float f1 = as_complex_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1));
                struct complex_float f2 = as_complex_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
                value->complex_floating_point.real = f1.real * f2.real - f1.imaginary * f2.imaginary;
                value->complex_floating_point.imaginary = f1.real * f2.imaginary + f2.real * f1.imaginary;
            } else if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT)) {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT;
                value->floating_point = as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)) *
                                        as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(
                    get_type_info(mem, context, node->base.properties.type, &node->base.source_location, &type_info));
                APPLY_SIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), *,
                                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            }
            break;

        case KEFIR_AST_OPERATION_DIVIDE:
            if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT)) {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT;
                struct complex_float f1 = as_complex_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1));
                struct complex_float f2 = as_complex_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
                kefir_ast_constant_expression_float_t u = f1.real, x = f2.real, v = f1.imaginary, y = f2.imaginary;
                value->complex_floating_point.real = (u * x + v * y) / (x * x + y * y);
                value->complex_floating_point.imaginary = (v * x - u * y) / (x * x + y * y);
            } else if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT)) {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT;
                value->floating_point = as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)) /
                                        as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else if (common_type_signed_integer) {
                REQUIRE(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)->integer != 0,
                        KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->arg2->source_location,
                                               "Expected non-zero divisor in constant expression"));
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(
                    get_type_info(mem, context, node->base.properties.type, &node->base.source_location, &type_info));
                APPLY_SIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), /,
                                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else {
                REQUIRE(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)->integer != 0,
                        KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->arg2->source_location,
                                               "Expected non-zero divisor in constant expression"));
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(
                    get_type_info(mem, context, node->base.properties.type, &node->base.source_location, &type_info));
                APPLY_UNSIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), /,
                                  KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            }
            break;

        case KEFIR_AST_OPERATION_MODULO:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            REQUIRE(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)->integer != 0,
                    KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->arg2->source_location,
                                           "Expected non-zero divisor in constant expression"));
            if (common_type_signed_integer) {
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(
                    get_type_info(mem, context, node->base.properties.type, &node->base.source_location, &type_info));
                APPLY_SIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), %,
                                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else {
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(
                    get_type_info(mem, context, node->base.properties.type, &node->base.source_location, &type_info));
                APPLY_UNSIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), %,
                                  KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            }
            break;

        case KEFIR_AST_OPERATION_SHIFT_LEFT: {
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            struct kefir_ast_target_environment_object_info type_info;
            REQUIRE_OK(
                get_type_info(mem, context, node->base.properties.type, &node->base.source_location, &type_info));
            if (common_type_signed_integer) {
                APPLY_SIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), <<,
                                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else {
                APPLY_UNSIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), <<,
                                  KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            }
        } break;

        case KEFIR_AST_OPERATION_SHIFT_RIGHT: {
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            struct kefir_ast_target_environment_object_info type_info;
            REQUIRE_OK(
                get_type_info(mem, context, node->base.properties.type, &node->base.source_location, &type_info));
            if (common_type_signed_integer) {
                APPLY_SIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), >>,
                                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else {
                APPLY_UNSIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), >>,
                                  KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            }
        } break;

        case KEFIR_AST_OPERATION_LESS:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS)) {
                return KEFIR_SET_ERROR(KEFIR_NOT_CONSTANT,
                                       "Constant expressions with address comparisons are not supported");
            } else if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT)) {
                return KEFIR_SET_SOURCE_ERROR(
                    KEFIR_NOT_CONSTANT, &node->base.source_location,
                    "Constant expressions with complex floating point comparisons are invalid");
            } else if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT)) {
                value->integer = as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)) <
                                 as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else if (common_type_signed_integer) {
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(get_type_info(mem, context, common_arith_type, &node->base.source_location, &type_info));
                APPLY_SIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), <,
                                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else {
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(get_type_info(mem, context, common_arith_type, &node->base.source_location, &type_info));
                APPLY_UNSIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), <,
                                  KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            }
            break;

        case KEFIR_AST_OPERATION_LESS_EQUAL:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS)) {
                return KEFIR_SET_ERROR(KEFIR_NOT_CONSTANT,
                                       "Constant expressions with address comparisons are not supported");
            } else if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT)) {
                return KEFIR_SET_SOURCE_ERROR(
                    KEFIR_NOT_CONSTANT, &node->base.source_location,
                    "Constant expressions with complex floating point comparisons are invalid");
            } else if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT)) {
                value->integer = as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)) <=
                                 as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else if (common_type_signed_integer) {
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(get_type_info(mem, context, common_arith_type, &node->base.source_location, &type_info));
                APPLY_SIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), <=,
                                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else {
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(get_type_info(mem, context, common_arith_type, &node->base.source_location, &type_info));
                APPLY_UNSIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), <=,
                                  KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            }
            break;

        case KEFIR_AST_OPERATION_GREATER:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS)) {
                return KEFIR_SET_ERROR(KEFIR_NOT_CONSTANT,
                                       "Constant expressions with address comparisons are not supported");
            } else if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT)) {
                return KEFIR_SET_SOURCE_ERROR(
                    KEFIR_NOT_CONSTANT, &node->base.source_location,
                    "Constant expressions with complex floating point comparisons are invalid");
            } else if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT)) {
                value->integer = as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)) >
                                 as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else if (common_type_signed_integer) {
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(get_type_info(mem, context, common_arith_type, &node->base.source_location, &type_info));
                APPLY_SIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), >,
                                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else {
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(get_type_info(mem, context, common_arith_type, &node->base.source_location, &type_info));
                APPLY_UNSIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), >,
                                  KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            }
            break;

        case KEFIR_AST_OPERATION_GREATER_EQUAL:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS)) {
                return KEFIR_SET_ERROR(KEFIR_NOT_CONSTANT,
                                       "Constant expressions with address comparisons are not supported");
            } else if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT)) {
                return KEFIR_SET_SOURCE_ERROR(
                    KEFIR_NOT_CONSTANT, &node->base.source_location,
                    "Constant expressions with complex floating point comparisons are invalid");
            } else if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT)) {
                value->integer = as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)) >=
                                 as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else if (common_type_signed_integer) {
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(get_type_info(mem, context, common_arith_type, &node->base.source_location, &type_info));
                APPLY_SIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), >=,
                                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else {
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(get_type_info(mem, context, common_arith_type, &node->base.source_location, &type_info));
                APPLY_UNSIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), >=,
                                  KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            }
            break;

        case KEFIR_AST_OPERATION_EQUAL:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS)) {
                return KEFIR_SET_ERROR(KEFIR_NOT_CONSTANT,
                                       "Constant expressions with address comparisons are not supported");
            } else if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT)) {
                struct complex_float f1 = as_complex_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1));
                struct complex_float f2 = as_complex_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
                value->integer = f1.real == f2.real && f1.imaginary == f2.imaginary;
            } else if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT)) {
                value->integer = as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)) ==
                                 as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else {
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(get_type_info(mem, context, common_arith_type, &node->base.source_location, &type_info));
                APPLY_SIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), ==,
                                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            }
            break;

        case KEFIR_AST_OPERATION_NOT_EQUAL:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS)) {
                return KEFIR_SET_ERROR(KEFIR_NOT_CONSTANT,
                                       "Constant expressions with address comparisons are not supported");
            } else if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT)) {
                struct complex_float f1 = as_complex_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1));
                struct complex_float f2 = as_complex_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
                value->integer = f1.real != f2.real || f1.imaginary != f2.imaginary;
            } else if (ANY_OF(node->arg1, node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT)) {
                value->integer = as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)) !=
                                 as_float(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            } else {
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(get_type_info(mem, context, common_arith_type, &node->base.source_location, &type_info));
                APPLY_SIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), !=,
                                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
            }
            break;

        case KEFIR_AST_OPERATION_BITWISE_AND: {
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            struct kefir_ast_target_environment_object_info type_info;
            REQUIRE_OK(get_type_info(mem, context, common_arith_type, &node->base.source_location, &type_info));
            APPLY_UNSIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), &,
                              KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
        } break;

        case KEFIR_AST_OPERATION_BITWISE_OR: {
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            struct kefir_ast_target_environment_object_info type_info;
            REQUIRE_OK(get_type_info(mem, context, common_arith_type, &node->base.source_location, &type_info));
            APPLY_UNSIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), |,
                              KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
        } break;

        case KEFIR_AST_OPERATION_BITWISE_XOR: {
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            struct kefir_ast_target_environment_object_info type_info;
            REQUIRE_OK(get_type_info(mem, context, common_arith_type, &node->base.source_location, &type_info));
            APPLY_UNSIGNED_OP(type_info.size, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1), ^,
                              KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
        } break;

#define CONV_BOOL(_size, _result, _arg)                                        \
    do {                                                                       \
        switch ((_size)) {                                                     \
            case 1:                                                            \
                *(_result) = (kefir_bool_t) (kefir_uint8_t) (_arg)->uinteger;  \
                break;                                                         \
                                                                               \
            case 2:                                                            \
                *(_result) = (kefir_bool_t) (kefir_uint16_t) (_arg)->uinteger; \
                break;                                                         \
                                                                               \
            case 3:                                                            \
            case 4:                                                            \
                *(_result) = (kefir_bool_t) (kefir_uint32_t) (_arg)->uinteger; \
                break;                                                         \
                                                                               \
            default:                                                           \
                *(_result) = (kefir_bool_t) (_arg)->uinteger;                  \
                break;                                                         \
        }                                                                      \
    } while (0)
        case KEFIR_AST_OPERATION_LOGICAL_AND: {
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            kefir_bool_t arg1_bool = false;
            kefir_bool_t arg2_bool = false;
            if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg1, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT)) {
                arg1_bool = (kefir_bool_t) KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)->floating_point;
            } else if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg1,
                                                                KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT)) {
                arg1_bool =
                    (kefir_bool_t) KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)->complex_floating_point.real ||
                    (kefir_bool_t) KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)
                        ->complex_floating_point.imaginary;
            } else if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg1,
                                                                KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS)) {
                arg1_bool = KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)->pointer.type !=
                                KEFIR_AST_CONSTANT_EXPRESSION_POINTER_INTEGER ||
                            KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)->pointer.base.integral +
                                    KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)->pointer.offset !=
                                0;
            } else {
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(
                    get_type_info(mem, context, node->arg1->properties.type, &node->arg1->source_location, &type_info));
                CONV_BOOL(type_info.size, &arg1_bool, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1));
            }

            if (arg1_bool) {
                REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION(node->arg2),
                        KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->arg2->source_location,
                                               "Unable to evaluate constant expression"));
                if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT)) {
                    arg2_bool = (kefir_bool_t) KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)->floating_point;
                } else if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(
                               node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT)) {
                    arg2_bool = (kefir_bool_t) KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)
                                    ->complex_floating_point.real ||
                                (kefir_bool_t) KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)
                                    ->complex_floating_point.imaginary;
                } else if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg2,
                                                                    KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS)) {
                    arg2_bool = KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)->pointer.type !=
                                    KEFIR_AST_CONSTANT_EXPRESSION_POINTER_INTEGER ||
                                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)->pointer.base.integral +
                                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)->pointer.offset !=
                                    0;
                } else {
                    struct kefir_ast_target_environment_object_info type_info;
                    REQUIRE_OK(get_type_info(mem, context, node->arg2->properties.type, &node->arg2->source_location,
                                             &type_info));
                    CONV_BOOL(type_info.size, &arg2_bool, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
                }
            }

            value->integer = arg1_bool && arg2_bool;
        } break;

        case KEFIR_AST_OPERATION_LOGICAL_OR: {
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            kefir_bool_t arg1_bool = false;
            kefir_bool_t arg2_bool = false;
            if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg1, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT)) {
                arg1_bool = (kefir_bool_t) KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)->floating_point;
            } else if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg1,
                                                                KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT)) {
                arg1_bool =
                    (kefir_bool_t) KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)->complex_floating_point.real ||
                    (kefir_bool_t) KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)
                        ->complex_floating_point.imaginary;
            } else if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg1,
                                                                KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS)) {
                arg1_bool = KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)->pointer.type !=
                                KEFIR_AST_CONSTANT_EXPRESSION_POINTER_INTEGER ||
                            KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)->pointer.base.integral +
                                    KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1)->pointer.offset !=
                                0;
            } else {
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(
                    get_type_info(mem, context, node->arg1->properties.type, &node->arg1->source_location, &type_info));
                CONV_BOOL(type_info.size, &arg1_bool, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg1));
            }

            if (!arg1_bool) {
                REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION(node->arg2),
                        KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->arg2->source_location,
                                               "Unable to evaluate constant expression"));
                if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT)) {
                    arg2_bool = (kefir_bool_t) KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)->floating_point;
                } else if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(
                               node->arg2, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT)) {
                    arg2_bool = (kefir_bool_t) KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)
                                    ->complex_floating_point.real ||
                                (kefir_bool_t) KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)
                                    ->complex_floating_point.imaginary;
                } else if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg2,
                                                                    KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS)) {
                    arg2_bool = KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)->pointer.type !=
                                    KEFIR_AST_CONSTANT_EXPRESSION_POINTER_INTEGER ||
                                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)->pointer.base.integral +
                                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2)->pointer.offset !=
                                    0;
                } else {
                    struct kefir_ast_target_environment_object_info type_info;
                    REQUIRE_OK(get_type_info(mem, context, node->arg2->properties.type, &node->arg2->source_location,
                                             &type_info));
                    CONV_BOOL(type_info.size, &arg2_bool, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg2));
                }
            }

            value->integer = arg1_bool || arg2_bool;
        } break;
#undef CONV_BOOL
#undef APPLY_SIGNED_OP
#undef APPLY_UNSIGNED_OP
    }
    return KEFIR_OK;
}
