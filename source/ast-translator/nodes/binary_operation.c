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
#include "kefir/ast-translator/type.h"
#include "kefir/ast-translator/temporaries.h"
#include "kefir/ast/type_conv.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

static kefir_result_t binary_prologue(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                      struct kefir_irbuilder_block *builder,
                                      const struct kefir_ast_binary_operation *node) {
    const struct kefir_ast_type *arg1_init_normalized_type =
        kefir_ast_translator_normalize_type(node->arg1->properties.type);
    const struct kefir_ast_type *arg1_normalized_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, arg1_init_normalized_type);
    REQUIRE(arg1_normalized_type != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to perform lvalue conversions"));
    const struct kefir_ast_type *arg2_init_normalized_type =
        kefir_ast_translator_normalize_type(node->arg2->properties.type);
    const struct kefir_ast_type *arg2_normalized_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, arg2_init_normalized_type);
    REQUIRE(arg2_normalized_type != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to perform lvalue conversions"));
    const struct kefir_ast_type *result_normalized_type =
        kefir_ast_translator_normalize_type(node->base.properties.type);

    REQUIRE(!KEFIR_AST_TYPE_IS_LONG_DOUBLE(result_normalized_type) ||
                (KEFIR_AST_TYPE_IS_LONG_DOUBLE(arg1_normalized_type) ||
                 KEFIR_AST_TYPE_IS_LONG_DOUBLE(arg2_init_normalized_type)),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected conversion to long double"));

    REQUIRE_OK(kefir_ast_translate_expression(mem, node->arg1, builder, context));
    REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                            arg1_normalized_type, result_normalized_type));
    REQUIRE_OK(kefir_ast_translate_expression(mem, node->arg2, builder, context));
    if ((node->type == KEFIR_AST_OPERATION_SHIFT_LEFT || node->type == KEFIR_AST_OPERATION_SHIFT_RIGHT) &&
        KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(result_normalized_type)) {
        REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                arg2_normalized_type, context->ast_context->type_traits->size_type));
    } else {
        REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                arg2_normalized_type, result_normalized_type));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_addition(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                         struct kefir_irbuilder_block *builder,
                                         const struct kefir_ast_binary_operation *node) {
    const struct kefir_ast_type *arg1_normalized_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, node->arg1->properties.type);
    const struct kefir_ast_type *arg2_normalized_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, node->arg2->properties.type);
    const struct kefir_ast_type *result_normalized_type =
        kefir_ast_translator_normalize_type(node->base.properties.type);

    if (KEFIR_AST_TYPE_IS_ARITHMETIC_TYPE(arg1_normalized_type) &&
        KEFIR_AST_TYPE_IS_ARITHMETIC_TYPE(arg2_normalized_type)) {
        REQUIRE_OK(binary_prologue(mem, context, builder, node));
        kefir_ast_type_data_model_classification_t result_type_classification;
        REQUIRE_OK(kefir_ast_type_data_model_classify(context->ast_context->type_traits, result_normalized_type,
                                                      &result_type_classification));
        switch (result_type_classification) {
            case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_LONG_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT64_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_FLOAT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT32_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT16_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT32_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_ADD,
                                                           result_normalized_type->bitprecise.width));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
        }
    } else {
        const struct kefir_ast_type *referenced_type = result_normalized_type->referenced_type;
        if (context->ast_context->configuration->analysis.ext_pointer_arithmetics &&
            (referenced_type->tag == KEFIR_AST_TYPE_FUNCTION ||
             kefir_ast_unqualified_type(referenced_type)->tag == KEFIR_AST_TYPE_VOID)) {
            referenced_type = context->ast_context->type_traits->incomplete_type_substitute;
        }

        struct kefir_ast_translator_type *translator_type = NULL;
        REQUIRE_OK(kefir_ast_translator_type_new(mem, context->ast_context, context->environment, context->module,
                                                 referenced_type, 0, &translator_type, &node->base.source_location));

        kefir_result_t res = KEFIR_OK;
        if (arg1_normalized_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER) {
            REQUIRE_CHAIN(&res, kefir_ast_translate_expression(mem, node->arg1, builder, context));
            REQUIRE_CHAIN(&res, kefir_ast_translate_expression(mem, node->arg2, builder, context));
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                    node->arg2->properties.type,
                                                    context->ast_context->type_traits->ptrdiff_type));
        } else {
            REQUIRE_CHAIN(&res, kefir_ast_translate_expression(mem, node->arg2, builder, context));
            REQUIRE_CHAIN(&res, kefir_ast_translate_expression(mem, node->arg1, builder, context));
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                    node->arg1->properties.type,
                                                    context->ast_context->type_traits->ptrdiff_type));
        }
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST,
                                                            translator_type->object.layout->properties.size));
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_MUL, 0));
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_ADD, 0));

        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_translator_type_free(mem, translator_type);
            return res;
        });
        REQUIRE_OK(kefir_ast_translator_type_free(mem, translator_type));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_subtraction(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                            struct kefir_irbuilder_block *builder,
                                            const struct kefir_ast_binary_operation *node) {
    const struct kefir_ast_type *arg1_normalized_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, node->arg1->properties.type);
    const struct kefir_ast_type *arg2_normalized_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, node->arg2->properties.type);
    const struct kefir_ast_type *result_normalized_type =
        kefir_ast_translator_normalize_type(node->base.properties.type);

    REQUIRE_OK(binary_prologue(mem, context, builder, node));
    if (KEFIR_AST_TYPE_IS_ARITHMETIC_TYPE(arg1_normalized_type) &&
        KEFIR_AST_TYPE_IS_ARITHMETIC_TYPE(arg2_normalized_type)) {
        kefir_ast_type_data_model_classification_t result_type_classification;
        REQUIRE_OK(kefir_ast_type_data_model_classify(context->ast_context->type_traits, result_normalized_type,
                                                      &result_type_classification));
        switch (result_type_classification) {
            case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_SUB, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_SUB, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_SUB, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_LONG_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_SUB, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT64_SUB, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_FLOAT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT32_SUB, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_SUB, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT16_SUB, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT32_SUB, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_SUB, 0));
                break;

            case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_SUB,
                                                           result_normalized_type->bitprecise.width));
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
        }
    } else if (arg2_normalized_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER) {
        const struct kefir_ast_type *referenced_type = arg1_normalized_type->referenced_type;
        if (context->ast_context->configuration->analysis.ext_pointer_arithmetics &&
            (referenced_type->tag == KEFIR_AST_TYPE_FUNCTION ||
             kefir_ast_unqualified_type(referenced_type)->tag == KEFIR_AST_TYPE_VOID)) {
            referenced_type = context->ast_context->type_traits->incomplete_type_substitute;
        }

        kefir_ast_target_environment_opaque_type_t opaque_type;
        struct kefir_ast_target_environment_object_info type_info;
        REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_GET_TYPE(mem, context->ast_context, &context->environment->target_env,
                                                         referenced_type, &opaque_type, &node->base.source_location));
        kefir_result_t res = KEFIR_AST_TARGET_ENVIRONMENT_OBJECT_INFO(mem, &context->environment->target_env,
                                                                      opaque_type, NULL, &type_info);
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, &context->environment->target_env, opaque_type);
            return res;
        });
        REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, &context->environment->target_env, opaque_type));

        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_SUB, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST, type_info.size));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_DIV, 0));
    } else {
        const struct kefir_ast_type *referenced_type = arg1_normalized_type->referenced_type;
        if (context->ast_context->configuration->analysis.ext_pointer_arithmetics &&
            (referenced_type->tag == KEFIR_AST_TYPE_FUNCTION ||
             kefir_ast_unqualified_type(referenced_type)->tag == KEFIR_AST_TYPE_VOID)) {
            referenced_type = context->ast_context->type_traits->incomplete_type_substitute;
        }

        struct kefir_ast_translator_type *translator_type = NULL;
        REQUIRE_OK(kefir_ast_translator_type_new(mem, context->ast_context, context->environment, context->module,
                                                 referenced_type, 0, &translator_type, &node->base.source_location));

        kefir_result_t res = KEFIR_OK;
        REQUIRE_CHAIN(
            &res, kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                               arg2_normalized_type, context->ast_context->type_traits->ptrdiff_type));
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_UINT_CONST,
                                                            translator_type->object.layout->properties.size));
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_MUL, 0));
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IR_OPCODE_INT64_SUB, 0));

        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_translator_type_free(mem, translator_type);
            return res;
        });
        REQUIRE_OK(kefir_ast_translator_type_free(mem, translator_type));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_multiplication(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                               struct kefir_irbuilder_block *builder,
                                               const struct kefir_ast_binary_operation *node) {
    const struct kefir_ast_type *result_normalized_type =
        kefir_ast_translator_normalize_type(node->base.properties.type);

    REQUIRE_OK(binary_prologue(mem, context, builder, node));
    kefir_bool_t result_type_signedness;
    kefir_ast_type_data_model_classification_t result_type_classification;
    REQUIRE_OK(kefir_ast_type_data_model_classify(context->ast_context->type_traits, result_normalized_type,
                                                  &result_type_classification));
    switch (result_type_classification) {
        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_MUL, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_MUL, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_MUL, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_MUL, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT64_MUL, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT32_MUL, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT8:
            REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                &result_type_signedness));
            if (result_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_MUL, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT8_MUL, 0));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT16:
            REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                &result_type_signedness));
            if (result_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT16_MUL, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT16_MUL, 0));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT32:
            REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                &result_type_signedness));
            if (result_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT32_MUL, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT32_MUL, 0));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT64:
            REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                &result_type_signedness));
            if (result_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_MUL, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT64_MUL, 0));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
            REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                &result_type_signedness));
            if (result_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_IMUL,
                                                           result_normalized_type->bitprecise.width));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_UMUL,
                                                           result_normalized_type->bitprecise.width));
            }
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
    }
    return KEFIR_OK;
}

