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
#include "kefir/ast/initializer_traversal.h"
#include "kefir/ast/downcast.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

static kefir_result_t calculate_member_offset(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                              const struct kefir_ast_struct_member *node, kefir_size_t *offset) {
    struct kefir_ast_designator designator = {
        .type = KEFIR_AST_DESIGNATOR_MEMBER, .member = node->member, .next = NULL};

    const struct kefir_ast_type *structure_type = kefir_ast_unqualified_type(node->structure->properties.type);
    if (node->base.klass->type == KEFIR_AST_STRUCTURE_INDIRECT_MEMBER) {
        REQUIRE(structure_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER,
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->structure->source_location,
                                       "Expected constant expression of pointer type"));
        structure_type = structure_type->referenced_type;
    }

    struct kefir_ast_target_environment_object_info object_info;
    kefir_ast_target_environment_opaque_type_t opaque_type;
    REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_GET_TYPE(mem, context, context->target_env, structure_type, &opaque_type,
                                                     &node->base.source_location));
    kefir_result_t res =
        KEFIR_AST_TARGET_ENVIRONMENT_OBJECT_INFO(mem, context->target_env, opaque_type, &designator, &object_info);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->target_env, opaque_type);
        return res;
    });
    REQUIRE_OK(KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(mem, context->target_env, opaque_type));

    *offset = object_info.relative_offset;
    return KEFIR_OK;
}

struct compound_traversal_param {
    struct kefir_mem *mem;
    const struct kefir_ast_context *context;
    const char *member_name;
    struct kefir_ast_node_base **initializer_expr;
};

static kefir_result_t retrieve_scalar_initializer_visit_value(const struct kefir_ast_designator *designator,
                                                              struct kefir_ast_node_base *expression, void *payload) {
    REQUIRE(designator != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST designator"));
    REQUIRE(expression != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST expression node"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid payload"));
    ASSIGN_DECL_CAST(struct compound_traversal_param *, param, payload);

    if (designator->next == NULL && designator->type == KEFIR_AST_DESIGNATOR_MEMBER &&
        strcmp(designator->member, param->member_name) == 0) {
        *param->initializer_expr = expression;
        return KEFIR_YIELD;
    }

    return KEFIR_OK;
}

static kefir_result_t retrieve_scalar_initializer(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                  const char *member_name, const struct kefir_ast_type *type,
                                                  const struct kefir_ast_initializer *initializer,
                                                  struct kefir_ast_node_base **initializer_expr) {
    struct kefir_ast_initializer_traversal initializer_traversal;
    KEFIR_AST_INITIALIZER_TRAVERSAL_INIT(&initializer_traversal);

    struct compound_traversal_param param = {
        .mem = mem, .context = context, .member_name = member_name, .initializer_expr = initializer_expr};

    initializer_traversal.visit_value = retrieve_scalar_initializer_visit_value;
    initializer_traversal.payload = &param;

    *initializer_expr = NULL;
    kefir_result_t res = kefir_ast_traverse_initializer(mem, context, initializer, type, &initializer_traversal);
    if (res == KEFIR_YIELD) {
        res = KEFIR_OK;
    }
    REQUIRE_OK(res);

    return KEFIR_OK;
}

kefir_result_t kefir_ast_evaluate_struct_member_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                     const struct kefir_ast_struct_member *node,
                                                     struct kefir_ast_constant_expression_value *value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant node"));
    REQUIRE(value != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST constant expression value pointer"));
    REQUIRE(node->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
            KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                   "Expected constant expression AST node"));

    const struct kefir_ast_type *unqualified_type = kefir_ast_unqualified_type(node->base.properties.type);
    if (unqualified_type->tag == KEFIR_AST_TYPE_ARRAY) {
        kefir_size_t member_offset = 0;
        REQUIRE_OK(calculate_member_offset(mem, context, node, &member_offset));

        if (node->base.klass->type == KEFIR_AST_STRUCTURE_INDIRECT_MEMBER) {
            REQUIRE(
                KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->structure, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->structure->source_location,
                                       "Expected constant expression of pointer type"));
            *value = *KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->structure);
        } else {
            REQUIRE_OK(kefir_ast_constant_expression_value_evaluate_lvalue_reference(mem, context, node->structure,
                                                                                     &value->pointer));
            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS;
        }
        value->pointer.offset += member_offset;
        value->pointer.pointer_node = KEFIR_AST_NODE_BASE(node);
    } else if (KEFIR_AST_TYPE_IS_SCALAR_TYPE(unqualified_type)) {
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->structure, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPOUND),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->structure->source_location,
                                       "Expected compound constant expression"));

        struct kefir_ast_node_base *initializer_expr = NULL;
        REQUIRE_OK(retrieve_scalar_initializer(
            mem, context, node->member, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->structure)->compound.type,
            KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(node->structure)->compound.initializer, &initializer_expr));
        if (initializer_expr != NULL) {
            REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION(initializer_expr),
                    KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &initializer_expr->source_location,
                                           "Expected constant expression"));
            REQUIRE_OK(kefir_ast_constant_expression_value_cast(
                mem, context, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(initializer_expr), initializer_expr,
                node->base.properties.type, initializer_expr->properties.type));
        } else {
            struct kefir_ast_constant_expression_value zero_value = {
                .klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER, .integer = 0};
            REQUIRE_OK(kefir_ast_constant_expression_value_cast(mem, context, value, &zero_value,
                                                                KEFIR_AST_NODE_BASE(node), node->base.properties.type,
                                                                kefir_ast_type_signed_int()));
        }
    } else {
        return KEFIR_SET_ERROR(
            KEFIR_NOT_CONSTANT,
            "Structure member constant expression access for non-scalar non-array members is not implemented yet");
    }
    return KEFIR_OK;
}
