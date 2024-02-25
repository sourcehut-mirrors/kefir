/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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
#include "kefir/ast-translator/value.h"
#include "kefir/ast-translator/lvalue.h"
#include "kefir/ast-translator/util.h"
#include "kefir/ast-translator/typeconv.h"
#include "kefir/ast-translator/type.h"
#include "kefir/ast-translator/temporaries.h"
#include "kefir/ast/type_conv.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

static kefir_result_t store_value(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                  struct kefir_irbuilder_block *builder,
                                  const struct kefir_ast_assignment_operator *node,
                                  const struct kefir_ast_type *result_normalized_type) {
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_PICK, 1));
    REQUIRE_OK(kefir_ast_translator_store_lvalue(mem, context, builder, node->target));

    if (node->target->properties.expression_props.bitfield_props.bitfield &&
        KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(result_normalized_type)) {
        kefir_bool_t signedness;
        REQUIRE_OK(kefir_ast_type_is_signed(context->ast_context->type_traits, result_normalized_type, &signedness));
        const kefir_size_t bitwidth = node->target->properties.expression_props.bitfield_props.width;
        if (signedness) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU32(builder, KEFIR_IROPCODE_EXTSBITS, 0, bitwidth));
        } else {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU32(builder, KEFIR_IROPCODE_EXTUBITS, 0, bitwidth));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_simple(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                       struct kefir_irbuilder_block *builder,
                                       const struct kefir_ast_assignment_operator *node) {
    const struct kefir_ast_type *value_init_normalized_type =
        kefir_ast_translator_normalize_type(node->value->properties.type);
    const struct kefir_ast_type *value_normalized_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, value_init_normalized_type);
    REQUIRE(value_normalized_type != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to perform lvalue conversions"));
    const struct kefir_ast_type *result_normalized_type =
        kefir_ast_translator_normalize_type(node->base.properties.type);

    REQUIRE_OK(kefir_ast_translate_expression(mem, node->value, builder, context));
    if (KEFIR_AST_TYPE_IS_SCALAR_TYPE(result_normalized_type)) {
        REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                value_normalized_type, result_normalized_type));
    }

    REQUIRE_OK(kefir_ast_translate_lvalue(mem, context, builder, node->target));

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_XCHG, 1));  // [VALUE*, ARGUMENT]
    REQUIRE_OK(store_value(mem, context, builder, node, result_normalized_type));
    return KEFIR_OK;
}

static kefir_result_t reorder_assignment_arguments(struct kefir_irbuilder_block *builder) {
    // [ARGUMENT, VALUE*, CURRENT]
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_XCHG, 1));  // [ARGUMENT, CURRENT, VALUE*]
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_XCHG, 2));  // [VALUE*, CURRENT, ARGUMENT]
    return KEFIR_OK;
}

struct generate_op_parameters {
    struct kefir_mem *mem;
    struct kefir_ast_translator_context *context;
    struct kefir_irbuilder_block *builder;
    const struct kefir_ast_assignment_operator *node;
    const struct kefir_ast_type *common_type;
    const struct kefir_ast_type *value_normalized_type;
    const struct kefir_ast_type *target_normalized_type;
    const struct kefir_ast_type *result_normalized_type;
};