static kefir_result_t translate_division(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                         struct kefir_irbuilder_block *builder,
                                         const struct kefir_ast_binary_operation *node) {
    const struct kefir_ast_type *result_normalized_type =
        kefir_ast_translator_normalize_type(node->base.properties.type);

    REQUIRE_OK(binary_prologue(mem, context, builder, node));
    kefir_bool_t result_type_signedness;
    kefir_ast_type_data_model_classification_t result_type_classification;
    REQUIRE_OK(kefir_ast_type_data_model_classify(context->ast_context->type_traits, result_normalized_type,
                                                  &result_type_classification));
    switch (result_type_classification) {
        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_DIV, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_DIV, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_DIV, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_DIV, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT64_DIV, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_FLOAT32_DIV, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT8:
            REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                &result_type_signedness));
            if (result_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_DIV, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT8_DIV, 0));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT16:
            REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                &result_type_signedness));
            if (result_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT16_DIV, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT16_DIV, 0));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT32:
            REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                &result_type_signedness));
            if (result_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT32_DIV, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT32_DIV, 0));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT64:
            REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                &result_type_signedness));
            if (result_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_DIV, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT64_DIV, 0));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
            REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                &result_type_signedness));
            if (result_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_IDIV,
                                                           result_normalized_type->bitprecise.width));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_UDIV,
                                                           result_normalized_type->bitprecise.width));
            }
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
    }
    return KEFIR_OK;
}

