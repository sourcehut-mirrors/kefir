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

#include "kefir/ast/format.h"
#include "kefir/lexer/format.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

struct visitor_param {
    struct kefir_json_output *json;
    kefir_bool_t display_source_location;
};

static kefir_result_t format_source_location(struct kefir_json_output *json, const struct kefir_ast_node_base *node) {
    REQUIRE_OK(kefir_json_output_object_key(json, "source_location"));
    if (kefir_source_location_get(&node->source_location, NULL, NULL, NULL)) {
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "source"));
        REQUIRE_OK(kefir_json_output_string(json, node->source_location.source));
        REQUIRE_OK(kefir_json_output_object_key(json, "line"));
        REQUIRE_OK(kefir_json_output_uinteger(json, node->source_location.line));
        REQUIRE_OK(kefir_json_output_object_key(json, "column"));
        REQUIRE_OK(kefir_json_output_uinteger(json, node->source_location.column));
        REQUIRE_OK(kefir_json_output_object_end(json));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    return KEFIR_OK;
}

static kefir_result_t format_attributes(struct kefir_json_output *json,
                                        const struct kefir_ast_node_attributes *attributes,
                                        kefir_bool_t display_source_location) {
    REQUIRE_OK(kefir_json_output_object_key(json, "attributes"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (const struct kefir_list_entry *iter = kefir_list_head(&attributes->attributes); iter != NULL;
         kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(struct kefir_ast_attribute_list *, attribute_list, iter->value);
        REQUIRE_OK(kefir_ast_format(json, KEFIR_AST_NODE_BASE(attribute_list), display_source_location));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));

    return KEFIR_OK;
}

static kefir_result_t visit_not_impl(const struct kefir_ast_visitor *visitor, const struct kefir_ast_node_base *node,
                                     void *payload) {
    UNUSED(visitor);
    UNUSED(node);
    UNUSED(payload);
    return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Missing AST node JSON formatter");
}

static kefir_result_t visit_identifier(const struct kefir_ast_visitor *visitor, const struct kefir_ast_identifier *node,
                                       void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST identifier node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "identifier"));
    REQUIRE_OK(kefir_json_output_object_key(json, "identifier"));
    REQUIRE_OK(kefir_json_output_string(json, node->identifier));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_constant(const struct kefir_ast_visitor *visitor, const struct kefir_ast_constant *node,
                                     void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "constant"));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    switch (node->type) {
        case KEFIR_AST_NULLPTR_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "nullptr"));
            break;

        case KEFIR_AST_BOOL_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "boolean"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_boolean(json, node->value.boolean));
            break;

        case KEFIR_AST_CHAR_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "character"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_integer(json, node->value.character));
            break;

        case KEFIR_AST_UNICODE8_CHAR_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "unicode8_character"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_integer(json, node->value.character));
            break;

        case KEFIR_AST_WIDE_CHAR_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "wide_character"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_integer(json, node->value.wide_character));
            break;

        case KEFIR_AST_UNICODE16_CHAR_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "unicode16_character"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_uinteger(json, node->value.unicode16_character));
            break;

        case KEFIR_AST_UNICODE32_CHAR_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "unicode32_character"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_uinteger(json, node->value.unicode32_character));
            break;

        case KEFIR_AST_INT_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "integer"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_integer(json, node->value.integer));
            break;

        case KEFIR_AST_UINT_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "unsigned_integer"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_uinteger(json, node->value.uinteger));
            break;

        case KEFIR_AST_LONG_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "long"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_integer(json, node->value.long_integer));
            break;

        case KEFIR_AST_ULONG_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "unsigned_long"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_uinteger(json, node->value.ulong_integer));
            break;

        case KEFIR_AST_LONG_LONG_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "long_long"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_integer(json, node->value.long_long));
            break;

        case KEFIR_AST_ULONG_LONG_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "unsigned_long_long"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_uinteger(json, node->value.ulong_long));
            break;

        case KEFIR_AST_BITPRECISE_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "bitprecise"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_bigint(json, &node->value.bitprecise));
            REQUIRE_OK(kefir_json_output_object_key(json, "width"));
            REQUIRE_OK(kefir_json_output_integer(json, node->value.bitprecise.bitwidth));
            break;

        case KEFIR_AST_UNSIGNED_BITPRECISE_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "unsigned_bitprecise"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_bigint(json, &node->value.bitprecise));
            REQUIRE_OK(kefir_json_output_object_key(json, "width"));
            REQUIRE_OK(kefir_json_output_integer(json, node->value.bitprecise.bitwidth));
            break;

        case KEFIR_AST_FLOAT_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "float"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_float(json, node->value.float32));
            break;

        case KEFIR_AST_DOUBLE_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "double"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_float(json, node->value.float64));
            break;

        case KEFIR_AST_LONG_DOUBLE_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "long_double"));
            REQUIRE_OK(kefir_json_output_object_key(json, "value"));
            REQUIRE_OK(kefir_json_output_long_double(json, node->value.long_double));
            break;

        case KEFIR_AST_COMPLEX_FLOAT_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "complex_float"));
            REQUIRE_OK(kefir_json_output_object_key(json, "real_value"));
            REQUIRE_OK(kefir_json_output_float(json, node->value.complex_float32.real));
            REQUIRE_OK(kefir_json_output_object_key(json, "imaginary_value"));
            REQUIRE_OK(kefir_json_output_float(json, node->value.complex_float32.imaginary));
            break;

        case KEFIR_AST_COMPLEX_DOUBLE_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "complex_double"));
            REQUIRE_OK(kefir_json_output_object_key(json, "real_value"));
            REQUIRE_OK(kefir_json_output_float(json, node->value.complex_float64.real));
            REQUIRE_OK(kefir_json_output_object_key(json, "imaginary_value"));
            REQUIRE_OK(kefir_json_output_float(json, node->value.complex_float64.imaginary));
            break;

        case KEFIR_AST_COMPLEX_LONG_DOUBLE_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "complex_long_double"));
            REQUIRE_OK(kefir_json_output_object_key(json, "real_value"));
            REQUIRE_OK(kefir_json_output_float(json, node->value.complex_long_double.real));
            REQUIRE_OK(kefir_json_output_object_key(json, "imaginary_value"));
            REQUIRE_OK(kefir_json_output_float(json, node->value.complex_long_double.imaginary));
            break;
    }
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_string_literal(const struct kefir_ast_visitor *visitor,
                                           const struct kefir_ast_string_literal *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST string literal node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "string_literal"));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    switch (node->type) {
        case KEFIR_AST_STRING_LITERAL_MULTIBYTE:
            REQUIRE_OK(kefir_json_output_string(json, "multibyte"));
            REQUIRE_OK(kefir_json_output_object_key(json, "content"));
            REQUIRE_OK(kefir_json_output_raw_string(json, node->literal, node->length));
            break;

        case KEFIR_AST_STRING_LITERAL_UNICODE8:
            REQUIRE_OK(kefir_json_output_string(json, "unicode8"));
            REQUIRE_OK(kefir_json_output_object_key(json, "content"));
            REQUIRE_OK(kefir_json_output_raw_string(json, node->literal, node->length));
            break;

        case KEFIR_AST_STRING_LITERAL_UNICODE16:
            REQUIRE_OK(kefir_json_output_string(json, "unicode16"));
            REQUIRE_OK(kefir_json_output_object_key(json, "content"));
            REQUIRE_OK(kefir_json_output_raw_string(json, node->literal, node->length * sizeof(kefir_char16_t)));
            break;

        case KEFIR_AST_STRING_LITERAL_UNICODE32:
            REQUIRE_OK(kefir_json_output_string(json, "unicode32"));
            REQUIRE_OK(kefir_json_output_object_key(json, "content"));
            REQUIRE_OK(kefir_json_output_raw_string(json, node->literal, node->length * sizeof(kefir_char32_t)));
            break;

        case KEFIR_AST_STRING_LITERAL_WIDE:
            REQUIRE_OK(kefir_json_output_string(json, "wide"));
            REQUIRE_OK(kefir_json_output_object_key(json, "content"));
            REQUIRE_OK(kefir_json_output_raw_string(json, node->literal, node->length * sizeof(kefir_wchar_t)));
            break;
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "length"));
    REQUIRE_OK(kefir_json_output_uinteger(json, node->length));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_array_subscript(const struct kefir_ast_visitor *visitor,
                                            const struct kefir_ast_array_subscript *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST array subscript node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "array_subscript"));
    REQUIRE_OK(kefir_json_output_object_key(json, "array"));
    REQUIRE_OK(kefir_ast_format(json, node->array, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "subscript"));
    REQUIRE_OK(kefir_ast_format(json, node->subscript, param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_function_call(const struct kefir_ast_visitor *visitor,
                                          const struct kefir_ast_function_call *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST function call node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "function_call"));
    REQUIRE_OK(kefir_json_output_object_key(json, "function"));
    REQUIRE_OK(kefir_ast_format(json, node->function, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "arguments"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (const struct kefir_list_entry *iter = kefir_list_head(&node->arguments); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_node_base *, arg, iter->value);
        REQUIRE_OK(kefir_ast_format(json, arg, param->display_source_location));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_builtin(const struct kefir_ast_visitor *visitor, const struct kefir_ast_builtin *node,
                                    void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST builtin node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "builtin"));
    REQUIRE_OK(kefir_json_output_object_key(json, "builtin"));
    switch (node->builtin) {
        case KEFIR_AST_BUILTIN_VA_START:
            REQUIRE_OK(kefir_json_output_string(json, "va_start"));
            break;

        case KEFIR_AST_BUILTIN_VA_END:
            REQUIRE_OK(kefir_json_output_string(json, "va_end"));
            break;

        case KEFIR_AST_BUILTIN_VA_COPY:
            REQUIRE_OK(kefir_json_output_string(json, "va_copy"));
            break;

        case KEFIR_AST_BUILTIN_VA_ARG:
            REQUIRE_OK(kefir_json_output_string(json, "va_args"));
            break;

        case KEFIR_AST_BUILTIN_ALLOCA:
            REQUIRE_OK(kefir_json_output_string(json, "alloca"));
            break;

        case KEFIR_AST_BUILTIN_ALLOCA_WITH_ALIGN:
            REQUIRE_OK(kefir_json_output_string(json, "alloca_with_align"));
            break;

        case KEFIR_AST_BUILTIN_ALLOCA_WITH_ALIGN_AND_MAX:
            REQUIRE_OK(kefir_json_output_string(json, "alloca_with_align_and_max"));
            break;

        case KEFIR_AST_BUILTIN_OFFSETOF:
            REQUIRE_OK(kefir_json_output_string(json, "offset"));
            break;

        case KEFIR_AST_BUILTIN_TYPES_COMPATIBLE:
            REQUIRE_OK(kefir_json_output_string(json, "types_compatible"));
            break;

        case KEFIR_AST_BUILTIN_CHOOSE_EXPRESSION:
            REQUIRE_OK(kefir_json_output_string(json, "choose_expression"));
            break;

        case KEFIR_AST_BUILTIN_CONSTANT:
            REQUIRE_OK(kefir_json_output_string(json, "constant_p"));
            break;

        case KEFIR_AST_BUILTIN_CLASSIFY_TYPE:
            REQUIRE_OK(kefir_json_output_string(json, "classify_type"));
            break;

        case KEFIR_AST_BUILTIN_INFINITY_FLOAT32:
            REQUIRE_OK(kefir_json_output_string(json, "inf_float32"));
            break;

        case KEFIR_AST_BUILTIN_INFINITY_FLOAT64:
            REQUIRE_OK(kefir_json_output_string(json, "inf_float64"));
            break;

        case KEFIR_AST_BUILTIN_INFINITY_LONG_DOUBLE:
            REQUIRE_OK(kefir_json_output_string(json, "inf_long_double"));
            break;

        case KEFIR_AST_BUILTIN_NAN_FLOAT32:
            REQUIRE_OK(kefir_json_output_string(json, "nan_float32"));
            break;

        case KEFIR_AST_BUILTIN_NAN_FLOAT64:
            REQUIRE_OK(kefir_json_output_string(json, "nan_float64"));
            break;

        case KEFIR_AST_BUILTIN_NAN_LONG_DOUBLE:
            REQUIRE_OK(kefir_json_output_string(json, "nan_long_double"));
            break;

        case KEFIR_AST_BUILTIN_ADD_OVERFLOW:
            REQUIRE_OK(kefir_json_output_string(json, "add_overflow"));
            break;

        case KEFIR_AST_BUILTIN_SUB_OVERFLOW:
            REQUIRE_OK(kefir_json_output_string(json, "sub_overflow"));
            break;

        case KEFIR_AST_BUILTIN_MUL_OVERFLOW:
            REQUIRE_OK(kefir_json_output_string(json, "mul_overflow"));
            break;

        case KEFIR_AST_BUILTIN_FFSG:
            REQUIRE_OK(kefir_json_output_string(json, "ffsg"));
            break;

        case KEFIR_AST_BUILTIN_CTZG:
            REQUIRE_OK(kefir_json_output_string(json, "ctzg"));
            break;

        case KEFIR_AST_BUILTIN_CLZG:
            REQUIRE_OK(kefir_json_output_string(json, "clzg"));
            break;

        case KEFIR_AST_BUILTIN_CLRSBG:
            REQUIRE_OK(kefir_json_output_string(json, "clrsbg"));
            break;

        case KEFIR_AST_BUILTIN_POPCOUNTG:
            REQUIRE_OK(kefir_json_output_string(json, "popcountg"));
            break;
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "arguments"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (const struct kefir_list_entry *iter = kefir_list_head(&node->arguments); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_node_base *, arg, iter->value);
        REQUIRE_OK(kefir_ast_format(json, arg, param->display_source_location));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_struct_member(const struct kefir_ast_visitor *visitor,
                                          const struct kefir_ast_struct_member *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST struct member node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "struct_member"));
    REQUIRE_OK(kefir_json_output_object_key(json, "structure"));
    REQUIRE_OK(kefir_ast_format(json, node->structure, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "member"));
    REQUIRE_OK(kefir_json_output_string(json, node->member));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_indirect_struct_member(const struct kefir_ast_visitor *visitor,
                                                   const struct kefir_ast_struct_member *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST struct member node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "indirect_struct_member"));
    REQUIRE_OK(kefir_json_output_object_key(json, "structure"));
    REQUIRE_OK(kefir_ast_format(json, node->structure, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "member"));
    REQUIRE_OK(kefir_json_output_string(json, node->member));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_unary_operation(const struct kefir_ast_visitor *visitor,
                                            const struct kefir_ast_unary_operation *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST unary operation node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "unary_operation"));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    switch (node->type) {
        case KEFIR_AST_OPERATION_PLUS:
            REQUIRE_OK(kefir_json_output_string(json, "plus"));
            break;

        case KEFIR_AST_OPERATION_NEGATE:
            REQUIRE_OK(kefir_json_output_string(json, "negate"));
            break;

        case KEFIR_AST_OPERATION_INVERT:
            REQUIRE_OK(kefir_json_output_string(json, "invert"));
            break;

        case KEFIR_AST_OPERATION_LOGICAL_NEGATE:
            REQUIRE_OK(kefir_json_output_string(json, "logical_negate"));
            break;

        case KEFIR_AST_OPERATION_POSTFIX_INCREMENT:
            REQUIRE_OK(kefir_json_output_string(json, "postfix_increment"));
            break;

        case KEFIR_AST_OPERATION_POSTFIX_DECREMENT:
            REQUIRE_OK(kefir_json_output_string(json, "postfix_decrement"));
            break;

        case KEFIR_AST_OPERATION_PREFIX_INCREMENT:
            REQUIRE_OK(kefir_json_output_string(json, "prefix_increment"));
            break;

        case KEFIR_AST_OPERATION_PREFIX_DECREMENT:
            REQUIRE_OK(kefir_json_output_string(json, "prefix_decrement"));
            break;

        case KEFIR_AST_OPERATION_ADDRESS:
            REQUIRE_OK(kefir_json_output_string(json, "address"));
            break;

        case KEFIR_AST_OPERATION_INDIRECTION:
            REQUIRE_OK(kefir_json_output_string(json, "indirection"));
            break;

        case KEFIR_AST_OPERATION_SIZEOF:
            REQUIRE_OK(kefir_json_output_string(json, "sizeof"));
            break;

        case KEFIR_AST_OPERATION_ALIGNOF:
            REQUIRE_OK(kefir_json_output_string(json, "alignof"));
            break;
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "argument"));
    REQUIRE_OK(kefir_ast_format(json, node->arg, param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_cast_operator(const struct kefir_ast_visitor *visitor,
                                          const struct kefir_ast_cast_operator *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST cast operator node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "cast_operator"));
    REQUIRE_OK(kefir_json_output_object_key(json, "type_name"));
    REQUIRE_OK(kefir_ast_format(json, KEFIR_AST_NODE_BASE(node->type_name), param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "expression"));
    REQUIRE_OK(kefir_ast_format(json, node->expr, param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_binary_operation(const struct kefir_ast_visitor *visitor,
                                             const struct kefir_ast_binary_operation *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST binary operation node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "binary_operation"));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    switch (node->type) {
        case KEFIR_AST_OPERATION_ADD:
            REQUIRE_OK(kefir_json_output_string(json, "add"));
            break;

        case KEFIR_AST_OPERATION_SUBTRACT:
            REQUIRE_OK(kefir_json_output_string(json, "subtract"));
            break;

        case KEFIR_AST_OPERATION_MULTIPLY:
            REQUIRE_OK(kefir_json_output_string(json, "multiply"));
            break;

        case KEFIR_AST_OPERATION_DIVIDE:
            REQUIRE_OK(kefir_json_output_string(json, "divide"));
            break;

        case KEFIR_AST_OPERATION_MODULO:
            REQUIRE_OK(kefir_json_output_string(json, "modulo"));
            break;

        case KEFIR_AST_OPERATION_SHIFT_LEFT:
            REQUIRE_OK(kefir_json_output_string(json, "shift_left"));
            break;

        case KEFIR_AST_OPERATION_SHIFT_RIGHT:
            REQUIRE_OK(kefir_json_output_string(json, "shift_right"));
            break;

        case KEFIR_AST_OPERATION_LESS:
            REQUIRE_OK(kefir_json_output_string(json, "less"));
            break;

        case KEFIR_AST_OPERATION_LESS_EQUAL:
            REQUIRE_OK(kefir_json_output_string(json, "less_equal"));
            break;

        case KEFIR_AST_OPERATION_GREATER:
            REQUIRE_OK(kefir_json_output_string(json, "greater"));
            break;

        case KEFIR_AST_OPERATION_GREATER_EQUAL:
            REQUIRE_OK(kefir_json_output_string(json, "greater_equal"));
            break;

        case KEFIR_AST_OPERATION_EQUAL:
            REQUIRE_OK(kefir_json_output_string(json, "equal"));
            break;

        case KEFIR_AST_OPERATION_NOT_EQUAL:
            REQUIRE_OK(kefir_json_output_string(json, "not_equal"));
            break;

        case KEFIR_AST_OPERATION_BITWISE_AND:
            REQUIRE_OK(kefir_json_output_string(json, "bitwise_and"));
            break;

        case KEFIR_AST_OPERATION_BITWISE_OR:
            REQUIRE_OK(kefir_json_output_string(json, "bitwise_or"));
            break;

        case KEFIR_AST_OPERATION_BITWISE_XOR:
            REQUIRE_OK(kefir_json_output_string(json, "bitwise_xor"));
            break;

        case KEFIR_AST_OPERATION_LOGICAL_AND:
            REQUIRE_OK(kefir_json_output_string(json, "logical_and"));
            break;

        case KEFIR_AST_OPERATION_LOGICAL_OR:
            REQUIRE_OK(kefir_json_output_string(json, "logical_or"));
            break;
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "left"));
    REQUIRE_OK(kefir_ast_format(json, node->arg1, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "right"));
    REQUIRE_OK(kefir_ast_format(json, node->arg2, param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_conditional_operator(const struct kefir_ast_visitor *visitor,
                                                 const struct kefir_ast_conditional_operator *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST conditional operation node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "conditional_operator"));
    REQUIRE_OK(kefir_json_output_object_key(json, "condition"));
    REQUIRE_OK(kefir_ast_format(json, node->condition, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "thenBranch"));
    if (node->expr1 != NULL) {
        REQUIRE_OK(kefir_ast_format(json, node->expr1, param->display_source_location));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "elseBranch"));
    REQUIRE_OK(kefir_ast_format(json, node->expr2, param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_assignment_operator(const struct kefir_ast_visitor *visitor,
                                                const struct kefir_ast_assignment_operator *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST assignment operator node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "assignment_operator"));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    switch (node->operation) {
        case KEFIR_AST_ASSIGNMENT_SIMPLE:
            REQUIRE_OK(kefir_json_output_string(json, "simple"));
            break;

        case KEFIR_AST_ASSIGNMENT_MULTIPLY:
            REQUIRE_OK(kefir_json_output_string(json, "multiply"));
            break;

        case KEFIR_AST_ASSIGNMENT_DIVIDE:
            REQUIRE_OK(kefir_json_output_string(json, "divide"));
            break;

        case KEFIR_AST_ASSIGNMENT_MODULO:
            REQUIRE_OK(kefir_json_output_string(json, "modulo"));
            break;

        case KEFIR_AST_ASSIGNMENT_ADD:
            REQUIRE_OK(kefir_json_output_string(json, "add"));
            break;

        case KEFIR_AST_ASSIGNMENT_SUBTRACT:
            REQUIRE_OK(kefir_json_output_string(json, "subtract"));
            break;

        case KEFIR_AST_ASSIGNMENT_SHIFT_LEFT:
            REQUIRE_OK(kefir_json_output_string(json, "shift_left"));
            break;

        case KEFIR_AST_ASSIGNMENT_SHIFT_RIGHT:
            REQUIRE_OK(kefir_json_output_string(json, "shift_right"));
            break;

        case KEFIR_AST_ASSIGNMENT_BITWISE_AND:
            REQUIRE_OK(kefir_json_output_string(json, "bitwise_and"));
            break;

        case KEFIR_AST_ASSIGNMENT_BITWISE_OR:
            REQUIRE_OK(kefir_json_output_string(json, "bitwise_or"));
            break;

        case KEFIR_AST_ASSIGNMENT_BITWISE_XOR:
            REQUIRE_OK(kefir_json_output_string(json, "bitwise_xor"));
            break;
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "target"));
    REQUIRE_OK(kefir_ast_format(json, node->target, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "value"));
    REQUIRE_OK(kefir_ast_format(json, node->value, param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_comma_operator(const struct kefir_ast_visitor *visitor,
                                           const struct kefir_ast_comma_operator *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST assignment operator node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "comma_operator"));
    REQUIRE_OK(kefir_json_output_object_key(json, "expressions"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (const struct kefir_list_entry *iter = kefir_list_head(&node->expressions); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_node_base *, expr, iter->value);
        REQUIRE_OK(kefir_ast_format(json, expr, param->display_source_location));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_type_name(const struct kefir_ast_visitor *visitor, const struct kefir_ast_type_name *node,
                                      void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type name node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "type_name"));
    REQUIRE_OK(kefir_json_output_object_key(json, "specifiers"));
    REQUIRE_OK(
        kefir_ast_format_declarator_specifier_list(json, &node->type_decl.specifiers, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "declarator"));
    REQUIRE_OK(kefir_ast_format_declarator(json, node->type_decl.declarator, param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_declaration(const struct kefir_ast_visitor *visitor,
                                        const struct kefir_ast_declaration *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST declaration node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "declaration"));
    REQUIRE_OK(kefir_json_output_object_key(json, "specifiers"));
    REQUIRE_OK(kefir_ast_format_declarator_specifier_list(json, &node->specifiers, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "init_declarators"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (const struct kefir_list_entry *iter = kefir_list_head(&node->init_declarators); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_node_base *, decl, iter->value);
        REQUIRE_OK(kefir_ast_format(json, decl, param->display_source_location));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_attribute_declaration(const struct kefir_ast_visitor *visitor,
                                                  const struct kefir_ast_attribute_declaration *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST declaration node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "attribute_declaration"));
    REQUIRE_OK(format_attributes(json, &node->attributes, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_init_declarator(const struct kefir_ast_visitor *visitor,
                                            const struct kefir_ast_init_declarator *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST init declarator node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "init_declarator"));
    REQUIRE_OK(kefir_json_output_object_key(json, "declarator"));
    REQUIRE_OK(kefir_ast_format_declarator(json, node->declarator, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "initializer"));
    if (node->initializer != NULL) {
        REQUIRE_OK(kefir_ast_format_initializer(json, node->initializer, param->display_source_location));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_static_assertion(const struct kefir_ast_visitor *visitor,
                                             const struct kefir_ast_static_assertion *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST static assertion node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "static_assertion"));
    REQUIRE_OK(kefir_json_output_object_key(json, "assertion"));
    REQUIRE_OK(kefir_ast_format(json, node->condition, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "string_literal"));
    if (node->string != NULL) {
        REQUIRE_OK(kefir_ast_format(json, KEFIR_AST_NODE_BASE(node->string), param->display_source_location));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_generic_selection(const struct kefir_ast_visitor *visitor,
                                              const struct kefir_ast_generic_selection *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST generic selection node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "generic_selection"));
    REQUIRE_OK(kefir_json_output_object_key(json, "control_expression"));
    REQUIRE_OK(kefir_ast_format(json, node->control, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "associations"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (const struct kefir_list_entry *iter = kefir_list_head(&node->associations); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_generic_selection_assoc *, assoc, iter->value);

        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "type_name"));
        REQUIRE_OK(kefir_ast_format(json, KEFIR_AST_NODE_BASE(assoc->type_name), param->display_source_location));
        REQUIRE_OK(kefir_json_output_object_key(json, "expression"));
        REQUIRE_OK(kefir_ast_format(json, assoc->expr, param->display_source_location));
        REQUIRE_OK(kefir_json_output_object_end(json));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "default_association"));
    if (node->default_assoc != NULL) {
        REQUIRE_OK(kefir_ast_format(json, node->default_assoc, param->display_source_location));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_compound_literal(const struct kefir_ast_visitor *visitor,
                                             const struct kefir_ast_compound_literal *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST compound literal node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "compound_literal"));
    REQUIRE_OK(kefir_json_output_object_key(json, "type"));
    REQUIRE_OK(kefir_ast_format(json, KEFIR_AST_NODE_BASE(node->type_name), param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "initializer"));
    REQUIRE_OK(kefir_ast_format_initializer(json, node->initializer, param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_expression_statement(const struct kefir_ast_visitor *visitor,
                                                 const struct kefir_ast_expression_statement *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST expression statement node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "expression_statement"));
    REQUIRE_OK(kefir_json_output_object_key(json, "expression"));
    if (node->expression != NULL) {
        REQUIRE_OK(kefir_ast_format(json, node->expression, param->display_source_location));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    REQUIRE_OK(format_attributes(json, &node->attributes, param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_compound_statement(const struct kefir_ast_visitor *visitor,
                                               const struct kefir_ast_compound_statement *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST compound statement node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "compound_statement"));
    REQUIRE_OK(kefir_json_output_object_key(json, "items"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (const struct kefir_list_entry *iter = kefir_list_head(&node->block_items); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_node_base *, item, iter->value);
        REQUIRE_OK(kefir_ast_format(json, item, param->display_source_location));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(format_attributes(json, &node->attributes, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_labeled_statement(const struct kefir_ast_visitor *visitor,
                                              const struct kefir_ast_labeled_statement *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST labeled statement node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "labeled_statement"));
    REQUIRE_OK(kefir_json_output_object_key(json, "label"));
    REQUIRE_OK(kefir_json_output_string(json, node->label));
    REQUIRE_OK(kefir_json_output_object_key(json, "statement"));
    if (node->statement != NULL) {
        REQUIRE_OK(kefir_ast_format(json, node->statement, param->display_source_location));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(format_attributes(json, &node->attributes, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_case_statement(const struct kefir_ast_visitor *visitor,
                                           const struct kefir_ast_case_statement *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST case statement node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "case_statement"));
    REQUIRE_OK(kefir_json_output_object_key(json, "expression"));
    if (node->expression != NULL) {
        REQUIRE_OK(kefir_ast_format(json, node->expression, param->display_source_location));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "range_end_expression"));
    if (node->range_end_expression != NULL) {
        REQUIRE_OK(kefir_ast_format(json, node->range_end_expression, param->display_source_location));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "statement"));
    if (node->statement != NULL) {
        REQUIRE_OK(kefir_ast_format(json, node->statement, param->display_source_location));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(format_attributes(json, &node->attributes, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_if_statement(const struct kefir_ast_visitor *visitor,
                                         const struct kefir_ast_conditional_statement *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST conditional statement node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "if_statement"));
    REQUIRE_OK(kefir_json_output_object_key(json, "expression"));
    REQUIRE_OK(kefir_ast_format(json, node->condition, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "thenBranch"));
    REQUIRE_OK(kefir_ast_format(json, node->thenBranch, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "elseBranch"));
    if (node->elseBranch != NULL) {
        REQUIRE_OK(kefir_ast_format(json, node->elseBranch, param->display_source_location));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(format_attributes(json, &node->attributes, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_switch_statement(const struct kefir_ast_visitor *visitor,
                                             const struct kefir_ast_switch_statement *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST switch statement node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "switch_statement"));
    REQUIRE_OK(kefir_json_output_object_key(json, "expression"));
    REQUIRE_OK(kefir_ast_format(json, node->expression, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "statement"));
    REQUIRE_OK(kefir_ast_format(json, node->statement, param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(format_attributes(json, &node->attributes, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_while_statement(const struct kefir_ast_visitor *visitor,
                                            const struct kefir_ast_while_statement *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST while statement node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "while_statement"));
    REQUIRE_OK(kefir_json_output_object_key(json, "controlling_expression"));
    REQUIRE_OK(kefir_ast_format(json, node->controlling_expr, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "body"));
    REQUIRE_OK(kefir_ast_format(json, node->body, param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(format_attributes(json, &node->attributes, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_do_while_statement(const struct kefir_ast_visitor *visitor,
                                               const struct kefir_ast_do_while_statement *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST do while statement node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "do_while_statement"));
    REQUIRE_OK(kefir_json_output_object_key(json, "controlling_expression"));
    REQUIRE_OK(kefir_ast_format(json, node->controlling_expr, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "body"));
    REQUIRE_OK(kefir_ast_format(json, node->body, param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(format_attributes(json, &node->attributes, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_for_statement(const struct kefir_ast_visitor *visitor,
                                          const struct kefir_ast_for_statement *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST for statement node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "for_statement"));

    REQUIRE_OK(kefir_json_output_object_key(json, "init"));
    if (node->init != NULL) {
        REQUIRE_OK(kefir_ast_format(json, node->init, param->display_source_location));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }

    REQUIRE_OK(kefir_json_output_object_key(json, "controlling_expression"));
    if (node->controlling_expr != NULL) {
        REQUIRE_OK(kefir_ast_format(json, node->controlling_expr, param->display_source_location));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }

    REQUIRE_OK(kefir_json_output_object_key(json, "tail"));
    if (node->tail != NULL) {
        REQUIRE_OK(kefir_ast_format(json, node->tail, param->display_source_location));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }

    REQUIRE_OK(kefir_json_output_object_key(json, "body"));
    REQUIRE_OK(kefir_ast_format(json, node->body, param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(format_attributes(json, &node->attributes, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_return_statement(const struct kefir_ast_visitor *visitor,
                                             const struct kefir_ast_return_statement *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST return statement node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "return_statement"));
    REQUIRE_OK(kefir_json_output_object_key(json, "expression"));
    if (node->expression != NULL) {
        REQUIRE_OK(kefir_ast_format(json, node->expression, param->display_source_location));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    REQUIRE_OK(format_attributes(json, &node->attributes, param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_goto_statement(const struct kefir_ast_visitor *visitor,
                                           const struct kefir_ast_goto_statement *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST goto statement node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "goto_statement"));
    REQUIRE_OK(kefir_json_output_object_key(json, "identifier"));
    REQUIRE_OK(kefir_json_output_string(json, node->identifier));
    REQUIRE_OK(format_attributes(json, &node->attributes, param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_continue_statement(const struct kefir_ast_visitor *visitor,
                                               const struct kefir_ast_continue_statement *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST continue statement node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "continue_statement"));
    REQUIRE_OK(format_attributes(json, &node->attributes, param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_break_statement(const struct kefir_ast_visitor *visitor,
                                            const struct kefir_ast_break_statement *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST break statement node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "break_statement"));
    REQUIRE_OK(format_attributes(json, &node->attributes, param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_function_definitions(const struct kefir_ast_visitor *visitor,
                                                 const struct kefir_ast_function_definition *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST function definition node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "function_definition"));
    REQUIRE_OK(kefir_json_output_object_key(json, "specifiers"));
    REQUIRE_OK(kefir_ast_format_declarator_specifier_list(json, &node->specifiers, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "declarator"));
    REQUIRE_OK(kefir_ast_format_declarator(json, node->declarator, param->display_source_location));
    REQUIRE_OK(kefir_json_output_object_key(json, "declarations"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (const struct kefir_list_entry *iter = kefir_list_head(&node->declarations); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_node_base *, declaration, iter->value);
        REQUIRE_OK(kefir_ast_format(json, declaration, param->display_source_location));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "body"));
    REQUIRE_OK(kefir_ast_format(json, KEFIR_AST_NODE_BASE(node->body), param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_translation_unit(const struct kefir_ast_visitor *visitor,
                                             const struct kefir_ast_translation_unit *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST translation unit node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "translation_unit"));
    REQUIRE_OK(kefir_json_output_object_key(json, "external_declarations"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (const struct kefir_list_entry *iter = kefir_list_head(&node->external_definitions); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_node_base *, ext_def, iter->value);
        REQUIRE_OK(kefir_ast_format(json, ext_def, param->display_source_location));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_label_address(const struct kefir_ast_visitor *visitor,
                                          const struct kefir_ast_label_address *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST label address node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "label_address"));
    REQUIRE_OK(kefir_json_output_object_key(json, "label"));
    REQUIRE_OK(kefir_json_output_string(json, node->label));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_goto_address_statement(const struct kefir_ast_visitor *visitor,
                                                   const struct kefir_ast_goto_statement *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST goto address statement node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "goto_address_statement"));
    REQUIRE_OK(kefir_json_output_object_key(json, "target"));
    REQUIRE_OK(kefir_ast_format(json, node->target, param->display_source_location));
    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_statement_expression(const struct kefir_ast_visitor *visitor,
                                                 const struct kefir_ast_statement_expression *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST statement expression node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "statement_expression"));
    REQUIRE_OK(kefir_json_output_object_key(json, "items"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (const struct kefir_list_entry *iter = kefir_list_head(&node->block_items); iter != NULL;
         kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_node_base *, item, iter->value);
        REQUIRE_OK(kefir_ast_format(json, item, param->display_source_location));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));

    REQUIRE_OK(kefir_json_output_object_key(json, "result"));
    if (node->result != NULL) {
        REQUIRE_OK(kefir_ast_format(json, node->result, param->display_source_location));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    REQUIRE_OK(format_attributes(json, &node->attributes, param->display_source_location));

    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_attribute_list(const struct kefir_ast_visitor *visitor,
                                           const struct kefir_ast_attribute_list *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST attrbite list node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "attribute_list"));
    REQUIRE_OK(kefir_json_output_object_key(json, "attributes"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (const struct kefir_list_entry *iter = kefir_list_head(&node->list); iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_attribute *, attr, iter->value);
        REQUIRE_OK(kefir_json_output_object_begin(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "prefix"));
        if (attr->prefix != NULL) {
            REQUIRE_OK(kefir_json_output_string(json, attr->prefix));
        } else {
            REQUIRE_OK(kefir_json_output_null(json));
        }
        REQUIRE_OK(kefir_json_output_object_key(json, "name"));
        REQUIRE_OK(kefir_json_output_string(json, attr->name));
        REQUIRE_OK(kefir_json_output_object_key(json, "parameters"));
        REQUIRE_OK(kefir_json_output_array_begin(json));
        for (const struct kefir_list_entry *iter2 = kefir_list_head(&attr->parameters); iter2 != NULL;
             kefir_list_next(&iter2)) {
            ASSIGN_DECL_CAST(struct kefir_ast_node_base *, attr_param, iter2->value);
            REQUIRE_OK(kefir_ast_format(json, attr_param, param->display_source_location));
        }
        REQUIRE_OK(kefir_json_output_array_end(json));
        REQUIRE_OK(kefir_json_output_object_key(json, "unstructured_parameters"));
        REQUIRE_OK(kefir_json_output_array_begin(json));
        for (kefir_size_t i = 0; i < kefir_token_buffer_length(&attr->unstructured_parameters); i++) {
            REQUIRE_OK(kefir_token_format(json, kefir_token_buffer_at(&attr->unstructured_parameters, i),
                                          param->display_source_location));
        }
        REQUIRE_OK(kefir_json_output_array_end(json));
        REQUIRE_OK(kefir_json_output_object_end(json));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));

    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t format_inline_assembly_param(struct kefir_json_output *json,
                                                   const struct kefir_ast_inline_assembly_parameter *parameter,
                                                   kefir_bool_t display_source_location) {
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "symbolicName"));
    if (parameter->parameter_name != NULL) {
        REQUIRE_OK(kefir_json_output_string(json, parameter->parameter_name));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "constraint"));
    REQUIRE_OK(kefir_json_output_string(json, parameter->constraint));
    REQUIRE_OK(kefir_json_output_object_key(json, "explicit_register"));
    if (parameter->explicit_register != NULL) {
        REQUIRE_OK(kefir_json_output_string(json, parameter->explicit_register));
    } else {
        REQUIRE_OK(kefir_json_output_null(json));
    }
    REQUIRE_OK(kefir_json_output_object_key(json, "parameter"));
    REQUIRE_OK(kefir_ast_format(json, parameter->parameter, display_source_location));
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

static kefir_result_t visit_inline_assembly(const struct kefir_ast_visitor *visitor,
                                            const struct kefir_ast_inline_assembly *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST inline assembly node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct visitor_param *, param, payload);
    struct kefir_json_output *json = param->json;

    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "class"));
    REQUIRE_OK(kefir_json_output_string(json, "inline_assembly"));
    REQUIRE_OK(kefir_json_output_object_key(json, "template"));
    REQUIRE_OK(kefir_json_output_string(json, node->asm_template));
    REQUIRE_OK(kefir_json_output_object_key(json, "qualifiers"));
    REQUIRE_OK(kefir_json_output_object_begin(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "volatile"));
    REQUIRE_OK(kefir_json_output_boolean(json, node->qualifiers.volatile_qualifier));
    REQUIRE_OK(kefir_json_output_object_key(json, "inline"));
    REQUIRE_OK(kefir_json_output_boolean(json, node->qualifiers.inline_qualifier));
    REQUIRE_OK(kefir_json_output_object_key(json, "goto"));
    REQUIRE_OK(kefir_json_output_boolean(json, node->qualifiers.goto_qualifier));
    REQUIRE_OK(kefir_json_output_object_end(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "outputs"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (const struct kefir_list_entry *iter = kefir_list_head(&node->outputs); iter != NULL; kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(const struct kefir_ast_inline_assembly_parameter *, parameter, iter->value);
        REQUIRE_OK(format_inline_assembly_param(json, parameter, param->display_source_location));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "inputs"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (const struct kefir_list_entry *iter = kefir_list_head(&node->inputs); iter != NULL; kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(const struct kefir_ast_inline_assembly_parameter *, parameter, iter->value);
        REQUIRE_OK(format_inline_assembly_param(json, parameter, param->display_source_location));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "clobbers"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (const struct kefir_list_entry *iter = kefir_list_head(&node->clobbers); iter != NULL; kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(const char *, clobber, iter->value);
        REQUIRE_OK(kefir_json_output_string(json, clobber));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));
    REQUIRE_OK(kefir_json_output_object_key(json, "jump_labels"));
    REQUIRE_OK(kefir_json_output_array_begin(json));
    for (const struct kefir_list_entry *iter = kefir_list_head(&node->jump_labels); iter != NULL;
         kefir_list_next(&iter)) {

        ASSIGN_DECL_CAST(const char *, jump_label, iter->value);
        REQUIRE_OK(kefir_json_output_string(json, jump_label));
    }
    REQUIRE_OK(kefir_json_output_array_end(json));

    if (param->display_source_location) {
        REQUIRE_OK(format_source_location(json, KEFIR_AST_NODE_BASE(node)));
    }
    REQUIRE_OK(kefir_json_output_object_end(json));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_format(struct kefir_json_output *json, const struct kefir_ast_node_base *node,
                                kefir_bool_t display_source_location) {
    REQUIRE(json != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid JSON output"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));

    struct visitor_param param = {.json = json, .display_source_location = display_source_location};

    struct kefir_ast_visitor visitor;
    REQUIRE_OK(kefir_ast_visitor_init(&visitor, visit_not_impl));
    visitor.identifier = visit_identifier;
    visitor.constant = visit_constant;
    visitor.string_literal = visit_string_literal;
    visitor.array_subscript = visit_array_subscript;
    visitor.function_call = visit_function_call;
    visitor.struct_member = visit_struct_member;
    visitor.struct_indirect_member = visit_indirect_struct_member;
    visitor.unary_operation = visit_unary_operation;
    visitor.cast_operator = visit_cast_operator;
    visitor.binary_operation = visit_binary_operation;
    visitor.conditional_operator = visit_conditional_operator;
    visitor.assignment_operator = visit_assignment_operator;
    visitor.comma_operator = visit_comma_operator;
    visitor.type_name = visit_type_name;
    visitor.declaration = visit_declaration;
    visitor.attribute_declaration = visit_attribute_declaration;
    visitor.init_declarator = visit_init_declarator;
    visitor.static_assertion = visit_static_assertion;
    visitor.generic_selection = visit_generic_selection;
    visitor.compound_literal = visit_compound_literal;
    visitor.expression_statement = visit_expression_statement;
    visitor.compound_statement = visit_compound_statement;
    visitor.labeled_statement = visit_labeled_statement;
    visitor.case_statement = visit_case_statement;
    visitor.conditional_statement = visit_if_statement;
    visitor.switch_statement = visit_switch_statement;
    visitor.while_statement = visit_while_statement;
    visitor.do_while_statement = visit_do_while_statement;
    visitor.for_statement = visit_for_statement;
    visitor.return_statement = visit_return_statement;
    visitor.goto_statement = visit_goto_statement;
    visitor.continue_statement = visit_continue_statement;
    visitor.break_statement = visit_break_statement;
    visitor.function_definition = visit_function_definitions;
    visitor.translation_unit = visit_translation_unit;
    visitor.builtin = visit_builtin;
    visitor.label_address = visit_label_address;
    visitor.goto_address_statement = visit_goto_address_statement;
    visitor.statement_expression = visit_statement_expression;
    visitor.attribute_list = visit_attribute_list;
    visitor.inline_assembly = visit_inline_assembly;
    REQUIRE_OK(node->klass->visit(node, &visitor, &param));
    return KEFIR_OK;
}
