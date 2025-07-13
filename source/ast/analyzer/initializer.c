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

#include "kefir/ast/analyzer/initializer.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/ast/analyzer/type_traversal.h"
#include "kefir/ast/type_conv.h"
#include "kefir/ast/initializer_traversal.h"
#include "kefir/ast/downcast.h"
#include "kefir/core/hashtreeset.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"
#include <stdio.h>

static kefir_result_t preanalyze_initializer(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                             const struct kefir_ast_initializer *initializer,
                                             struct kefir_ast_initializer_properties *properties) {
    if (initializer->type == KEFIR_AST_INITIALIZER_EXPRESSION) {
        REQUIRE_OK(kefir_ast_analyze_node(mem, context, initializer->expression));
        if (properties != NULL && !initializer->expression->properties.expression_props.constant_expression) {
            struct kefir_ast_compound_literal *compound_literal;
            kefir_result_t res = kefir_ast_downcast_compound_literal(initializer->expression, &compound_literal, false);
            if (res != KEFIR_NO_MATCH) {
                REQUIRE_OK(res);
                struct kefir_ast_initializer_properties initializer_properties;
                REQUIRE_OK(kefir_ast_analyze_initializer(mem, context,
                                                         compound_literal->type_name->base.properties.type,
                                                         compound_literal->initializer, &initializer_properties));
                properties->constant = properties->constant && initializer_properties.constant;
            } else {
                properties->constant = false;
            }
        }
    } else {
        for (const struct kefir_list_entry *iter = kefir_list_head(&initializer->list.initializers); iter != NULL;
             kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(struct kefir_ast_initializer_list_entry *, entry, iter->value);
            if (entry->designation != NULL && entry->designator == NULL) {
                REQUIRE_OK(
                    kefir_ast_evaluate_initializer_designation(mem, context, entry->designation, &entry->designator));
            }

            REQUIRE_OK(preanalyze_initializer(mem, context, entry->value, properties));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t analyze_scalar(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                     const struct kefir_ast_type *type, const struct kefir_ast_initializer *initializer,
                                     struct kefir_ast_initializer_properties *properties) {
    struct kefir_ast_node_base *expr = kefir_ast_initializer_head(initializer);
    if (expr != NULL) {
        kefir_result_t res;
        REQUIRE_MATCH_OK(&res, kefir_ast_node_assignable(mem, context, expr, type),
                         KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &expr->source_location,
                                                "Expression shall be assignable to scalar type"));
        REQUIRE_OK(res);
    } else {
        return KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &initializer->source_location,
                                      "Scalar initializer list cannot be empty");
    }
    if (properties != NULL) {
        properties->type = type;
        if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(kefir_ast_unqualified_type(type))) {
            properties->contains_long_double = true;
        }
    }
    return KEFIR_OK;
}

static kefir_bool_t is_char_array(const struct kefir_ast_type *type, void *payload) {
    UNUSED(payload);
    return type->tag == KEFIR_AST_TYPE_ARRAY &&
           KEFIR_AST_TYPE_IS_CHARACTER(kefir_ast_unqualified_type(type->array_type.element_type));
}

static kefir_bool_t is_array_of(const struct kefir_ast_type *type, void *payload) {
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid payload"));
    ASSIGN_DECL_CAST(const struct kefir_ast_type *, expected_type, payload);
    return type->tag == KEFIR_AST_TYPE_ARRAY &&
           KEFIR_AST_TYPE_SAME(kefir_ast_unqualified_type(type->array_type.element_type), expected_type);
}

static kefir_result_t string_literal_stop_fn(struct kefir_ast_node_base *node,
                                             const struct kefir_ast_type_traits *type_traits,
                                             kefir_bool_t (**stop_fn)(const struct kefir_ast_type *, void *),
                                             void **stop_payload) {
    switch (node->properties.expression_props.string_literal.type) {
        case KEFIR_AST_STRING_LITERAL_MULTIBYTE:
        case KEFIR_AST_STRING_LITERAL_UNICODE8:
            *stop_fn = is_char_array;
            *stop_payload = NULL;
            break;

        case KEFIR_AST_STRING_LITERAL_UNICODE16:
            *stop_fn = is_array_of;
            *stop_payload = (void *) type_traits->unicode16_char_type;
            break;

        case KEFIR_AST_STRING_LITERAL_UNICODE32:
            *stop_fn = is_array_of;
            *stop_payload = (void *) type_traits->unicode32_char_type;
            break;

        case KEFIR_AST_STRING_LITERAL_WIDE:
            *stop_fn = is_array_of;
            *stop_payload = (void *) type_traits->wide_char_type;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected string literal type");
    }
    return KEFIR_OK;
}