static kefir_result_t translate_modulo(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                       struct kefir_irbuilder_block *builder,
                                       const struct kefir_ast_binary_operation *node) {
    const struct kefir_ast_type *result_normalized_type =
        kefir_ast_translator_normalize_type(node->base.properties.type);

    REQUIRE_OK(binary_prologue(mem, context, builder, node));
    kefir_bool_t result_type_signedness;
    kefir_ast_type_data_model_classification_t result_type_classification;
    REQUIRE_OK(kefir_ast_type_data_model_classify(context->ast_context->type_traits, result_normalized_type,
                                                  &result_type_classification));
    switch (result_type_classification) {
        case KEFIR_AST_TYPE_DATA_MODEL_INT8:
            REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                &result_type_signedness));
            if (result_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_MOD, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT8_MOD, 0));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT16:
            REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                &result_type_signedness));
            if (result_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT16_MOD, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT16_MOD, 0));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT32:
            REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                &result_type_signedness));
            if (result_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT32_MOD, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT32_MOD, 0));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT64:
            REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                &result_type_signedness));
            if (result_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_MOD, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_UINT64_MOD, 0));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
            REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                &result_type_signedness));
            if (result_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_IMOD,
                                                           result_normalized_type->bitprecise.width));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_UMOD,
                                                           result_normalized_type->bitprecise.width));
            }
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
    }
    return KEFIR_OK;
}

