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

static kefir_result_t cast_integral_type(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                         const struct kefir_ast_type *destination_type,
                                         const struct kefir_ast_type *source_type,
                                         struct kefir_ast_constant_expression_value *value,
                                         kefir_ast_constant_expression_int_t source,
                                         const struct kefir_bigint *source_bitprecise,
                                         const struct kefir_source_location *source_location) {
    kefir_ast_target_environment_opaque_type_t opaque_type;
    struct kefir_ast_target_environment_object_info type_info;
    REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_GET_TYPE(mem, context, context->target_env, destination_type, &opaque_type,
                                                     source_location));
    kefir_result_t res =
        KEFIR_AST_TARGET_ENVIRONMENT_OBJECT_INFO(mem, context->target_env, opaque_type, NULL, &type_info);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->target_env, opaque_type);
        return res;
    });
    REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->target_env, opaque_type));

    kefir_bool_t signed_destination_integer = false, signed_source_integer = false;
    REQUIRE_OK(kefir_ast_type_is_signed(context->type_traits, destination_type, &signed_destination_integer));
    REQUIRE_OK(kefir_ast_type_is_signed(context->type_traits, source_type, &signed_source_integer));

    if (destination_type->tag == KEFIR_AST_TYPE_SCALAR_BOOL) {
        value->integer = (bool) source;
    } else if (signed_destination_integer) {
        switch (type_info.size) {
            case 1:
                value->integer = (kefir_int8_t) source;
                break;

            case 2:
                value->integer = (kefir_int16_t) source;
                break;

            case 3:
            case 4:
                value->integer = (kefir_int32_t) source;
                break;

            default:
                value->integer = source;
                break;
        }
    } else {
        switch (type_info.size) {
            case 1:
                value->uinteger = (kefir_uint8_t) source;
                break;

            case 2:
                value->uinteger = (kefir_uint16_t) source;
                break;

            case 3:
            case 4:
                value->uinteger = (kefir_uint32_t) source;
                break;

            default:
                value->uinteger = source;
                break;
        }
    }

    if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(destination_type)) {
        REQUIRE_OK(kefir_bigint_pool_alloc(mem, context->bigint_pool, &value->bitprecise));
        if (signed_source_integer) {
            if (source_bitprecise) {
                REQUIRE_OK(kefir_bigint_copy_resize(mem, value->bitprecise, source_bitprecise));
                REQUIRE_OK(kefir_bigint_resize_cast_signed(mem, value->bitprecise, destination_type->bitprecise.width));
            } else {
                REQUIRE_OK(kefir_bigint_resize_nocast(mem, value->bitprecise, destination_type->bitprecise.width));
                REQUIRE_OK(kefir_bigint_set_signed_value(value->bitprecise, value->integer));
            }
        } else {
            if (source_bitprecise) {
                REQUIRE_OK(kefir_bigint_copy_resize(mem, value->bitprecise, source_bitprecise));
                REQUIRE_OK(
                    kefir_bigint_resize_cast_unsigned(mem, value->bitprecise, destination_type->bitprecise.width));
            } else {
                REQUIRE_OK(kefir_bigint_resize_nocast(mem, value->bitprecise, destination_type->bitprecise.width));
                REQUIRE_OK(kefir_bigint_set_unsigned_value(value->bitprecise, value->integer));
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t cast_integral_type_from_float(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                    const struct kefir_ast_type *type,
                                                    struct kefir_ast_constant_expression_value *value,
                                                    kefir_ast_constant_expression_float_t source,
                                                    const struct kefir_source_location *source_location) {
    kefir_ast_target_environment_opaque_type_t opaque_type;
    struct kefir_ast_target_environment_object_info type_info;
    REQUIRE_OK(
        KEFIR_AST_TARGET_ENVIRONMENT_GET_TYPE(mem, context, context->target_env, type, &opaque_type, source_location));
    kefir_result_t res =
        KEFIR_AST_TARGET_ENVIRONMENT_OBJECT_INFO(mem, context->target_env, opaque_type, NULL, &type_info);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->target_env, opaque_type);
        return res;
    });
    REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->target_env, opaque_type));

    kefir_bool_t signed_integer = false;
    REQUIRE_OK(kefir_ast_type_is_signed(context->type_traits, type, &signed_integer));

    if (type->tag == KEFIR_AST_TYPE_SCALAR_BOOL) {
        value->integer = (bool) source;
    } else if (signed_integer) {
        switch (type_info.size) {
            case 1:
                value->integer = (kefir_int8_t) source;
                break;

            case 2:
                value->integer = (kefir_int16_t) source;
                break;

            case 3:
            case 4:
                value->integer = (kefir_int32_t) source;
                break;

            default:
                value->integer = source;
                break;
        }

        if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(type)) {
            REQUIRE_OK(kefir_bigint_pool_alloc(mem, context->bigint_pool, &value->bitprecise));
            REQUIRE_OK(kefir_bigint_resize_nocast(mem, value->bitprecise, type->bitprecise.width));
            REQUIRE_OK(kefir_bigint_signed_from_long_double(value->bitprecise, source));
        }
    } else {
        switch (type_info.size) {
            case 1:
                value->uinteger = (kefir_uint8_t) source;
                break;

            case 2:
                value->uinteger = (kefir_uint16_t) source;
                break;

            case 3:
            case 4:
                value->uinteger = (kefir_uint32_t) source;
                break;

            default:
                value->uinteger = source;
                break;
        }

        if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(type)) {
            REQUIRE_OK(kefir_bigint_pool_alloc(mem, context->bigint_pool, &value->bitprecise));
            REQUIRE_OK(kefir_bigint_resize_nocast(mem, value->bitprecise, type->bitprecise.width));
            REQUIRE_OK(kefir_bigint_unsigned_from_long_double(value->bitprecise, source));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_constant_expression_value_cast(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                        struct kefir_ast_constant_expression_value *value,
                                                        const struct kefir_ast_constant_expression_value *source,
                                                        const struct kefir_ast_node_base *node,
                                                        const struct kefir_ast_type *destination_type,
                                                        const struct kefir_ast_type *source_type) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(value != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant expression value pointer"));
    REQUIRE(source != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source AST constant expression value"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid base AST node"));
    REQUIRE(destination_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination AST type"));
    REQUIRE(source_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source AST type"));

    memset(value, 0, sizeof(struct kefir_ast_constant_expression_value));
    const struct kefir_ast_type *unqualified_destination_type = kefir_ast_unqualified_type(destination_type);
    const struct kefir_ast_type *unqualified_source_type = kefir_ast_unqualified_type(source_type);
    if (unqualified_destination_type->tag == KEFIR_AST_TYPE_VOID) {
        value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_NONE;
    } else if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(unqualified_destination_type)) {
        value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
        switch (source->klass) {
            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER:
                REQUIRE_OK(cast_integral_type(mem, context, unqualified_destination_type, unqualified_source_type,
                                              value, source->integer, source->bitprecise, &node->source_location));
                break;

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT:
                if (unqualified_destination_type->tag == KEFIR_AST_TYPE_SCALAR_BOOL) {
                    value->integer = (kefir_bool_t) source->floating_point;
                } else {
                    REQUIRE_OK(cast_integral_type_from_float(mem, context, unqualified_destination_type, value,
                                                             source->floating_point, &node->source_location));
                }
                break;

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT:
                if (unqualified_destination_type->tag == KEFIR_AST_TYPE_SCALAR_BOOL) {
                    value->integer = (kefir_bool_t) source->complex_floating_point.real;
                } else {
                    REQUIRE_OK(cast_integral_type_from_float(mem, context, unqualified_destination_type, value,
                                                             source->complex_floating_point.real,
                                                             &node->source_location));
                }
                break;

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS:
                if (source->pointer.type == KEFIR_AST_CONSTANT_EXPRESSION_POINTER_INTEGER) {
                    value->integer = source->pointer.base.integral + source->pointer.offset;
                } else {
                    value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS;
                    value->pointer = source->pointer;
                    value->pointer.pointer_node = node;
                }
                break;

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPOUND:
                return KEFIR_SET_ERROR(KEFIR_NOT_CONSTANT, "Unable to cast compound constant expression");

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_NONE:
                return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Non-evaluated constant expression");
        }
    } else if (KEFIR_AST_TYPE_IS_REAL_FLOATING_POINT(unqualified_destination_type)) {
        value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT;
        switch (source->klass) {
            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER:
                if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(unqualified_source_type)) {
                    struct kefir_bigint *tmp_bigint, *tmp2_bigint;
                    REQUIRE_OK(kefir_bigint_pool_alloc(mem, context->bigint_pool, &tmp_bigint));
                    REQUIRE_OK(kefir_bigint_pool_alloc(mem, context->bigint_pool, &tmp2_bigint));
                    REQUIRE_OK(kefir_bigint_resize_nocast(
                        mem, tmp2_bigint, MAX(source->bitprecise->bitwidth, sizeof(kefir_long_double_t) * CHAR_BIT)));
                    REQUIRE_OK(kefir_bigint_copy_resize(mem, tmp_bigint, source->bitprecise));

                    kefir_bool_t signed_integer = false;
                    REQUIRE_OK(
                        kefir_ast_type_is_signed(context->type_traits, unqualified_source_type, &signed_integer));
                    if (signed_integer) {
                        REQUIRE_OK(kefir_bigint_resize_cast_signed(mem, tmp_bigint, tmp2_bigint->bitwidth));
                        REQUIRE_OK(kefir_bigint_signed_to_long_double(tmp_bigint, tmp2_bigint, &value->floating_point));
                    } else {
                        REQUIRE_OK(kefir_bigint_resize_cast_unsigned(mem, tmp_bigint, tmp2_bigint->bitwidth));
                        REQUIRE_OK(
                            kefir_bigint_unsigned_to_long_double(tmp_bigint, tmp2_bigint, &value->floating_point));
                    }
                } else {
                    value->floating_point = (kefir_ast_constant_expression_float_t) source->integer;
                }
                break;

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT:
                value->floating_point = source->floating_point;
                break;

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT:
                value->floating_point = source->complex_floating_point.real;
                break;

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS:
                return KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->source_location,
                                              "Address to floating point cast is not a constant expression");

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPOUND:
                return KEFIR_SET_ERROR(KEFIR_NOT_CONSTANT, "Unable to cast compound constant expression");

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_NONE:
                return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Non-evaluated constant expression");
        }
    } else if (KEFIR_AST_TYPE_IS_COMPLEX_TYPE(unqualified_destination_type)) {
        value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT;
        switch (source->klass) {
            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER:
                if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(unqualified_source_type)) {
                    struct kefir_bigint *tmp_bigint, *tmp2_bigint;
                    REQUIRE_OK(kefir_bigint_pool_alloc(mem, context->bigint_pool, &tmp_bigint));
                    REQUIRE_OK(kefir_bigint_pool_alloc(mem, context->bigint_pool, &tmp2_bigint));
                    REQUIRE_OK(kefir_bigint_resize_nocast(
                        mem, tmp_bigint, MAX(source->bitprecise->bitwidth, sizeof(kefir_long_double_t) * CHAR_BIT)));
                    REQUIRE_OK(kefir_bigint_resize_nocast(mem, tmp2_bigint, tmp_bigint->bitwidth));
                    REQUIRE_OK(kefir_bigint_copy(tmp_bigint, source->bitprecise));

                    kefir_bool_t signed_integer = false;
                    REQUIRE_OK(
                        kefir_ast_type_is_signed(context->type_traits, unqualified_source_type, &signed_integer));
                    if (signed_integer) {
                        REQUIRE_OK(
                            kefir_bigint_cast_signed(tmp_bigint, source->bitprecise->bitwidth, tmp_bigint->bitwidth));
                        REQUIRE_OK(kefir_bigint_signed_to_long_double(tmp_bigint, tmp2_bigint,
                                                                      &value->complex_floating_point.real));
                    } else {
                        REQUIRE_OK(
                            kefir_bigint_cast_unsigned(tmp_bigint, source->bitprecise->bitwidth, tmp_bigint->bitwidth));
                        REQUIRE_OK(kefir_bigint_unsigned_to_long_double(tmp_bigint, tmp2_bigint,
                                                                        &value->complex_floating_point.real));
                    }
                } else {
                    value->complex_floating_point.real = (kefir_ast_constant_expression_float_t) source->integer;
                }
                value->complex_floating_point.imaginary = 0.0;
                break;

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT:
                value->complex_floating_point.real = source->floating_point;
                value->complex_floating_point.imaginary = 0.0;
                break;

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT:
                value->complex_floating_point.real = source->complex_floating_point.real;
                value->complex_floating_point.imaginary = source->complex_floating_point.imaginary;
                break;

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPOUND:
                return KEFIR_SET_ERROR(KEFIR_NOT_CONSTANT, "Unable to cast compound constant expression");

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS:
                return KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->source_location,
                                              "Address to floating point cast is not a constant expression");

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_NONE:
                return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Non-evaluated constant expression");
        }
    } else if (unqualified_destination_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER) {
        value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS;
        switch (source->klass) {
            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER:
                value->pointer.type = KEFIR_AST_CONSTANT_EXPRESSION_POINTER_INTEGER;
                value->pointer.base.integral = source->integer;
                value->pointer.offset = 0;
                value->pointer.pointer_node = node;
                break;

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT:
            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT:
                return KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->source_location,
                                              "Unable to cast floating point to address");

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS:
                value->pointer = source->pointer;
                value->pointer.pointer_node = node;
                break;

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPOUND:
                return KEFIR_SET_ERROR(KEFIR_NOT_CONSTANT, "Unable to cast compound constant expression");

            case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_NONE:
                return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Non-evaluated constant expression");
        }
    } else if (unqualified_destination_type->tag == KEFIR_AST_TYPE_SCALAR_NULL_POINTER) {
        value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS;
        value->pointer.type = KEFIR_AST_CONSTANT_EXPRESSION_POINTER_INTEGER;
        value->pointer.base.integral = 0;
        value->pointer.offset = 0;
        value->pointer.pointer_node = node;
    } else {
        return KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->source_location, "Expected constant expression");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_evaluate_cast_operator_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                     const struct kefir_ast_cast_operator *node,
                                                     struct kefir_ast_constant_expression_value *value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant node"));
    REQUIRE(value != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant expression value pointer"));
    REQUIRE(node->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
            KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                   "Expected constant expression AST node"));

    REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION(node->expr),
            KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->expr->source_location,
                                   "Unable to evaluate constant expression"));

    REQUIRE_OK(kefir_ast_constant_expression_value_cast(
        mem, context, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->expr), KEFIR_AST_NODE_BASE(node),
        node->base.properties.type, node->expr->properties.type));
    return KEFIR_OK;
}