static kefir_result_t traverse_aggregate_union_string_literal(struct kefir_mem *mem,
                                                              const struct kefir_ast_context *context,
                                                              struct kefir_ast_type_traversal *traversal,
                                                              struct kefir_ast_initializer_list_entry *entry) {
    const struct kefir_ast_type *type = NULL;
    kefir_bool_t (*stop_fn)(const struct kefir_ast_type *, void *) = NULL;
    void *stop_payload = NULL;
    REQUIRE_OK(string_literal_stop_fn(entry->value->expression, context->type_traits, &stop_fn, &stop_payload));

    REQUIRE_OK(kefir_ast_type_traversal_next_recursive2(mem, traversal, stop_fn, stop_payload, &type, NULL));
    if (!stop_fn(type, stop_payload)) {
        kefir_result_t res;
        REQUIRE_MATCH_OK(
            &res, kefir_ast_node_assignable(mem, context, entry->value->expression, kefir_ast_unqualified_type(type)),
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &entry->value->expression->source_location,
                                   "Expression value shall be assignable to field type"));
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

struct traverse_aggregate_union_param {
    struct kefir_mem *mem;
    const struct kefir_ast_context *context;
    const struct kefir_ast_initializer *initializer;
    struct kefir_ast_type_traversal *traversal;
    struct kefir_ast_initializer_list_entry *entry;
};

static kefir_result_t traverse_aggregate_union_impl(struct kefir_ast_designator *entry_designator, void *payload) {
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid designator unroll payload"));
    ASSIGN_DECL_CAST(struct traverse_aggregate_union_param *, param, payload);
    struct kefir_mem *mem = param->mem;
    const struct kefir_ast_context *context = param->context;
    struct kefir_ast_type_traversal *traversal = param->traversal;
    struct kefir_ast_initializer_list_entry *entry = param->entry;

    if (entry_designator != NULL) {
        REQUIRE_OK(kefir_ast_type_traversal_navigate(mem, traversal, entry_designator));
    } else if (kefir_ast_type_traversal_empty(traversal)) {
        return KEFIR_YIELD;
    }

    if (entry->value->type == KEFIR_AST_INITIALIZER_LIST) {
        const struct kefir_ast_type *type = NULL;
        REQUIRE_OK(kefir_ast_type_traversal_next(mem, traversal, &type, NULL));
        REQUIRE_OK(kefir_ast_analyze_initializer(mem, context, type, entry->value, NULL));
    } else if (entry->value->expression->properties.expression_props.string_literal.content != NULL) {
        REQUIRE_OK(traverse_aggregate_union_string_literal(mem, context, traversal, entry));
    } else if (KEFIR_AST_TYPE_IS_SCALAR_TYPE(entry->value->expression->properties.type)) {
        const struct kefir_ast_type *type = NULL;
        REQUIRE_OK(kefir_ast_type_traversal_next_recursive(mem, traversal, &type, NULL));

        kefir_result_t res;
        REQUIRE_MATCH_OK(
            &res, kefir_ast_node_assignable(mem, context, entry->value->expression, kefir_ast_unqualified_type(type)),
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &entry->value->expression->source_location,
                                   "Expression value shall be assignable to field type"));
        REQUIRE_OK(res);
    } else {
        const struct kefir_ast_type *type = NULL;
        REQUIRE_OK(kefir_ast_type_traversal_next(mem, traversal, &type, NULL));
        kefir_result_t res =
            kefir_ast_node_assignable(mem, context, entry->value->expression, kefir_ast_unqualified_type(type));
        while (res == KEFIR_NO_MATCH) {
            REQUIRE_OK(kefir_ast_type_traversal_step(mem, traversal));
            REQUIRE_OK(kefir_ast_type_traversal_next(mem, traversal, &type, NULL));
            res = kefir_ast_node_assignable(mem, context, entry->value->expression, kefir_ast_unqualified_type(type));
        }

        if (res == KEFIR_NO_MATCH) {
            res = KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &entry->value->expression->source_location,
                                         "Expression value shall be assignable to field type");
        }
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t traverse_aggregate_union(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                               const struct kefir_ast_initializer *initializer,
                                               struct kefir_ast_type_traversal *traversal) {
    const struct kefir_list_entry *init_iter = kefir_list_head(&initializer->list.initializers);
    for (; init_iter != NULL; kefir_list_next(&init_iter)) {
        ASSIGN_DECL_CAST(struct kefir_ast_initializer_list_entry *, entry, init_iter->value);

        kefir_result_t res = kefir_ast_designator_unroll(
            entry->designator, traverse_aggregate_union_impl,
            &(struct traverse_aggregate_union_param) {
                .mem = mem, .context = context, .initializer = initializer, .traversal = traversal, .entry = entry});
        if (res != KEFIR_YIELD) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t analyze_struct_union(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                           const struct kefir_ast_type *type,
                                           const struct kefir_ast_initializer *initializer,
                                           struct kefir_ast_initializer_properties *properties) {
    REQUIRE(!KEFIR_AST_TYPE_IS_INCOMPLETE(type),
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &initializer->source_location,
                                   "Cannot initialize incomplete object type"));
    if (initializer->type == KEFIR_AST_INITIALIZER_EXPRESSION) {
        kefir_result_t res;
        REQUIRE_MATCH_OK(&res, kefir_ast_node_assignable(mem, context, initializer->expression, type),
                         KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &initializer->expression->source_location,
                                                "Expression value shall be assignable to field type"));
        REQUIRE_OK(res);
    } else {
        struct kefir_ast_type_traversal traversal;
        REQUIRE_OK(kefir_ast_type_traversal_init(mem, &traversal, type));
        kefir_result_t res = traverse_aggregate_union(mem, context, initializer, &traversal);
        REQUIRE_OK(kefir_ast_type_traversal_free(mem, &traversal));
        REQUIRE_OK(res);
    }
    if (properties != NULL) {
        properties->type = type;
    }
    return KEFIR_OK;
}

static kefir_result_t array_layer_next(const struct kefir_ast_type_traversal *traversal,
                                       const struct kefir_ast_type_traversal_layer *layer, void *payload) {
    UNUSED(traversal);
    REQUIRE(layer != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid type traversal layer"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid type traversal listener payload"));
    ASSIGN_DECL_CAST(kefir_size_t *, array_length, payload)
    if (layer->parent == NULL) {
        REQUIRE(layer->type == KEFIR_AST_TYPE_TRAVERSAL_ARRAY,
                KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected top traversal level type to be array"));
        *array_length = MAX(*array_length, layer->array.index + 1);
    }
    return KEFIR_OK;
}

static kefir_result_t array_layer_end(const struct kefir_ast_type_traversal *traversal,
                                      const struct kefir_ast_type_traversal_layer *layer, void *payload) {
    UNUSED(traversal);
    REQUIRE(layer != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid type traversal layer"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid type traversal listener payload"));
    ASSIGN_DECL_CAST(kefir_size_t *, array_length, payload)
    if (layer->parent == NULL) {
        REQUIRE(layer->type == KEFIR_AST_TYPE_TRAVERSAL_ARRAY,
                KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected top traversal level type to be array"));
        *array_length = MAX(*array_length, layer->array.index + (layer->init ? 0 : 1));
    }
    return KEFIR_OK;
}

static kefir_result_t analyze_array(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                    const struct kefir_ast_type *type, const struct kefir_ast_initializer *initializer,
                                    struct kefir_ast_initializer_properties *properties) {
    struct kefir_ast_node_base *head_expr = kefir_ast_initializer_head(initializer);
    kefir_size_t array_length = 0;
    kefir_bool_t is_string = false;
    if (head_expr != NULL && head_expr->properties.expression_props.string_literal.content != NULL) {
        kefir_bool_t (*stop_fn)(const struct kefir_ast_type *, void *) = NULL;
        void *stop_payload = NULL;
        REQUIRE_OK(string_literal_stop_fn(head_expr, context->type_traits, &stop_fn, &stop_payload));
        if (stop_fn(type, stop_payload)) {
            array_length = head_expr->properties.expression_props.string_literal.length;
            is_string = true;
        }
    }

    if (!is_string) {
        REQUIRE(initializer->type == KEFIR_AST_INITIALIZER_LIST,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &initializer->source_location,
                                       "Unable to initialize array by non-string literal expression"));
        struct kefir_ast_type_traversal traversal;
        REQUIRE_OK(kefir_ast_type_traversal_init(mem, &traversal, type));
        traversal.events.layer_next = array_layer_next;
        traversal.events.layer_end = array_layer_end;
        traversal.events.payload = &array_length;
        kefir_result_t res = traverse_aggregate_union(mem, context, initializer, &traversal);
        REQUIRE_OK(kefir_ast_type_traversal_free(mem, &traversal));
        REQUIRE_OK(res);
    }
    if (properties != NULL) {
        if (type->array_type.boundary == KEFIR_AST_ARRAY_UNBOUNDED) {
            properties->type = kefir_ast_type_array(mem, context->type_bundle, type->array_type.element_type,
                                                    array_length, &type->array_type.qualifications);
        } else {
            properties->type = type;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t analyze_auto_type(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                        const struct kefir_ast_type *type,
                                        const struct kefir_ast_initializer *initializer,
                                        struct kefir_ast_initializer_properties *properties) {
    UNUSED(mem);
    UNUSED(context);
    UNUSED(type);

    REQUIRE(initializer->type == KEFIR_AST_INITIALIZER_EXPRESSION,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &initializer->source_location,
                                   "Declaration with auto type shall be initialized with an expression"));

    struct kefir_ast_node_base *expr = initializer->expression;
    const struct kefir_ast_type *unqualified_type =
        KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->type_bundle, expr->properties.type);
    REQUIRE(unqualified_type->tag != KEFIR_AST_TYPE_VOID,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &initializer->source_location,
                                   "Declaration with auto type shall be initialized with a void type expression"));

    if (properties != NULL) {
        properties->type = unqualified_type;
        if (KEFIR_AST_TYPE_IS_LONG_DOUBLE(unqualified_type)) {
            properties->contains_long_double = true;
        }
    }
    return KEFIR_OK;
}