static kefir_result_t generate_multiplication_op(const struct generate_op_parameters *parameters) {
    switch (parameters->common_type->tag) {
        case KEFIR_AST_TYPE_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(parameters->builder, KEFIR_IROPCODE_CMPF32MUL, 0));
            break;

        case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(parameters->builder, KEFIR_IROPCODE_CMPF64MUL, 0));
            break;

        case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(parameters->builder, KEFIR_IROPCODE_CMPLDMUL, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(parameters->builder, KEFIR_IROPCODE_LDMUL, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(parameters->builder, KEFIR_IROPCODE_F64MUL, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(parameters->builder, KEFIR_IROPCODE_F32MUL, 0));
            break;

        default:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(parameters->builder, KEFIR_IROPCODE_IMUL, 0));
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t translate_binary_op(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                          struct kefir_irbuilder_block *builder,
                                          const struct kefir_ast_assignment_operator *node,
                                          kefir_result_t (*generate_op)(const struct generate_op_parameters *)) {
    const struct kefir_ast_type *target_normalized_type =
        kefir_ast_translator_normalize_type(node->target->properties.type);
    const struct kefir_ast_type *value_init_normalized_type =
        kefir_ast_translator_normalize_type(node->value->properties.type);
    const struct kefir_ast_type *value_normalized_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->ast_context->type_bundle, value_init_normalized_type);
    REQUIRE(value_normalized_type != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to perform lvalue conversions"));
    const struct kefir_ast_type *common_type = NULL;
    if (KEFIR_AST_TYPE_IS_ARITHMETIC_TYPE(target_normalized_type) &&
        KEFIR_AST_TYPE_IS_ARITHMETIC_TYPE(value_normalized_type)) {
        common_type = kefir_ast_type_common_arithmetic(context->ast_context->type_traits, target_normalized_type,
                                                       node->target->properties.expression_props.bitfield_props,
                                                       value_normalized_type,
                                                       node->value->properties.expression_props.bitfield_props);
        REQUIRE(common_type != NULL,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to determine common arithmetic type"));
    }
    const struct kefir_ast_type *result_normalized_type =
        kefir_ast_translator_normalize_type(node->base.properties.type);

    const struct generate_op_parameters generate_params = {.mem = mem,
                                                           .context = context,
                                                           .builder = builder,
                                                           .node = node,
                                                           .target_normalized_type = target_normalized_type,
                                                           .value_normalized_type = value_normalized_type,
                                                           .common_type = common_type,
                                                           .result_normalized_type = result_normalized_type};

    REQUIRE_OK(kefir_ast_translate_expression(mem, node->value, builder, context));
    if (common_type != NULL) {
        REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                value_normalized_type, common_type));
    }

    kefir_bool_t preserve_fenv = false;
    if (node->target->properties.expression_props.atomic && common_type != NULL &&
        KEFIR_AST_TYPE_IS_FLOATING_POINT(common_type)) {
        preserve_fenv = true;
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_FENV_SAVE, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_XCHG, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_FENV_CLEAR, 0));
    }

    kefir_bool_t atomic_aggregate_target_value;
    REQUIRE_OK(kefir_ast_translate_lvalue(mem, context, builder, node->target));
    if (!node->target->properties.expression_props.atomic) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_PICK, 0));
        REQUIRE_OK(
            kefir_ast_translator_resolve_lvalue(mem, context, builder, node->target, &atomic_aggregate_target_value));
        if (common_type != NULL) {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                    target_normalized_type, common_type));
        }
        REQUIRE(!atomic_aggregate_target_value,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected atomic aggregate value"));

        REQUIRE_OK(reorder_assignment_arguments(builder));
        REQUIRE_OK(generate_op(&generate_params));
        if (common_type != NULL) {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                    common_type, result_normalized_type));
        }
        REQUIRE_OK(store_value(mem, context, builder, node, result_normalized_type));
    } else {
        const kefir_size_t failBranchTarget = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_PICK, 0));
        REQUIRE_OK(
            kefir_ast_translator_resolve_lvalue(mem, context, builder, node->target, &atomic_aggregate_target_value));
        if (common_type != NULL) {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                    target_normalized_type, common_type));
        }
        REQUIRE(!atomic_aggregate_target_value,
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected atomic aggregate value"));

        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_PICK, 1));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_PICK, 3));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_PICK, 2));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_XCHG, 1));

        REQUIRE_OK(generate_op(&generate_params));
        if (common_type != NULL) {
            REQUIRE_OK(kefir_ast_translate_typeconv(mem, context->module, builder, context->ast_context->type_traits,
                                                    common_type, result_normalized_type));
        }

        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_XCHG, 2));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_PICK, 2));

        REQUIRE_OK(kefir_ast_translator_atomic_compare_exchange_value(mem, result_normalized_type, context, builder,
                                                                      &node->base.source_location));

        const kefir_size_t successBranch = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_BRANCH, 0));
        ;
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_POP, 0));
        if (preserve_fenv) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_FENV_CLEAR, 0));
        }
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDU64(builder, KEFIR_IROPCODE_JMP, failBranchTarget));

        KEFIR_IRBUILDER_BLOCK_INSTR_AT(builder, successBranch)->arg.i64 = KEFIR_IRBUILDER_BLOCK_CURRENT_INDEX(builder);
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_XCHG, 2));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_POP, 0));
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_POP, 0));

        if (preserve_fenv) {
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_XCHG, 1));
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(builder, KEFIR_IROPCODE_FENV_UPDATE, 0));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t translate_multiplication(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                               struct kefir_irbuilder_block *builder,
                                               const struct kefir_ast_assignment_operator *node) {
    REQUIRE_OK(translate_binary_op(mem, context, builder, node, generate_multiplication_op));
    return KEFIR_OK;
}

