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
#include "kefir/ast/type_completion.h"
#include "kefir/ast/downcast.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

static kefir_result_t unwrap_vla_type(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                      const struct kefir_ast_type *type, const struct kefir_ast_type **result) {
    REQUIRE_OK(kefir_ast_type_completion(mem, context, &type, type));

    if (KEFIR_AST_TYPE_IS_VL_ARRAY(type)) {
        const struct kefir_ast_type *element_type = NULL;
        REQUIRE_OK(unwrap_vla_type(mem, context, type->array_type.element_type, &element_type));
        const struct kefir_ast_type *array_type =
            kefir_ast_type_array(mem, context->type_bundle, element_type, 1, NULL);
        REQUIRE(array_type != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed toa allocate AST array type"));
        *result = array_type;
    } else {
        *result = type;
    }
    return KEFIR_OK;
}

static kefir_result_t get_type_info(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                    const struct kefir_ast_type *type,
                                    const struct kefir_source_location *source_location,
                                    struct kefir_ast_target_environment_object_info *type_info) {
    kefir_ast_target_environment_opaque_type_t opaque_type;
    REQUIRE_OK(kefir_ast_context_type_cache_get_type(mem, context->cache, type, &opaque_type, source_location));
    REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_OBJECT_INFO(mem, context->target_env, opaque_type, NULL, type_info));

    return KEFIR_OK;
}

static kefir_result_t calculate_type_alignment(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                               const struct kefir_ast_type *base_type, kefir_size_t *alignment,
                                               const struct kefir_source_location *source_location) {
    const struct kefir_ast_type *type = NULL;
    REQUIRE_OK(unwrap_vla_type(mem, context, base_type, &type));
    struct kefir_ast_target_environment_object_info type_info;
    REQUIRE_OK(get_type_info(mem, context, type, source_location, &type_info));
    *alignment = type_info.alignment;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_evaluate_unary_operation_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                       const struct kefir_ast_unary_operation *node,
                                                       struct kefir_ast_constant_expression_value *value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST unary operation node"));
    REQUIRE(value != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant expression value pointer"));
    REQUIRE(node->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
            KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                   "Expected constant expression AST node"));

    kefir_bool_t type_signed_integer = false;
    if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(node->base.properties.type)) {
        REQUIRE_OK(kefir_ast_type_is_signed(context->type_traits, node->base.properties.type, &type_signed_integer));
    }

    switch (node->type) {
        case KEFIR_AST_OPERATION_PLUS:
            if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER)) {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
                value->integer = KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)->integer;
                value->bitprecise = KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)->bitprecise;
            } else if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg,
                                                                KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT)) {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT;
                value->complex_floating_point.real =
                    KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)->complex_floating_point.real;
                value->complex_floating_point.imaginary =
                    KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)->complex_floating_point.imaginary;
            } else if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT)) {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT;
                value->floating_point = KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)->floating_point;
            } else {
                return KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->arg->source_location,
                                              "Expected integeral or floating-point constant expression");
            }
            break;

#define APPLY_UNARY_SIGNED_OP(_size, _result, _op, _arg)                \
    do {                                                                \
        switch ((_size)) {                                              \
            case 1:                                                     \
                (_result)->integer = _op(kefir_int8_t)(_arg)->integer;  \
                break;                                                  \
                                                                        \
            case 2:                                                     \
                (_result)->integer = _op(kefir_int16_t)(_arg)->integer; \
                break;                                                  \
                                                                        \
            case 3:                                                     \
            case 4:                                                     \
                (_result)->integer = _op(kefir_int32_t)(_arg)->integer; \
                break;                                                  \
                                                                        \
            default:                                                    \
                (_result)->integer = _op(_arg)->integer;                \
                break;                                                  \
        }                                                               \
    } while (0)
