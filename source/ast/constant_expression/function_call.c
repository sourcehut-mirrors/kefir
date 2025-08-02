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

kefir_result_t kefir_ast_evaluate_builtin_ffs_constant_expression_value(
    kefir_uint64_t arg, kefir_size_t bits, struct kefir_ast_constant_expression_value *value) {
    REQUIRE(value != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST constant expression value"));

    if (bits < CHAR_BIT * sizeof(kefir_uint64_t)) {
        arg &= (1ull << bits) - 1;
    }

    value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
    value->uinteger = 0;
    for (kefir_size_t i = 0; i < bits; i++) {
        if (((arg >> i) & 1) == 1) {
            value->uinteger = i + 1;
            break;
        }
    }

    return KEFIR_OK;
}

static kefir_result_t evaluate_clz(kefir_uint64_t arg, kefir_size_t bits,
                                   struct kefir_ast_constant_expression_value *value,
                                   const struct kefir_ast_function_call *node) {
    if (bits < CHAR_BIT * sizeof(kefir_uint64_t)) {
        arg &= (1ull << bits) - 1;
    }
    REQUIRE(arg != 0, KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                             "Expected constant expression AST node"));

    value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
    value->uinteger = 0;
    for (kefir_size_t i = 0; i < bits; i++) {
        if (((arg >> (bits - i - 1)) & 1) == 1) {
            value->uinteger = i;
            break;
        }
    }

    return KEFIR_OK;
}

static kefir_result_t evaluate_ctz(kefir_uint64_t arg, kefir_size_t bits,
                                   struct kefir_ast_constant_expression_value *value,
                                   const struct kefir_ast_function_call *node) {
    if (bits < CHAR_BIT * sizeof(kefir_uint64_t)) {
        arg &= (1ull << bits) - 1;
    }
    REQUIRE(arg != 0, KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                             "Expected constant expression AST node"));

    value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
    value->uinteger = 0;
    for (kefir_size_t i = 0; i < bits; i++) {
        if (((arg >> i) & 1) == 1) {
            value->uinteger = i;
            break;
        }
    }

    return KEFIR_OK;
}

static kefir_result_t evaluate_clrsb(kefir_uint64_t arg, kefir_size_t bits,
                                     struct kefir_ast_constant_expression_value *value) {
    if (bits < CHAR_BIT * sizeof(kefir_uint64_t)) {
        arg &= (1ull << bits) - 1;
    }
    const kefir_uint64_t msb = (arg >> (bits - 1)) & 1;

    value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
    value->uinteger = bits - 1;
    for (kefir_size_t i = 0; i < bits; i++) {
        const kefir_size_t index = bits - i - 1;
        const kefir_uint64_t bit = (arg >> index) & 1;
        if (bit != msb) {
            value->uinteger = i - 1;
        }
    }

    return KEFIR_OK;
}

static kefir_result_t evaluate_popcount(kefir_uint64_t arg, kefir_size_t bits,
                                        struct kefir_ast_constant_expression_value *value) {
    if (bits < CHAR_BIT * sizeof(kefir_uint64_t)) {
        arg &= (1ull << bits) - 1;
    }

    value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
    value->uinteger = 0;
    for (kefir_size_t i = 0; i < bits; i++) {
        if (((arg >> i) & 1) == 1) {
            value->uinteger++;
        }
    }

    return KEFIR_OK;
}

static kefir_result_t evaluate_parity(kefir_uint64_t arg, kefir_size_t bits,
                                      struct kefir_ast_constant_expression_value *value) {
    if (bits < CHAR_BIT * sizeof(kefir_uint64_t)) {
        arg &= (1ull << bits) - 1;
    }

    value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER;
    kefir_size_t ones = 0;
    for (kefir_size_t i = 0; i < bits; i++) {
        if (((arg >> i) & 1) == 1) {
            ones++;
        }
    }
    value->uinteger = ones % 2;

    return KEFIR_OK;
}