static kefir_result_t translate_bitwise(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                        struct kefir_irbuilder_block *builder,
                                        const struct kefir_ast_binary_operation *node) {
    REQUIRE_OK(binary_prologue(mem, context, builder, node));
    const struct kefir_ast_type *result_normalized_type =
        kefir_ast_translator_normalize_type(node->base.properties.type);

    kefir_bool_t result_type_signedness;
    kefir_ast_type_data_model_classification_t result_type_classification;
    REQUIRE_OK(kefir_ast_type_data_model_classify(context->ast_context->type_traits, result_normalized_type,
                                                  &result_type_classification));
    switch (node->type) {
        case KEFIR_AST_OPERATION_SHIFT_LEFT:
            switch (result_type_classification) {
                case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_LSHIFT, 0));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT16_LSHIFT, 0));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT32_LSHIFT, 0));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_LSHIFT, 0));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_LSHIFT,
                                                               result_normalized_type->bitprecise.width));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
            }
            break;

        case KEFIR_AST_OPERATION_SHIFT_RIGHT:
            switch (result_type_classification) {
                case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                    REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                        &result_type_signedness));
                    if (result_type_signedness) {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_ARSHIFT, 0));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_RSHIFT, 0));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                    REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                        &result_type_signedness));
                    if (result_type_signedness) {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT16_ARSHIFT, 0));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT16_RSHIFT, 0));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                    REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                        &result_type_signedness));
                    if (result_type_signedness) {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT32_ARSHIFT, 0));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT32_RSHIFT, 0));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                    REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                        &result_type_signedness));
                    if (result_type_signedness) {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_ARSHIFT, 0));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_RSHIFT, 0));
                    }
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
                    REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type,
                                                        &result_type_signedness));
                    if (result_type_signedness) {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_ARSHIFT,
                                                                   result_normalized_type->bitprecise.width));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_RSHIFT,
                                                                   result_normalized_type->bitprecise.width));
                    }
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
            }
            break;

        case KEFIR_AST_OPERATION_BITWISE_AND:
            switch (result_type_classification) {
                case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_AND, 0));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT16_AND, 0));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT32_AND, 0));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_AND, 0));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_AND,
                                                               result_normalized_type->bitprecise.width));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
            }
            break;

        case KEFIR_AST_OPERATION_BITWISE_OR:
            switch (result_type_classification) {
                case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_OR, 0));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT16_OR, 0));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT32_OR, 0));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_OR, 0));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_OR,
                                                               result_normalized_type->bitprecise.width));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
            }
            break;

        case KEFIR_AST_OPERATION_BITWISE_XOR:
            switch (result_type_classification) {
                case KEFIR_AST_TYPE_DATA_MODEL_INT8:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_XOR, 0));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT16:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT16_XOR, 0));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT32:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT32_XOR, 0));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_INT64:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_XOR, 0));
                    break;

                case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_XOR,
                                                               result_normalized_type->bitprecise.width));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
            }
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected bitwise operation");
    }
    return KEFIR_OK;
}