#define APPLY_UNARY_UNSIGNED_OP(_size, _result, _op, _arg)                 \
    do {                                                                   \
        switch ((_size)) {                                                 \
            case 1:                                                        \
                (_result)->uinteger = _op(kefir_uint8_t)(_arg)->uinteger;  \
                break;                                                     \
                                                                           \
            case 2:                                                        \
                (_result)->uinteger = _op(kefir_uint16_t)(_arg)->uinteger; \
                break;                                                     \
                                                                           \
            case 3:                                                        \
            case 4:                                                        \
                (_result)->uinteger = _op(kefir_uint32_t)(_arg)->uinteger; \
                break;                                                     \
                                                                           \
            default:                                                       \
                (_result)->uinteger = _op(_arg)->uinteger;                 \
                break;                                                     \
        }                                                                  \
    } while (0)
        case KEFIR_AST_OPERATION_NEGATE:
            if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER)) {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(
                    get_type_info(mem, context, node->base.properties.type, &node->base.source_location, &type_info));
                if (type_signed_integer) {
                    if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(node->base.properties.type)) {
                        REQUIRE_OK(kefir_bigint_pool_alloc(mem, context->bigint_pool, &value->bitprecise));
                        REQUIRE_OK(kefir_bigint_copy_resize(
                            mem, value->bitprecise, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)->bitprecise));
                        REQUIRE_OK(kefir_bigint_negate(value->bitprecise));
                        REQUIRE_OK(kefir_bigint_get_signed(value->bitprecise, &value->integer));
                    } else {
                        APPLY_UNARY_SIGNED_OP(type_info.size, value, -,
                                              KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg));
                    }
                } else {
                    if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(node->base.properties.type)) {
                        REQUIRE_OK(kefir_bigint_pool_alloc(mem, context->bigint_pool, &value->bitprecise));
                        REQUIRE_OK(kefir_bigint_copy_resize(
                            mem, value->bitprecise, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)->bitprecise));
                        REQUIRE_OK(kefir_bigint_negate(value->bitprecise));
                        REQUIRE_OK(kefir_bigint_get_unsigned(value->bitprecise, &value->uinteger));
                    } else {
                        APPLY_UNARY_UNSIGNED_OP(type_info.size, value, -,
                                                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg));
                    }
                }
            } else if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg,
                                                                KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT)) {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT;
                value->complex_floating_point.real =
                    -KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)->complex_floating_point.real;
                value->complex_floating_point.imaginary =
                    -KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)->complex_floating_point.imaginary;
            } else if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT)) {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT;
                value->floating_point = -KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)->floating_point;
            } else {
                return KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->arg->source_location,
                                              "Expected integeral or floating-point constant expression");
            }
            break;

        case KEFIR_AST_OPERATION_INVERT:
            if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER)) {
                value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(
                    get_type_info(mem, context, node->base.properties.type, &node->base.source_location, &type_info));
                if (type_signed_integer) {
                    if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(node->base.properties.type)) {
                        REQUIRE_OK(kefir_bigint_pool_alloc(mem, context->bigint_pool, &value->bitprecise));
                        REQUIRE_OK(kefir_bigint_copy_resize(
                            mem, value->bitprecise, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)->bitprecise));
                        REQUIRE_OK(kefir_bigint_invert(value->bitprecise));
                        REQUIRE_OK(kefir_bigint_get_signed(value->bitprecise, &value->integer));
                    } else {
                        APPLY_UNARY_SIGNED_OP(type_info.size, value, ~,
                                              KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg));
                    }
                } else {
                    if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(node->base.properties.type)) {
                        REQUIRE_OK(kefir_bigint_pool_alloc(mem, context->bigint_pool, &value->bitprecise));
                        REQUIRE_OK(kefir_bigint_copy_resize(
                            mem, value->bitprecise, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)->bitprecise));
                        REQUIRE_OK(kefir_bigint_invert(value->bitprecise));
                        REQUIRE_OK(kefir_bigint_get_unsigned(value->bitprecise, &value->uinteger));
                    } else {
                        APPLY_UNARY_UNSIGNED_OP(type_info.size, value, ~,
                                                KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg));
                    }
                }
            } else {
                return KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->arg->source_location,
                                              "Expected integeral constant expression");
            }
            break;

        case KEFIR_AST_OPERATION_LOGICAL_NEGATE:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER)) {
                struct kefir_ast_target_environment_object_info type_info;
                REQUIRE_OK(
                    get_type_info(mem, context, node->arg->properties.type, &node->base.source_location, &type_info));
                if (KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(node->arg->properties.type)) {
                    kefir_bool_t is_zero = false;
                    REQUIRE_OK(kefir_bigint_is_zero(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)->bitprecise,
                                                    &is_zero));
                    value->integer = is_zero ? 1 : 0;
                } else if (type_signed_integer) {
                    APPLY_UNARY_SIGNED_OP(type_info.size, value, !,
                                          KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg));
                } else {
                    APPLY_UNARY_UNSIGNED_OP(type_info.size, value, !,
                                            KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg));
                }
            } else if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg,
                                                                KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT)) {
                value->integer =
                    !((kefir_bool_t) KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)->complex_floating_point.real ||
                      (kefir_bool_t) KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)
                          ->complex_floating_point.imaginary);
            } else if (KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->arg, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT)) {
                value->integer = !KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)->floating_point;
            } else {
                REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION(node->arg),
                        KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->arg->source_location,
                                               "Unable to evaluate constant expression"));
                switch (KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)->pointer.type) {
                    case KEFIR_AST_CONSTANT_EXPRESSION_POINTER_IDENTIFER:
                    case KEFIR_AST_CONSTANT_EXPRESSION_POINTER_LITERAL:
                        value->integer = false;
                        break;

                    case KEFIR_AST_CONSTANT_EXPRESSION_POINTER_INTEGER:
                        value->integer = !(
                            kefir_bool_t) (KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)->pointer.base.integral +
                                           KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->arg)->pointer.offset);
                        break;
                }
            }
            break;
