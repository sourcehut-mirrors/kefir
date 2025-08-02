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

struct eval_param {
    struct kefir_mem *mem;
    const struct kefir_ast_context *context;
    struct kefir_ast_constant_expression_value *value;
};

static kefir_result_t visit_non_constant_expression(const struct kefir_ast_visitor *visitor,
                                                    const struct kefir_ast_node_base *base, void *payload) {
    UNUSED(visitor);
    UNUSED(base);
    UNUSED(payload);
    return KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &base->source_location,
                                  "Unable to evaluate non-constant AST node");
}

#define VISITOR(_id, _type)                                                                                    \
    kefir_result_t evaluate_##_id(const struct kefir_ast_visitor *visitor, const _type *node, void *payload) { \
        UNUSED(visitor);                                                                                       \
        ASSIGN_DECL_CAST(struct eval_param *, param, payload);                                                 \
        return kefir_ast_evaluate_##_id##_node(param->mem, param->context, node, param->value);                \
    }
VISITOR(scalar, struct kefir_ast_constant)
VISITOR(identifier, struct kefir_ast_identifier)
VISITOR(struct_member, struct kefir_ast_struct_member)
VISITOR(array_subscript, struct kefir_ast_array_subscript)
VISITOR(string_literal, struct kefir_ast_string_literal)
VISITOR(compound_literal, struct kefir_ast_compound_literal)
VISITOR(label_address, struct kefir_ast_label_address)
VISITOR(generic_selection, struct kefir_ast_generic_selection)
VISITOR(unary_operation, struct kefir_ast_unary_operation)
VISITOR(binary_operation, struct kefir_ast_binary_operation)
VISITOR(conditional_operator, struct kefir_ast_conditional_operator)
VISITOR(cast_operator, struct kefir_ast_cast_operator)
VISITOR(builtin, struct kefir_ast_builtin)
VISITOR(comma_operator, struct kefir_ast_comma_operator)
VISITOR(function_call, struct kefir_ast_function_call)
#undef VISITOR

static kefir_result_t evaluate_extension_node(const struct kefir_ast_visitor *visitor,
                                              const struct kefir_ast_extension_node *node, void *payload) {
    UNUSED(visitor);
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));
    ASSIGN_DECL_CAST(struct eval_param *, param, payload);
    REQUIRE(param != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant expression parameter"));

    REQUIRE(param->context->extensions != NULL && param->context->extensions->evaluate_constant_extension_node != NULL,
            KEFIR_SET_ERROR(KEFIR_NOT_CONSTANT, "Unable to evaluate extension node"));
    REQUIRE_OK(param->context->extensions->evaluate_constant_extension_node(param->mem, param->context,
                                                                            KEFIR_AST_NODE_BASE(node), param->value));
    return KEFIR_OK;
}

kefir_result_t kefir_ast_constant_expression_value_evaluate(struct kefir_mem *mem,
                                                            const struct kefir_ast_context *context,
                                                            const struct kefir_ast_node_base *node,
                                                            struct kefir_ast_constant_expression_value *value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));
    REQUIRE(value != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant expression value pointer"));

    REQUIRE(
        node->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
        KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->source_location, "Expected constant expression AST node"));

    memset(value, 0, sizeof(struct kefir_ast_constant_expression_value));
    struct eval_param param = {.mem = mem, .context = context, .value = value};
    struct kefir_ast_visitor visitor;
    REQUIRE_OK(kefir_ast_visitor_init(&visitor, visit_non_constant_expression));
    visitor.constant = evaluate_scalar;
    visitor.identifier = evaluate_identifier;
    visitor.struct_member = evaluate_struct_member;
    visitor.struct_indirect_member = evaluate_struct_member;
    visitor.array_subscript = evaluate_array_subscript;
    visitor.string_literal = evaluate_string_literal;
    visitor.compound_literal = evaluate_compound_literal;
    visitor.label_address = evaluate_label_address;
    visitor.generic_selection = evaluate_generic_selection;
    visitor.unary_operation = evaluate_unary_operation;
    visitor.binary_operation = evaluate_binary_operation;
    visitor.conditional_operator = evaluate_conditional_operator;
    visitor.cast_operator = evaluate_cast_operator;
    visitor.builtin = evaluate_builtin;
    visitor.comma_operator = evaluate_comma_operator;
    visitor.function_call = evaluate_function_call;
    visitor.extension_node = evaluate_extension_node;
    return KEFIR_AST_NODE_VISIT(&visitor, node, &param);
}