struct obtain_temporaries_for_ranges_param {
    struct kefir_mem *mem;
    const struct kefir_ast_context *context;
    struct kefir_hashtreeset repeated_nodes;
};

static kefir_result_t traverse_scalar(const struct kefir_ast_designator *designator,
                                      struct kefir_ast_node_base *expression, void *payload) {
    UNUSED(designator);
    REQUIRE(expression != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST expression node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));
    REQUIRE(!expression->properties.expression_props.preserve_after_eval.enabled, KEFIR_OK);
    ASSIGN_DECL_CAST(struct obtain_temporaries_for_ranges_param *, param, payload);

    if (kefir_hashtreeset_has(&param->repeated_nodes, (kefir_hashtreeset_entry_t) expression)) {
        REQUIRE_OK(param->context->allocate_temporary_value(
            param->mem, param->context, expression->properties.type, KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_UNKNOWN, NULL,
            &expression->source_location,
            &expression->properties.expression_props.preserve_after_eval.temporary_identifier));
        expression->properties.expression_props.preserve_after_eval.enabled = true;
    }
    REQUIRE_OK(kefir_hashtreeset_add(param->mem, &param->repeated_nodes, (kefir_hashtreeset_entry_t) expression));

    return KEFIR_OK;
}

static kefir_result_t obtain_temporaries_for_ranges(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                    const struct kefir_ast_type *type,
                                                    const struct kefir_ast_initializer *initializer) {
    struct obtain_temporaries_for_ranges_param param = {.mem = mem, .context = context};
    REQUIRE_OK(kefir_hashtreeset_init(&param.repeated_nodes, &kefir_hashtree_uint_ops));

    struct kefir_ast_initializer_traversal initializer_traversal;
    KEFIR_AST_INITIALIZER_TRAVERSAL_INIT(&initializer_traversal);
    initializer_traversal.visit_value = traverse_scalar;
    initializer_traversal.payload = &param;

    kefir_result_t res = kefi_ast_traverse_initializer(mem, context, initializer, type, &initializer_traversal);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &param.repeated_nodes);
        return res;
    });
    REQUIRE_OK(kefir_hashtreeset_free(mem, &param.repeated_nodes));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_analyze_initializer(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                             const struct kefir_ast_type *type,
                                             const struct kefir_ast_initializer *initializer,
                                             struct kefir_ast_initializer_properties *properties) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(initializer != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST initializer"));

    type = kefir_ast_unqualified_type(type);

    if (properties != NULL) {
        properties->type = NULL;
        properties->constant = true;
    }
    struct kefir_ast_initializer_properties props = {.constant = true, .type = NULL, .contains_long_double = false};
    REQUIRE_OK(preanalyze_initializer(mem, context, initializer, &props));
    if (KEFIR_AST_TYPE_IS_SCALAR_TYPE(type)) {
        REQUIRE_OK(analyze_scalar(mem, context, type, initializer, &props));
    } else if (type->tag == KEFIR_AST_TYPE_ARRAY) {
        REQUIRE_OK(analyze_array(mem, context, type, initializer, &props));
    } else if (type->tag == KEFIR_AST_TYPE_STRUCTURE || type->tag == KEFIR_AST_TYPE_UNION) {
        REQUIRE_OK(analyze_struct_union(mem, context, type, initializer, &props));
    } else if (KEFIR_AST_TYPE_IS_AUTO(type)) {
        REQUIRE_OK(analyze_auto_type(mem, context, type, initializer, &props));
    } else {
        return KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &initializer->source_location,
                                      "Cannot initialize incomplete object type");
    }
    REQUIRE_OK(obtain_temporaries_for_ranges(mem, context, props.type, initializer));
    ASSIGN_PTR(properties, props);
    return KEFIR_OK;
}