static kefir_result_t translate_relational_equals(const struct kefir_ast_type_traits *type_traits,
                                                  const struct kefir_ast_type *common_type,
                                                  struct kefir_irbuilder_block *builder) {
    kefir_ast_type_data_model_classification_t common_type_classification;
    REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, common_type, &common_type_classification));

    switch (common_type_classification) {
        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_LONG_DOUBLE_EQUALS, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT64_EQUALS, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_COMPLEX_FLOAT32_EQUALS, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_EQUALS, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                       KEFIR_IR_COMPARE_FLOAT64_EQUALS));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                       KEFIR_IR_COMPARE_FLOAT32_EQUALS));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT8:
            REQUIRE_OK(
                KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE, KEFIR_IR_COMPARE_INT8_EQUALS));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT16:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                       KEFIR_IR_COMPARE_INT16_EQUALS));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT32:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                       KEFIR_IR_COMPARE_INT32_EQUALS));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT64:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                       KEFIR_IR_COMPARE_INT64_EQUALS));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
            REQUIRE_OK(
                KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_EQUAL, common_type->bitprecise.width));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of a scalar type");
    }
    return KEFIR_OK;
}

static kefir_result_t translate_relational_not_equals(const struct kefir_ast_type_traits *type_traits,
                                                      const struct kefir_ast_type *common_type,
                                                      struct kefir_irbuilder_block *builder) {
    REQUIRE_OK(translate_relational_equals(type_traits, common_type, builder));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_BOOL_NOT, 0));
    return KEFIR_OK;
}

static kefir_result_t translate_relational_less(const struct kefir_ast_type_traits *type_traits,
                                                const struct kefir_ast_type *common_type,
                                                struct kefir_irbuilder_block *builder) {
    kefir_bool_t common_type_signedness;
    kefir_ast_type_data_model_classification_t common_type_classification;
    REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, common_type, &common_type_classification));

    switch (common_type_classification) {
        case KEFIR_AST_TYPE_DATA_MODEL_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_LESSER, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                       KEFIR_IR_COMPARE_FLOAT64_LESSER));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                       KEFIR_IR_COMPARE_FLOAT32_LESSER));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT8:
            REQUIRE_OK(kefir_ast_type_is_signed(type_traits, common_type, &common_type_signedness));
            if (common_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                           KEFIR_IR_COMPARE_INT8_LESSER));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                           KEFIR_IR_COMPARE_INT8_BELOW));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT16:
            REQUIRE_OK(kefir_ast_type_is_signed(type_traits, common_type, &common_type_signedness));
            if (common_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                           KEFIR_IR_COMPARE_INT16_LESSER));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                           KEFIR_IR_COMPARE_INT16_BELOW));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT32:
            REQUIRE_OK(kefir_ast_type_is_signed(type_traits, common_type, &common_type_signedness));
            if (common_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                           KEFIR_IR_COMPARE_INT32_LESSER));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                           KEFIR_IR_COMPARE_INT32_BELOW));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT64:
            REQUIRE_OK(kefir_ast_type_is_signed(type_traits, common_type, &common_type_signedness));
            if (common_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                           KEFIR_IR_COMPARE_INT64_LESSER));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                           KEFIR_IR_COMPARE_INT64_BELOW));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
            REQUIRE_OK(kefir_ast_type_is_signed(type_traits, common_type, &common_type_signedness));
            if (common_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_LESS,
                                                           common_type->bitprecise.width));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_BELOW,
                                                           common_type->bitprecise.width));
            }
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of a scalar type");
    }
    return KEFIR_OK;
}

