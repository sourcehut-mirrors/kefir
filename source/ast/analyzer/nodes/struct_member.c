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

#include "kefir/ast/analyzer/nodes.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/ast/type_completion.h"
#include "kefir/ast/type_conv.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"
#include "kefir/ast/type_completion.h"

kefir_result_t kefir_ast_analyze_struct_member_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                    const struct kefir_ast_struct_member *node,
                                                    struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST struct member"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST base node"));

    REQUIRE_OK(kefir_ast_analyze_node(mem, context, node->structure));

    REQUIRE(node->structure->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->structure->source_location,
                                   "Structure shall be an expression"));

    const struct kefir_ast_type *struct_type = node->structure->properties.type;
    const struct kefir_ast_type_qualification *qualification = NULL;
    if (base->klass->type == KEFIR_AST_STRUCTURE_INDIRECT_MEMBER) {
        struct_type = KEFIR_AST_TYPE_CONV_EXPRESSION_ALL(mem, context->type_bundle, struct_type);
        REQUIRE(struct_type->tag == KEFIR_AST_TYPE_SCALAR_POINTER,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->structure->source_location,
                                       "Expected pointer type on the left side of arrow operator"));
        struct_type = struct_type->referenced_type;
    }
    if (struct_type->tag == KEFIR_AST_TYPE_QUALIFIED) {
        REQUIRE(struct_type->qualified_type.type->tag == KEFIR_AST_TYPE_STRUCTURE ||
                    struct_type->qualified_type.type->tag == KEFIR_AST_TYPE_UNION,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->structure->source_location,
                                       "Expected qualified structure or union type on the left side"));
        qualification = &struct_type->qualified_type.qualification;
        struct_type = struct_type->qualified_type.type;
    } else {
        REQUIRE(struct_type->tag == KEFIR_AST_TYPE_STRUCTURE || struct_type->tag == KEFIR_AST_TYPE_UNION,
                KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->structure->source_location,
                                       "Expected structure or union type on the left side"));
    }

    REQUIRE_OK(kefir_ast_type_completion(mem, context, &struct_type, struct_type));
    REQUIRE((struct_type->tag == KEFIR_AST_TYPE_STRUCTURE || struct_type->tag == KEFIR_AST_TYPE_UNION) &&
                struct_type->structure_type.complete,
            KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &node->structure->source_location,
                                   "Expected expression of complete structure/union type"));

    const struct kefir_ast_struct_field *field = NULL;
    kefir_result_t res = kefir_ast_struct_type_resolve_field(&struct_type->structure_type, node->member, &field);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_SOURCE_ERRORF(KEFIR_ANALYSIS_ERROR, &node->base.source_location,
                                      "Cannot find field %s in structure", node->member);
    }
    REQUIRE_OK(res);
    const struct kefir_ast_type *type = NULL;
    REQUIRE_OK(kefir_ast_type_completion(mem, context, &type, field->type));
    if (qualification != NULL) {
        type = kefir_ast_type_qualified(mem, context->type_bundle, type, *qualification);
    }

    REQUIRE_OK(kefir_ast_node_properties_init(&base->properties));
    base->properties.category = KEFIR_AST_NODE_CATEGORY_EXPRESSION;
    base->properties.type = type;
    base->properties.expression_props.lvalue = true;
    base->properties.expression_props.addressable = !field->bitfield;
    base->properties.expression_props.bitfield_props.bitfield = field->bitfield;
    base->properties.expression_props.atomic =
        type->tag == KEFIR_AST_TYPE_QUALIFIED && type->qualified_type.qualification.atomic_type;
    if (field->bitfield) {
        base->properties.expression_props.bitfield_props.width = field->bitwidth->value.integer;
    }

    const struct kefir_ast_type *unqualified_type = kefir_ast_unqualified_type(type);
    if (base->properties.expression_props.atomic &&
        KEFIR_AST_TYPE_IS_AGGREGATE_TYPE(unqualified_type)) {
        REQUIRE_OK(context->allocate_temporary_value(mem, context, type, NULL, &base->source_location,
                                                     &base->properties.expression_props.temporary_identifier));
    }

    if (unqualified_type->tag == KEFIR_AST_TYPE_ARRAY) {
        base->properties.expression_props.constant_expression =
            node->structure->properties.expression_props.constant_expression ||
            (node->structure->klass->type == KEFIR_AST_IDENTIFIER &&
             node->structure->properties.expression_props.scoped_id->klass == KEFIR_AST_SCOPE_IDENTIFIER_OBJECT &&
             (node->structure->properties.expression_props.scoped_id->object.storage ==
                  KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN ||
              node->structure->properties.expression_props.scoped_id->object.storage ==
                  KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC));
    return KEFIR_OK;
}
