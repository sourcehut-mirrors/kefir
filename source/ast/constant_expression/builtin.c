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
#include "kefir/ast/type_conv.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"
#include "kefir/ir/type.h"
#include <math.h>

struct designator_param {
    struct kefir_mem *mem;
    const struct kefir_ast_context *context;
    const struct kefir_ast_node_base *base;
    struct kefir_ast_designator *designator;
};

static kefir_result_t visit_non_designator(const struct kefir_ast_visitor *visitor,
                                           const struct kefir_ast_node_base *base, void *payload) {
    UNUSED(visitor);
    UNUSED(base);
    UNUSED(payload);
    return KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &base->source_location, "Expected a member designator");
}

static kefir_result_t visit_identifier(const struct kefir_ast_visitor *visitor,
                                       const struct kefir_ast_identifier *identifier, void *payload) {
    REQUIRE(visitor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST visitor"));
    REQUIRE(identifier != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST identifier"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct designator_param *, param, payload);

    struct kefir_ast_designator *designator =
        kefir_ast_new_member_designator(param->mem, param->context->symbols, identifier->identifier, param->designator);
    REQUIRE(designator != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate member designator"));
    param->designator = designator;
    return KEFIR_OK;
}

static kefir_result_t visit_struct_member(const struct kefir_ast_visitor *visitor,
                                          const struct kefir_ast_struct_member *member, void *payload) {
    REQUIRE(visitor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST visitor"));
    REQUIRE(member != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST struct member"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct designator_param *, param, payload);

    REQUIRE_OK(KEFIR_AST_NODE_VISIT(visitor, member->structure, payload));

    struct kefir_ast_designator *designator =
        kefir_ast_new_member_designator(param->mem, param->context->symbols, member->member, param->designator);
    REQUIRE(designator != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate member designator"));
    param->designator = designator;
    return KEFIR_OK;
}

static kefir_result_t visit_array_subscript(const struct kefir_ast_visitor *visitor,
                                            const struct kefir_ast_array_subscript *subscript, void *payload) {
    REQUIRE(visitor != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST visitor"));
    REQUIRE(subscript != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST array subscript"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct designator_param *, param, payload);

    REQUIRE_OK(KEFIR_AST_NODE_VISIT(visitor, subscript->array, payload));

    const struct kefir_ast_type *array_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(param->mem, param->context->type_bundle, subscript->array->properties.type);

    struct kefir_ast_node_base *subscript_node;
    if (array_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER) {
        subscript_node = subscript->subscript;
    } else if (array_type->tag == KEFIR_AST_TYPE_SCALAR_NULL_POINTER) {
        return KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &subscript->array->source_location,
                                      "Unexpected null pointer");
    } else {
        subscript_node = subscript->array;
    }

    REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subscript_node, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
            KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subscript_node->source_location,
                                   "Unable to evaluate constant expression"));

    struct kefir_ast_designator *designator = kefir_ast_new_index_designator(
        param->mem, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subscript_node)->integer, param->designator);
    REQUIRE(designator != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate member designator"));
    param->designator = designator;
    return KEFIR_OK;
}

static kefir_result_t build_member_designator(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                              const struct kefir_ast_node_base *base,
                                              struct kefir_ast_designator **designator) {

    struct designator_param param = {.mem = mem, .context = context, .base = base, .designator = NULL};
    struct kefir_ast_visitor visitor;
    REQUIRE_OK(kefir_ast_visitor_init(&visitor, visit_non_designator));
    visitor.identifier = visit_identifier;
    visitor.struct_member = visit_struct_member;
    visitor.array_subscript = visit_array_subscript;
    kefir_result_t res = KEFIR_AST_NODE_VISIT(&visitor, base, &param);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_designator_free(mem, param.designator);
        return res;
    });
    *designator = param.designator;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_evaluate_builtin_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                               const struct kefir_ast_builtin *node,
                                               struct kefir_ast_constant_expression_value *value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant node"));
    REQUIRE(value != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant expression value pointer"));
    REQUIRE(node->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
            KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                   "Expected constant expression AST node"));

    const struct kefir_list_entry *iter = kefir_list_head(&node->arguments);
    switch (node->builtin) {
        case KEFIR_AST_BUILTIN_OFFSETOF: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, offset_base, iter->value);
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, member_designator, iter->value);

            struct kefir_ast_designator *designator = NULL;
            REQUIRE_OK(build_member_designator(mem, context, member_designator, &designator));

            kefir_ast_target_environment_opaque_type_t opaque_type;
            kefir_result_t res =
                KEFIR_AST_TARGET_ENVIRONMENT_GET_TYPE(mem, context, context->target_env, offset_base->properties.type,
                                                      &opaque_type, &node->base.source_location);
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_ast_designator_free(mem, designator);
                return res;
            });

            struct kefir_ast_target_environment_object_info objinfo;
            res = KEFIR_AST_TARGET_ENVIRONMENT_OBJECT_INFO(mem, context->target_env, opaque_type, designator, &objinfo);
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_ast_designator_free(mem, designator);
                KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->target_env, opaque_type);
                return res;
            });
            res = KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->target_env, opaque_type);
            REQUIRE_ELSE(res == KEFIR_OK, {
                kefir_ast_designator_free(mem, designator);
                return res;
            });
            REQUIRE_OK(kefir_ast_designator_free(mem, designator));

            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            value->integer = objinfo.relative_offset;
        } break;

        case KEFIR_AST_BUILTIN_TYPES_COMPATIBLE: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, type1_node, iter->value);
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, type2_node, iter->value);

            const struct kefir_ast_type *type1 = kefir_ast_unqualified_type(type1_node->properties.type);
            const struct kefir_ast_type *type2 = kefir_ast_unqualified_type(type2_node->properties.type);

            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            value->integer = KEFIR_AST_TYPE_COMPATIBLE(context->type_traits, type1, type2) ? 1 : 0;
        } break;

        case KEFIR_AST_BUILTIN_CHOOSE_EXPRESSION: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, cond_node, iter->value);
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, expr1_node, iter->value);
            kefir_list_next(&iter);
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, expr2_node, iter->value);

            REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(cond_node, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &cond_node->source_location,
                                           "Expected a constant expression evaluating to an integer"));

            if (KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(cond_node)->integer != 0) {
                REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION(expr1_node),
                        KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &expr1_node->source_location,
                                               "Unable to evaluate constant expression"));
                *value = expr1_node->properties.expression_props.constant_expression_value;
            } else {
                REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION(expr2_node),
                        KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &expr2_node->source_location,
                                               "Unable to evaluate constant expression"));
                *value = expr2_node->properties.expression_props.constant_expression_value;
            }
        } break;

        case KEFIR_AST_BUILTIN_CONSTANT: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, iter->value);

            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            if (!node->properties.expression_props.constant_expression) {
                value->integer = 0;
            } else {
                kefir_bool_t is_statically_known;
                REQUIRE_OK(kefir_ast_constant_expression_is_statically_known(
                    KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node), &is_statically_known));
                value->integer = is_statically_known ? 1 : 0;
            }
        } break;

        case KEFIR_AST_BUILTIN_CLASSIFY_TYPE: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, iter->value);

            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            kefir_int_t klass;
            REQUIRE_OK(kefir_ast_type_classify(node->properties.type, &klass));
            value->integer = klass;
        } break;

        case KEFIR_AST_BUILTIN_INFINITY_FLOAT32:
        case KEFIR_AST_BUILTIN_INFINITY_FLOAT64:
        case KEFIR_AST_BUILTIN_INFINITY_LONG_DOUBLE:
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT;
            value->floating_point = INFINITY;
            break;

        case KEFIR_AST_BUILTIN_NAN_FLOAT32:
        case KEFIR_AST_BUILTIN_NAN_FLOAT64:
        case KEFIR_AST_BUILTIN_NAN_LONG_DOUBLE: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, iter->value);
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT;
            value->floating_point = nanl(node->properties.expression_props.string_literal.content);
        } break;

        case KEFIR_AST_BUILTIN_FFSG: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, iter->value);
            const struct kefir_ast_type *unqualified_type = kefir_ast_unqualified_type(node->properties.type);
            if (unqualified_type->tag == KEFIR_AST_TYPE_ENUMERATION) {
                unqualified_type = unqualified_type->enumeration_type.underlying_type;
            }

            kefir_ast_type_data_model_classification_t classification;
            REQUIRE_OK(kefir_ast_type_data_model_classify(context->type_traits, unqualified_type, &classification));
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->source_location,
                                           "Expected integral constant expression"));
            switch (classification) {
                case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_ffs_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 8, value));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_ffs_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 16, value));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_ffs_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 32, value));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_ffs_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 64, value));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_BITINT: {
                    const struct kefir_ast_constant_expression_value *node_value =
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node);
                    if (node_value->bitprecise != NULL) {
                        kefir_size_t index;
                        REQUIRE_OK(kefir_bigint_least_significant_nonzero(node_value->bitprecise, &index));
                        value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
                        value->uinteger = index;
                    } else {
                        REQUIRE_OK(
                            kefir_ast_evaluate_builtin_ffs_constant_expression_value(node_value->uinteger, 64, value));
                    }
                } break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpectd ffsg builtin argument type");
            }
        } break;

        case KEFIR_AST_BUILTIN_CLZG: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, iter->value);
            struct kefir_ast_node_base *default_value_node = NULL;
            kefir_list_next(&iter);
            if (iter != NULL) {
                default_value_node = (struct kefir_ast_node_base *) iter->value;
            }
            const struct kefir_ast_type *unqualified_type = kefir_ast_unqualified_type(node->properties.type);
            if (unqualified_type->tag == KEFIR_AST_TYPE_ENUMERATION) {
                unqualified_type = unqualified_type->enumeration_type.underlying_type;
            }

            kefir_int64_t *default_value = NULL;
            if (default_value_node != NULL) {
                REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(default_value_node,
                                                                 KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                        KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->source_location,
                                               "Expected integral constant expression"));
                default_value = &KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(default_value_node)->integer;
            }

            kefir_ast_type_data_model_classification_t classification;
            REQUIRE_OK(kefir_ast_type_data_model_classify(context->type_traits, unqualified_type, &classification));
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->source_location,
                                           "Expected integral constant expression"));
            switch (classification) {
                case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_clz_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 8, default_value, value,
                        &node->source_location));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_clz_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 16, default_value, value,
                        &node->source_location));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_clz_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 32, default_value, value,
                        &node->source_location));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_clz_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 64, default_value, value,
                        &node->source_location));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_BITINT: {
                    const struct kefir_ast_constant_expression_value *node_value =
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node);
                    if (node_value->bitprecise != NULL) {
                        kefir_size_t count = 0;
                        if (default_value != NULL) {
                            REQUIRE_OK(kefir_bigint_leading_zeros(node_value->bitprecise, &count, *default_value));
                        } else {
                            REQUIRE_OK(kefir_bigint_leading_zeros(node_value->bitprecise, &count, ~0ull));
                            REQUIRE(count != ~0ull, KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->source_location,
                                                                           "Expected constant expression AST node"));
                        }
                        value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
                        value->uinteger = count;
                    } else {
                        REQUIRE_OK(kefir_ast_evaluate_builtin_clz_constant_expression_value(
                            node_value->uinteger, 64, default_value, value, &node->source_location));
                    }
                } break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpectd clzg builtin argument type");
            }
        } break;

        case KEFIR_AST_BUILTIN_CTZG: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, iter->value);
            struct kefir_ast_node_base *default_value_node = NULL;
            kefir_list_next(&iter);
            if (iter != NULL) {
                default_value_node = (struct kefir_ast_node_base *) iter->value;
            }
            const struct kefir_ast_type *unqualified_type = kefir_ast_unqualified_type(node->properties.type);
            if (unqualified_type->tag == KEFIR_AST_TYPE_ENUMERATION) {
                unqualified_type = unqualified_type->enumeration_type.underlying_type;
            }

            kefir_int64_t *default_value = NULL;
            if (default_value_node != NULL) {
                REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(default_value_node,
                                                                 KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                        KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->source_location,
                                               "Expected integral constant expression"));
                default_value = &KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(default_value_node)->integer;
            }

            kefir_ast_type_data_model_classification_t classification;
            REQUIRE_OK(kefir_ast_type_data_model_classify(context->type_traits, unqualified_type, &classification));
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->source_location,
                                           "Expected integral constant expression"));
            switch (classification) {
                case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_ctz_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 8, default_value, value,
                        &node->source_location));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_ctz_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 16, default_value, value,
                        &node->source_location));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_ctz_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 32, default_value, value,
                        &node->source_location));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_ctz_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 64, default_value, value,
                        &node->source_location));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_BITINT: {
                    const struct kefir_ast_constant_expression_value *node_value =
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node);
                    if (node_value->bitprecise != NULL) {
                        kefir_size_t count = 0;
                        if (default_value != NULL) {
                            REQUIRE_OK(kefir_bigint_trailing_zeros(node_value->bitprecise, &count, *default_value));
                        } else {
                            REQUIRE_OK(kefir_bigint_trailing_zeros(node_value->bitprecise, &count, ~0ull));
                            REQUIRE(count != ~0ull, KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->source_location,
                                                                           "Expected constant expression AST node"));
                        }
                        value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
                        value->uinteger = count;
                    } else {
                        REQUIRE_OK(kefir_ast_evaluate_builtin_ctz_constant_expression_value(
                            node_value->uinteger, 64, default_value, value, &node->source_location));
                    }
                } break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpectd clzg builtin argument type");
            }
        } break;

        case KEFIR_AST_BUILTIN_CLRSBG: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, iter->value);
            const struct kefir_ast_type *unqualified_type = kefir_ast_unqualified_type(node->properties.type);
            if (unqualified_type->tag == KEFIR_AST_TYPE_ENUMERATION) {
                unqualified_type = unqualified_type->enumeration_type.underlying_type;
            }

            kefir_ast_type_data_model_classification_t classification;
            REQUIRE_OK(kefir_ast_type_data_model_classify(context->type_traits, unqualified_type, &classification));
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->source_location,
                                           "Expected integral constant expression"));
            switch (classification) {
                case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_clrsb_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 8, value));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_clrsb_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 16, value));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_clrsb_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 32, value));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_clrsb_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 64, value));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_BITINT: {
                    const struct kefir_ast_constant_expression_value *node_value =
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node);
                    if (node_value->bitprecise != NULL) {
                        kefir_size_t count;
                        REQUIRE_OK(kefir_bigint_redundant_sign_bits(node_value->bitprecise, &count));
                        value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
                        value->uinteger = count;
                    } else {
                        REQUIRE_OK(kefir_ast_evaluate_builtin_clrsb_constant_expression_value(node_value->uinteger, 64,
                                                                                              value));
                    }
                } break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpectd clrsbg builtin argument type");
            }
        } break;

        case KEFIR_AST_BUILTIN_POPCOUNTG: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, iter->value);
            const struct kefir_ast_type *unqualified_type = kefir_ast_unqualified_type(node->properties.type);
            if (unqualified_type->tag == KEFIR_AST_TYPE_ENUMERATION) {
                unqualified_type = unqualified_type->enumeration_type.underlying_type;
            }

            kefir_ast_type_data_model_classification_t classification;
            REQUIRE_OK(kefir_ast_type_data_model_classify(context->type_traits, unqualified_type, &classification));
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->source_location,
                                           "Expected integral constant expression"));
            switch (classification) {
                case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_popcount_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 8, value));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_popcount_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 16, value));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_popcount_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 32, value));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_popcount_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 64, value));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_BITINT: {
                    const struct kefir_ast_constant_expression_value *node_value =
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node);
                    if (node_value->bitprecise != NULL) {
                        kefir_size_t count;
                        REQUIRE_OK(kefir_bigint_nonzero_count(node_value->bitprecise, &count));
                        value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
                        value->uinteger = count;
                    } else {
                        REQUIRE_OK(kefir_ast_evaluate_builtin_popcount_constant_expression_value(node_value->uinteger,
                                                                                                 64, value));
                    }
                } break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpectd popcountg builtin argument type");
            }
        } break;

        case KEFIR_AST_BUILTIN_PARITYG: {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, node, iter->value);
            const struct kefir_ast_type *unqualified_type = kefir_ast_unqualified_type(node->properties.type);
            if (unqualified_type->tag == KEFIR_AST_TYPE_ENUMERATION) {
                unqualified_type = unqualified_type->enumeration_type.underlying_type;
            }

            kefir_ast_type_data_model_classification_t classification;
            REQUIRE_OK(kefir_ast_type_data_model_classify(context->type_traits, unqualified_type, &classification));
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
            REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->source_location,
                                           "Expected integral constant expression"));
            switch (classification) {
                case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_parity_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 8, value));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_parity_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 16, value));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_parity_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 32, value));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                    REQUIRE_OK(kefir_ast_evaluate_builtin_parity_constant_expression_value(
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node)->uinteger, 64, value));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_BITINT: {
                    const struct kefir_ast_constant_expression_value *node_value =
                        KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node);
                    if (node_value->bitprecise != NULL) {
                        kefir_size_t count;
                        REQUIRE_OK(kefir_bigint_parity(node_value->bitprecise, &count));
                        value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
                        value->uinteger = count;
                    } else {
                        REQUIRE_OK(kefir_ast_evaluate_builtin_parity_constant_expression_value(node_value->uinteger, 64,
                                                                                               value));
                    }
                } break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpectd parityg builtin argument type");
            }
        } break;

        case KEFIR_AST_BUILTIN_VA_START:
        case KEFIR_AST_BUILTIN_VA_END:
        case KEFIR_AST_BUILTIN_VA_ARG:
        case KEFIR_AST_BUILTIN_VA_COPY:
        case KEFIR_AST_BUILTIN_ALLOCA:
        case KEFIR_AST_BUILTIN_ALLOCA_WITH_ALIGN:
        case KEFIR_AST_BUILTIN_ALLOCA_WITH_ALIGN_AND_MAX:
        case KEFIR_AST_BUILTIN_ADD_OVERFLOW:
        case KEFIR_AST_BUILTIN_SUB_OVERFLOW:
        case KEFIR_AST_BUILTIN_MUL_OVERFLOW:
            return KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                          "Builtin operation is not a constant expression");
    }

    return KEFIR_OK;
}