static kefir_result_t translate_relational_greater(const struct kefir_ast_type_traits *type_traits,
                                                   const struct kefir_ast_type *common_type,
                                                   struct kefir_irbuilder_block *builder) {
    kefir_bool_t common_type_signedness;
    kefir_ast_type_data_model_classification_t common_type_classification;
    REQUIRE_OK(kefir_ast_type_data_model_classify(type_traits, common_type, &common_type_classification));

    switch (common_type_classification) {
        case KEFIR_AST_TYPE_DATA_MODEL_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_LONG_DOUBLE_GREATER, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                       KEFIR_IR_COMPARE_FLOAT64_GREATER));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                       KEFIR_IR_COMPARE_FLOAT32_GREATER));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT8:
            REQUIRE_OK(kefir_ast_type_is_signed(type_traits, common_type, &common_type_signedness));
            if (common_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                           KEFIR_IR_COMPARE_INT8_GREATER));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                           KEFIR_IR_COMPARE_INT8_ABOVE));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT16:
            REQUIRE_OK(kefir_ast_type_is_signed(type_traits, common_type, &common_type_signedness));
            if (common_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                           KEFIR_IR_COMPARE_INT16_GREATER));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                           KEFIR_IR_COMPARE_INT16_ABOVE));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT32:
            REQUIRE_OK(kefir_ast_type_is_signed(type_traits, common_type, &common_type_signedness));
            if (common_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                           KEFIR_IR_COMPARE_INT32_GREATER));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                           KEFIR_IR_COMPARE_INT32_ABOVE));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT64:
            REQUIRE_OK(kefir_ast_type_is_signed(type_traits, common_type, &common_type_signedness));
            if (common_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                           KEFIR_IR_COMPARE_INT64_GREATER));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                                           KEFIR_IR_COMPARE_INT64_ABOVE));
            }
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
            REQUIRE_OK(kefir_ast_type_is_signed(type_traits, common_type, &common_type_signedness));
            if (common_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_GREATER,
                                                           common_type->bitprecise.width));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_ABOVE,
                                                           common_type->bitprecise.width));
            }
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of a scalar type");
    }
    return KEFIR_OK;
}

static kefir_result_t translate_relational_equality(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                                    struct kefir_irbuilder_block *builder,
                                                    const struct kefir_ast_binary_operation *node) {
    const struct kefir_ast_type *arg1_init_normalized_type =
        kefir_ast_translator_normalize_type(node->arg1->properties.type);
    const struct kefir_ast_type *arg1_normalized_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, arg1_init_normalized_type);
    REQUIRE(arg1_normalized_type != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to perform lvalue conversions"));
    const struct kefir_ast_type *arg2_init_normalized_type =
        kefir_ast_translator_normalize_type(node->arg2->properties.type);
    const struct kefir_ast_type *arg2_normalized_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, arg2_init_normalized_type);
    REQUIRE(arg2_normalized_type != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to perform lvalue conversions"));

    const struct kefir_ast_type *common_type = NULL;
    if (KEFIR_AST_TYPE_IS_ARITHMETIC_TYPE(arg1_normalized_type) &&
        KEFIR_AST_TYPE_IS_ARITHMETIC_TYPE(arg2_normalized_type)) {
        common_type = kefir_ast_type_common_arithmetic(context->ast_context->type_traits, arg1_normalized_type,
                                                       node->arg1->properties.expression_props.bitfield_props,
                                                       arg2_normalized_type,
                                                       node->arg2->properties.expression_props.bitfield_props);
        REQUIRE(
            !KEFIR_AST_TYPE_IS_LONG_DOUBLE(common_type) || (KEFIR_AST_TYPE_IS_LONG_DOUBLE(arg1_normalized_type) ||
                                                            KEFIR_AST_TYPE_IS_LONG_DOUBLE(arg2_init_normalized_type)),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected conversion to long double"));
        REQUIRE_OK(kefir_ast_translate_expression(mem, node->arg1, builder, context));

        REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                arg1_normalized_type, common_type));
        REQUIRE_OK(kefir_ast_translate_expression(mem, node->arg2, builder, context));
        REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                arg2_normalized_type, common_type));
    } else {
        REQUIRE_OK(kefir_ast_translate_expression(mem, node->arg1, builder, context));
        REQUIRE_OK(kefir_ast_translate_expression(mem, node->arg2, builder, context));
        common_type = arg1_normalized_type;
    }

    switch (node->type) {
        case KEFIR_AST_OPERATION_EQUAL:
            REQUIRE_OK(translate_relational_equals(context->ast_context->type_traits, common_type, builder));
            break;

        case KEFIR_AST_OPERATION_NOT_EQUAL:
            REQUIRE_OK(translate_relational_not_equals(context->ast_context->type_traits, common_type, builder));
            break;

        case KEFIR_AST_OPERATION_LESS:
            REQUIRE_OK(translate_relational_less(context->ast_context->type_traits, common_type, builder));
            break;

        case KEFIR_AST_OPERATION_GREATER:
            REQUIRE_OK(translate_relational_greater(context->ast_context->type_traits, common_type, builder));
            break;

        case KEFIR_AST_OPERATION_LESS_EQUAL:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
            REQUIRE_OK(translate_relational_less(context->ast_context->type_traits, common_type, builder));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 2));
            REQUIRE_OK(translate_relational_equals(context->ast_context->type_traits, common_type, builder));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_BOOL_OR, 0));
            break;

        case KEFIR_AST_OPERATION_GREATER_EQUAL:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
            REQUIRE_OK(translate_relational_greater(context->ast_context->type_traits, common_type, builder));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 2));
            REQUIRE_OK(translate_relational_equals(context->ast_context->type_traits, common_type, builder));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_BOOL_OR, 0));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected relational or equality operation");
    }
    return KEFIR_OK;
}