kefir_result_t kefir_ast_constant_expression_value_to_boolean(const struct kefir_ast_constant_expression_value *value,
                                                              kefir_bool_t *boolean) {
    REQUIRE(value != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant expression value"));
    REQUIRE(boolean != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean"));

    *boolean = false;
    switch (value->klass) {
        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER:
            if (value->bitprecise != NULL) {
                kefir_bool_t is_zero;
                REQUIRE_OK(kefir_bigint_is_zero(value->bitprecise, &is_zero));
                *boolean = !is_zero;
            } else {
                *boolean = value->integer;
            }
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT:
            *boolean = (kefir_bool_t) value->floating_point;
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT:
            *boolean = (kefir_bool_t) value->complex_floating_point.real ||
                       (kefir_bool_t) value->complex_floating_point.imaginary;
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS:
            switch (value->pointer.type) {
                case KEFIR_AST_CONSTANT_EXPRESSION_POINTER_IDENTIFER:
                case KEFIR_AST_CONSTANT_EXPRESSION_POINTER_LITERAL:
                    *boolean = true;
                    break;

                case KEFIR_AST_CONSTANT_EXPRESSION_POINTER_INTEGER:
                    *boolean = (kefir_bool_t) (value->pointer.base.integral + value->pointer.offset);
                    break;
            }
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPOUND:
            return KEFIR_SET_ERROR(KEFIR_NOT_CONSTANT, "Unable to cast compound constant expression");

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_NONE:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Non-evaluated constant expression");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_evaluate_comparison(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                             struct kefir_ast_node_base *lhs_node, struct kefir_ast_node_base *rhs_node,
                                             kefir_int_t *comparison) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(lhs_node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));
    REQUIRE(rhs_node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));
    REQUIRE(comparison != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to comparison result"));

    struct kefir_ast_node_base *lt_comparison = KEFIR_AST_NODE_BASE(kefir_ast_new_binary_operation(
        mem, KEFIR_AST_OPERATION_LESS, KEFIR_AST_NODE_REF(lhs_node), KEFIR_AST_NODE_REF(rhs_node)));
    REQUIRE(lt_comparison != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed toa allocate AST node"));

    lt_comparison->properties.category = KEFIR_AST_NODE_CATEGORY_EXPRESSION;
    lt_comparison->properties.type = kefir_ast_type_signed_int();

    struct kefir_ast_constant_expression_value lt_value;
    kefir_result_t res = kefir_ast_constant_expression_value_evaluate(mem, context, lt_comparison, &lt_value);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_NODE_FREE(mem, lt_comparison);
        return res;
    });
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, lt_comparison));

    struct kefir_ast_node_base *eq_comparison = KEFIR_AST_NODE_BASE(kefir_ast_new_binary_operation(
        mem, KEFIR_AST_OPERATION_EQUAL, KEFIR_AST_NODE_REF(lhs_node), KEFIR_AST_NODE_REF(rhs_node)));
    REQUIRE(eq_comparison != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed toa allocate AST node"));

    eq_comparison->properties.category = KEFIR_AST_NODE_CATEGORY_EXPRESSION;
    eq_comparison->properties.type = kefir_ast_type_signed_int();

    struct kefir_ast_constant_expression_value eq_value;
    res = kefir_ast_constant_expression_value_evaluate(mem, context, eq_comparison, &eq_value);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_NODE_FREE(mem, eq_comparison);
        return res;
    });
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, eq_comparison));

    REQUIRE(lt_value.klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected comparison result"));
    REQUIRE(eq_value.klass == KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER,
            KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected comparison result"));

    if (lt_value.integer) {
        *comparison = -1;
    } else if (eq_value.integer) {
        *comparison = 0;
    } else {
        *comparison = 1;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_constant_expression_value_equal(const struct kefir_ast_constant_expression_value *lhs_value,
                                                         const struct kefir_ast_constant_expression_value *rhs_value,
                                                         kefir_bool_t *equal_ptr) {
    REQUIRE(lhs_value != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant expression value"));
    REQUIRE(rhs_value != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant expression value"));
    REQUIRE(equal_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to equality flag"));

    if (lhs_value->klass != rhs_value->klass) {
        *equal_ptr = false;
        return KEFIR_OK;
    }

    switch (lhs_value->klass) {
        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_NONE:
            *equal_ptr = true;
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER:
            if (lhs_value->bitprecise != rhs_value->bitprecise) {
                *equal_ptr = false;
            } else if (lhs_value->bitprecise != NULL) {
                kefir_int_t cmp;
                REQUIRE_OK(kefir_bigint_signed_compare(lhs_value->bitprecise, rhs_value->bitprecise, &cmp));
                *equal_ptr = cmp == 0;
            } else {
                *equal_ptr = lhs_value->integer == rhs_value->integer;
            }
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT:
            *equal_ptr = lhs_value->floating_point == rhs_value->floating_point;
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT:
            *equal_ptr = lhs_value->complex_floating_point.real == rhs_value->complex_floating_point.real &&
                         lhs_value->complex_floating_point.imaginary == rhs_value->complex_floating_point.imaginary;
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS:
            if (lhs_value->pointer.type != rhs_value->pointer.type) {
                *equal_ptr = false;
            } else if (lhs_value->pointer.offset != rhs_value->pointer.offset) {
                *equal_ptr = false;
            } else {
                switch (lhs_value->pointer.type) {
                    case KEFIR_AST_CONSTANT_EXPRESSION_POINTER_IDENTIFER:
                        *equal_ptr = strcmp(lhs_value->pointer.base.literal, rhs_value->pointer.base.literal) == 0;
                        break;

                    case KEFIR_AST_CONSTANT_EXPRESSION_POINTER_INTEGER:
                        *equal_ptr = lhs_value->pointer.base.integral == rhs_value->pointer.base.integral;
                        break;

                    case KEFIR_AST_CONSTANT_EXPRESSION_POINTER_LITERAL:
                        *equal_ptr = false;
                        break;
                }
            }
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPOUND:
            return KEFIR_SET_ERROR(KEFIR_NOT_CONSTANT, "Unable to compare compound constant expressions");
    }
    return KEFIR_OK;
}

static kefir_result_t is_initializer_statically_known(const struct kefir_ast_initializer *initializer,
                                                      kefir_bool_t *is_statically_known) {
    if (initializer->type == KEFIR_AST_INITIALIZER_EXPRESSION) {
        REQUIRE_OK(kefir_ast_constant_expression_is_statically_known(
            &initializer->expression->properties.expression_props.constant_expression_value, is_statically_known));
    } else {
        *is_statically_known = true;
        for (const struct kefir_list_entry *iter = kefir_list_head(&initializer->list.initializers);
             iter != NULL && *is_statically_known; kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(const struct kefir_ast_initializer_list_entry *, entry, iter->value);
            kefir_bool_t is_entry_known;
            REQUIRE_OK(is_initializer_statically_known(entry->value, &is_entry_known));
            *is_statically_known = *is_statically_known && is_entry_known;
        }
    }

    return KEFIR_OK;
}

kefir_result_t kefir_ast_constant_expression_is_statically_known(
    const struct kefir_ast_constant_expression_value *value, kefir_bool_t *is_statically_known) {
    REQUIRE(value != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid constant expression value"));
    REQUIRE(is_statically_known != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    switch (value->klass) {
        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_NONE:
        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER:
        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT:
        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT:
            *is_statically_known = true;
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS:
            *is_statically_known = value->pointer.type != KEFIR_AST_CONSTANT_EXPRESSION_POINTER_IDENTIFER;
            break;

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPOUND:
            REQUIRE_OK(is_initializer_statically_known(value->compound.initializer, is_statically_known));
            break;
    }

    return KEFIR_OK;
}
