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
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

kefir_result_t kefir_ast_try_analyze_identifier(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                const struct kefir_ast_identifier *node,
                                                struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST identifier"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST base node"));

    const struct kefir_ast_scoped_identifier *scoped_id = NULL;
    REQUIRE_OK(context->resolve_ordinary_identifier(context, node->identifier, &scoped_id));

    switch (scoped_id->klass) {
        case KEFIR_AST_SCOPE_IDENTIFIER_OBJECT:
            REQUIRE_OK(kefir_ast_node_properties_init(&base->properties));
            base->properties.category = KEFIR_AST_NODE_CATEGORY_EXPRESSION;
            base->properties.type = scoped_id->object.type;
            base->properties.expression_props.lvalue = scoped_id->object.type->tag != KEFIR_AST_TYPE_ARRAY;
            base->properties.expression_props.addressable =
                scoped_id->object.storage != KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_REGISTER;
            base->properties.expression_props.atomic = scoped_id->object.type->tag == KEFIR_AST_TYPE_QUALIFIED &&
                                                       scoped_id->object.type->qualified_type.qualification.atomic_type;
            base->properties.expression_props.constant_expression =
                scoped_id->klass == KEFIR_AST_SCOPE_IDENTIFIER_OBJECT && scoped_id->type->tag == KEFIR_AST_TYPE_ARRAY &&
                (scoped_id->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN ||
                 scoped_id->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC);
            base->properties.expression_props.alignment =
                scoped_id->object.alignment != NULL ? scoped_id->object.alignment->value : 0;

            if (base->properties.expression_props.atomic &&
                KEFIR_AST_TYPE_IS_AGGREGATE_TYPE(kefir_ast_unqualified_type(scoped_id->object.type))) {
                REQUIRE_OK(context->allocate_temporary_value(mem, context, scoped_id->object.type, NULL,
                                                             &base->source_location,
                                                             &base->properties.expression_props.temporary_identifier));
            }
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION:
            REQUIRE_OK(kefir_ast_node_properties_init(&base->properties));
            base->properties.category = KEFIR_AST_NODE_CATEGORY_EXPRESSION;
            base->properties.type = scoped_id->function.type;
            base->properties.expression_props.addressable = true;
            base->properties.expression_props.constant_expression = true;
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_ENUM_CONSTANT:
            REQUIRE_OK(kefir_ast_node_properties_init(&base->properties));
            base->properties.category = KEFIR_AST_NODE_CATEGORY_EXPRESSION;
            base->properties.type = scoped_id->enum_constant.type;
            base->properties.expression_props.constant_expression = true;
            break;

        case KEFIR_AST_SCOPE_IDENTIFIER_TYPE_DEFINITION:
            REQUIRE_OK(kefir_ast_node_properties_init(&base->properties));
            base->properties.category = KEFIR_AST_NODE_CATEGORY_TYPE;
            base->properties.type = scoped_id->type_definition.type;
            base->properties.type_props.alignment =
                scoped_id->type_definition.alignment != NULL ? scoped_id->type_definition.alignment->value : 0;
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected scoped identifier type");
    }

    base->properties.expression_props.identifier =
        kefir_string_pool_insert(mem, context->symbols, node->identifier, NULL);
    REQUIRE(base->properties.expression_props.identifier != NULL,
            KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert identifier into symbol table"));
    base->properties.expression_props.scoped_id = scoped_id;
    return KEFIR_OK;
}

kefir_result_t kefir_ast_analyze_identifier_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                 const struct kefir_ast_identifier *node,
                                                 struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST identifier"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST base node"));

    kefir_result_t res = kefir_ast_try_analyze_identifier(mem, context, node, base);
    if (res == KEFIR_NOT_FOUND) {
        return KEFIR_SET_SOURCE_ERRORF(KEFIR_ANALYSIS_ERROR, &base->source_location,
                                       "Unable to find '%s' in current scope", node->identifier);
    } else {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}
