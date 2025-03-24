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

#define _XOPEN_SOURCE 700
#include "kefir/compiler/compiler.h"
#include "kefir/compiler/configuration.h"
#include "kefir/parser/rule_helpers.h"
#include "kefir/ast/node.h"
#include "kefir/ast/downcast.h"
#include "kefir/ast-translator/translator.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <math.h>

struct parser_extension_payload {
    struct kefir_parser_ruleset original_ruleset;
};

static struct kefir_ast_extension_node_class pi_ext_node_class = {0};

static kefir_result_t parser_identifier_rule(struct kefir_mem *mem, struct kefir_parser *parser,
    struct kefir_ast_node_base **result, void *payload) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(parser != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser"));
    REQUIRE(result != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to AST node"));
    UNUSED(payload);
    ASSIGN_DECL_CAST(struct parser_extension_payload *, extension_payload,
        parser->extension_payload);
    
    if (PARSER_TOKEN_IS_IDENTIFIER(parser, 0) && strcmp(PARSER_CURSOR(parser, 0)->identifier, "__extension_pi") == 0) {
        REQUIRE_OK(PARSER_SHIFT(parser));
        *result = KEFIR_AST_NODE_BASE(kefir_ast_new_extension_node(mem, &pi_ext_node_class, NULL));
    } else {
        REQUIRE_OK(extension_payload->original_ruleset.rules[KEFIR_PARSER_RULESET_IDENTIFIER(identifier)](mem, parser, result, payload));
    }
    return KEFIR_OK;
}

static kefir_result_t on_parser_init(struct kefir_mem *mem, struct kefir_parser *parser) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(parser != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser"));

    struct parser_extension_payload *extension_payload = KEFIR_MALLOC(mem, sizeof(struct parser_extension_payload));
    REQUIRE(extension_payload != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate parser extension payload"));
    extension_payload->original_ruleset = parser->ruleset;
    parser->extension_payload = extension_payload;

    parser->ruleset.rules[KEFIR_PARSER_RULESET_IDENTIFIER(identifier)] = parser_identifier_rule;
    return KEFIR_OK;
}

static kefir_result_t on_parser_free(struct kefir_mem *mem, struct kefir_parser *parser) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(parser != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid parser"));

    ASSIGN_DECL_CAST(struct parser_extension_payload *, extension_payload,
        parser->extension_payload);
    parser->ruleset = extension_payload->original_ruleset;
    KEFIR_FREE(mem, extension_payload);
    return KEFIR_OK;
}

static kefir_result_t analyze_extension_node(struct kefir_mem *mem, const struct kefir_ast_context *context, struct kefir_ast_node_base *node) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));

    struct kefir_ast_extension_node *ext_node;
    REQUIRE_OK(kefir_ast_downcast_extension_node(node, &ext_node, false));
    REQUIRE(ext_node->klass == &pi_ext_node_class, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected extension node class"));

    node->properties.category = KEFIR_AST_NODE_CATEGORY_EXPRESSION;
    node->properties.type = kefir_ast_type_double();

    return KEFIR_OK;
}

static kefir_result_t evaluate_constant_extension_node(struct kefir_mem *mem, const struct kefir_ast_context *context, const struct kefir_ast_node_base *node, struct kefir_ast_constant_expression_value *value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));
    REQUIRE(value != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to constant expression value"));

    struct kefir_ast_extension_node *ext_node;
    REQUIRE_OK(kefir_ast_downcast_extension_node(node, &ext_node, false));
    REQUIRE(ext_node->klass == &pi_ext_node_class, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected extension node class"));

    value->klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT;
    value->floating_point = M_PI;

    return KEFIR_OK;
}


static kefir_result_t translate_extension_node(struct kefir_mem *mem, struct kefir_ast_translator_context *context,
    const struct kefir_ast_extension_node *node, struct kefir_irbuilder_block *builder,
    kefir_ast_translator_context_extension_tag_t tag) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(context != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST context"));
    REQUIRE(node != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node"));
    REQUIRE(builder != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR block builder"));
    UNUSED(tag);

    REQUIRE(node->klass == &pi_ext_node_class, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected extension node class"));

    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_APPENDF64(builder, KEFIR_IROPCODE_PUSHF64, M_PI));

    return KEFIR_OK;
}

const struct kefir_parser_extensions parser_extensions = {
    .on_init = on_parser_init,
    .on_free = on_parser_free
};

const struct kefir_ast_context_extensions analyzer_extensions = {
    .analyze_extension_node = analyze_extension_node,
    .evaluate_constant_extension_node = evaluate_constant_extension_node
};

const struct kefir_ast_translator_context_extensions translator_extensions = {
    .translate_extension_node = translate_extension_node
};

const struct kefir_compiler_extensions extensions = {
    .parser = &parser_extensions,
    .analyzer = &analyzer_extensions,
    .translator = &translator_extensions
};

kefir_result_t kefir_compiler_extension_entry(struct kefir_mem *mem, const struct kefir_compiler_runner_configuration *config, const struct kefir_compiler_extensions **extensions_ptr) {
    UNUSED(mem);
    UNUSED(config);
    REQUIRE(extensions_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to compiler extensions"));

    *extensions_ptr = &extensions;
    return KEFIR_OK;
}