static kefir_result_t generate_division_op(const struct generate_op_parameters *params) {
    switch (params->common_type->tag) {
        case KEFIR_AST_TYPE_COMPLEX_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_CMPF32DIV, 0));
            break;

        case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_CMPF64DIV, 0));
            break;

        case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_CMPLDDIV, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_LDDIV, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_DOUBLE:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_F64DIV, 0));
            break;

        case KEFIR_AST_TYPE_SCALAR_FLOAT:
            REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_F32DIV, 0));
            break;

        default: {
            kefir_bool_t signed_integer = false;
            REQUIRE_OK(kefir_ast_type_is_signed(params->context->ast_context->type_traits, params->common_type,
                                                &signed_integer));
            if (signed_integer) {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_IDIV, 0));
            } else {
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_UDIV, 0));
            }
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t translate_divide(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                       struct kefir_irbuilder_block *builder,
                                       const struct kefir_ast_assignment_operator *node) {
    REQUIRE_OK(translate_binary_op(mem, context, builder, node, generate_division_op));
    return KEFIR_OK;
}

static kefir_result_t generate_modulo(const struct generate_op_parameters *params) {
    REQUIRE(!KEFIR_AST_TYPE_IS_LONG_DOUBLE(params->common_type),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected value of long double type"));
    REQUIRE(!KEFIR_AST_TYPE_IS_LONG_DOUBLE(params->result_normalized_type),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected value of long double type"));

    kefir_bool_t signedness;
    REQUIRE_OK(kefir_ast_type_is_signed(params->context->ast_context->type_traits, params->value_normalized_type,
                                        &signedness));

    if (signedness) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_IMOD, 0));
    } else {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_UMOD, 0));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_modulo(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                       struct kefir_irbuilder_block *builder,
                                       const struct kefir_ast_assignment_operator *node) {
    REQUIRE_OK(translate_binary_op(mem, context, builder, node, generate_modulo));
    return KEFIR_OK;
}

static kefir_result_t generate_lshift(const struct generate_op_parameters *params) {
    REQUIRE(!KEFIR_AST_TYPE_IS_LONG_DOUBLE(params->common_type),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected value of long double type"));
    REQUIRE(!KEFIR_AST_TYPE_IS_LONG_DOUBLE(params->result_normalized_type),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected value of long double type"));

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_ILSHIFT, 0));
    return KEFIR_OK;
}

static kefir_result_t translate_lshift(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                       struct kefir_irbuilder_block *builder,
                                       const struct kefir_ast_assignment_operator *node) {
    REQUIRE_OK(translate_binary_op(mem, context, builder, node, generate_lshift));
    return KEFIR_OK;
}

static kefir_result_t generate_rshift(const struct generate_op_parameters *params) {
    REQUIRE(!KEFIR_AST_TYPE_IS_LONG_DOUBLE(params->common_type),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected value of long double type"));
    REQUIRE(!KEFIR_AST_TYPE_IS_LONG_DOUBLE(params->result_normalized_type),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected value of long double type"));

    kefir_bool_t signedness;
    REQUIRE_OK(kefir_ast_type_is_signed(params->context->ast_context->type_traits, params->value_normalized_type,
                                        &signedness));

    if (signedness) {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_IARSHIFT, 0));
    } else {
        REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_IRSHIFT, 0));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_rshift(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                       struct kefir_irbuilder_block *builder,
                                       const struct kefir_ast_assignment_operator *node) {
    REQUIRE_OK(translate_binary_op(mem, context, builder, node, generate_rshift));
    return KEFIR_OK;
}

