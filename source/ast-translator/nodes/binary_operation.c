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
    REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                            arg2_normalized_type, result_normalized_type));
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
        switch (result_normalized_type->tag) {
            case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPLDADD, 0));
                break;

            case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPF64ADD, 0));
                break;

            case KEFIR_AST_TYPE_COMPLEX_FLOAT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPF32ADD, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_LDADD, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F64ADD, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_FLOAT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F32ADD, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_BOOL:
            case KEFIR_AST_TYPE_SCALAR_CHAR:
            case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IADD8, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IADD16, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IADD32, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
            case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IADD64, 0));
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
                                                    context->ast_context->type_traits->size_type));
        } else {
            REQUIRE_CHAIN(&res, kefir_ast_translate_expression(mem, node->arg2, builder, context));
            REQUIRE_CHAIN(&res, kefir_ast_translate_expression(mem, node->arg1, builder, context));
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                    node->arg1->properties.type,
                                                    context->ast_context->type_traits->size_type));
        }
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PUSHU64,
                                                            translator_type->object.layout->properties.size));
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_IMUL64, 0));
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_IADD64, 0));

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
        switch (result_normalized_type->tag) {
            case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPLDSUB, 0));
                break;

            case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPF64SUB, 0));
                break;

            case KEFIR_AST_TYPE_COMPLEX_FLOAT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPF32SUB, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_LDSUB, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F64SUB, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_FLOAT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F32SUB, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_BOOL:
            case KEFIR_AST_TYPE_SCALAR_CHAR:
            case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_ISUB8, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_ISUB16, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_ISUB32, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
            case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
            case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_ISUB64, 0));
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

        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_ISUB64, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PUSHU64, type_info.size));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IDIV64, 0));
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
        REQUIRE_CHAIN(&res,
                      kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                   arg2_normalized_type, context->ast_context->type_traits->size_type));
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_PUSHU64,
                                                            translator_type->object.layout->properties.size));
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_IMUL64, 0));
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_ISUB64, 0));

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
    switch (result_normalized_type->tag) {
        case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPLDMUL, 0));
            break;

        case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPF64MUL, 0));
            break;

        case KEFIR_AST_TYPE_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPF32MUL, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_LDMUL, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F64MUL, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F32MUL, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_BOOL:
        case KEFIR_AST_TYPE_SCALAR_CHAR:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IMUL8, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IMUL16, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IMUL32, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IMUL64, 0));
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
    switch (result_normalized_type->tag) {
        case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPLDDIV, 0));
            break;

        case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPF64DIV, 0));
            break;

        case KEFIR_AST_TYPE_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPF32DIV, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_LDDIV, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F64DIV, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F32DIV, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_BOOL:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_UDIV8, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IDIV8, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_CHAR:
            if (context->ast_context->type_traits->character_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IDIV8, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_UDIV8, 0));
            }
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_UDIV16, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IDIV16, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_UDIV32, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IDIV32, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_UDIV64, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IDIV64, 0));
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
    switch (result_normalized_type->tag) {
        case KEFIR_AST_TYPE_SCALAR_BOOL:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_UMOD8, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IMOD8, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_CHAR:
            if (context->ast_context->type_traits->character_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IMOD8, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_UMOD8, 0));
            }
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_UMOD16, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IMOD16, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_UMOD32, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IMOD32, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_UMOD64, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IMOD64, 0));
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
    const struct kefir_ast_type *arg1_normalized_type =
        kefir_ast_translator_normalize_type(node->arg1->properties.type);
    const struct kefir_ast_type *result_normalized_type =
        kefir_ast_translator_normalize_type(node->base.properties.type);

    switch (node->type) {
        case KEFIR_AST_OPERATION_SHIFT_LEFT:
            switch (result_normalized_type->tag) {
                case KEFIR_AST_TYPE_SCALAR_BOOL:
                case KEFIR_AST_TYPE_SCALAR_CHAR:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_ILSHIFT8, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_ILSHIFT16, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_ILSHIFT32, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_ILSHIFT64, 0));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
            }
            break;

        case KEFIR_AST_OPERATION_SHIFT_RIGHT:
            switch (arg1_normalized_type->tag) {
                case KEFIR_AST_TYPE_SCALAR_BOOL:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IRSHIFT8, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IARSHIFT8, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_CHAR:
                    if (context->ast_context->type_traits->character_type_signedness) {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IARSHIFT8, 0));
                    } else {
                        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IRSHIFT8, 0));
                    }
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IARSHIFT16, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IRSHIFT16, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IARSHIFT32, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IRSHIFT32, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IARSHIFT64, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IRSHIFT64, 0));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
            }
            break;

        case KEFIR_AST_OPERATION_BITWISE_AND:
            switch (result_normalized_type->tag) {
                case KEFIR_AST_TYPE_SCALAR_BOOL:
                case KEFIR_AST_TYPE_SCALAR_CHAR:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IAND8, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IAND16, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IAND32, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IAND64, 0));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
            }
            break;

        case KEFIR_AST_OPERATION_BITWISE_OR:
            switch (result_normalized_type->tag) {
                case KEFIR_AST_TYPE_SCALAR_BOOL:
                case KEFIR_AST_TYPE_SCALAR_CHAR:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IOR8, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IOR16, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IOR32, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IOR64, 0));
                    break;

                default:
                    return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of an integral type");
            }
            break;

        case KEFIR_AST_OPERATION_BITWISE_XOR:
            switch (result_normalized_type->tag) {
                case KEFIR_AST_TYPE_SCALAR_BOOL:
                case KEFIR_AST_TYPE_SCALAR_CHAR:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IXOR8, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IXOR16, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IXOR32, 0));
                    break;

                case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
                case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
                case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
                    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IXOR64, 0));
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

