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
#include "kefir/ast-translator/lvalue.h"
#include "kefir/ast-translator/value.h"
#include "kefir/ast-translator/typeconv.h"
#include "kefir/ast-translator/util.h"
#include "kefir/ast-translator/misc.h"
#include "kefir/ast-translator/flow_control.h"
#include "kefir/ast-translator/type.h"
#include "kefir/ast-translator/temporaries.h"
#include "kefir/ast/type_conv.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

static kefir_result_t translate_arithmetic_unary(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                                 struct kefir_irbuilder_block *builder,
                                                 const struct kefir_ast_unary_operation *node) {
    const struct kefir_ast_type *normalized_type = kefir_ast_translator_normalize_type(node->base.properties.type);
    REQUIRE_OK(kefir_ast_translate_expression(mem, node->arg, builder, context));
    REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                            node->arg->properties.type, node->base.properties.type));
    switch (normalized_type->tag) {
        case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
            switch (node->type) {
                case KEFIR_AST_OPERATION_PLUS:
                    break;

                case KEFIR_AST_OPERATION_NEGATE:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_NEG, 0));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected unary operator");
            }
            break;

        case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
            switch (node->type) {
                case KEFIR_AST_OPERATION_PLUS:
                    break;

                case KEFIR_AST_OPERATION_NEGATE:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_NEG, 0));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected unary operator");
            }
            break;

        case KEFIR_AST_TYPE_COMPLEX_FLOAT:
            switch (node->type) {
                case KEFIR_AST_OPERATION_PLUS:
                    break;

                case KEFIR_AST_OPERATION_NEGATE:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_NEG, 0));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected unary operator");
            }
            break;

        case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
            switch (node->type) {
                case KEFIR_AST_OPERATION_PLUS:
                    break;

                case KEFIR_AST_OPERATION_NEGATE:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_NEG, 0));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected unary operator");
            }
            break;

        case KEFIR_AST_TYPE_SCALAR_DOUBLE:
            switch (node->type) {
                case KEFIR_AST_OPERATION_PLUS:
                    break;

                case KEFIR_AST_OPERATION_NEGATE:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT64_NEG, 0));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected unary operator");
            }
            break;

        case KEFIR_AST_TYPE_SCALAR_FLOAT:
            switch (node->type) {
                case KEFIR_AST_OPERATION_PLUS:
                    break;

                case KEFIR_AST_OPERATION_NEGATE:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT32_NEG, 0));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected unary operator");
            }
            break;

        default:
            REQUIRE(
                KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(normalized_type),
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected integral type as operand of unary arithmetic operator"));
            switch (node->type) {
                case KEFIR_AST_OPERATION_PLUS:
                    break;

                case KEFIR_AST_OPERATION_NEGATE: {
                    kefir_ast_type_data_model_classification_t normalized_type_classification;
                    REQUIRE_OK(kefir_ast_type_data_model_classify(context->ast_context->type_traits, normalized_type,
                                                                  &normalized_type_classification));
                    switch (normalized_type_classification) {
                        case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_NEG, 0));
                            break;

                        case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT16_NEG, 0));
                            break;

                        case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT32_NEG, 0));
                            break;

                        case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_NEG, 0));
                            break;

                        case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
                            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_NEGATE,
                                                                       normalized_type->bitprecise.width));
                            break;

                        default:
                            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
                    }
                } break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected unary operator");
            }
            break;
    }

    return KEFIR_OK;
}

static kefir_result_t translate_unary_inversion(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                                struct kefir_irbuilder_block *builder,
                                                const struct kefir_ast_unary_operation *node) {
    const struct kefir_ast_type *normalized_type = kefir_ast_translator_normalize_type(node->base.properties.type);
    REQUIRE_OK(kefir_ast_translate_expression(mem, node->arg, builder, context));
    REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                            node->arg->properties.type, node->base.properties.type));
    kefir_ast_type_data_model_classification_t normalized_type_classification;
    REQUIRE_OK(kefir_ast_type_data_model_classify(context->ast_context->type_traits, normalized_type,
                                                  &normalized_type_classification));
    switch (normalized_type_classification) {
        case KEFIR_AST_TYPE_DATA_MODEL_INT8:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_NOT, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT16:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT16_NOT, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT32:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT32_NOT, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT64:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_NOT, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_INVERT,
                                                       normalized_type->bitprecise.width));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
    }
    return KEFIR_OK;
}