#undef APPLY_UNARY_SIGNED_OP
#undef APPLY_UNARY_UNSIGNED_OP

        case KEFIR_AST_OPERATION_POSTFIX_INCREMENT:
        case KEFIR_AST_OPERATION_POSTFIX_DECREMENT:
        case KEFIR_AST_OPERATION_PREFIX_INCREMENT:
        case KEFIR_AST_OPERATION_PREFIX_DECREMENT:
            return KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                          "Constant expressions shall not contain increment/decrement operators");

        case KEFIR_AST_OPERATION_ADDRESS:
            REQUIRE_OK(kefir_ast_constant_expression_value_evaluate_lvalue_reference(mem, context, node->arg,
                                                                                     &value->pointer));
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS;
            break;

        case KEFIR_AST_OPERATION_INDIRECTION: {
            struct kefir_ast_unary_operation *indirect_op;
            kefir_result_t res = kefir_ast_downcast_unary_operation(node->arg, &indirect_op, false);
            if (res != KEFIR_NO_MATCH) {
                REQUIRE_OK(res);
                REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION(indirect_op->arg),
                        KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                               "Constant expression cannot contain indirection operator"));
                *value = *KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(indirect_op->arg);
            } else {
                return KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                              "Constant expression cannot contain indirection operator");
            }
        } break;

        case KEFIR_AST_OPERATION_SIZEOF: {
            const struct kefir_ast_type *type = kefir_ast_unqualified_type(node->arg->properties.type);
            if (context->configuration->analysis.ext_pointer_arithmetics &&
                (type->tag == KEFIR_AST_TYPE_FUNCTION ||
                 kefir_ast_unqualified_type(type)->tag == KEFIR_AST_TYPE_VOID)) {
                type = context->type_traits->incomplete_type_substitute;
            }

            REQUIRE_OK(kefir_ast_type_completion(mem, context, &type, type));
            if (type->tag == KEFIR_AST_TYPE_ARRAY) {
                REQUIRE(
                    !KEFIR_AST_TYPE_IS_VL_ARRAY(type),
                    KEFIR_SET_ERROR(KEFIR_NOT_CONSTANT, "Size of variable-length array is not a constant expression"));
            }

            struct kefir_ast_target_environment_object_info type_info;
            REQUIRE_OK(get_type_info(mem, context, type, &node->base.source_location, &type_info));
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            value->integer = type_info.size;
        } break;

        case KEFIR_AST_OPERATION_ALIGNOF: {
            kefir_size_t alignment = 0;
            if (node->arg->properties.category == KEFIR_AST_NODE_CATEGORY_TYPE) {
                if (node->arg->properties.type_props.alignment == 0) {
                    REQUIRE_OK(calculate_type_alignment(mem, context, node->arg->properties.type, &alignment,
                                                        &node->base.source_location));
                } else {
                    alignment = node->arg->properties.type_props.alignment;
                }
            } else if (node->arg->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION) {
                if (node->arg->properties.expression_props.alignment == 0) {
                    REQUIRE_OK(calculate_type_alignment(mem, context, node->arg->properties.type, &alignment,
                                                        &node->base.source_location));
                } else {
                    alignment = node->arg->properties.expression_props.alignment;
                }
            }
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            value->integer = alignment;
        } break;
    }
    return KEFIR_OK;
}
