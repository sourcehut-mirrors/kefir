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

#include <string.h>
#include "kefir/ast/analyzer/nodes.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/ast/analyzer/declarator.h"
#include "kefir/ast/deprecation.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/core/source_error.h"

kefir_result_t kefir_ast_analyze_init_declarator_node(struct kefir_mem *mem, const struct kefir_ast_context *context,
                                                      const struct kefir_ast_declarator_specifier_list *specifiers,
                                                      const struct kefir_ast_init_declarator *node,
                                                      struct kefir_ast_node_base *base,
                                                      const struct kefir_ast_type *type,
                                                      kefir_ast_scoped_identifier_storage_t storage,
                                                      kefir_ast_function_specifier_t function, kefir_size_t aligment) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(specifiers != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST declarator specifier list"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST init declarator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST base node"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));

    REQUIRE_OK(kefir_ast_node_properties_init(&base->properties));
    base->properties.category = KEFIR_AST_NODE_CATEGORY_INIT_DECLARATOR;
    const char *identifier = NULL;
    struct kefir_ast_declarator_attributes attributes;
    REQUIRE_OK(kefir_ast_analyze_declaration_declarator(mem, context, specifiers, node->declarator, &identifier, &type,
                                                        &aligment, KEFIR_AST_DECLARATION_ANALYSIS_NORMAL, &attributes));
    base->properties.declaration_props.function = function;
    base->properties.declaration_props.alignment = aligment;

    REQUIRE(
        !KEFIR_AST_TYPE_IS_AUTO(type) || node->initializer != NULL,
        KEFIR_SET_SOURCE_ERROR(KEFIR_ANALYSIS_ERROR, &base->source_location,
                               "Auto type specifier shall be used only for declarations with a single initializer"));

    if (identifier != NULL) {
        identifier = kefir_string_pool_insert(mem, context->symbols, identifier, NULL);
        REQUIRE(identifier != NULL,
                KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert declarator identifier into symbol table"));
    }

    base->properties.declaration_props.identifier = identifier;
    REQUIRE_OK(kefir_ast_analyze_type(mem, context, context->type_analysis_context, type, &node->base.source_location));

    if (base->properties.declaration_props.identifier != NULL) {
        struct kefir_ast_alignment *alignment = NULL;
        if (base->properties.declaration_props.alignment != 0) {
            alignment = kefir_ast_alignment_const_expression(mem, base->properties.declaration_props.alignment);
            REQUIRE(alignment != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate AST alignment"));
        }

        const struct kefir_ast_scoped_identifier *scoped_id = NULL;
        kefir_result_t res = kefir_ast_check_type_deprecation(context, type, &base->source_location);
        REQUIRE_CHAIN(&res, context->define_identifier(
                                mem, context, node->initializer == NULL, base->properties.declaration_props.identifier,
                                type, storage, base->properties.declaration_props.function, alignment,
                                node->initializer, &attributes, &base->source_location, &scoped_id));
        REQUIRE_ELSE(res == KEFIR_OK, {
            if (alignment != NULL) {
                kefir_ast_alignment_free(mem, alignment);
            }
            return res;
        });

        REQUIRE(scoped_id != NULL, KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Expected valid AST scoped identifier"));
        switch (scoped_id->klass) {
            case KEFIR_AST_SCOPE_IDENTIFIER_OBJECT:
                base->properties.type = scoped_id->object.type;
                base->properties.declaration_props.storage = scoped_id->object.storage;
                if (scoped_id->object.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_CONSTEXPR) {
                    REQUIRE_OK(context->allocate_temporary_value(
                        mem, context, scoped_id->object.type, KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_CONSTEXPR_STATIC,
                        node->initializer, &node->base.source_location,
                        &base->properties.declaration_props.temporary_identifier));
                }
                break;

            case KEFIR_AST_SCOPE_IDENTIFIER_FUNCTION:
                base->properties.type = scoped_id->function.type;
                base->properties.declaration_props.storage = scoped_id->function.storage;
                break;

            case KEFIR_AST_SCOPE_IDENTIFIER_TYPE_DEFINITION:
                base->properties.type = scoped_id->type;
                base->properties.declaration_props.storage = storage;
                break;

            case KEFIR_AST_SCOPE_IDENTIFIER_ENUM_CONSTANT:
            case KEFIR_AST_SCOPE_IDENTIFIER_TYPE_TAG:
            case KEFIR_AST_SCOPE_IDENTIFIER_LABEL:
                return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected AST scoped identifier class");
        }

        base->properties.declaration_props.original_type = type;
        base->properties.declaration_props.scoped_id = scoped_id;
    } else {
        base->properties.type = type;
        base->properties.declaration_props.original_type = type;
        base->properties.declaration_props.storage = storage;
    }
    return KEFIR_OK;
}