static kefir_result_t truncate_value(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                     struct kefir_irbuilder_block *builder, const struct kefir_ast_type *type) {
    const struct kefir_ast_type *normalized_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, type);
    REQUIRE_OK(kefir_ast_translate_typeconv_to_bool(context->ast_context->type_traits, builder, normalized_type));
    return KEFIR_OK;
}

static kefir_result_t translate_logical_and(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                            struct kefir_irbuilder_block *builder,
                                            const struct kefir_ast_binary_operation *node) {
    const struct kefir_ast_type *normalized_type1 =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, node->arg1->properties.type);
    const struct kefir_ast_type *normalized_type2 =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, node->arg2->properties.type);

    REQUIRE_OK(kefir_ast_translate_expression(mem, node->arg1, builder, context));
    REQUIRE_OK(kefir_ast_translate_typeconv_to_bool(context->ast_context->type_traits, builder, normalized_type1));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_BOOL_NOT, 0));
    kefir_size_t jmpIndex = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64_2(builder, KEFIR_IR_OPCODE_BRANCH, 0, KEFIR_IR_BRANCH_CONDITION_8BIT));
    REQUIRE_OK(kefir_ast_translate_expression(mem, node->arg2, builder, context));

    kefir_ast_type_data_model_classification_t normalized_type2_classification;
    REQUIRE_OK(kefir_ast_type_data_model_classify(context->ast_context->type_traits, normalized_type2,
                                                  &normalized_type2_classification));
    switch (normalized_type2_classification) {
        case KEFIR_AST_TYPE_DATA_MODEL_INT8:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_BOOL_AND, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT16:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT16_BOOL_AND, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT32:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT32_BOOL_AND, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT64:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_BOOL_AND, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_TO_BOOL,
                                                       normalized_type2->bitprecise.width));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_BOOL_AND, 0));
            break;

        default:
            REQUIRE_OK(
                kefir_ast_translate_typeconv_to_bool(context->ast_context->type_traits, builder, normalized_type2));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_BOOL_AND, 0));
            break;
    }
    kefir_size_t jmpTarget = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
    KEFIR_IRBUILDER_BLOCK_INSTR_AT(builder, jmpIndex)->arg.i64 = jmpTarget;
    return KEFIR_OK;
}