static kefir_result_t generate_iand(const struct generate_op_parameters *params) {
    REQUIRE(!KEFIR_AST_TYPE_IS_LONG_DOUBLE(params->common_type),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected value of long double type"));
    REQUIRE(!KEFIR_AST_TYPE_IS_LONG_DOUBLE(params->result_normalized_type),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected value of long double type"));

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_IAND, 0));
    return KEFIR_OK;
}

static kefir_result_t translate_iand(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                     struct kefir_irbuilder_block *builder,
                                     const struct kefir_ast_assignment_operator *node) {
    REQUIRE_OK(translate_binary_op(mem, context, builder, node, generate_iand));
    return KEFIR_OK;
}

static kefir_result_t generate_ior(const struct generate_op_parameters *params) {
    REQUIRE(!KEFIR_AST_TYPE_IS_LONG_DOUBLE(params->common_type),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected value of long double type"));
    REQUIRE(!KEFIR_AST_TYPE_IS_LONG_DOUBLE(params->result_normalized_type),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected value of long double type"));

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_IOR, 0));
    return KEFIR_OK;
}

static kefir_result_t translate_ior(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                    struct kefir_irbuilder_block *builder,
                                    const struct kefir_ast_assignment_operator *node) {
    REQUIRE_OK(translate_binary_op(mem, context, builder, node, generate_ior));
    return KEFIR_OK;
}

static kefir_result_t generate_ixor(const struct generate_op_parameters *params) {
    REQUIRE(!KEFIR_AST_TYPE_IS_LONG_DOUBLE(params->common_type),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected value of long double type"));
    REQUIRE(!KEFIR_AST_TYPE_IS_LONG_DOUBLE(params->result_normalized_type),
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected value of long double type"));

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_IXOR, 0));
    return KEFIR_OK;
}

static kefir_result_t translate_ixor(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                     struct kefir_irbuilder_block *builder,
                                     const struct kefir_ast_assignment_operator *node) {
    REQUIRE_OK(translate_binary_op(mem, context, builder, node, generate_ixor));
    return KEFIR_OK;
}

static kefir_result_t generate_add(const struct generate_op_parameters *params) {
    if (KEFIR_AST_TYPE_IS_ARITHMETIC_TYPE(params->target_normalized_type) &&
        KEFIR_AST_TYPE_IS_ARITHMETIC_TYPE(params->value_normalized_type)) {
        switch (params->common_type->tag) {
            case KEFIR_AST_TYPE_COMPLEX_FLOAT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_CMPF32ADD, 0));
                break;

            case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_CMPF64ADD, 0));
                break;

            case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_CMPLDADD, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_LDADD, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_F64ADD, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_FLOAT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_F32ADD, 0));
                break;

            default:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_IADD, 0));
                break;
        }
    } else {
        REQUIRE(params->target_normalized_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER &&
                    KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(params->value_normalized_type),
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                "Expected scalar pointer on the left side, and an integer on the right"));

        struct kefir_ast_translator_type *translator_type = NULL;
        REQUIRE_OK(kefir_ast_translator_type_new(
            params->mem, params->context->ast_context, params->context->environment, params->context->module,
            params->target_normalized_type->referenced_type, 0, &translator_type, &params->node->base.source_location));

        kefir_result_t res = KEFIR_IRBUILDER_BLOCK_APPENDU64(params->builder, KEFIR_IROPCODE_IADDX,
                                                             translator_type->object.layout->properties.size);

        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_translator_type_free(params->mem, translator_type);
            return res;
        });
        REQUIRE_OK(kefir_ast_translator_type_free(params->mem, translator_type));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_add(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                    struct kefir_irbuilder_block *builder,
                                    const struct kefir_ast_assignment_operator *node) {
    REQUIRE_OK(translate_binary_op(mem, context, builder, node, generate_add));
    return KEFIR_OK;
}

