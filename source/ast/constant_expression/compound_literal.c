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

static kefir_result_t is_initializer_constant(const struct kefir_ast_initializer *initializer, kefir_bool_t *constant) {
    switch (initializer->type) {
        case KEFIR_AST_INITIALIZER_EXPRESSION:
            *constant = initializer->expression->properties.expression_props.constant_expression;
            break;

        case KEFIR_AST_INITIALIZER_LIST:
            *constant = true;
            for (const struct kefir_list_entry *iter = kefir_list_head(&initializer->list.initializers);
                 iter != NULL && *constant; kefir_list_next(&iter)) {
                ASSIGN_DECL_CAST(const struct kefir_ast_initializer_list_entry *, entry, iter->value);
                kefir_bool_t is_constant_entry = true;
                REQUIRE_OK(is_initializer_constant(entry->value, &is_constant_entry));
                *constant = *constant && is_constant_entry;
            }
            break;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_ast_evaluate_compound_literal_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                        const struct kefir_ast_compound_literal *node,
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
        const struct kefir_ast_scoped_identifier *scoped_id =
            node->base.properties.expression_props.temporary_identifier.scoped_id;
        REQUIRE(unqualified_type->tag == KEFIR_AST_TYPE_ARRAY &&
                    (scoped_id->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_EXTERN ||
                     scoped_id->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_STATIC),
                KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                       "Constant compound literal shall be either scalar, compound, or an array with "
                                       "external/static storage"));
        value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS;
        value->pointer.type = KEFIR_AST_CONSTANT_EXPRESSION_POINTER_IDENTIFER;
        value->pointer.base.literal = node->base.properties.expression_props.temporary_identifier.identifier;
        value->pointer.offset = 0;
        value->pointer.pointer_node = KEFIR_AST_NODE_BASE(node);
        value->pointer.scoped_id = scoped_id;
    } else {
        const struct kefir_ast_node_base *initializer = kefir_ast_initializer_head(node->initializer);
        if (KEFIR_AST_TYPE_IS_SCALAR_TYPE(unqualified_type) && initializer != NULL &&
            KEFIR_AST_NODE_IS_CONSTANT_EXPRESSION(initializer)) {
            REQUIRE_OK(kefir_ast_constant_expression_value_cast(
                mem, context, value, KEFIR_AST_NODE_CONSTANT_EXPRESSION_VALUE(initializer), initializer,
                node->base.properties.type, initializer->properties.type));
        } else {
            kefir_bool_t is_constant = false;
            REQUIRE_OK(is_initializer_constant(node->initializer, &is_constant));
            REQUIRE(is_constant, KEFIR_SET_SOURCE_ERROR(KEFIR_NOT_CONSTANT, &node->base.source_location,
                                                        "Compound literal initializer is not constant"));

            value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPOUND;
            value->compound.type = unqualified_type;
            value->compound.initializer = node->initializer;
        }
    }
    return KEFIR_OK;
}