static kefir_result_t translate_logical_not_inversion(struct kefir_mem *mem,
                                                      struct kefir_ast_translator_context *context,
                                                      struct kefir_irbuilder_block *builder,
                                                      const struct kefir_ast_unary_operation *node) {
    const struct kefir_ast_type *normalized_type = kefir_ast_translator_normalize_type(
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, node->arg->properties.type));
    REQUIRE_OK(kefir_ast_translate_expression(mem, node->arg, builder, context));
    if (KEFIR_AST_TYPE_IS_FLOATING_POINT(normalized_type)) {
        REQUIRE_OK(kefir_ast_translate_typeconv_to_bool(context->ast_context->type_traits, builder, normalized_type));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_BOOL_NOT, 0));
    } else {
        kefir_ast_type_data_model_classification_t normalized_type_classification;
        REQUIRE_OK(kefir_ast_type_data_model_classify(context->ast_context->type_traits, normalized_type,
                                                      &normalized_type_classification));
        switch (normalized_type_classification) {
            case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_BOOL_NOT, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT16_BOOL_NOT, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT32_BOOL_NOT, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_BOOL_NOT, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_BOOL_NOT,
                                                           normalized_type->bitprecise.width));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected condition type");
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_sizeof(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                       struct kefir_irbuilder_block *builder,
                                       const struct kefir_ast_unary_operation *node) {
    if (KEFIR_AST_TYPE_IS_VL_ARRAY(node->arg->properties.type) &&
        node->arg->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION &&
        node->arg->properties.expression_props.scoped_id != NULL) {
        ASSIGN_DECL_CAST(struct kefir_ast_translator_scoped_identifier_object *, identifier_data,
                         node->arg->properties.expression_props.scoped_id->payload.ptr);
        REQUIRE_OK(
            kefir_ast_translator_resolve_local_type_layout(builder, identifier_data->type_id, identifier_data->layout));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST,
                                                   identifier_data->layout->vl_array.array_size_relative_offset));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_LOAD, KEFIR_IR_MEMORY_FLAG_NONE));
    } else {
        const struct kefir_ast_type *type = node->arg->properties.type;
        if (context->ast_context->configuration->analysis.ext_pointer_arithmetics &&
            (type->tag == KEFIR_AST_TYPE_FUNCTION || kefir_ast_unqualified_type(type)->tag == KEFIR_AST_TYPE_VOID)) {
            type = context->ast_context->type_traits->incomplete_type_substitute;
        }
        REQUIRE_OK(kefir_ast_translate_sizeof(mem, context, builder, type, &node->base.source_location));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_indirection(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                            struct kefir_irbuilder_block *builder,
                                            const struct kefir_ast_unary_operation *node) {
    const struct kefir_ast_type *normalized_type = kefir_ast_translator_normalize_type(node->base.properties.type);
    REQUIRE_OK(kefir_ast_translate_expression(mem, node->arg, builder, context));
    if (normalized_type->tag != KEFIR_AST_TYPE_VOID) {
        if (node->base.properties.expression_props.atomic) {
            kefir_bool_t atomic_aggregate;
            REQUIRE_OK(kefir_ast_translator_atomic_load_value(
                node->base.properties.type, context->ast_context->type_traits, builder, &atomic_aggregate));
            if (atomic_aggregate) {
                REQUIRE_OK(kefir_ast_translator_load_atomic_aggregate_value(
                    mem, node->base.properties.type, context, builder,
                    &node->base.properties.expression_props.temporary_identifier, &node->base.source_location));
            }
        } else {
            REQUIRE_OK(kefir_ast_translator_load_value(node->base.properties.type, context->ast_context->type_traits,
                                                       builder));
        }
    } else {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_VSTACK_POP, 0));
    }
    return KEFIR_OK;
}

