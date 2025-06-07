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

        case KEFIR_AST_CONSTANT_EXPRESSION_CLASS_NONE:
            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Non-evaluated constant expression");
    }
    return KEFIR_OK;
}