static kefir_result_t generate_sub(const struct generate_op_parameters *params) {
    if (KEFIR_AST_TYPE_IS_ARITHMETIC_TYPE(params->target_normalized_type) &&
        KEFIR_AST_TYPE_IS_ARITHMETIC_TYPE(params->value_normalized_type)) {
        switch (params->common_type->tag) {
            case KEFIR_AST_TYPE_COMPLEX_FLOAT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_CMPF32SUB, 0));
                break;

            case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_CMPF64SUB, 0));
                break;

            case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_CMPLDSUB, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_LDSUB, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_DOUBLE:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_F64SUB, 0));
                break;

            case KEFIR_AST_TYPE_SCALAR_FLOAT:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_F32SUB, 0));
                break;

            default:
                REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_ISUB, 0));
                break;
        }
    } else {
        REQUIRE(params->target_normalized_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER &&
                    KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(params->value_normalized_type),
                KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                "Expected scalar pointer on the left side, and an integer on the right"));

        struct kefir_ast_translator_type *translator_type = NULL;
        REQUIRE_OK(kefir_ast_translator_type_new(
            params->mem, params->context->ast_context, params->context->environment, params->context->module,
            params->target_normalized_type->referenced_type, 0, &translator_type, &params->node->base.source_location));

        kefir_result_t res = KEFIR_IRBUILDER_BLOCK_APPENDI64(params->builder, KEFIR_IROPCODE_INEG, 0);
        REQUIRE_CHAIN(&res, KEFIR_IRBUILDER_BLOCK_APPENDU64(params->builder, KEFIR_IROPCODE_IADDX,
                                                            translator_type->object.layout->properties.size));

        REQUIRE_ELSE(res == KEFIR_OK, {
            kefir_ast_translator_type_free(params->mem, translator_type);
            return res;
        });
        REQUIRE_OK(kefir_ast_translator_type_free(params->mem, translator_type));
    }
    return KEFIR_OK;
}

static kefir_result_t translate_sub(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
                                    struct kefir_irbuilder_block *builder,
                                    const struct kefir_ast_assignment_operator *node) {
    REQUIRE_OK(translate_binary_op(mem, context, builder, node, generate_sub));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_translate_assignment_operator_node(struct kefir_mem *mem,
                                                            struct kefir_ast_translator_context *context,
                                                            struct kefir_irbuilder_block *builder,
                                                            const struct kefir_ast_assignment_operator *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation context"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant node"));

    switch (node->operation) {
        case KEFIR_AST_ASSIGNMENT_SIMPLE:
            REQUIRE_OK(translate_simple(mem, context, builder, node));
            break;

        case KEFIR_AST_ASSIGNMENT_MULTIPLY:
            REQUIRE_OK(translate_multiplication(mem, context, builder, node));
            break;

        case KEFIR_AST_ASSIGNMENT_DIVIDE:
            REQUIRE_OK(translate_divide(mem, context, builder, node));
            break;

        case KEFIR_AST_ASSIGNMENT_MODULO:
            REQUIRE_OK(translate_modulo(mem, context, builder, node));
            break;

        case KEFIR_AST_ASSIGNMENT_SHIFT_LEFT:
            REQUIRE_OK(translate_lshift(mem, context, builder, node));
            break;

        case KEFIR_AST_ASSIGNMENT_SHIFT_RIGHT:
            REQUIRE_OK(translate_rshift(mem, context, builder, node));
            break;

        case KEFIR_AST_ASSIGNMENT_BITWISE_AND:
            REQUIRE_OK(translate_iand(mem, context, builder, node));
            break;

        case KEFIR_AST_ASSIGNMENT_BITWISE_OR:
            REQUIRE_OK(translate_ior(mem, context, builder, node));
            break;

        case KEFIR_AST_ASSIGNMENT_BITWISE_XOR:
            REQUIRE_OK(translate_ixor(mem, context, builder, node));
            break;

        case KEFIR_AST_ASSIGNMENT_ADD:
            REQUIRE_OK(translate_add(mem, context, builder, node));
            break;

        case KEFIR_AST_ASSIGNMENT_SUBTRACT:
            REQUIRE_OK(translate_sub(mem, context, builder, node));
            break;
    }
    return KEFIR_OK;
}
