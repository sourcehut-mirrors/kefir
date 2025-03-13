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
    REQUIRE(unqualified_type->tag == KEFIR_AST_TYPE_ARRAY,
            KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                   "Expected constant expression AST node"));

    kefir_size_t member_offset = 0;
    REQUIRE_OK(calculate_member_offset(mem, context, node, &member_offset));

    if (node->base.klass->type == KEFIR_AST_STRUCTURE_INDIRECT_MEMBER) {
        REQUIRE(KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION_OF(node->structure, KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS),
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
    return KEFIR_OK;
}