kefir_result_t kefir_ast_evaluate_function_call_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                     const struct kefir_ast_function_call *node,
                                                     struct kefir_ast_constant_expression_value *value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant node"));
    REQUIRE(value != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant expression value pointer"));
    REQUIRE(node->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
            KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                   "Expected constant expression AST node"));

    const char *function_name = NULL;
    if (node->function->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION &&
        node->function->properties.expression_props.identifier != NULL &&
        node->function->properties.expression_props.scoped_id->klass == KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION) {
        function_name = node->function->properties.expression_props.identifier;
    }

    REQUIRE(function_name != NULL, KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                                          "Expected constant expression AST node"));

    if (strcmp(function_name, "__kefir_builtin_ffs") == 0) {
        REQUIRE(kefir_list_length(&node->arguments) == 1,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Expected single argument to __builtin_ffs"));
        ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, subnode, kefir_list_head(&node->arguments)->value);
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subnode, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subnode->source_location,
                                       "Unable to evaluate constant expression"));
        REQUIRE_OK(kefir_ast_evaluate_builtin_ffs_constant_expression_value(
            KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subnode)->uinteger,
            context->type_traits->data_model->scalar_width.int_bits, value));
    } else if (strcmp(function_name, "__kefir_builtin_ffsl") == 0) {
        REQUIRE(kefir_list_length(&node->arguments) == 1,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Expected single argument to __builtin_ffsl"));
        ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, subnode, kefir_list_head(&node->arguments)->value);
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subnode, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subnode->source_location,
                                       "Unable to evaluate constant expression"));
        REQUIRE_OK(kefir_ast_evaluate_builtin_ffs_constant_expression_value(
            KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subnode)->uinteger,
            context->type_traits->data_model->scalar_width.long_bits, value));
    } else if (strcmp(function_name, "__kefir_builtin_ffsll") == 0) {
        REQUIRE(kefir_list_length(&node->arguments) == 1,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Expected single argument to __builtin_ffsll"));
        ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, subnode, kefir_list_head(&node->arguments)->value);
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subnode, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subnode->source_location,
                                       "Unable to evaluate constant expression"));
        REQUIRE_OK(kefir_ast_evaluate_builtin_ffs_constant_expression_value(
            KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subnode)->uinteger,
            context->type_traits->data_model->scalar_width.long_long_bits, value));
    } else if (strcmp(function_name, "__kefir_builtin_clz") == 0) {
        REQUIRE(kefir_list_length(&node->arguments) == 1,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Expected single argument to __builtin_clz"));
        ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, subnode, kefir_list_head(&node->arguments)->value);
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subnode, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subnode->source_location,
                                       "Unable to evaluate constant expression"));
        REQUIRE_OK(evaluate_clz(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subnode)->uinteger,
                                context->type_traits->data_model->scalar_width.int_bits, value, node));
    } else if (strcmp(function_name, "__kefir_builtin_clzl") == 0) {
        REQUIRE(kefir_list_length(&node->arguments) == 1,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Expected single argument to __builtin_clzl"));
        ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, subnode, kefir_list_head(&node->arguments)->value);
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subnode, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subnode->source_location,
                                       "Unable to evaluate constant expression"));
        REQUIRE_OK(evaluate_clz(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subnode)->uinteger,
                                context->type_traits->data_model->scalar_width.long_bits, value, node));
    } else if (strcmp(function_name, "__kefir_builtin_clzll") == 0) {
        REQUIRE(kefir_list_length(&node->arguments) == 1,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Expected single argument to __builtin_clzll"));
        ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, subnode, kefir_list_head(&node->arguments)->value);
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subnode, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subnode->source_location,
                                       "Unable to evaluate constant expression"));
        REQUIRE_OK(evaluate_clz(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subnode)->uinteger,
                                context->type_traits->data_model->scalar_width.long_long_bits, value, node));
    } else if (strcmp(function_name, "__kefir_builtin_ctz") == 0) {
        REQUIRE(kefir_list_length(&node->arguments) == 1,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Expected single argument to __builtin_ctz"));
        ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, subnode, kefir_list_head(&node->arguments)->value);
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subnode, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subnode->source_location,
                                       "Unable to evaluate constant expression"));
        REQUIRE_OK(evaluate_ctz(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subnode)->uinteger,
                                context->type_traits->data_model->scalar_width.int_bits, value, node));
    } else if (strcmp(function_name, "__kefir_builtin_ctzl") == 0) {
        REQUIRE(kefir_list_length(&node->arguments) == 1,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Expected single argument to __builtin_ctzl"));
        ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, subnode, kefir_list_head(&node->arguments)->value);
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subnode, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subnode->source_location,
                                       "Unable to evaluate constant expression"));
        REQUIRE_OK(evaluate_ctz(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subnode)->uinteger,
                                context->type_traits->data_model->scalar_width.long_bits, value, node));
    } else if (strcmp(function_name, "__kefir_builtin_ctzll") == 0) {
        REQUIRE(kefir_list_length(&node->arguments) == 1,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Expected single argument to __builtin_ctzll"));
        ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, subnode, kefir_list_head(&node->arguments)->value);
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subnode, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subnode->source_location,
                                       "Unable to evaluate constant expression"));
        REQUIRE_OK(evaluate_ctz(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subnode)->uinteger,
                                context->type_traits->data_model->scalar_width.long_long_bits, value, node));
    } else if (strcmp(function_name, "__kefir_builtin_clrsb") == 0) {
        REQUIRE(kefir_list_length(&node->arguments) == 1,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Expected single argument to __builtin_clrsb"));
        ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, subnode, kefir_list_head(&node->arguments)->value);
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subnode, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subnode->source_location,
                                       "Unable to evaluate constant expression"));
        REQUIRE_OK(evaluate_clrsb(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subnode)->uinteger,
                                  context->type_traits->data_model->scalar_width.int_bits, value));
    } else if (strcmp(function_name, "__kefir_builtin_clrsbl") == 0) {
        REQUIRE(kefir_list_length(&node->arguments) == 1,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Expected single argument to __builtin_clrsbl"));
        ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, subnode, kefir_list_head(&node->arguments)->value);
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subnode, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subnode->source_location,
                                       "Unable to evaluate constant expression"));
        REQUIRE_OK(evaluate_clrsb(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subnode)->uinteger,
                                  context->type_traits->data_model->scalar_width.long_bits, value));
    } else if (strcmp(function_name, "__kefir_builtin_clrsbll") == 0) {
        REQUIRE(kefir_list_length(&node->arguments) == 1,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Expected single argument to __builtin_clrsbll"));
        ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, subnode, kefir_list_head(&node->arguments)->value);
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subnode, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subnode->source_location,
                                       "Unable to evaluate constant expression"));
        REQUIRE_OK(evaluate_clrsb(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subnode)->uinteger,
                                  context->type_traits->data_model->scalar_width.long_long_bits, value));
    } else if (strcmp(function_name, "__kefir_builtin_popcount") == 0) {
        REQUIRE(kefir_list_length(&node->arguments) == 1,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Expected single argument to __builtin_popcount"));
        ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, subnode, kefir_list_head(&node->arguments)->value);
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subnode, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subnode->source_location,
                                       "Unable to evaluate constant expression"));
        REQUIRE_OK(evaluate_popcount(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subnode)->uinteger,
                                     context->type_traits->data_model->scalar_width.int_bits, value));
    } else if (strcmp(function_name, "__kefir_builtin_popcountl") == 0) {
        REQUIRE(kefir_list_length(&node->arguments) == 1,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Expected single argument to __builtin_popcountl"));
        ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, subnode, kefir_list_head(&node->arguments)->value);
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subnode, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subnode->source_location,
                                       "Unable to evaluate constant expression"));
        REQUIRE_OK(evaluate_popcount(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subnode)->uinteger,
                                     context->type_traits->data_model->scalar_width.long_bits, value));
    } else if (strcmp(function_name, "__kefir_builtin_popcountll") == 0) {
        REQUIRE(kefir_list_length(&node->arguments) == 1,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Expected single argument to __builtin_popcountll"));
        ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, subnode, kefir_list_head(&node->arguments)->value);
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subnode, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subnode->source_location,
                                       "Unable to evaluate constant expression"));
        REQUIRE_OK(evaluate_popcount(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subnode)->uinteger,
                                     context->type_traits->data_model->scalar_width.long_long_bits, value));
    } else if (strcmp(function_name, "__kefir_builtin_parity") == 0) {
        REQUIRE(kefir_list_length(&node->arguments) == 1,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Expected single argument to __builtin_parity"));
        ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, subnode, kefir_list_head(&node->arguments)->value);
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subnode, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subnode->source_location,
                                       "Unable to evaluate constant expression"));
        REQUIRE_OK(evaluate_parity(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subnode)->uinteger,
                                   context->type_traits->data_model->scalar_width.int_bits, value));
    } else if (strcmp(function_name, "__kefir_builtin_parityl") == 0) {
        REQUIRE(kefir_list_length(&node->arguments) == 1,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Expected single argument to __builtin_parityl"));
        ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, subnode, kefir_list_head(&node->arguments)->value);
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subnode, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subnode->source_location,
                                       "Unable to evaluate constant expression"));
        REQUIRE_OK(evaluate_parity(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subnode)->uinteger,
                                   context->type_traits->data_model->scalar_width.long_bits, value));
    } else if (strcmp(function_name, "__kefir_builtin_parityll") == 0) {
        REQUIRE(kefir_list_length(&node->arguments) == 1,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Expected single argument to __builtin_parityll"));
        ASSIGN_DECL_CAST(const struct kefir_ast_node_base *, subnode, kefir_list_head(&node->arguments)->value);
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(subnode, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &subnode->source_location,
                                       "Unable to evaluate constant expression"));
        REQUIRE_OK(evaluate_parity(KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(subnode)->uinteger,
                                   context->type_traits->data_model->scalar_width.long_long_bits, value));
    } else {
        return KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                      "Expected constant expression AST node");
    }

    return KEFIR_OK;
}