static kefir_result_t incdec_impl(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                  struct kefir_irbuilder_block *builder, const struct kefir_ast_unary_operation *node,
                                  const struct kefir_ast_type *normalized_type) {
    kefir_int64_t diff =
        node->type == KEFIR_AST_OPERATION_POSTFIX_INCREMENT || node->type == KEFIR_AST_OPERATION_PREFIX_INCREMENT ? 1
                                                                                                                  : -1;
    if (normalized_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER) {
        struct kefir_ast_translator_type *translator_type = NULL;
        REQUIRE_OK(kefir_ast_translator_type_new(mem, context->ast_context, context->environment, context->module,
                                                 node->base.properties.type->referenced_type, 0, &translator_type,
                                                 &node->base.source_location));

        kefir_result_t res = KEFIR_OK;
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, diff));
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST,
                                                            translator_type->object.layout->properties.size));
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_MUL, 0));
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));

        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_translator_type_free(mem, translator_type);
            return res;
        });
        REQUIRE_OK(kefir_ast_translator_type_free(mem, translator_type));
    } else {
        REQUIRE(normalized_type->tag != KEFIR_AST_TYPE_SCALAR_NULL_POINTER,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->base.source_location, "Unexpected null pointer"));

        kefir_ast_type_data_model_classification_t normalized_type_classification;
        REQUIRE_OK(kefir_ast_type_data_model_classify(context->ast_context->type_traits, normalized_type,
                                                      &normalized_type_classification));
        switch (normalized_type_classification) {
            case KEFIR_AST_TYPE_DATA_MODEL_FLOAT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IR_OPCODE_FLOAT32_CONST,
                                                           (kefir_float32_t) diff, 0.0f));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT32_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_DOUBLE:
                REQUIRE_OK(
                    KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST, (kefir_float64_t) diff));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT64_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_LONG_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST,
                                                                    (kefir_long_double_t) diff));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IR_OPCODE_FLOAT32_CONST,
                                                           (kefir_float32_t) diff, 0.0f));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF32(builder, KEFIR_IR_OPCODE_FLOAT32_CONST, 0.0f, 0.0f));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_FROM, 0));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
                REQUIRE_OK(
                    KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST, (kefir_float64_t) diff));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IR_OPCODE_FLOAT64_CONST, 0.0));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_FROM, 0));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST,
                                                                    (kefir_long_double_t) diff));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPEND_LONG_DOUBLE(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST, 0.0L));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_FROM, 0));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(normalized_type),
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type"));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, diff));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(normalized_type),
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type"));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, diff));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT16_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(normalized_type),
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type"));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, diff));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT32_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                REQUIRE(KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(normalized_type),
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type"));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, diff));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
                REQUIRE(KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(normalized_type),
                        KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of a bit-precise integral type"));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_CONST, diff));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_FROM_SIGNED,
                                                           normalized_type->bitprecise.width));
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_ADD,
                                                           normalized_type->bitprecise.width));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_preincdec(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                          struct kefir_irbuilder_block *builder,
                                          const struct kefir_ast_unary_operation *node) {
    const struct kefir_ast_type *normalized_type = kefir_ast_translator_normalize_type(node->base.properties.type);
    kefir_bool_t atomic_aggregate_target_value;

    REQUIRE_OK(kefir_ast_translate_lvalue(mem, context, builder, node->arg));

    kefir_bool_t preserve_fenv = false;
    if (node->arg->properties.expression_props.atomic && KEFIR_AST_TYPE_IS_FLOATING_POINT(normalized_type)) {
        preserve_fenv = true;
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FENV_SAVE, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FENV_CLEAR, 0));
    }

    if (!node->arg->properties.expression_props.atomic) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
        REQUIRE_OK(
            kefir_ast_translator_resolve_lvalue(mem, context, builder, node->arg, &atomic_aggregate_target_value));
        REQUIRE(!atomic_aggregate_target_value,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected atomic aggregate value"));

        REQUIRE_OK(incdec_impl(mem, context, builder, node, normalized_type));

        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
        REQUIRE_OK(kefir_ast_translator_store_lvalue(mem, context, builder, node->arg));
    } else {
        kefir_size_t successBranch, failTarget = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_PLACEHOLDER, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
        REQUIRE_OK(
            kefir_ast_translator_resolve_lvalue(mem, context, builder, node->arg, &atomic_aggregate_target_value));
        REQUIRE(!atomic_aggregate_target_value,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected atomic aggregate value"));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));

        REQUIRE_OK(incdec_impl(mem, context, builder, node, normalized_type));

        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 5));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_POP, 0));

        REQUIRE_OK(kefir_ast_translator_atomic_compare_exchange_value(mem, node->arg->properties.type, context, builder,
                                                                      &node->base.source_location));

        successBranch = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
        REQUIRE_OK(
            KEFIR_IRBUILDER_BLOCK_APPENDU64_2(builder, KEFIR_IR_OPCODE_BRANCH, 0, KEFIR_IR_BRANCH_CONDITION_8BIT));

        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_POP, 0));
        if (preserve_fenv) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FENV_CLEAR, 0));
        }
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_JUMP, failTarget));

        KEFIR_IRBUILDER_BLOCK_INSTR_AT(builder, successBranch)->arg.i64 = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_POP, 0));

        if (preserve_fenv) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FENV_UPDATE, 0));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_postincdec(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                           struct kefir_irbuilder_block *builder,
                                           const struct kefir_ast_unary_operation *node) {
    const struct kefir_ast_type *normalized_type = kefir_ast_translator_normalize_type(node->base.properties.type);
    kefir_bool_t atomic_aggregate_target_value;

    REQUIRE_OK(kefir_ast_translate_lvalue(mem, context, builder, node->arg));

    kefir_bool_t preserve_fenv = false;
    if (node->arg->properties.expression_props.atomic && KEFIR_AST_TYPE_IS_FLOATING_POINT(normalized_type)) {
        preserve_fenv = true;
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FENV_SAVE, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FENV_CLEAR, 0));
    }

    if (!node->arg->properties.expression_props.atomic) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
        REQUIRE_OK(
            kefir_ast_translator_resolve_lvalue(mem, context, builder, node->arg, &atomic_aggregate_target_value));
        REQUIRE(!atomic_aggregate_target_value,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected atomic aggregate value"));

        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));

        REQUIRE_OK(incdec_impl(mem, context, builder, node, normalized_type));
        REQUIRE_OK(kefir_ast_translator_store_lvalue(mem, context, builder, node->arg));
    } else {
        kefir_size_t successBranch, failTarget = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT_PLACEHOLDER, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
        REQUIRE_OK(
            kefir_ast_translator_resolve_lvalue(mem, context, builder, node->arg, &atomic_aggregate_target_value));
        REQUIRE(!atomic_aggregate_target_value,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected atomic aggregate value"));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));

        REQUIRE_OK(incdec_impl(mem, context, builder, node, normalized_type));

        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 5));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_POP, 0));

        REQUIRE_OK(kefir_ast_translator_atomic_compare_exchange_value(mem, node->arg->properties.type, context, builder,
                                                                      &node->base.source_location));

        successBranch = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
        REQUIRE_OK(
            KEFIR_IRBUILDER_BLOCK_APPENDU64_2(builder, KEFIR_IR_OPCODE_BRANCH, 0, KEFIR_IR_BRANCH_CONDITION_8BIT));

        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_POP, 0));
        if (preserve_fenv) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FENV_CLEAR, 0));
        }
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_JUMP, failTarget));

        KEFIR_IRBUILDER_BLOCK_INSTR_AT(builder, successBranch)->arg.i64 = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_POP, 0));

        if (preserve_fenv) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FENV_UPDATE, 0));
        }
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_unary_operation_node(struct kefir_mem *mem,
                                                        struct kefir_ast_translator_context *context,
                                                        struct kefir_irbuilder_block *builder,
                                                        const struct kefir_ast_unary_operation *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST unary operation node"));

    switch (node->type) {
        case KEFIR_AST_OPERATION_PLUS:
        case KEFIR_AST_OPERATION_NEGATE:
            REQUIRE_OK(translate_arithmetic_unary(mem, context, builder, node));
            break;

        case KEFIR_AST_OPERATION_INVERT:
            REQUIRE_OK(translate_unary_inversion(mem, context, builder, node));
            break;

        case KEFIR_AST_OPERATION_LOGICAL_NEGATE:
            REQUIRE_OK(translate_logical_not_inversion(mem, context, builder, node));
            break;

        case KEFIR_AST_OPERATION_PREFIX_INCREMENT:
        case KEFIR_AST_OPERATION_PREFIX_DECREMENT:
            REQUIRE_OK(translate_preincdec(mem, context, builder, node));
            break;

        case KEFIR_AST_OPERATION_POSTFIX_INCREMENT:
        case KEFIR_AST_OPERATION_POSTFIX_DECREMENT:
            REQUIRE_OK(translate_postincdec(mem, context, builder, node));
            break;

        case KEFIR_AST_OPERATION_SIZEOF:
            REQUIRE_OK(translate_sizeof(mem, context, builder, node));
            break;

        case KEFIR_AST_OPERATION_ALIGNOF:
            if (node->arg->properties.category == KEFIR_AST_NODE_CATEGORY_TYPE) {
                if (node->arg->properties.type_props.alignment == 0) {
                    REQUIRE_OK(kefir_ast_translate_alignof(mem, context, builder, node->arg->properties.type,
                                                           &node->base.source_location));
                } else {
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST,
                                                               node->arg->properties.type_props.alignment));
                }
            } else if (node->arg->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION) {
                if (node->arg->properties.expression_props.alignment == 0) {
                    REQUIRE_OK(kefir_ast_translate_alignof(mem, context, builder, node->arg->properties.type,
                                                           &node->base.source_location));
                } else {
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST,
                                                               node->arg->properties.expression_props.alignment));
                }
            }
            break;

        case KEFIR_AST_OPERATION_ADDRESS:
            REQUIRE_OK(kefir_ast_translate_lvalue(mem, context, builder, node->arg));
            break;

        case KEFIR_AST_OPERATION_INDIRECTION:
            REQUIRE_OK(translate_indirection(mem, context, builder, node));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected AST unary operation");
    }
    return KEFIR_OK;
}
