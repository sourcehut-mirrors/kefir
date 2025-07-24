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

#include "kefir/parser/rule_helpers.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include "kefir/parser/builder.h"

static kefir_result_t builder_callback(struct kefir_mem *mem, struct kefir_parser_ast_builder *builder, void *payload) {
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser AST builder"));
    struct kefir_parser *parser = builder->parser;

    struct kefir_ast_node_attributes attributes;
    REQUIRE_OK(kefir_ast_node_attributes_init(&attributes));

    kefir_result_t res = KEFIR_OK;
    SCAN_ATTRIBUTES(&res, mem, parser, &attributes);
    if (res == KEFIR_OK && kefir_list_length(&attributes.attributes) == 0) {
        res = KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match attribute declaration");
    }
    if (res == KEFIR_OK) {
        if (!PARSER_TOKEN_IS_PUNCTUATOR(parser, 0, KEFIR_PUNCTUATOR_SEMICOLON)) {
            res = KEFIR_SET_ERROR(KEFIR_NO_MATCH, "Unable to match attribute declaration");
        } else {
            REQUIRE_CHAIN(&res, PARSER_SHIFT(parser));
        }
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_node_attributes_free(mem, &attributes);
        return res;
    });

    struct kefir_ast_attribute_declaration *attr_decl = kefir_ast_new_attribute_declaration(mem);
    REQUIRE_ELSE(attr_decl != NULL, {
        kefir_ast_node_attributes_free(mem, &attributes);
        return KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to allocate AST attribute declaration");
    });

    res = kefir_ast_node_attributes_move(&attr_decl->attributes, &attributes);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_ast_node_attributes_free(mem, &attributes);
        KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(attr_decl));
        return res;
    });
    res = kefir_ast_node_attributes_free(mem, &attributes);
    REQUIRE_CHAIN(&res, kefir_parser_ast_builder_push(mem, builder, KEFIR_AST_NODE_BASE(attr_decl)));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_AST_NODE_FREE(mem, KEFIR_AST_NODE_BASE(attr_decl));
        return res;
    });

    return KEFIR_OK;
}

kefir_result_t KEFIR_PARSER_RULE_FN_PREFIX(attribute_declaration)(struct kefir_mem *mem, struct kefir_parser *parser,
                                                                  struct kefir_ast_node_base **result, void *payload) {
    APPLY_PROLOGUE(mem, parser, result, payload);
    REQUIRE_OK(kefir_parser_ast_builder_wrap(mem, parser, result, builder_callback, NULL));
    return KEFIR_OK;
}