static kefir_result_t translate_logical_or(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                           struct kefir_irbuilder_block *builder,
                                           const struct kefir_ast_binary_operation *node) {
    const struct kefir_ast_type *normalized_type2 =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, node->arg2->properties.type);

    REQUIRE_OK(kefir_ast_translate_expression(mem, node->arg1, builder, context));
    REQUIRE_OK(truncate_value(mem, context, builder, node->arg1->properties.type));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_VSTACK_PICK, 0));
    kefir_size_t jmpIndex = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64_2(builder, KEFIR_IR_OPCODE_BRANCH, 0, KEFIR_IR_BRANCH_CONDITION_8BIT));
    REQUIRE_OK(kefir_ast_translate_expression(mem, node->arg2, builder, context));
    kefir_ast_type_data_model_classification_t normalized_type2_classification;
    REQUIRE_OK(kefir_ast_type_data_model_classify(context->ast_context->type_traits, normalized_type2,
                                                  &normalized_type2_classification));
    switch (normalized_type2_classification) {
        case KEFIR_AST_TYPE_DATA_MODEL_INT8:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_BOOL_OR, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT16:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT16_BOOL_OR, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT32:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT32_BOOL_OR, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_INT64:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_BOOL_OR, 0));
            break;

        case KEFIR_AST_TYPE_DATA_MODEL_BITINT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_BITINT_TO_BOOL,
                                                       normalized_type2->bitprecise.width));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT8_BOOL_OR, 0));
            break;

        default:
            REQUIRE_OK(
                kefir_ast_translate_typeconv_to_bool(context->ast_context->type_traits, builder, normalized_type2));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IR_OPCODE_INT64_BOOL_OR, 0));
            break;
    }
    kefir_size_t jmpTarget = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
    KEFIR_IRBUILDER_BLOCK_INSTR_AT(builder, jmpIndex)->arg.i64 = jmpTarget;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_binary_operation_node(struct kefir_mem *mem,
                                                         struct kefir_ast_translator_context *context,
                                                         struct kefir_irbuilder_block *builder,
                                                         const struct kefir_ast_binary_operation *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST binary operation node"));

    switch (node->type) {
        case KEFIR_AST_OPERATION_ADD:
            REQUIRE_OK(translate_addition(mem, context, builder, node));
            break;

        case KEFIR_AST_OPERATION_SUBTRACT:
            REQUIRE_OK(translate_subtraction(mem, context, builder, node));
            break;

        case KEFIR_AST_OPERATION_MULTIPLY:
            REQUIRE_OK(translate_multiplication(mem, context, builder, node));
            break;

        case KEFIR_AST_OPERATION_DIVIDE:
            REQUIRE_OK(translate_division(mem, context, builder, node));
            break;

        case KEFIR_AST_OPERATION_MODULO:
            REQUIRE_OK(translate_modulo(mem, context, builder, node));
            break;

        case KEFIR_AST_OPERATION_SHIFT_RIGHT:
        case KEFIR_AST_OPERATION_SHIFT_LEFT:
        case KEFIR_AST_OPERATION_BITWISE_AND:
        case KEFIR_AST_OPERATION_BITWISE_OR:
        case KEFIR_AST_OPERATION_BITWISE_XOR:
            REQUIRE_OK(translate_bitwise(mem, context, builder, node));
            break;

        case KEFIR_AST_OPERATION_LESS:
        case KEFIR_AST_OPERATION_LESS_EQUAL:
        case KEFIR_AST_OPERATION_GREATER:
        case KEFIR_AST_OPERATION_GREATER_EQUAL:
        case KEFIR_AST_OPERATION_EQUAL:
        case KEFIR_AST_OPERATION_NOT_EQUAL:
            REQUIRE_OK(translate_relational_equality(mem, context, builder, node));
            break;

        case KEFIR_AST_OPERATION_LOGICAL_AND:
            REQUIRE_OK(translate_logical_and(mem, context, builder, node));
            break;

        case KEFIR_AST_OPERATION_LOGICAL_OR:
            REQUIRE_OK(translate_logical_or(mem, context, builder, node));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected AST binary operation type");
    }
    return KEFIR_OK;
}