static kefir_result_t translate_relational_equals(const struct kefir_ast_type *common_type,
                                                  struct kefir_irbuilder_block *builder) {
    switch (common_type->tag) {
        case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPLDEQUALS, 0));
            break;

        case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPF64EQUALS, 0));
            break;

        case KEFIR_AST_TYPE_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_CMPF32EQUALS, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_LDEQUALS, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F64EQUALS, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F32EQUALS, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_BOOL:
        case KEFIR_AST_TYPE_SCALAR_CHAR:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IEQUALS8, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IEQUALS16, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IEQUALS32, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_POINTER:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IEQUALS64, 0));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of a scalar type");
    }
    return KEFIR_OK;
}

static kefir_result_t translate_relational_not_equals(const struct kefir_ast_type *common_type,
                                                      struct kefir_irbuilder_block *builder) {
    REQUIRE_OK(translate_relational_equals(common_type, builder));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BNOT8, 0));
    return KEFIR_OK;
}

static kefir_result_t translate_relational_less(const struct kefir_ast_type_traits *type_traits,
                                                const struct kefir_ast_type *common_type,
                                                struct kefir_irbuilder_block *builder) {
    switch (common_type->tag) {
        case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_LDLESSER, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F64LESSER, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F32LESSER, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_POINTER:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IBELOW64, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_BOOL:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IBELOW8, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_ILESSER8, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_CHAR:
            if (type_traits->character_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_ILESSER8, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IBELOW8, 0));
            }
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IBELOW16, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_ILESSER16, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IBELOW32, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_ILESSER32, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IBELOW64, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_ILESSER64, 0));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected value of a scalar type");
    }
    return KEFIR_OK;
}

static kefir_result_t translate_relational_greater(const struct kefir_ast_type_traits *type_traits,
                                                   const struct kefir_ast_type *common_type,
                                                   struct kefir_irbuilder_block *builder) {
    switch (common_type->tag) {
        case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_LDGREATER, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F64GREATER, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_F32GREATER, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_POINTER:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IABOVE64, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_BOOL:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IABOVE8, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IGREATER8, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_CHAR:
            if (type_traits->character_type_signedness) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IGREATER8, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IABOVE8, 0));
            }
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IABOVE16, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IGREATER16, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IABOVE32, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IGREATER32, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IABOVE64, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_IGREATER64, 0));
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
            REQUIRE_OK(translate_relational_equals(common_type, builder));
            break;

        case KEFIR_AST_OPERATION_NOT_EQUAL:
            REQUIRE_OK(translate_relational_not_equals(common_type, builder));
            break;

        case KEFIR_AST_OPERATION_LESS:
            REQUIRE_OK(translate_relational_less(context->ast_context->type_traits, common_type, builder));
            break;

        case KEFIR_AST_OPERATION_GREATER:
            REQUIRE_OK(translate_relational_greater(context->ast_context->type_traits, common_type, builder));
            break;

        case KEFIR_AST_OPERATION_LESS_EQUAL:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_PICK, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_PICK, 1));
            REQUIRE_OK(translate_relational_less(context->ast_context->type_traits, common_type, builder));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_XCHG, 2));
            REQUIRE_OK(translate_relational_equals(common_type, builder));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BOR64, 0));
            break;

        case KEFIR_AST_OPERATION_GREATER_EQUAL:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_PICK, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_PICK, 1));
            REQUIRE_OK(translate_relational_greater(context->ast_context->type_traits, common_type, builder));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_XCHG, 2));
            REQUIRE_OK(translate_relational_equals(common_type, builder));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BOR64, 0));
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
    REQUIRE_OK(kefir_ast_translate_typeconv_to_bool(builder, normalized_type));
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
    REQUIRE_OK(kefir_ast_translate_typeconv_to_bool(builder, normalized_type1));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_PICK, 0));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BNOT8, 0));
    kefir_size_t jmpIndex = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BRANCH8, 0));
    REQUIRE_OK(kefir_ast_translate_expression(mem, node->arg2, builder, context));
    switch (normalized_type2->tag) {
        case KEFIR_AST_TYPE_SCALAR_BOOL:
        case KEFIR_AST_TYPE_SCALAR_CHAR:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BAND8, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BAND16, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BAND32, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_POINTER:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BAND64, 0));
            break;

        default:
            REQUIRE_OK(kefir_ast_translate_typeconv_to_bool(builder, normalized_type2));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BAND8, 0));
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
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_PICK, 0));
    kefir_size_t jmpIndex = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BRANCH8, 0));
    REQUIRE_OK(kefir_ast_translate_expression(mem, node->arg2, builder, context));
    switch (normalized_type2->tag) {
        case KEFIR_AST_TYPE_SCALAR_BOOL:
        case KEFIR_AST_TYPE_SCALAR_CHAR:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BOR8, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BOR16, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BOR32, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_POINTER:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BOR64, 0));
            break;

        default:
            REQUIRE_OK(kefir_ast_translate_typeconv_to_bool(builder, normalized_type2));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_BOR64, 0));
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
